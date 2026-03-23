// File: ADumpDetailExt.cpp
// Version: v0.1.2
// Changelog:
// - v0.1.0: Blueprint details(class defaults/components) 추출기 구현 추가.
// - v0.1.1: object_ref fallback 보강. 내부 컴포넌트/SCS 변수명/컴포넌트 경로 문자열 대체 기록 추가.
// - v0.1.2: property alias fallback 추가. class_defaults 내부 컴포넌트 참조가 null이어도 컴포넌트 이름으로 복원.

#include "ADumpDetailExt.h"

#include "ADumpSummaryExt.h"

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UnrealType.h"

namespace
{
	struct FDetailExtractContext
	{
		TMap<const UObject*, FString> FriendlyNameByObject;
		TMap<FString, FString> FriendlyNameByObjectName;
		TMap<FString, FString> FriendlyNameByAlias;
	};

	void AddIssue(
		TArray<FADumpIssue>& InOutIssues,
		const FString& InCode,
		const FString& InMessage,
		EADumpIssueSeverity InSeverity,
		EADumpPhase InPhase,
		const FString& InTargetPath)
	{
		FADumpIssue NewIssue;
		NewIssue.Code = InCode;
		NewIssue.Message = InMessage;
		NewIssue.Severity = InSeverity;
		NewIssue.Phase = InPhase;
		NewIssue.TargetPath = InTargetPath;
		InOutIssues.Add(MoveTemp(NewIssue));
	}

	FString NormalizeObjectReferenceText(const FString& InReferenceText)
	{
		FString NormalizedReferenceText = InReferenceText;
		NormalizedReferenceText.TrimStartAndEndInline();

		if (NormalizedReferenceText.IsEmpty()
			|| NormalizedReferenceText.Equals(TEXT("None"), ESearchCase::IgnoreCase)
			|| NormalizedReferenceText.Equals(TEXT("nullptr"), ESearchCase::IgnoreCase))
		{
			return FString();
		}

		return NormalizedReferenceText;
	}

	bool IsReferenceValueKind(EADumpValueKind InValueKind)
	{
		return InValueKind == EADumpValueKind::ObjectRef
			|| InValueKind == EADumpValueKind::ClassRef
			|| InValueKind == EADumpValueKind::SoftObjectRef
			|| InValueKind == EADumpValueKind::SoftClassRef;
	}

	void RegisterFriendlyAlias(
		FDetailExtractContext& InOutExtractContext,
		const FString& InAliasName,
		const FString& InFriendlyName)
	{
		const FString NormalizedAliasName = NormalizeObjectReferenceText(InAliasName);
		const FString NormalizedFriendlyName = NormalizeObjectReferenceText(InFriendlyName);
		if (NormalizedAliasName.IsEmpty() || NormalizedFriendlyName.IsEmpty())
		{
			return;
		}

		InOutExtractContext.FriendlyNameByAlias.FindOrAdd(NormalizedAliasName) = NormalizedFriendlyName;
	}

	void RegisterFriendlyObjectName(
		FDetailExtractContext& InOutExtractContext,
		const UObject* InObject,
		const FString& InFriendlyName)
	{
		const FString NormalizedFriendlyName = NormalizeObjectReferenceText(InFriendlyName);
		if (!InObject || NormalizedFriendlyName.IsEmpty())
		{
			return;
		}

		InOutExtractContext.FriendlyNameByObject.FindOrAdd(InObject) = NormalizedFriendlyName;

		const FString ObjectName = InObject->GetName();
		if (!ObjectName.IsEmpty())
		{
			InOutExtractContext.FriendlyNameByObjectName.FindOrAdd(ObjectName) = NormalizedFriendlyName;
			RegisterFriendlyAlias(InOutExtractContext, ObjectName, NormalizedFriendlyName);
		}
	}

	FString BuildObjectOuterPath(const UObject* InObject)
	{
		if (!InObject)
		{
			return FString();
		}

		TArray<FString> PathSegments;
		for (const UObject* CurrentObject = InObject; CurrentObject; CurrentObject = CurrentObject->GetOuter())
		{
			const FString SegmentName = CurrentObject->GetName();
			if (!SegmentName.IsEmpty())
			{
				PathSegments.Insert(SegmentName, 0);
			}
		}

		return PathSegments.Num() > 0
			? FString::Join(PathSegments, TEXT("."))
			: FString();
	}

