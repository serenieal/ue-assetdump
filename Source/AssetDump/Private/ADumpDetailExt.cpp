// File: ADumpDetailExt.cpp
// Version: v0.2.4
// Changelog:
// - v0.2.4: 내부 컴포넌트 참조 복구, actor CDO 컴포넌트 수집, Root/UpdatedComponent fallback을 복원해 details 회귀를 수정.
// - v0.2.3: FClassProperty의 TObjectPtr 반환 타입에 맞게 안전한 class path 추출로 수정.
// - v0.2.2: 디버그 크래시를 피하기 위해 안전한 값 읽기 경로만 사용하고 override 비교를 임시 비활성화.
// - v0.2.1: uint8 오분류를 줄이고 unsupported property issue 기록을 추가.
// - v0.2.0: Blueprint details 추출기 추가.

#include "ADumpDetailExt.h"

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UnrealType.h"

namespace
{
	// FDetailExtractContext는 details 추출 중 공통 상태와 참조 복구용 맵을 전달한다.
	struct FDetailExtractContext
	{
		// OwnerAssetPath는 issue target과 메시지에 사용할 현재 자산 경로다.
		FString OwnerAssetPath;

		// Issues는 details 단계 issue 출력 대상이다.
		TArray<FADumpIssue>* Issues = nullptr;

		// Perf는 details 단계 처리량 누적 대상이다.
		FADumpPerf* Perf = nullptr;

		// FriendlyNameByObject는 UObject 포인터를 복구용 friendly path로 매핑한다.
		TMap<const UObject*, FString> FriendlyNameByObject;

		// FriendlyNameByObjectName는 UObject 이름 문자열을 복구용 friendly path로 매핑한다.
		TMap<FString, FString> FriendlyNameByObjectName;

		// FriendlyNameByAlias는 property 이름 / display 이름 / SCS 변수명 별칭을 복구용 friendly path로 매핑한다.
		TMap<FString, FString> FriendlyNameByAlias;

		// OwningActorDefaultObject는 RootComponent / UpdatedComponent fallback에 사용할 actor CDO다.
		const AActor* OwningActorDefaultObject = nullptr;
	};

