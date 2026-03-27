// File: ADumpDetailExt.cpp
// Version: v0.7.2
// Changelog:
// - v0.7.2: UWorld도 generic reflected property 경로로 details 추출을 지원.
// - v0.7.1: CurveFloat도 generic reflected property 경로로 details 추출을 지원.
// - v0.7.0: DataTable도 row별 property 펼침 방식으로 details 추출을 지원.
// - v0.6.0: Blueprint 외에 DataAsset / PrimaryDataAsset도 class_defaults 기반 details 추출을 지원.
// - v0.5.1: Blueprint 공통 asset family 분류 helper를 연결해 details-only 실행에서도 자산군 메타를 유지.
// - v0.5.0: details.value_json이 배열/맵/셋/구조체의 실제 내부 원소와 필드까지 담도록 1차 deep dump를 추가.
// - v0.4.1: details.property_type가 명세 예시와 맞도록 reflection 프로퍼티 클래스명을 별도 기록하도록 보정.
// - v0.4.0: 부모 CDO 기준의 is_overridden 비교를 단순 지원 타입과 부모 컴포넌트 매핑 범위까지 복구.
// - v0.3.0: details 스키마용 owner/property/editable 필드를 채우고 델리게이트 계열 노이즈를 기본 제외.
// - v0.2.4: 내부 컴포넌트 참조 복구, actor CDO 컴포넌트 수집, Root/UpdatedComponent fallback을 복원해 details 회귀를 수정.
// - v0.2.3: FClassProperty의 TObjectPtr 반환 타입에 맞게 안전한 class path 추출로 수정.
// - v0.2.2: 디버그 크래시를 피하기 위해 안전한 값 읽기 경로만 사용하고 override 비교를 임시 비활성화.
// - v0.2.1: uint8 오분류를 줄이고 unsupported property issue 기록을 추가.
// - v0.2.0: Blueprint details 추출기 추가.

#include "ADumpDetailExt.h"