	FString ResolveAliasReferenceText(
		const FProperty* InProperty,
		const FDetailExtractContext* InExtractContext)
	{
		if (!InProperty || !InExtractContext)
		{
			return FString();
		}

		const FString PropertyName = InProperty->GetName();
		if (const FString* FriendlyAlias = InExtractContext->FriendlyNameByAlias.Find(PropertyName))
		{
			return *FriendlyAlias;
		}

		const FString DisplayName = InProperty->GetDisplayNameText().ToString();
		if (const FString* FriendlyAliasByDisplayName = InExtractContext->FriendlyNameByAlias.Find(DisplayName))
		{
			return *FriendlyAliasByDisplayName;
		}

		if (PropertyName.Equals(TEXT("RootComponent"), ESearchCase::CaseSensitive))
		{
			if (const FString* DefaultSceneRootAlias = InExtractContext->FriendlyNameByAlias.Find(TEXT("DefaultSceneRoot")))
			{
				return *DefaultSceneRootAlias;
			}
		}

		return FString();
	}

	FString ResolveObjectReferenceText(
		const UObject* InObject,
		const FString& InExportedValueText,
		const FDetailExtractContext* InExtractContext)
	{
		if (InObject)
		{
			const FString ObjectPath = NormalizeObjectReferenceText(InObject->GetPathName());
			if (!ObjectPath.IsEmpty())
			{
				return ObjectPath;
			}

			if (InExtractContext)
			{
				if (const FString* FriendlyName = InExtractContext->FriendlyNameByObject.Find(InObject))
				{
					return *FriendlyName;
				}

				if (const FString* FriendlyNameByObjectName = InExtractContext->FriendlyNameByObjectName.Find(InObject->GetName()))
				{
					return *FriendlyNameByObjectName;
				}
			}

			if (const UActorComponent* ReferencedComponent = Cast<UActorComponent>(InObject))
			{
				const FString ComponentName = NormalizeObjectReferenceText(ReferencedComponent->GetName());
				if (!ComponentName.IsEmpty())
				{
					return ComponentName;
				}
			}

			const FString ComponentPath = NormalizeObjectReferenceText(BuildObjectOuterPath(InObject));
			if (!ComponentPath.IsEmpty())
			{
				return ComponentPath;
			}
		}

		return NormalizeObjectReferenceText(InExportedValueText);
	}

	FString GetJsonStringValue(const TSharedPtr<FJsonValue>& InJsonValue)
	{
		if (!InJsonValue.IsValid())
		{
			return FString();
		}

		FString JsonStringValue;
		if (InJsonValue->TryGetString(JsonStringValue))
		{
			return NormalizeObjectReferenceText(JsonStringValue);
		}

		return FString();
	}