	// AddDetailIssue는 detail 추출기에서 공통 issue 기록을 단순화한다.
	void AddDetailIssue(
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

	// LoadBlueprintByPath는 object path로 Blueprint를 로드한다.
	bool LoadBlueprintByPath(const FString& AssetObjectPath, UBlueprint*& OutBlueprint, TArray<FADumpIssue>& OutIssues)
	{
		OutBlueprint = nullptr;

		const FSoftObjectPath BlueprintSoftPath(AssetObjectPath);
		UObject* LoadedObject = BlueprintSoftPath.TryLoad();
		if (!LoadedObject)
		{
			AddDetailIssue(
				OutIssues,
				TEXT("ASSET_LOAD_FAIL"),
				FString::Printf(TEXT("Failed to load asset: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Error,
				EADumpPhase::LoadAsset,
				AssetObjectPath);
			return false;
		}

		OutBlueprint = Cast<UBlueprint>(LoadedObject);
		if (!OutBlueprint)
		{
			AddDetailIssue(
				OutIssues,
				TEXT("NOT_BLUEPRINT_ASSET"),
				FString::Printf(TEXT("Loaded asset is not a UBlueprint: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Error,
				EADumpPhase::ValidateAsset,
				AssetObjectPath);
			return false;
		}

		return true;
	}

	// NormalizeReferenceText는 비어 있거나 무의미한 참조 문자열을 정규화한다.
	FString NormalizeReferenceText(const FString& InReferenceText)
	{
		FString NormalizedText = InReferenceText;
		NormalizedText.TrimStartAndEndInline();

		if (NormalizedText.IsEmpty()
			|| NormalizedText.Equals(TEXT("None"), ESearchCase::IgnoreCase)
			|| NormalizedText.Equals(TEXT("null"), ESearchCase::IgnoreCase)
			|| NormalizedText.Equals(TEXT("nullptr"), ESearchCase::IgnoreCase))
		{
			return FString();
		}

		return NormalizedText;
	}

	// BuildFriendlyObjectPath는 UObject를 dump에 남길 안정적인 friendly path 문자열로 만든다.
	FString BuildFriendlyObjectPath(const UObject* InObject)
	{
		if (!InObject)
		{
			return FString();
		}

		const FString ObjectPath = NormalizeReferenceText(InObject->GetPathName());
		if (!ObjectPath.IsEmpty())
		{
			return ObjectPath;
		}

		return NormalizeReferenceText(InObject->GetName());
	}

	// RegisterFriendlyAlias는 alias 이름을 friendly path로 저장한다.
	void RegisterFriendlyAlias(FDetailExtractContext& InOutExtractContext, const FString& InAliasName, const FString& InFriendlyPath)
	{
		const FString NormalizedAliasName = NormalizeReferenceText(InAliasName);
		const FString NormalizedFriendlyPath = NormalizeReferenceText(InFriendlyPath);
		if (NormalizedAliasName.IsEmpty() || NormalizedFriendlyPath.IsEmpty())
		{
			return;
		}

		InOutExtractContext.FriendlyNameByAlias.FindOrAdd(NormalizedAliasName) = NormalizedFriendlyPath;
	}

	// RegisterFriendlyObjectName는 UObject 포인터와 이름/별칭을 복구용 friendly path로 저장한다.
	void RegisterFriendlyObjectName(
		FDetailExtractContext& InOutExtractContext,
		const UObject* InObject,
		const FString& InFriendlyPath,
		const FString& InPreferredAlias)
	{
		const FString NormalizedFriendlyPath = NormalizeReferenceText(InFriendlyPath);
		if (!InObject || NormalizedFriendlyPath.IsEmpty())
		{
			return;
		}

		InOutExtractContext.FriendlyNameByObject.FindOrAdd(InObject) = NormalizedFriendlyPath;

		const FString ObjectName = NormalizeReferenceText(InObject->GetName());
		if (!ObjectName.IsEmpty())
		{
			InOutExtractContext.FriendlyNameByObjectName.FindOrAdd(ObjectName) = NormalizedFriendlyPath;
			RegisterFriendlyAlias(InOutExtractContext, ObjectName, NormalizedFriendlyPath);
		}

		RegisterFriendlyAlias(InOutExtractContext, InPreferredAlias, NormalizedFriendlyPath);
	}

	// ResolveFriendlyNameForObject는 포인터와 이름 매핑을 사용해 friendly path를 찾는다.
	FString ResolveFriendlyNameForObject(const UObject* InObject, const FDetailExtractContext& InExtractContext)
	{
		if (!InObject)
		{
			return FString();
		}

		if (const FString* FriendlyName = InExtractContext.FriendlyNameByObject.Find(InObject))
		{
			return *FriendlyName;
		}

		if (const FString* FriendlyNameByObjectName = InExtractContext.FriendlyNameByObjectName.Find(InObject->GetName()))
		{
			return *FriendlyNameByObjectName;
		}

		return FString();
	}

	// ResolveAliasReferenceText는 property 이름 / display 이름 기반으로 내부 참조를 복구한다.
	FString ResolveAliasReferenceText(const FProperty& InProperty, const FDetailExtractContext& InExtractContext)
	{
		const FString PropertyName = NormalizeReferenceText(InProperty.GetName());
		if (!PropertyName.IsEmpty())
		{
			if (const FString* FriendlyName = InExtractContext.FriendlyNameByAlias.Find(PropertyName))
			{
				return *FriendlyName;
			}
		}

		const FString DisplayName = NormalizeReferenceText(InProperty.GetDisplayNameText().ToString());
		if (!DisplayName.IsEmpty())
		{
			if (const FString* FriendlyName = InExtractContext.FriendlyNameByAlias.Find(DisplayName))
			{
				return *FriendlyName;
			}
		}

		return FString();
	}

	// ResolveOwnerFallbackReferenceText는 RootComponent / UpdatedComponent 같은 owner 기반 참조를 복구한다.
	FString ResolveOwnerFallbackReferenceText(
		const FProperty& InProperty,
		const UObject* InContainerObject,
		const FDetailExtractContext& InExtractContext)
	{
		const FString PropertyName = NormalizeReferenceText(InProperty.GetName());
		if (PropertyName.IsEmpty())
		{
			return FString();
		}

		const AActor* OwnerActor = nullptr;
		if (const UActorComponent* ContainerComponent = Cast<UActorComponent>(InContainerObject))
		{
			OwnerActor = ContainerComponent->GetOwner();
		}
		else if (const AActor* ContainerActor = Cast<AActor>(InContainerObject))
		{
			OwnerActor = ContainerActor;
		}

		if (!OwnerActor)
		{
			OwnerActor = InExtractContext.OwningActorDefaultObject;
		}

		if (!OwnerActor)
		{
			return FString();
		}

		USceneComponent* RootComponent = OwnerActor->GetRootComponent();
		if (!RootComponent)
		{
			return FString();
		}

		if (PropertyName.Equals(TEXT("RootComponent"), ESearchCase::CaseSensitive)
			|| PropertyName.Equals(TEXT("UpdatedComponent"), ESearchCase::CaseSensitive))
		{
			const FString FriendlyRootPath = ResolveFriendlyNameForObject(RootComponent, InExtractContext);
			return !FriendlyRootPath.IsEmpty() ? FriendlyRootPath : BuildFriendlyObjectPath(RootComponent);
		}

		if (PropertyName.Equals(TEXT("UpdatedPrimitive"), ESearchCase::CaseSensitive))
		{
			if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(RootComponent))
			{
				const FString FriendlyPrimitivePath = ResolveFriendlyNameForObject(RootPrimitive, InExtractContext);
				return !FriendlyPrimitivePath.IsEmpty() ? FriendlyPrimitivePath : BuildFriendlyObjectPath(RootPrimitive);
			}
		}

		return FString();
	}

	// ShouldSkipProperty는 details 출력에서 제외할 reflection property를 판별한다.
	bool ShouldSkipProperty(const FProperty& InProperty)
	{
		if (InProperty.HasAnyPropertyFlags(CPF_Deprecated | CPF_Transient))
		{
			return true;
		}

		const FString PropertyName = InProperty.GetName();
		return PropertyName.StartsWith(TEXT("UberGraphFrame"));
	}

	// MakePropertyPath는 prefix와 property name을 합쳐 안정적인 property path를 만든다.
	FString MakePropertyPath(const FString& InPrefix, const FString& InPropertyName)
	{
		if (InPrefix.IsEmpty())
		{
			return InPropertyName;
		}

		return FString::Printf(TEXT("%s.%s"), *InPrefix, *InPropertyName);
	}

	// GetValueKind는 reflection property를 dump schema의 value kind로 매핑한다.
	EADumpValueKind GetValueKind(const FProperty& InProperty)
	{
		if (CastField<const FBoolProperty>(&InProperty))
		{
			return EADumpValueKind::Bool;
		}

		if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(&InProperty))
		{
			return ByteProperty->Enum ? EADumpValueKind::Enum : EADumpValueKind::Int;
		}

		if (CastField<const FEnumProperty>(&InProperty))
		{
			return EADumpValueKind::Enum;
		}

		if (const FNumericProperty* NumericProperty = CastField<const FNumericProperty>(&InProperty))
		{
			if (NumericProperty->IsFloatingPoint())
			{
				return NumericProperty->GetSize() > 4 ? EADumpValueKind::Double : EADumpValueKind::Float;
			}

			return EADumpValueKind::Int;
		}

		if (CastField<const FStrProperty>(&InProperty))
		{
			return EADumpValueKind::String;
		}

		if (CastField<const FNameProperty>(&InProperty))
		{
			return EADumpValueKind::Name;
		}

		if (CastField<const FTextProperty>(&InProperty))
		{
			return EADumpValueKind::Text;
		}

		if (CastField<const FSoftClassProperty>(&InProperty))
		{
			return EADumpValueKind::SoftClassRef;
		}

		if (CastField<const FSoftObjectProperty>(&InProperty))
		{
			return EADumpValueKind::SoftObjectRef;
		}

		if (CastField<const FClassProperty>(&InProperty))
		{
			return EADumpValueKind::ClassRef;
		}

		if (CastField<const FObjectPropertyBase>(&InProperty))
		{
			return EADumpValueKind::ObjectRef;
		}

		if (CastField<const FArrayProperty>(&InProperty))
		{
			return EADumpValueKind::Array;
		}

		if (CastField<const FStructProperty>(&InProperty))
		{
			return EADumpValueKind::Struct;
		}

		if (CastField<const FMapProperty>(&InProperty))
		{
			return EADumpValueKind::Map;
		}

		if (CastField<const FSetProperty>(&InProperty))
		{
			return EADumpValueKind::Set;
		}

		return EADumpValueKind::Unsupported;
	}

	// ExportValueText는 property 값을 안전한 텍스트로 내보낸다.
	FString ExportValueText(const FProperty& InProperty, const void* InContainerPtr)
	{
		if (!InContainerPtr)
		{
			return FString();
		}

		const void* ValuePtr = InProperty.ContainerPtrToValuePtr<void>(InContainerPtr);
		if (!ValuePtr)
		{
			return FString();
		}

		if (const FBoolProperty* BoolProperty = CastField<const FBoolProperty>(&InProperty))
		{
			return BoolProperty->GetPropertyValue(ValuePtr) ? TEXT("true") : TEXT("false");
		}

		if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(&InProperty))
		{
			if (ByteProperty->Enum)
			{
				const int64 EnumValue = ByteProperty->GetSignedIntPropertyValue(ValuePtr);
				return ByteProperty->Enum->GetNameStringByValue(EnumValue);
			}

			return FString::FromInt(static_cast<int32>(ByteProperty->GetUnsignedIntPropertyValue(ValuePtr)));
		}

		if (const FEnumProperty* EnumProperty = CastField<const FEnumProperty>(&InProperty))
		{
			const int64 EnumValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr);
			return EnumProperty->GetEnum() ? EnumProperty->GetEnum()->GetNameStringByValue(EnumValue) : FString::Printf(TEXT("%lld"), EnumValue);
		}

		if (const FNumericProperty* NumericProperty = CastField<const FNumericProperty>(&InProperty))
		{
			if (NumericProperty->IsInteger())
			{
				return FString::Printf(TEXT("%lld"), NumericProperty->GetSignedIntPropertyValue(ValuePtr));
			}

			return FString::SanitizeFloat(NumericProperty->GetFloatingPointPropertyValue(ValuePtr));
		}

		if (const FStrProperty* StringProperty = CastField<const FStrProperty>(&InProperty))
		{
			return StringProperty->GetPropertyValue(ValuePtr);
		}

		if (const FNameProperty* NameProperty = CastField<const FNameProperty>(&InProperty))
		{
			return NameProperty->GetPropertyValue(ValuePtr).ToString();
		}

		if (const FTextProperty* TextProperty = CastField<const FTextProperty>(&InProperty))
		{
			return TextProperty->GetPropertyValue(ValuePtr).ToString();
		}

		if (const FSoftObjectProperty* SoftObjectProperty = CastField<const FSoftObjectProperty>(&InProperty))
		{
			return SoftObjectProperty->GetPropertyValue(ValuePtr).ToString();
		}

		if (const FSoftClassProperty* SoftClassProperty = CastField<const FSoftClassProperty>(&InProperty))
		{
			return SoftClassProperty->GetPropertyValue(ValuePtr).ToString();
		}

		if (const FClassProperty* ClassProperty = CastField<const FClassProperty>(&InProperty))
		{
			const UObject* ClassObjectValue = ClassProperty->GetObjectPropertyValue(ValuePtr);
			const UClass* ClassValue = Cast<UClass>(ClassObjectValue);
			return ClassValue ? ClassValue->GetPathName() : FString();
		}

		if (const FObjectPropertyBase* ObjectProperty = CastField<const FObjectPropertyBase>(&InProperty))
		{
			const UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(ValuePtr);
			return ObjectValue ? ObjectValue->GetPathName() : FString();
		}

		if (const FArrayProperty* ArrayProperty = CastField<const FArrayProperty>(&InProperty))
		{
			FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
			return FString::Printf(TEXT("Array[%d]"), ArrayHelper.Num());
		}

		if (const FMapProperty* MapProperty = CastField<const FMapProperty>(&InProperty))
		{
			FScriptMapHelper MapHelper(MapProperty, ValuePtr);
			return FString::Printf(TEXT("Map[%d]"), MapHelper.Num());
		}

		if (const FSetProperty* SetProperty = CastField<const FSetProperty>(&InProperty))
		{
			FScriptSetHelper SetHelper(SetProperty, ValuePtr);
			return FString::Printf(TEXT("Set[%d]"), SetHelper.Num());
		}

		if (CastField<const FStructProperty>(&InProperty))
		{
			return TEXT("<struct>");
		}

		return FString();
	}

	// BuildSimpleJsonValue는 주요 property kind에 대해 기계가 읽기 쉬운 JSON 값을 만든다.
	TSharedPtr<FJsonValue> BuildSimpleJsonValue(
		const FProperty& InProperty,
		EADumpValueKind InValueKind,
		const void* InContainerPtr,
		const FString& InValueText)
	{
		if (!InContainerPtr)
		{
			return MakeShared<FJsonValueNull>();
		}

		const void* ValuePtr = InProperty.ContainerPtrToValuePtr<void>(InContainerPtr);
		if (!ValuePtr)
		{
			return MakeShared<FJsonValueNull>();
		}

		switch (InValueKind)
		{
		case EADumpValueKind::Bool:
			{
				const FBoolProperty* BoolProperty = CastFieldChecked<const FBoolProperty>(&InProperty);
				return MakeShared<FJsonValueBoolean>(BoolProperty->GetPropertyValue(ValuePtr));
			}

		case EADumpValueKind::Int:
		case EADumpValueKind::Float:
		case EADumpValueKind::Double:
			{
				double NumericValue = 0.0;
				LexTryParseString(NumericValue, *InValueText);
				return MakeShared<FJsonValueNumber>(NumericValue);
			}

		case EADumpValueKind::String:
		case EADumpValueKind::Name:
		case EADumpValueKind::Text:
		case EADumpValueKind::Enum:
		case EADumpValueKind::ObjectRef:
		case EADumpValueKind::ClassRef:
		case EADumpValueKind::SoftObjectRef:
		case EADumpValueKind::SoftClassRef:
			{
				const FString NormalizedValueText = NormalizeReferenceText(InValueText);
				if (NormalizedValueText.IsEmpty())
				{
					return MakeShared<FJsonValueNull>();
				}

				return MakeShared<FJsonValueString>(NormalizedValueText);
			}

		case EADumpValueKind::Array:
			{
				const FArrayProperty* ArrayProperty = CastFieldChecked<const FArrayProperty>(&InProperty);
				FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
				TSharedRef<FJsonObject> ArraySummaryObject = MakeShared<FJsonObject>();
				ArraySummaryObject->SetNumberField(TEXT("count"), ArrayHelper.Num());
				ArraySummaryObject->SetStringField(TEXT("text"), InValueText);
				return MakeShared<FJsonValueObject>(ArraySummaryObject);
			}

		case EADumpValueKind::Map:
			{
				const FMapProperty* MapProperty = CastFieldChecked<const FMapProperty>(&InProperty);
				FScriptMapHelper MapHelper(MapProperty, ValuePtr);
				TSharedRef<FJsonObject> MapSummaryObject = MakeShared<FJsonObject>();
				MapSummaryObject->SetNumberField(TEXT("count"), MapHelper.Num());
				MapSummaryObject->SetStringField(TEXT("text"), InValueText);
				return MakeShared<FJsonValueObject>(MapSummaryObject);
			}

		case EADumpValueKind::Set:
			{
				const FSetProperty* SetProperty = CastFieldChecked<const FSetProperty>(&InProperty);
				FScriptSetHelper SetHelper(SetProperty, ValuePtr);
				TSharedRef<FJsonObject> SetSummaryObject = MakeShared<FJsonObject>();
				SetSummaryObject->SetNumberField(TEXT("count"), SetHelper.Num());
				SetSummaryObject->SetStringField(TEXT("text"), InValueText);
				return MakeShared<FJsonValueObject>(SetSummaryObject);
			}

		case EADumpValueKind::Struct:
			{
				TSharedRef<FJsonObject> StructSummaryObject = MakeShared<FJsonObject>();
				StructSummaryObject->SetStringField(TEXT("text"), InValueText);
				return MakeShared<FJsonValueObject>(StructSummaryObject);
			}

		case EADumpValueKind::Unsupported:
		case EADumpValueKind::None:
		default:
			break;
		}

		return MakeShared<FJsonValueNull>();
	}

	// ComputeIsOverride는 안전성 확보 전까지 override 비교를 비활성화한다.
	bool ComputeIsOverride(
		const FProperty& InProperty,
		const void* InCurrentContainerPtr,
		const void* InCompareContainerPtr)
	{
		return false;
	}

	// ResolveReferenceValueText는 object/class reference 계열 property의 최종 텍스트를 복구한다.
	FString ResolveReferenceValueText(
		const FProperty& InProperty,
		EADumpValueKind InValueKind,
		const void* InCurrentContainerPtr,
		const UObject* InCurrentContainerObject,
		const FDetailExtractContext& InExtractContext,
		const FString& InFallbackValueText)
	{
		FString ResolvedValueText = NormalizeReferenceText(InFallbackValueText);

		if (const void* ValuePtr = InCurrentContainerPtr ? InProperty.ContainerPtrToValuePtr<void>(InCurrentContainerPtr) : nullptr)
		{
			if (InValueKind == EADumpValueKind::ObjectRef)
			{
				if (const FObjectPropertyBase* ObjectProperty = CastField<const FObjectPropertyBase>(&InProperty))
				{
					const UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(ValuePtr);
					if (ObjectValue)
					{
						ResolvedValueText = BuildFriendlyObjectPath(ObjectValue);
						if (ResolvedValueText.IsEmpty())
						{
							ResolvedValueText = ResolveFriendlyNameForObject(ObjectValue, InExtractContext);
						}
					}
				}
			}
			else if (InValueKind == EADumpValueKind::ClassRef)
			{
				if (const FClassProperty* ClassProperty = CastField<const FClassProperty>(&InProperty))
				{
					const UObject* ClassObjectValue = ClassProperty->GetObjectPropertyValue(ValuePtr);
					const UClass* ClassValue = Cast<UClass>(ClassObjectValue);
					if (ClassValue)
					{
						ResolvedValueText = NormalizeReferenceText(ClassValue->GetPathName());
					}
				}
			}
		}

		if (ResolvedValueText.IsEmpty())
		{
			ResolvedValueText = ResolveAliasReferenceText(InProperty, InExtractContext);
		}

		if (ResolvedValueText.IsEmpty())
		{
			ResolvedValueText = ResolveOwnerFallbackReferenceText(InProperty, InCurrentContainerObject, InExtractContext);
		}

		return ResolvedValueText;
	}

	// BuildPropertyItem은 reflection property 하나를 dump-friendly property item으로 변환한다.
	FADumpPropertyItem BuildPropertyItem(
		const FProperty& InProperty,
		const void* InCurrentContainerPtr,
		const UObject* InCurrentContainerObject,
		const void* InCompareContainerPtr,
		const FString& InPropertyPathPrefix,
		const FDetailExtractContext& InExtractContext)
	{
		FADumpPropertyItem PropertyItem;
		PropertyItem.PropertyPath = MakePropertyPath(InPropertyPathPrefix, InProperty.GetName());
		PropertyItem.DisplayName = InProperty.GetDisplayNameText().ToString();
		PropertyItem.Category = InProperty.GetMetaData(TEXT("Category"));
		PropertyItem.Tooltip = InProperty.GetToolTipText().ToString();
		PropertyItem.CppType = InProperty.GetCPPType();
		PropertyItem.ValueKind = GetValueKind(InProperty);
		PropertyItem.ValueText = ExportValueText(InProperty, InCurrentContainerPtr);

		if (PropertyItem.ValueKind == EADumpValueKind::ObjectRef
			|| PropertyItem.ValueKind == EADumpValueKind::ClassRef
			|| PropertyItem.ValueKind == EADumpValueKind::SoftObjectRef
			|| PropertyItem.ValueKind == EADumpValueKind::SoftClassRef)
		{
			PropertyItem.ValueText = ResolveReferenceValueText(
				InProperty,
				PropertyItem.ValueKind,
				InCurrentContainerPtr,
				InCurrentContainerObject,
				InExtractContext,
				PropertyItem.ValueText);
		}

		PropertyItem.ValueJson = BuildSimpleJsonValue(
			InProperty,
			PropertyItem.ValueKind,
			InCurrentContainerPtr,
			PropertyItem.ValueText);
		PropertyItem.bIsOverride = ComputeIsOverride(
			InProperty,
			InCurrentContainerPtr,
			InCompareContainerPtr);
		return PropertyItem;
	}

	// PopulatePropertyItems는 주어진 struct/object container의 프로퍼티 목록을 채운다.
	void PopulatePropertyItems(
		const UStruct* InOwnerStruct,
		const void* InCurrentContainerPtr,
		const UObject* InCurrentContainerObject,
		const void* InCompareContainerPtr,
		const FString& InPropertyPathPrefix,
		const FDetailExtractContext& InExtractContext,
		TArray<FADumpPropertyItem>& OutPropertyItems)
	{
		if (!InOwnerStruct || !InCurrentContainerPtr)
		{
			return;
		}

		for (TFieldIterator<FProperty> PropertyIt(InOwnerStruct); PropertyIt; ++PropertyIt)
		{
			const FProperty* Property = *PropertyIt;
			if (!Property || ShouldSkipProperty(*Property))
			{
				continue;
			}

			FADumpPropertyItem PropertyItem = BuildPropertyItem(
				*Property,
				InCurrentContainerPtr,
				InCurrentContainerObject,
				InCompareContainerPtr,
				InPropertyPathPrefix,
				InExtractContext);

			if (PropertyItem.ValueKind == EADumpValueKind::Unsupported && InExtractContext.Issues)
			{
				AddDetailIssue(
					*InExtractContext.Issues,
					TEXT("DETAILS_UNSUPPORTED_PROPERTY"),
					FString::Printf(
						TEXT("Unsupported property kind recorded as text fallback: %s (%s)"),
						*PropertyItem.PropertyPath,
						*PropertyItem.CppType),
					EADumpIssueSeverity::Warning,
					EADumpPhase::Details,
					PropertyItem.PropertyPath);
			}

			OutPropertyItems.Add(MoveTemp(PropertyItem));

			if (InExtractContext.Perf)
			{
				InExtractContext.Perf->PropertyCount++;
			}
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
		const double DetailsStartSeconds = FPlatformTime::Seconds();

		OutDetails.ClassDefaults.Reset();
		OutDetails.Components.Reset();

		UBlueprint* BlueprintAsset = nullptr;
		if (!LoadBlueprintByPath(AssetObjectPath, BlueprintAsset, OutIssues))
		{
			InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - DetailsStartSeconds);
			return false;
		}

		OutAssetInfo.AssetName = BlueprintAsset->GetName();
		OutAssetInfo.AssetObjectPath = AssetObjectPath;
		OutAssetInfo.PackageName = BlueprintAsset->GetOutermost() ? BlueprintAsset->GetOutermost()->GetName() : FString();
		OutAssetInfo.ClassName = BlueprintAsset->GetClass()->GetName();

		UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(BlueprintAsset->GeneratedClass);
		if (!GeneratedClass)
		{
			AddDetailIssue(
				OutIssues,
				TEXT("MISSING_GENERATED_CLASS"),
				FString::Printf(TEXT("GeneratedClass is null: %s"), *AssetObjectPath),
				EADumpIssueSeverity::Error,
				EADumpPhase::Details,
				AssetObjectPath);
			InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - DetailsStartSeconds);
			return false;
		}

		OutAssetInfo.GeneratedClassPath = GeneratedClass->GetPathName();

		UObject* ClassDefaultObject = GeneratedClass->GetDefaultObject();
		UObject* ParentClassDefaultObject = GeneratedClass->GetSuperClass()
			? GeneratedClass->GetSuperClass()->GetDefaultObject()
			: nullptr;

		FDetailExtractContext ExtractContext;
		ExtractContext.OwnerAssetPath = AssetObjectPath;
		ExtractContext.Issues = &OutIssues;
		ExtractContext.Perf = &InOutPerf;

		TSet<FString> SeenComponentKeys;
		TInlineComponentArray<UActorComponent*> ActorComponentList;
		if (AActor* ActorDefaultObject = Cast<AActor>(ClassDefaultObject))
		{
			ExtractContext.OwningActorDefaultObject = ActorDefaultObject;
			ActorDefaultObject->GetComponents(ActorComponentList);

			for (UActorComponent* ActorComponent : ActorComponentList)
			{
				if (!ActorComponent)
				{
					continue;
				}

				const FString FriendlyComponentPath = BuildFriendlyObjectPath(ActorComponent);
				RegisterFriendlyObjectName(ExtractContext, ActorComponent, FriendlyComponentPath, ActorComponent->GetName());
			}

			if (USceneComponent* RootComponent = ActorDefaultObject->GetRootComponent())
			{
				const FString RootComponentPath = BuildFriendlyObjectPath(RootComponent);
				RegisterFriendlyObjectName(ExtractContext, RootComponent, RootComponentPath, TEXT("RootComponent"));
				RegisterFriendlyAlias(ExtractContext, TEXT("RootComponent"), RootComponentPath);
			}
		}

		if (BlueprintAsset->SimpleConstructionScript)
		{
			const TArray<USCS_Node*>& AllScsNodes = BlueprintAsset->SimpleConstructionScript->GetAllNodes();
			for (USCS_Node* ScsNode : AllScsNodes)
			{
				if (!ScsNode || !ScsNode->ComponentTemplate)
				{
					continue;
				}

				UActorComponent* ComponentTemplate = ScsNode->ComponentTemplate;
				const FString VariableName = ScsNode->GetVariableName().ToString();
				const FString ComponentName = VariableName.IsEmpty() ? ComponentTemplate->GetName() : VariableName;
				const FString FriendlyComponentPath = BuildFriendlyObjectPath(ComponentTemplate);

				RegisterFriendlyObjectName(ExtractContext, ComponentTemplate, FriendlyComponentPath, ComponentName);
				RegisterFriendlyAlias(ExtractContext, VariableName, FriendlyComponentPath);
				RegisterFriendlyAlias(ExtractContext, ComponentTemplate->GetName(), FriendlyComponentPath);
				RegisterFriendlyAlias(ExtractContext, ComponentName, FriendlyComponentPath);
			}
		}

		if (ClassDefaultObject)
		{
			PopulatePropertyItems(
				GeneratedClass,
				ClassDefaultObject,
				ClassDefaultObject,
				ParentClassDefaultObject,
				TEXT("class_defaults"),
				ExtractContext,
				OutDetails.ClassDefaults);
		}
		else
		{
			AddDetailIssue(
				OutIssues,
				TEXT("MISSING_CLASS_DEFAULT_OBJECT"),
				FString::Printf(TEXT("GeneratedClass default object is null: %s"), *GeneratedClass->GetPathName()),
				EADumpIssueSeverity::Error,
				EADumpPhase::Details,
				GeneratedClass->GetPathName());
		}

		for (UActorComponent* ActorComponent : ActorComponentList)
		{
			if (!ActorComponent)
			{
				continue;
			}

			FADumpComponentItem ComponentItem;
			ComponentItem.ComponentName = ActorComponent->GetName();
			ComponentItem.ComponentClass = ActorComponent->GetClass()->GetName();
			ComponentItem.AttachParentName = FString();
			ComponentItem.bIsSceneComponent = Cast<USceneComponent>(ActorComponent) != nullptr;
			ComponentItem.bFromSCS = false;

			if (const USceneComponent* SceneComponent = Cast<USceneComponent>(ActorComponent))
			{
				ComponentItem.AttachParentName = SceneComponent->GetAttachParent() ? SceneComponent->GetAttachParent()->GetName() : FString();
			}

			const FString ComponentKey = FString::Printf(TEXT("%s|%s"), *ComponentItem.ComponentName, *ComponentItem.ComponentClass);
			SeenComponentKeys.Add(ComponentKey);

			PopulatePropertyItems(
				ActorComponent->GetClass(),
				ActorComponent,
				ActorComponent,
				nullptr,
				FString::Printf(TEXT("components.%s"), *ComponentItem.ComponentName),
				ExtractContext,
				ComponentItem.Properties);

			OutDetails.Components.Add(MoveTemp(ComponentItem));
			InOutPerf.ComponentCount++;
		}

		if (BlueprintAsset->SimpleConstructionScript)
		{
			const TArray<USCS_Node*>& AllScsNodes = BlueprintAsset->SimpleConstructionScript->GetAllNodes();
			for (USCS_Node* ScsNode : AllScsNodes)
			{
				if (!ScsNode)
				{
					continue;
				}

				UActorComponent* ComponentTemplate = ScsNode->ComponentTemplate;
				if (!ComponentTemplate)
				{
					AddDetailIssue(
						OutIssues,
						TEXT("DETAILS_MISSING_COMPONENT_TEMPLATE"),
						FString::Printf(TEXT("SCS node component template is null: %s"), *ScsNode->GetVariableName().ToString()),
						EADumpIssueSeverity::Warning,
						EADumpPhase::Details,
						ScsNode->GetVariableName().ToString());
					continue;
				}

				FADumpComponentItem ComponentItem;
				ComponentItem.ComponentName = ScsNode->GetVariableName().IsNone()
					? ComponentTemplate->GetName()
					: ScsNode->GetVariableName().ToString();
				ComponentItem.ComponentClass = ComponentTemplate->GetClass()->GetName();
				ComponentItem.AttachParentName = ScsNode->ParentComponentOrVariableName.ToString();
				ComponentItem.bIsSceneComponent = Cast<USceneComponent>(ComponentTemplate) != nullptr;
				ComponentItem.bFromSCS = true;

				const FString ComponentKey = FString::Printf(TEXT("%s|%s"), *ComponentItem.ComponentName, *ComponentItem.ComponentClass);
				if (SeenComponentKeys.Contains(ComponentKey))
				{
					continue;
				}
				SeenComponentKeys.Add(ComponentKey);

				PopulatePropertyItems(
					ComponentTemplate->GetClass(),
					ComponentTemplate,
					ComponentTemplate,
					nullptr,
					FString::Printf(TEXT("components.%s"), *ComponentItem.ComponentName),
					ExtractContext,
					ComponentItem.Properties);

				OutDetails.Components.Add(MoveTemp(ComponentItem));
				InOutPerf.ComponentCount++;
			}
		}

		InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - DetailsStartSeconds);
		return true;
	}
}