#include "ADumpSummaryExt.h"

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Curves/CurveFloat.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "Kismet2/BlueprintEditorUtils.h"
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

		// ParentClassDefaultObject는 class default override 비교에 사용할 부모 CDO다.
		const UObject* ParentClassDefaultObject = nullptr;

		// ParentComponentByKey는 부모 컴포넌트 이름+클래스 키를 비교용 컴포넌트 객체로 매핑한다.
		TMap<FString, const UObject*> ParentComponentByKey;
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

	// GetPropertyTypeText는 details.property_type에 기록할 reflection 프로퍼티 클래스명을 반환한다.
	FString GetPropertyTypeText(const FProperty& InProperty)
	{
		// PropertyClassObject는 현재 reflection 프로퍼티를 설명하는 필드 클래스 객체다.
		const FFieldClass* PropertyClassObject = InProperty.GetClass();
		return PropertyClassObject ? PropertyClassObject->GetName() : FString();
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

		if (CastField<const FDelegateProperty>(&InProperty)
			|| CastField<const FMulticastDelegateProperty>(&InProperty)
			|| CastField<const FMulticastInlineDelegateProperty>(&InProperty)
			|| CastField<const FMulticastSparseDelegateProperty>(&InProperty))
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

	// BuildComponentCompareKey는 컴포넌트 이름과 클래스명으로 부모 비교 키를 만든다.
	FString BuildComponentCompareKey(const FString& InComponentName, const FString& InComponentClass)
	{
		return FString::Printf(TEXT("%s|%s"), *InComponentName, *InComponentClass);
	}

	// BuildDataTableRowPathPrefix는 DataTable row용 property_path prefix를 만든다.
	FString BuildDataTableRowPathPrefix(const FName& InRowName)
	{
		return FString::Printf(TEXT("rows.%s"), *InRowName.ToString());
	}

	// RegisterParentComponentCompareEntry는 부모 컴포넌트 비교 맵에 이름/클래스 키를 등록한다.
	void RegisterParentComponentCompareEntry(
		FDetailExtractContext& InOutExtractContext,
		const FString& InComponentName,
		const FString& InComponentClass,
		const UObject* InComponentObject)
	{
		// NormalizedComponentName은 비어 있는 비교 키 생성을 막기 위한 정규화된 이름이다.
		const FString NormalizedComponentName = NormalizeReferenceText(InComponentName);

		// NormalizedComponentClass는 비어 있는 비교 키 생성을 막기 위한 정규화된 클래스명이다.
		const FString NormalizedComponentClass = NormalizeReferenceText(InComponentClass);
		if (!InComponentObject || NormalizedComponentName.IsEmpty() || NormalizedComponentClass.IsEmpty())
		{
			return;
		}

		// ParentComponentKey는 부모 컴포넌트 조회에 사용할 고정 키다.
		const FString ParentComponentKey = BuildComponentCompareKey(NormalizedComponentName, NormalizedComponentClass);
		InOutExtractContext.ParentComponentByKey.FindOrAdd(ParentComponentKey) = InComponentObject;
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

	// IsOverrideComparableKind는 이번 종료 범위에서 override 비교를 지원하는 타입인지 판별한다.
	bool IsOverrideComparableKind(EADumpValueKind InValueKind)
	{
		switch (InValueKind)
		{
		case EADumpValueKind::Bool:
		case EADumpValueKind::Int:
		case EADumpValueKind::Float:
		case EADumpValueKind::Double:
		case EADumpValueKind::Enum:
		case EADumpValueKind::String:
		case EADumpValueKind::Name:
		case EADumpValueKind::Text:
		case EADumpValueKind::ObjectRef:
		case EADumpValueKind::ClassRef:
		case EADumpValueKind::SoftObjectRef:
		case EADumpValueKind::SoftClassRef:
			return true;
		default:
			return false;
		}
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

	// ExportValueTextFromValuePtr는 직접 value ptr를 받아 안전한 텍스트로 내보낸다.
	FString ExportValueTextFromValuePtr(const FProperty& InProperty, const void* InValuePtr)
	{
		if (!InValuePtr)
		{
			return FString();
		}

		// ExportedValueText는 reflection export 결과 문자열이다.
		FString ExportedValueText;
		InProperty.ExportTextItem_Direct(ExportedValueText, InValuePtr, nullptr, nullptr, PPF_None);
		return ExportedValueText;
	}

	// GetStructuredValueMaxDepth는 복합 타입 재귀 직렬화 최대 깊이를 반환한다.
	int32 GetStructuredValueMaxDepth()
	{
		return 3;
	}

	// BuildTruncatedStructuredValue는 최대 깊이 도달 시 남길 요약 JSON object를 만든다.
	TSharedPtr<FJsonValue> BuildTruncatedStructuredValue(
		const TCHAR* InKindText,
		const FString& InValueText,
		const FString& InTypeText)
	{
		// TruncatedObject는 최대 깊이 도달 시 사용하는 요약 object다.
		TSharedRef<FJsonObject> TruncatedObject = MakeShared<FJsonObject>();
		TruncatedObject->SetStringField(TEXT("kind"), InKindText);
		TruncatedObject->SetBoolField(TEXT("truncated"), true);
		if (!InTypeText.IsEmpty())
		{
			TruncatedObject->SetStringField(TEXT("type"), InTypeText);
		}

		if (!InValueText.IsEmpty())
		{
			TruncatedObject->SetStringField(TEXT("text"), InValueText);
		}

		return MakeShared<FJsonValueObject>(TruncatedObject);
	}

	// BuildStructuredJsonValueFromValuePtr는 직접 value ptr를 받아 복합 타입을 포함한 구조화 JSON 값을 만든다.
	TSharedPtr<FJsonValue> BuildStructuredJsonValueFromValuePtr(
		const FProperty& InProperty,
		EADumpValueKind InValueKind,
		const void* InValuePtr,
		int32 InRemainingDepth)
	{
		if (!InValuePtr)
		{
			return MakeShared<FJsonValueNull>();
		}

		if (InRemainingDepth <= 0)
		{
			return BuildTruncatedStructuredValue(
				ToString(InValueKind),
				ExportValueTextFromValuePtr(InProperty, InValuePtr),
				InProperty.GetCPPType());
		}

		switch (InValueKind)
		{
		case EADumpValueKind::Bool:
			{
				// BoolProperty는 현재 값을 읽을 bool 프로퍼티다.
				const FBoolProperty* BoolProperty = CastFieldChecked<const FBoolProperty>(&InProperty);
				return MakeShared<FJsonValueBoolean>(BoolProperty->GetPropertyValue(InValuePtr));
			}

		case EADumpValueKind::Int:
			{
				// NumericProperty는 정수 값을 읽을 숫자 프로퍼티다.
				const FNumericProperty* NumericProperty = CastFieldChecked<const FNumericProperty>(&InProperty);
				return MakeShared<FJsonValueNumber>(static_cast<double>(NumericProperty->GetSignedIntPropertyValue(InValuePtr)));
			}

		case EADumpValueKind::Float:
		case EADumpValueKind::Double:
			{
				// NumericProperty는 실수 값을 읽을 숫자 프로퍼티다.
				const FNumericProperty* NumericProperty = CastFieldChecked<const FNumericProperty>(&InProperty);
				return MakeShared<FJsonValueNumber>(NumericProperty->GetFloatingPointPropertyValue(InValuePtr));
			}

		case EADumpValueKind::String:
			{
				// StringProperty는 문자열 값을 읽을 string 프로퍼티다.
				const FStrProperty* StringProperty = CastFieldChecked<const FStrProperty>(&InProperty);
				return MakeShared<FJsonValueString>(StringProperty->GetPropertyValue(InValuePtr));
			}

		case EADumpValueKind::Name:
			{
				// NameProperty는 이름 값을 읽을 name 프로퍼티다.
				const FNameProperty* NameProperty = CastFieldChecked<const FNameProperty>(&InProperty);
				return MakeShared<FJsonValueString>(NameProperty->GetPropertyValue(InValuePtr).ToString());
			}

		case EADumpValueKind::Text:
			{
				// TextProperty는 텍스트 값을 읽을 text 프로퍼티다.
				const FTextProperty* TextProperty = CastFieldChecked<const FTextProperty>(&InProperty);
				return MakeShared<FJsonValueString>(TextProperty->GetPropertyValue(InValuePtr).ToString());
			}

		case EADumpValueKind::Enum:
		case EADumpValueKind::ObjectRef:
		case EADumpValueKind::ClassRef:
		case EADumpValueKind::SoftObjectRef:
		case EADumpValueKind::SoftClassRef:
			{
				// NormalizedValueText는 reference/enum 계열의 정규화된 문자열 값이다.
				const FString NormalizedValueText = NormalizeReferenceText(ExportValueTextFromValuePtr(InProperty, InValuePtr));
				if (NormalizedValueText.IsEmpty())
				{
					return MakeShared<FJsonValueNull>();
				}

				return MakeShared<FJsonValueString>(NormalizedValueText);
			}

		case EADumpValueKind::Array:
			{
				// ArrayProperty는 현재 배열 프로퍼티다.
				const FArrayProperty* ArrayProperty = CastFieldChecked<const FArrayProperty>(&InProperty);

				// ArrayHelper는 배열 원소 접근 helper다.
				FScriptArrayHelper ArrayHelper(ArrayProperty, InValuePtr);

				// ArrayObject는 배열 구조화 결과 object다.
				TSharedRef<FJsonObject> ArrayObject = MakeShared<FJsonObject>();
				ArrayObject->SetStringField(TEXT("kind"), TEXT("array"));
				ArrayObject->SetNumberField(TEXT("count"), ArrayHelper.Num());

				// ItemArray는 배열 원소 직렬화 결과 배열이다.
				TArray<TSharedPtr<FJsonValue>> ItemArray;
				for (int32 ArrayIndex = 0; ArrayIndex < ArrayHelper.Num(); ++ArrayIndex)
				{
					// ArrayItemValuePtr는 현재 배열 원소의 직접 값 포인터다.
					const void* ArrayItemValuePtr = ArrayHelper.GetRawPtr(ArrayIndex);
					ItemArray.Add(BuildStructuredJsonValueFromValuePtr(
						*ArrayProperty->Inner,
						GetValueKind(*ArrayProperty->Inner),
						ArrayItemValuePtr,
						InRemainingDepth - 1));
				}

				ArrayObject->SetArrayField(TEXT("items"), ItemArray);
				return MakeShared<FJsonValueObject>(ArrayObject);
			}

		case EADumpValueKind::Map:
			{
				// MapProperty는 현재 맵 프로퍼티다.
				const FMapProperty* MapProperty = CastFieldChecked<const FMapProperty>(&InProperty);

				// MapHelper는 맵 엔트리 접근 helper다.
				FScriptMapHelper MapHelper(MapProperty, InValuePtr);

				// MapObject는 맵 구조화 결과 object다.
				TSharedRef<FJsonObject> MapObject = MakeShared<FJsonObject>();
				MapObject->SetStringField(TEXT("kind"), TEXT("map"));
				MapObject->SetNumberField(TEXT("count"), MapHelper.Num());

				// EntryArray는 맵 엔트리 직렬화 결과 배열이다.
				TArray<TSharedPtr<FJsonValue>> EntryArray;
				for (int32 MapIndex = 0; MapIndex < MapHelper.GetMaxIndex(); ++MapIndex)
				{
					if (!MapHelper.IsValidIndex(MapIndex))
					{
						continue;
					}

					// EntryObject는 현재 맵 엔트리 object다.
					TSharedRef<FJsonObject> EntryObject = MakeShared<FJsonObject>();

					// KeyValuePtr는 현재 엔트리의 key 값 포인터다.
					const void* KeyValuePtr = MapHelper.GetKeyPtr(MapIndex);

					// ValueValuePtr는 현재 엔트리의 value 값 포인터다.
					const void* ValueValuePtr = MapHelper.GetValuePtr(MapIndex);
					EntryObject->SetField(TEXT("key"), BuildStructuredJsonValueFromValuePtr(
						*MapProperty->KeyProp,
						GetValueKind(*MapProperty->KeyProp),
						KeyValuePtr,
						InRemainingDepth - 1));
					EntryObject->SetField(TEXT("value"), BuildStructuredJsonValueFromValuePtr(
						*MapProperty->ValueProp,
						GetValueKind(*MapProperty->ValueProp),
						ValueValuePtr,
						InRemainingDepth - 1));
					EntryArray.Add(MakeShared<FJsonValueObject>(EntryObject));
				}

				MapObject->SetArrayField(TEXT("entries"), EntryArray);
				return MakeShared<FJsonValueObject>(MapObject);
			}

		case EADumpValueKind::Set:
			{
				// SetProperty는 현재 셋 프로퍼티다.
				const FSetProperty* SetProperty = CastFieldChecked<const FSetProperty>(&InProperty);

				// SetHelper는 셋 원소 접근 helper다.
				FScriptSetHelper SetHelper(SetProperty, InValuePtr);

				// SetObject는 셋 구조화 결과 object다.
				TSharedRef<FJsonObject> SetObject = MakeShared<FJsonObject>();
				SetObject->SetStringField(TEXT("kind"), TEXT("set"));
				SetObject->SetNumberField(TEXT("count"), SetHelper.Num());

				// ItemArray는 셋 원소 직렬화 결과 배열이다.
				TArray<TSharedPtr<FJsonValue>> ItemArray;
				for (int32 SetIndex = 0; SetIndex < SetHelper.GetMaxIndex(); ++SetIndex)
				{
					if (!SetHelper.IsValidIndex(SetIndex))
					{
						continue;
					}

					// ElementValuePtr는 현재 셋 원소 값 포인터다.
					const void* ElementValuePtr = SetHelper.GetElementPtr(SetIndex);
					ItemArray.Add(BuildStructuredJsonValueFromValuePtr(
						*SetProperty->ElementProp,
						GetValueKind(*SetProperty->ElementProp),
						ElementValuePtr,
						InRemainingDepth - 1));
				}

				SetObject->SetArrayField(TEXT("items"), ItemArray);
				return MakeShared<FJsonValueObject>(SetObject);
			}

		case EADumpValueKind::Struct:
			{
				// StructProperty는 현재 구조체 프로퍼티다.
				const FStructProperty* StructProperty = CastFieldChecked<const FStructProperty>(&InProperty);

				// StructObject는 구조체 구조화 결과 object다.
				TSharedRef<FJsonObject> StructObject = MakeShared<FJsonObject>();
				StructObject->SetStringField(TEXT("kind"), TEXT("struct"));
				StructObject->SetStringField(TEXT("struct_type"), StructProperty->Struct ? StructProperty->Struct->GetPathName() : InProperty.GetCPPType());

				// FieldObject는 구조체 하위 필드 직렬화 결과 object다.
				TSharedRef<FJsonObject> FieldObject = MakeShared<FJsonObject>();
				if (!StructProperty->Struct)
				{
					StructObject->SetObjectField(TEXT("fields"), FieldObject);
					return MakeShared<FJsonValueObject>(StructObject);
				}

				for (TFieldIterator<FProperty> FieldIterator(StructProperty->Struct); FieldIterator; ++FieldIterator)
				{
					// FieldProperty는 현재 구조체 필드 프로퍼티다.
					const FProperty* FieldProperty = *FieldIterator;
					if (!FieldProperty)
					{
						continue;
					}

					// FieldValuePtr는 현재 구조체 필드 값 포인터다.
					const void* FieldValuePtr = FieldProperty->ContainerPtrToValuePtr<void>(InValuePtr);
					FieldObject->SetField(
						FieldProperty->GetName(),
						BuildStructuredJsonValueFromValuePtr(
							*FieldProperty,
							GetValueKind(*FieldProperty),
							FieldValuePtr,
							InRemainingDepth - 1));
				}

				StructObject->SetObjectField(TEXT("fields"), FieldObject);
				return MakeShared<FJsonValueObject>(StructObject);
			}

		case EADumpValueKind::Unsupported:
		case EADumpValueKind::None:
		default:
			break;
		}

		return MakeShared<FJsonValueNull>();
	}

	// BuildSimpleJsonValue는 주요 property kind에 대해 기계가 읽기 쉬운 JSON 값을 만든다.
	TSharedPtr<FJsonValue> BuildSimpleJsonValue(
		const FProperty& InProperty,
		EADumpValueKind InValueKind,
		const void* InContainerPtr,
		const FString& /*InValueText*/)
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
		return BuildStructuredJsonValueFromValuePtr(InProperty, InValueKind, ValuePtr, GetStructuredValueMaxDepth());
	}

	// ResolveCompareProperty는 부모 owner struct에서 동일 이름의 비교 대상 property를 찾는다.
	const FProperty* ResolveCompareProperty(const FProperty& InProperty, const UStruct* InCompareOwnerStruct)
	{
		if (!InCompareOwnerStruct)
		{
			return nullptr;
		}

		// CompareProperty는 부모 owner struct에서 찾은 동명 프로퍼티다.
		const FProperty* CompareProperty = FindFProperty<FProperty>(InCompareOwnerStruct, InProperty.GetFName());
		if (!CompareProperty)
		{
			return nullptr;
		}

		if (GetValueKind(*CompareProperty) != GetValueKind(InProperty))
		{
			return nullptr;
		}

		return CompareProperty;
	}

	// ComputeIsOverride는 지원 타입에 한해 부모 값과 현재 값을 비교해 override 여부를 반환한다.
	bool ComputeIsOverride(
		const FProperty& InProperty,
		const void* InCurrentContainerPtr,
		const FProperty* InCompareProperty,
		const void* InCompareContainerPtr)
	{
		if (!InCurrentContainerPtr || !InCompareProperty || !InCompareContainerPtr)
		{
			return false;
		}

		// CurrentValueKind는 현재 프로퍼티의 dump 비교 타입 분류다.
		const EADumpValueKind CurrentValueKind = GetValueKind(InProperty);
		if (!IsOverrideComparableKind(CurrentValueKind))
		{
			return false;
		}

		// CompareValueKind는 부모 프로퍼티의 dump 비교 타입 분류다.
		const EADumpValueKind CompareValueKind = GetValueKind(*InCompareProperty);
		if (CurrentValueKind != CompareValueKind)
		{
			return false;
		}

		// CurrentValueText는 현재 CDO/컴포넌트의 정규 비교 문자열이다.
		const FString CurrentValueText = NormalizeReferenceText(ExportValueText(InProperty, InCurrentContainerPtr));

		// CompareValueText는 부모 CDO/부모 컴포넌트의 정규 비교 문자열이다.
		const FString CompareValueText = NormalizeReferenceText(ExportValueText(*InCompareProperty, InCompareContainerPtr));
		return !CurrentValueText.Equals(CompareValueText, ESearchCase::CaseSensitive);
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
		const FProperty* InCompareProperty,
		const void* InCompareContainerPtr,
		const FString& InOwnerKind,
		const FString& InOwnerName,
		const FString& InPropertyPathPrefix,
		const FDetailExtractContext& InExtractContext)
	{
		FADumpPropertyItem PropertyItem;
		PropertyItem.OwnerKind = InOwnerKind;
		PropertyItem.OwnerName = InOwnerName;
		PropertyItem.PropertyPath = MakePropertyPath(InPropertyPathPrefix, InProperty.GetName());
		PropertyItem.PropertyName = InProperty.GetName();
		PropertyItem.DisplayName = InProperty.GetDisplayNameText().ToString();
		PropertyItem.Category = InProperty.GetMetaData(TEXT("Category"));
		PropertyItem.Tooltip = InProperty.GetToolTipText().ToString();
		PropertyItem.PropertyType = GetPropertyTypeText(InProperty);
		PropertyItem.CppType = InProperty.GetCPPType();
		PropertyItem.ValueKind = GetValueKind(InProperty);
		PropertyItem.ValueText = ExportValueText(InProperty, InCurrentContainerPtr);
		PropertyItem.bIsEditable = InProperty.HasAnyPropertyFlags(CPF_Edit)
			&& !InProperty.HasAnyPropertyFlags(CPF_EditConst | CPF_DisableEditOnInstance);

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
			InCompareProperty,
			InCompareContainerPtr);
		return PropertyItem;
	}

	// PopulatePropertyItems는 주어진 struct/object container의 프로퍼티 목록을 채운다.
	void PopulatePropertyItems(
		const UStruct* InOwnerStruct,
		const void* InCurrentContainerPtr,
		const UObject* InCurrentContainerObject,
		const UStruct* InCompareOwnerStruct,
		const void* InCompareContainerPtr,
		const FString& InOwnerKind,
		const FString& InOwnerName,
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

			// CompareProperty는 부모 owner struct에서 찾은 동일 이름 프로퍼티다.
			const FProperty* CompareProperty = ResolveCompareProperty(*Property, InCompareOwnerStruct);

			FADumpPropertyItem PropertyItem = BuildPropertyItem(
				*Property,
				InCurrentContainerPtr,
				InCurrentContainerObject,
				CompareProperty,
				InCompareContainerPtr,
				InOwnerKind,
				InOwnerName,
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

		// LoadedAssetObject는 details 추출 대상 자산 객체다.
		UObject* LoadedAssetObject = nullptr;
		if (!ADumpSummaryExt::LoadAssetObjectByPath(AssetObjectPath, LoadedAssetObject, OutIssues))
		{
			InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - DetailsStartSeconds);
			return false;
		}

		ADumpSummaryExt::FillAssetInfoFromObject(AssetObjectPath, LoadedAssetObject, OutAssetInfo);

		// BlueprintAsset는 Blueprint 전용 details 분기 처리 대상이다.
		UBlueprint* BlueprintAsset = Cast<UBlueprint>(LoadedAssetObject);
		if (!BlueprintAsset)
		{
			if (!Cast<UDataAsset>(LoadedAssetObject) && !Cast<UDataTable>(LoadedAssetObject) && !Cast<UCurveFloat>(LoadedAssetObject) && !Cast<UWorld>(LoadedAssetObject))
			{
				AddDetailIssue(
					OutIssues,
					TEXT("DETAILS_UNSUPPORTED_ASSET_CLASS"),
					FString::Printf(TEXT("Details extraction currently supports Blueprint, DataAsset, DataTable, CurveFloat, and UWorld only: %s"), *LoadedAssetObject->GetClass()->GetName()),
					EADumpIssueSeverity::Error,
					EADumpPhase::Details,
					AssetObjectPath);
				InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - DetailsStartSeconds);
				return false;
			}

			// DataTableAsset는 row 기반 details 확장 처리 대상 DataTable 자산이다.
			UDataTable* DataTableAsset = Cast<UDataTable>(LoadedAssetObject);
			if (DataTableAsset)
			{
				// RowStructObject는 각 row property를 순회할 DataTable 행 구조체다.
				const UScriptStruct* RowStructObject = DataTableAsset->GetRowStruct();
				if (!RowStructObject)
				{
					AddDetailIssue(
						OutIssues,
						TEXT("DATATABLE_MISSING_ROW_STRUCT"),
						FString::Printf(TEXT("DataTable row struct is null: %s"), *AssetObjectPath),
						EADumpIssueSeverity::Error,
						EADumpPhase::Details,
						AssetObjectPath);
					InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - DetailsStartSeconds);
					return false;
				}

				FDetailExtractContext ExtractContext;
				ExtractContext.OwnerAssetPath = AssetObjectPath;
				ExtractContext.Issues = &OutIssues;
				ExtractContext.Perf = &InOutPerf;

				// RowMap는 DataTable 전체 row 이름/메모리 포인터 집합이다.
				const TMap<FName, uint8*>& RowMap = DataTableAsset->GetRowMap();
				for (const TPair<FName, uint8*>& RowPair : RowMap)
				{
					// RowNameText는 현재 row의 dump owner 이름이다.
					const FString RowNameText = RowPair.Key.ToString();

					// RowValuePtr는 현재 row의 실제 struct 메모리 포인터다.
					const uint8* RowValuePtr = RowPair.Value;
					if (!RowValuePtr)
					{
						continue;
					}

					PopulatePropertyItems(
						RowStructObject,
						RowValuePtr,
						DataTableAsset,
						nullptr,
						nullptr,
						TEXT("data_table_row"),
						RowNameText,
						BuildDataTableRowPathPrefix(RowPair.Key),
						ExtractContext,
						OutDetails.ClassDefaults);
				}

				InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - DetailsStartSeconds);
				return true;
			}

			// AssetClassObject는 DataAsset reflection property를 순회할 대상 클래스다.
			UClass* AssetClassObject = LoadedAssetObject->GetClass();

			// ParentClassObject는 override 비교에 사용할 부모 클래스다.
			UClass* ParentClassObject = AssetClassObject ? AssetClassObject->GetSuperClass() : nullptr;

			// ParentClassDefaultObject는 부모 클래스 CDO 비교 대상이다.
			UObject* ParentClassDefaultObject = ParentClassObject ? ParentClassObject->GetDefaultObject() : nullptr;

			FDetailExtractContext ExtractContext;
			ExtractContext.OwnerAssetPath = AssetObjectPath;
			ExtractContext.Issues = &OutIssues;
			ExtractContext.Perf = &InOutPerf;
			ExtractContext.ParentClassDefaultObject = ParentClassDefaultObject;

			PopulatePropertyItems(
				AssetClassObject,
				LoadedAssetObject,
				LoadedAssetObject,
				ParentClassObject,
				ParentClassDefaultObject,
				TEXT("asset_instance"),
				LoadedAssetObject->GetName(),
				TEXT("class_defaults"),
				ExtractContext,
				OutDetails.ClassDefaults);

			InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - DetailsStartSeconds);
			return true;
		}

		ADumpSummaryExt::FillBlueprintAssetInfo(AssetObjectPath, BlueprintAsset, OutAssetInfo);

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
		ExtractContext.ParentClassDefaultObject = ParentClassDefaultObject;

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

		if (const AActor* ParentActorDefaultObject = Cast<AActor>(ParentClassDefaultObject))
		{
			// ParentActorComponentList는 부모 actor CDO에서 비교 가능한 컴포넌트 목록이다.
			TInlineComponentArray<UActorComponent*> ParentActorComponentList;
			const_cast<AActor*>(ParentActorDefaultObject)->GetComponents(ParentActorComponentList);

			for (UActorComponent* ParentActorComponent : ParentActorComponentList)
			{
				if (!ParentActorComponent)
				{
					continue;
				}

				RegisterParentComponentCompareEntry(
					ExtractContext,
					ParentActorComponent->GetName(),
					ParentActorComponent->GetClass()->GetName(),
					ParentActorComponent);
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

		if (const UBlueprintGeneratedClass* ParentGeneratedClass = Cast<UBlueprintGeneratedClass>(GeneratedClass->GetSuperClass()))
		{
			// ParentBlueprintAsset는 부모 Blueprint SCS 템플릿 비교에 사용할 원본 Blueprint다.
			const UBlueprint* ParentBlueprintAsset = Cast<UBlueprint>(ParentGeneratedClass->ClassGeneratedBy);
			if (ParentBlueprintAsset && ParentBlueprintAsset->SimpleConstructionScript)
			{
				// ParentScsNodes는 부모 Blueprint SCS 전체 노드 목록이다.
				const TArray<USCS_Node*>& ParentScsNodes = ParentBlueprintAsset->SimpleConstructionScript->GetAllNodes();
				for (USCS_Node* ParentScsNode : ParentScsNodes)
				{
					if (!ParentScsNode || !ParentScsNode->ComponentTemplate)
					{
						continue;
					}

					// ParentComponentTemplate는 부모 SCS 템플릿 비교에 사용할 실제 객체다.
					UActorComponent* ParentComponentTemplate = ParentScsNode->ComponentTemplate;

					// ParentVariableName은 부모 SCS 변수명 비교 키다.
					const FString ParentVariableName = ParentScsNode->GetVariableName().ToString();

					// ParentComponentName은 변수명이 비면 템플릿 이름을 쓰는 비교용 이름이다.
					const FString ParentComponentName = ParentVariableName.IsEmpty() ? ParentComponentTemplate->GetName() : ParentVariableName;

					// ParentComponentClassName은 부모 템플릿 클래스명 비교 키다.
					const FString ParentComponentClassName = ParentComponentTemplate->GetClass()->GetName();

					RegisterParentComponentCompareEntry(
						ExtractContext,
						ParentComponentTemplate->GetName(),
						ParentComponentClassName,
						ParentComponentTemplate);
					RegisterParentComponentCompareEntry(
						ExtractContext,
						ParentComponentName,
						ParentComponentClassName,
						ParentComponentTemplate);
				}
			}
		}

		if (ClassDefaultObject)
		{
			PopulatePropertyItems(
				GeneratedClass,
				ClassDefaultObject,
				ClassDefaultObject,
				GeneratedClass->GetSuperClass(),
				ParentClassDefaultObject,
				TEXT("class_default"),
				ClassDefaultObject->GetName(),
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

			// ComponentKey는 현재 컴포넌트의 중복 방지 및 부모 비교 조회 키다.
			const FString ComponentKey = BuildComponentCompareKey(ComponentItem.ComponentName, ComponentItem.ComponentClass);
			SeenComponentKeys.Add(ComponentKey);

			// ParentCompareComponent는 동일 이름/클래스 기준으로 찾은 부모 컴포넌트다.
			const UObject* ParentCompareComponent = ExtractContext.ParentComponentByKey.FindRef(ComponentKey);

			PopulatePropertyItems(
				ActorComponent->GetClass(),
				ActorComponent,
				ActorComponent,
				ParentCompareComponent ? ParentCompareComponent->GetClass() : nullptr,
				ParentCompareComponent,
				TEXT("component_template"),
				ComponentItem.ComponentName,
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

				// ComponentKey는 SCS 컴포넌트의 중복 방지 및 부모 비교 조회 키다.
				const FString ComponentKey = BuildComponentCompareKey(ComponentItem.ComponentName, ComponentItem.ComponentClass);
				if (SeenComponentKeys.Contains(ComponentKey))
				{
					continue;
				}
				SeenComponentKeys.Add(ComponentKey);

				// ParentCompareComponent는 동일 이름/클래스 기준으로 찾은 부모 SCS 컴포넌트다.
				const UObject* ParentCompareComponent = ExtractContext.ParentComponentByKey.FindRef(ComponentKey);

				PopulatePropertyItems(
					ComponentTemplate->GetClass(),
					ComponentTemplate,
					ComponentTemplate,
					ParentCompareComponent ? ParentCompareComponent->GetClass() : nullptr,
					ParentCompareComponent,
					TEXT("component_template"),
					ComponentItem.ComponentName,
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