	bool ShouldIncludeDetailProperty(FProperty* InProperty)
	{
		if (!InProperty)
		{
			return false;
		}

		const FString PropertyName = InProperty->GetName();
		if (PropertyName.StartsWith(TEXT("UberGraphFrame")) || InProperty->HasAnyPropertyFlags(CPF_Deprecated))
		{
			return false;
		}

		return InProperty->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible);
	}

	EADumpValueKind GetValueKind(FProperty* InProperty)
	{
		if (!InProperty)
		{
			return EADumpValueKind::None;
		}
		if (CastField<FBoolProperty>(InProperty)) return EADumpValueKind::Bool;
		if (CastField<FIntProperty>(InProperty) || CastField<FInt8Property>(InProperty) || CastField<FInt16Property>(InProperty) || CastField<FInt64Property>(InProperty)) return EADumpValueKind::Int;
		if (CastField<FFloatProperty>(InProperty)) return EADumpValueKind::Float;
		if (CastField<FDoubleProperty>(InProperty)) return EADumpValueKind::Double;
		if (CastField<FStrProperty>(InProperty)) return EADumpValueKind::String;
		if (CastField<FNameProperty>(InProperty)) return EADumpValueKind::Name;
		if (CastField<FTextProperty>(InProperty)) return EADumpValueKind::Text;
		if (CastField<FEnumProperty>(InProperty) || (CastField<FByteProperty>(InProperty) && CastField<FByteProperty>(InProperty)->Enum)) return EADumpValueKind::Enum;
		if (CastField<FSoftObjectProperty>(InProperty)) return EADumpValueKind::SoftObjectRef;
		if (CastField<FSoftClassProperty>(InProperty)) return EADumpValueKind::SoftClassRef;
		if (CastField<FClassProperty>(InProperty)) return EADumpValueKind::ClassRef;
		if (CastField<FObjectPropertyBase>(InProperty)) return EADumpValueKind::ObjectRef;
		if (CastField<FArrayProperty>(InProperty)) return EADumpValueKind::Array;
		if (CastField<FStructProperty>(InProperty)) return EADumpValueKind::Struct;
		if (CastField<FMapProperty>(InProperty)) return EADumpValueKind::Map;
		if (CastField<FSetProperty>(InProperty)) return EADumpValueKind::Set;
		return EADumpValueKind::Unsupported;
	}

	TSharedPtr<FJsonValue> BuildValueJson(
		FProperty* InProperty,
		const void* InValuePointer,
		int32 InRemainingDepth,
		const FString& InExportedValueText,
		const FDetailExtractContext* InExtractContext)
	{
		if (!InProperty || !InValuePointer)
		{
			return MakeShared<FJsonValueNull>();
		}

		if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty))
		{
			return MakeShared<FJsonValueBoolean>(BoolProperty->GetPropertyValue(InValuePointer));
		}
		if (FIntProperty* IntProperty = CastField<FIntProperty>(InProperty))
		{
			return MakeShared<FJsonValueNumber>(IntProperty->GetPropertyValue(InValuePointer));
		}
		if (FInt8Property* Int8Property = CastField<FInt8Property>(InProperty))
		{
			return MakeShared<FJsonValueNumber>(Int8Property->GetPropertyValue(InValuePointer));
		}
		if (FInt16Property* Int16Property = CastField<FInt16Property>(InProperty))
		{
			return MakeShared<FJsonValueNumber>(Int16Property->GetPropertyValue(InValuePointer));
		}
		if (FInt64Property* Int64Property = CastField<FInt64Property>(InProperty))
		{
			return MakeShared<FJsonValueString>(LexToString(Int64Property->GetPropertyValue(InValuePointer)));
		}
		if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(InProperty))
		{
			return MakeShared<FJsonValueNumber>(FloatProperty->GetPropertyValue(InValuePointer));
		}
		if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(InProperty))
		{
			return MakeShared<FJsonValueNumber>(DoubleProperty->GetPropertyValue(InValuePointer));
		}
		if (FStrProperty* StringProperty = CastField<FStrProperty>(InProperty))
		{
			return MakeShared<FJsonValueString>(StringProperty->GetPropertyValue(InValuePointer));
		}
		if (FNameProperty* NameProperty = CastField<FNameProperty>(InProperty))
		{
			return MakeShared<FJsonValueString>(NameProperty->GetPropertyValue(InValuePointer).ToString());
		}
		if (FTextProperty* TextProperty = CastField<FTextProperty>(InProperty))
		{
			return MakeShared<FJsonValueString>(TextProperty->GetPropertyValue(InValuePointer).ToString());
		}
		if (FByteProperty* ByteProperty = CastField<FByteProperty>(InProperty))
		{
			if (ByteProperty->Enum)
			{
				TSharedRef<FJsonObject> EnumObject = MakeShared<FJsonObject>();
				const int64 EnumValue = ByteProperty->GetPropertyValue(InValuePointer);
				EnumObject->SetStringField(TEXT("value"), LexToString(EnumValue));
				EnumObject->SetStringField(TEXT("enum_name"), ByteProperty->Enum->GetName());
				EnumObject->SetStringField(TEXT("enum_value_name"), ByteProperty->Enum->GetNameStringByValue(EnumValue));
				return MakeShared<FJsonValueObject>(EnumObject);
			}
			return MakeShared<FJsonValueNumber>(ByteProperty->GetPropertyValue(InValuePointer));
		}
		if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(InProperty))
		{
			TSharedRef<FJsonObject> EnumObject = MakeShared<FJsonObject>();
			const int64 EnumValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(InValuePointer);
			EnumObject->SetStringField(TEXT("value"), LexToString(EnumValue));
			if (EnumProperty->GetEnum())
			{
				EnumObject->SetStringField(TEXT("enum_name"), EnumProperty->GetEnum()->GetName());
				EnumObject->SetStringField(TEXT("enum_value_name"), EnumProperty->GetEnum()->GetNameStringByValue(EnumValue));
			}
			return MakeShared<FJsonValueObject>(EnumObject);
		}
		if (FClassProperty* ClassProperty = CastField<FClassProperty>(InProperty))
		{
			UObject* ReferencedObject = ClassProperty->GetObjectPropertyValue(InValuePointer);
			UClass* ReferencedClass = Cast<UClass>(ReferencedObject);
			const FString ClassReferenceText = ReferencedClass
				? NormalizeObjectReferenceText(ReferencedClass->GetPathName())
				: NormalizeObjectReferenceText(InExportedValueText);
			return MakeShared<FJsonValueString>(ClassReferenceText);
		}
		if (FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(InProperty))
		{
			const FSoftObjectPtr SoftObjectValue = SoftObjectProperty->GetPropertyValue(InValuePointer);
			const FString SoftObjectPath = NormalizeObjectReferenceText(SoftObjectValue.ToSoftObjectPath().ToString());
			return MakeShared<FJsonValueString>(SoftObjectPath.IsEmpty() ? NormalizeObjectReferenceText(InExportedValueText) : SoftObjectPath);
		}
		if (FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(InProperty))
		{
			const FSoftObjectPtr SoftClassValue = SoftClassProperty->GetPropertyValue(InValuePointer);
			const FString SoftClassPath = NormalizeObjectReferenceText(SoftClassValue.ToSoftObjectPath().ToString());
			return MakeShared<FJsonValueString>(SoftClassPath.IsEmpty() ? NormalizeObjectReferenceText(InExportedValueText) : SoftClassPath);
		}
		if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty))
		{
			UObject* ReferencedObject = ObjectProperty->GetObjectPropertyValue(InValuePointer);
			return MakeShared<FJsonValueString>(ResolveObjectReferenceText(ReferencedObject, InExportedValueText, InExtractContext));
		}
		if (FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
		{
			if (InRemainingDepth <= 0)
			{
				return MakeShared<FJsonValueString>(TEXT("max_depth_reached"));
			}
			TSharedRef<FJsonObject> StructObject = MakeShared<FJsonObject>();
			StructObject->SetStringField(TEXT("struct_name"), StructProperty->Struct ? StructProperty->Struct->GetName() : TEXT(""));
			TArray<TSharedPtr<FJsonValue>> FieldArray;
			for (TFieldIterator<FProperty> FieldIt(StructProperty->Struct, EFieldIterationFlags::IncludeSuper); FieldIt; ++FieldIt)
			{
				FProperty* FieldProperty = *FieldIt;
				if (!FieldProperty)
				{
					continue;
				}
				const void* FieldValuePointer = FieldProperty->ContainerPtrToValuePtr<void>(InValuePointer);
				TSharedRef<FJsonObject> FieldObject = MakeShared<FJsonObject>();
				FieldObject->SetStringField(TEXT("name"), FieldProperty->GetName());
				FieldObject->SetStringField(TEXT("type"), FieldProperty->GetCPPType());
				FieldObject->SetField(TEXT("value"), BuildValueJson(FieldProperty, FieldValuePointer, InRemainingDepth - 1, FString(), InExtractContext));
				FieldArray.Add(MakeShared<FJsonValueObject>(FieldObject));
			}
			StructObject->SetArrayField(TEXT("fields"), FieldArray);
			return MakeShared<FJsonValueObject>(StructObject);
		}
		if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty))
		{
			if (InRemainingDepth <= 0)
			{
				return MakeShared<FJsonValueString>(TEXT("max_depth_reached"));
			}
			FScriptArrayHelper ArrayHelper(ArrayProperty, InValuePointer);
			TArray<TSharedPtr<FJsonValue>> ElementArray;
			for (int32 ArrayIndex = 0; ArrayIndex < ArrayHelper.Num(); ++ArrayIndex)
			{
				ElementArray.Add(BuildValueJson(ArrayProperty->Inner, ArrayHelper.GetRawPtr(ArrayIndex), InRemainingDepth - 1, FString(), InExtractContext));
			}
			return MakeShared<FJsonValueArray>(ElementArray);
		}
		if (FSetProperty* SetProperty = CastField<FSetProperty>(InProperty))
		{
			if (InRemainingDepth <= 0)
			{
				return MakeShared<FJsonValueString>(TEXT("max_depth_reached"));
			}
			FScriptSetHelper SetHelper(SetProperty, InValuePointer);
			TArray<TSharedPtr<FJsonValue>> ElementArray;
			for (int32 SetIndex = 0; SetIndex < SetHelper.GetMaxIndex(); ++SetIndex)
			{
				if (!SetHelper.IsValidIndex(SetIndex))
				{
					continue;
				}
				ElementArray.Add(BuildValueJson(SetProperty->ElementProp, SetHelper.GetElementPtr(SetIndex), InRemainingDepth - 1, FString(), InExtractContext));
			}
			return MakeShared<FJsonValueArray>(ElementArray);
		}
		if (FMapProperty* MapProperty = CastField<FMapProperty>(InProperty))
		{
			if (InRemainingDepth <= 0)
			{
				return MakeShared<FJsonValueString>(TEXT("max_depth_reached"));
			}
			FScriptMapHelper MapHelper(MapProperty, InValuePointer);
			TArray<TSharedPtr<FJsonValue>> EntryArray;
			for (int32 MapIndex = 0; MapIndex < MapHelper.GetMaxIndex(); ++MapIndex)
			{
				if (!MapHelper.IsValidIndex(MapIndex))
				{
					continue;
				}
				TSharedRef<FJsonObject> EntryObject = MakeShared<FJsonObject>();
				EntryObject->SetField(TEXT("key"), BuildValueJson(MapProperty->KeyProp, MapHelper.GetKeyPtr(MapIndex), InRemainingDepth - 1, FString(), InExtractContext));
				EntryObject->SetField(TEXT("value"), BuildValueJson(MapProperty->ValueProp, MapHelper.GetValuePtr(MapIndex), InRemainingDepth - 1, FString(), InExtractContext));
				EntryArray.Add(MakeShared<FJsonValueObject>(EntryObject));
			}
			return MakeShared<FJsonValueArray>(EntryArray);
		}

		return MakeShared<FJsonValueString>(TEXT("unsupported"));
	}

	FString ExportValueText(FProperty* InProperty, const void* InValuePointer)
	{
		if (!InProperty || !InValuePointer)
		{
			return FString();
		}

		FString ValueText;
		InProperty->ExportTextItem_Direct(ValueText, InValuePointer, nullptr, nullptr, PPF_None);
		return ValueText;
	}

	bool IsOverrideComparedToParent(UClass* InParentClass, FProperty* InProperty, const void* InValuePointer)
	{
		if (!InParentClass || !InProperty || !InValuePointer)
		{
			return false;
		}

		FProperty* ParentProperty = FindFProperty<FProperty>(InParentClass, InProperty->GetFName());
		if (!ParentProperty || !ParentProperty->SameType(InProperty))
		{
			return true;
		}

		UObject* ParentDefaultObject = InParentClass->GetDefaultObject();
		if (!ParentDefaultObject)
		{
			return false;
		}

		const void* ParentValuePointer = ParentProperty->ContainerPtrToValuePtr<void>(ParentDefaultObject);
		return !InProperty->Identical(InValuePointer, ParentValuePointer, PPF_None);
	}

	FADumpPropertyItem BuildPropertyItem(
		FProperty* InProperty,
		const void* InContainerPointer,
		const FString& InPropertyPathPrefix,
		UClass* InParentClass,
		const FDetailExtractContext* InExtractContext)
	{
		FADumpPropertyItem PropertyItem;
		PropertyItem.PropertyPath = InPropertyPathPrefix.IsEmpty()
			? InProperty->GetName()
			: FString::Printf(TEXT("%s.%s"), *InPropertyPathPrefix, *InProperty->GetName());
		PropertyItem.DisplayName = InProperty->GetDisplayNameText().ToString();
		PropertyItem.Category = InProperty->GetMetaData(TEXT("Category"));
		PropertyItem.Tooltip = InProperty->GetToolTipText().ToString();
		PropertyItem.CppType = InProperty->GetCPPType();
		PropertyItem.ValueKind = GetValueKind(InProperty);

		const void* ValuePointer = InProperty->ContainerPtrToValuePtr<void>(InContainerPointer);

		// ExportedValueText는 object_ref가 None으로 떨어질 때 사용할 fallback 후보 텍스트다.
		const FString ExportedValueText = ExportValueText(InProperty, ValuePointer);

		PropertyItem.ValueJson = BuildValueJson(InProperty, ValuePointer, 3, ExportedValueText, InExtractContext);
		PropertyItem.ValueText = ExportedValueText;

		if (IsReferenceValueKind(PropertyItem.ValueKind))
		{
			const FString NormalizedValueText = NormalizeObjectReferenceText(PropertyItem.ValueText);
			if (NormalizedValueText.IsEmpty())
			{
				PropertyItem.ValueText = GetJsonStringValue(PropertyItem.ValueJson);
			}
			else
			{
				PropertyItem.ValueText = NormalizedValueText;
			}

			if (PropertyItem.ValueText.IsEmpty())
			{
				PropertyItem.ValueText = ResolveAliasReferenceText(InProperty, InExtractContext);
				if (!PropertyItem.ValueText.IsEmpty())
				{
					PropertyItem.ValueJson = MakeShared<FJsonValueString>(PropertyItem.ValueText);
				}
			}
		}

		PropertyItem.bIsOverride = IsOverrideComparedToParent(InParentClass, InProperty, ValuePointer);
		return PropertyItem;
	}

	void PopulatePropertyItems(
		UStruct* InSourceStruct,
		const void* InContainerPointer,
		const FString& InPropertyPathPrefix,
		UClass* InParentClass,
		const FDetailExtractContext* InExtractContext,
		TArray<FADumpPropertyItem>& OutPropertyItems,
		FADumpPerf& InOutPerf)
	{
		if (!InSourceStruct || !InContainerPointer)
		{
			return;
		}

		for (TFieldIterator<FProperty> PropertyIt(InSourceStruct, EFieldIterationFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (!ShouldIncludeDetailProperty(Property))
			{
				continue;
			}

			OutPropertyItems.Add(BuildPropertyItem(Property, InContainerPointer, InPropertyPathPrefix, InParentClass, InExtractContext));
			InOutPerf.PropertyCount++;
		}
	}
}

namespace ADumpDetailExt
{
	bool ExtractDetails(
		const FString& AssetObjectPath,
		FADumpAssetInfo& OutAssetInfo,
		FADumpDetails& OutDetails,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf)
	{
		UBlueprint* BlueprintAsset = nullptr;
		if (!ADumpSummaryExt::LoadBlueprintByPath(AssetObjectPath, BlueprintAsset, OutIssues))
		{
			return false;
		}

		OutAssetInfo.AssetName = BlueprintAsset->GetName();
		OutAssetInfo.AssetObjectPath = AssetObjectPath;
		OutAssetInfo.PackageName = BlueprintAsset->GetOutermost() ? BlueprintAsset->GetOutermost()->GetName() : FString();
		OutAssetInfo.ClassName = BlueprintAsset->GetClass()->GetName();
		OutAssetInfo.GeneratedClassPath = BlueprintAsset->GeneratedClass ? BlueprintAsset->GeneratedClass->GetPathName() : FString();

		UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(BlueprintAsset->GeneratedClass);
		if (!GeneratedClass)
		{
			AddIssue(
				OutIssues,
				TEXT("DETAILS_GENERATED_CLASS_MISSING"),
				TEXT("Failed to resolve BlueprintGeneratedClass for details extraction."),
				EADumpIssueSeverity::Error,
				EADumpPhase::Details,
				AssetObjectPath);
			return false;
		}

		UObject* DefaultObject = GeneratedClass->GetDefaultObject();
		if (!DefaultObject)
		{
			AddIssue(
				OutIssues,
				TEXT("DETAILS_CDO_MISSING"),
				TEXT("Failed to resolve class default object for details extraction."),
				EADumpIssueSeverity::Error,
				EADumpPhase::Details,
				AssetObjectPath);
			return false;
		}

		OutDetails.ClassDefaults.Reset();
		OutDetails.Components.Reset();

		FDetailExtractContext ExtractContext;
		TInlineComponentArray<UActorComponent*> ActorComponents;
		if (AActor* ActorDefaultObject = Cast<AActor>(DefaultObject))
		{
			ActorDefaultObject->GetComponents(ActorComponents);
			for (UActorComponent* ActorComponent : ActorComponents)
			{
				if (!ActorComponent)
				{
					continue;
				}

				const FString FriendlyComponentName = ActorComponent->GetName();
				RegisterFriendlyObjectName(ExtractContext, ActorComponent, FriendlyComponentName);
				RegisterFriendlyAlias(ExtractContext, FriendlyComponentName, FriendlyComponentName);
			}

			if (USceneComponent* RootSceneComponent = ActorDefaultObject->GetRootComponent())
			{
				const FString RootComponentName = RootSceneComponent->GetName();
				RegisterFriendlyAlias(ExtractContext, TEXT("RootComponent"), RootComponentName);
			}
		}

		if (BlueprintAsset->SimpleConstructionScript)
		{
			for (USCS_Node* SCSNode : BlueprintAsset->SimpleConstructionScript->GetAllNodes())
			{
				if (!SCSNode || !SCSNode->ComponentTemplate)
				{
					continue;
				}

				const FString VariableName = SCSNode->GetVariableName().ToString();
				const FString FriendlyComponentName = VariableName.IsEmpty()
					? SCSNode->ComponentTemplate->GetName()
					: VariableName;

				RegisterFriendlyObjectName(ExtractContext, SCSNode->ComponentTemplate, FriendlyComponentName);
				RegisterFriendlyAlias(ExtractContext, FriendlyComponentName, FriendlyComponentName);
				if (!VariableName.IsEmpty())
				{
					RegisterFriendlyAlias(ExtractContext, VariableName, FriendlyComponentName);
				}
			}
		}

		PopulatePropertyItems(
			GeneratedClass,
			DefaultObject,
			TEXT("class_defaults"),
			GeneratedClass->GetSuperClass(),
			&ExtractContext,
			OutDetails.ClassDefaults,
			InOutPerf);

		TSet<FString> SeenComponentKeys;
		for (UActorComponent* ActorComponent : ActorComponents)
		{
			if (!ActorComponent)
			{
				continue;
			}

			FADumpComponentItem ComponentItem;
			ComponentItem.ComponentName = ActorComponent->GetName();
			ComponentItem.ComponentClass = ActorComponent->GetClass()->GetName();
			ComponentItem.bFromSCS = false;
			ComponentItem.bIsSceneComponent = ActorComponent->IsA<USceneComponent>();
			if (USceneComponent* SceneComponent = Cast<USceneComponent>(ActorComponent))
			{
				ComponentItem.AttachParentName = SceneComponent->GetAttachParent() ? SceneComponent->GetAttachParent()->GetName() : FString();
			}

			const FString ComponentKey = FString::Printf(TEXT("%s|%s"), *ComponentItem.ComponentName, *ComponentItem.ComponentClass);
			SeenComponentKeys.Add(ComponentKey);
			PopulatePropertyItems(
				ActorComponent->GetClass(),
				ActorComponent,
				FString::Printf(TEXT("components.%s"), *ComponentItem.ComponentName),
				nullptr,
				&ExtractContext,
				ComponentItem.Properties,
				InOutPerf);
			OutDetails.Components.Add(MoveTemp(ComponentItem));
			InOutPerf.ComponentCount++;
		}

		if (BlueprintAsset->SimpleConstructionScript)
		{
			for (USCS_Node* SCSNode : BlueprintAsset->SimpleConstructionScript->GetAllNodes())
			{
				if (!SCSNode || !SCSNode->ComponentTemplate)
				{
					continue;
				}

				UActorComponent* ComponentTemplate = SCSNode->ComponentTemplate;
				const FString VariableName = SCSNode->GetVariableName().ToString();
				const FString ComponentName = VariableName.IsEmpty() ? ComponentTemplate->GetName() : VariableName;
				const FString ComponentClass = ComponentTemplate->GetClass()->GetName();
				const FString ComponentKey = FString::Printf(TEXT("%s|%s"), *ComponentName, *ComponentClass);
				if (SeenComponentKeys.Contains(ComponentKey))
				{
					continue;
				}

				FADumpComponentItem ComponentItem;
				ComponentItem.ComponentName = ComponentName;
				ComponentItem.ComponentClass = ComponentClass;
				ComponentItem.bFromSCS = true;
				ComponentItem.bIsSceneComponent = ComponentTemplate->IsA<USceneComponent>();
				if (USceneComponent* SceneComponent = Cast<USceneComponent>(ComponentTemplate))
				{
					ComponentItem.AttachParentName = SceneComponent->GetAttachParent() ? SceneComponent->GetAttachParent()->GetName() : FString();
				}

				PopulatePropertyItems(
					ComponentTemplate->GetClass(),
					ComponentTemplate,
					FString::Printf(TEXT("components.%s"), *ComponentItem.ComponentName),
					nullptr,
					&ExtractContext,
					ComponentItem.Properties,
					InOutPerf);
				OutDetails.Components.Add(MoveTemp(ComponentItem));
				InOutPerf.ComponentCount++;
			}
		}

		return true;
	}
}
