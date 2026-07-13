// File: ADumpInput.cpp
// Version: v1.1.0
// Changelog:
// - v1.1.0: v0.7.2 계약 한도, typed setting descriptor, warning code, mapping field, chain signature 정렬을 정렬.
// - v1.0.0: InputAction/InputMappingContext용 input_summary_v1 추출, 얕은 modifier/trigger 설정 요약, 안정 정렬을 구현.
// Migration:
// - 기존 dump 호출 방식은 유지된다. 새 섹션은 full 모드의 지원 자산 또는 -Sections=input_summary 명시 요청에서만 출력된다.

#include "ADumpInput.h"

#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputTriggers.h"
#include "PlayerMappableKeySettings.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/PlatformTime.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UnrealType.h"

namespace
{
	// InputSummaryMaxMappings는 input_summary_v1이 한 번에 출력할 최대 mapping 개수다.
	constexpr int32 InputSummaryMaxMappings = 128;

	// InputSummaryMaxChainEntries는 action/mapping별 modifier 또는 trigger 최대 출력 개수다.
	constexpr int32 InputSummaryMaxChainEntries = 16;

	// InputSummaryMaxSettings는 modifier/trigger 한 개에서 출력할 최대 얕은 설정 개수다.
	constexpr int32 InputSummaryMaxSettings = 16;

	// InputSummaryMaxPreviewLines는 input_summary preview 최대 줄 수다.
	constexpr int32 InputSummaryMaxPreviewLines = 12;

	// InputSummaryMaxWarnings는 input_summary 내부 warning 최대 개수다.
	constexpr int32 InputSummaryMaxWarnings = 64;

	// AddInputIssue는 input_summary builder 전용 issue를 누적한다.
	void AddInputIssue(
		TArray<FADumpIssue>& OutIssues,
		const FString& InCode,
		const FString& InMessage,
		EADumpIssueSeverity InSeverity,
		const FString& InTargetPath)
	{
		// NewIssue는 input_summary builder가 추가할 issue다.
		FADumpIssue NewIssue;
		NewIssue.Code = InCode;
		NewIssue.Message = InMessage;
		NewIssue.Severity = InSeverity;
		NewIssue.Phase = EADumpPhase::Details;
		NewIssue.TargetPath = InTargetPath;
		OutIssues.Add(MoveTemp(NewIssue));
	}

	// AddInputSummaryWarning은 input_summary 내부에 stable warning을 제한 개수만큼 추가한다.
	void AddInputSummaryWarning(
		FADumpInputSummary& InOutInputSummary,
		const FString& InCode,
		const FString& InMessage,
		const FString& InTargetPath)
	{
		++InOutInputSummary.WarningCount;
		if (InOutInputSummary.Warnings.Num() >= InputSummaryMaxWarnings)
		{
			return;
		}

		// NewWarning은 input_summary 내부에 기록할 bounded warning 항목이다.
		FADumpInputWarning NewWarning;
		NewWarning.Code = InCode;
		NewWarning.Message = InMessage;
		NewWarning.TargetPath = InTargetPath;
		InOutInputSummary.Warnings.Add(MoveTemp(NewWarning));
	}

	// MakeBoundedValueText는 fallback value_text를 일정 길이로 제한한다.
	FString MakeBoundedValueText(const FString& InValueText)
	{
		// MaxValueTextLength는 setting fallback 문자열 최대 길이다.
		constexpr int32 MaxValueTextLength = 160;
		if (InValueText.Len() <= MaxValueTextLength)
		{
			return InValueText;
		}
		return InValueText.Left(MaxValueTextLength) + TEXT("...");
	}

	// ResolveEnumValueText는 UEnum 값 이름을 stable 문자열로 변환한다.
	FString ResolveEnumValueText(const UEnum* InEnumObject, int64 InRawValue)
	{
		if (!InEnumObject)
		{
			return FString::FromInt(static_cast<int32>(InRawValue));
		}

		// EnumNameText는 네임스페이스가 제거된 enum 값 이름이다.
		FString EnumNameText = InEnumObject->GetNameStringByValue(InRawValue);
		if (EnumNameText.Contains(TEXT("::")))
		{
			EnumNameText = EnumNameText.RightChop(EnumNameText.Find(TEXT("::")) + 2);
		}
		return EnumNameText;
	}

	// ResolveInputSummaryValueTypeText는 EnhancedInput value type enum을 JSON 친화 문자열로 변환한다.
	FString ResolveInputSummaryValueTypeText(EInputActionValueType InValueType)
	{
		switch (InValueType)
		{
		case EInputActionValueType::Boolean:
			return TEXT("boolean");
		case EInputActionValueType::Axis1D:
			return TEXT("axis1d");
		case EInputActionValueType::Axis2D:
			return TEXT("axis2d");
		case EInputActionValueType::Axis3D:
			return TEXT("axis3d");
		default:
			return TEXT("unknown");
		}
	}

	// ResolveAccumulationBehaviorText는 InputAction accumulation enum을 문자열로 변환한다.
	FString ResolveAccumulationBehaviorText(EInputActionAccumulationBehavior InBehavior)
	{
		return ResolveEnumValueText(StaticEnum<EInputActionAccumulationBehavior>(), static_cast<int64>(InBehavior));
	}

	// ResolveRegistrationTrackingText는 IMC registration tracking enum을 문자열로 변환한다.
	FString ResolveRegistrationTrackingText(EMappingContextRegistrationTrackingMode InMode)
	{
		return ResolveEnumValueText(StaticEnum<EMappingContextRegistrationTrackingMode>(), static_cast<int64>(InMode));
	}

	// ResolveInputModeFilterText는 IMC input mode filter enum을 문자열로 변환한다.
	FString ResolveInputModeFilterText(EMappingContextInputModeFilterOptions InOption)
	{
		return ResolveEnumValueText(StaticEnum<EMappingContextInputModeFilterOptions>(), static_cast<int64>(InOption));
	}

	// MakeObjectPathText는 UObject pointer를 안정적인 object path 문자열로 변환한다.
	FString MakeObjectPathText(const UObject* InObject)
	{
		return InObject ? InObject->GetPathName() : FString();
	}

	// TryMakeStructJsonValue는 vector-like struct 값을 JSON object로 변환한다.
	TSharedPtr<FJsonValue> TryMakeStructJsonValue(const FStructProperty* InStructProperty, const void* InValuePtr)
	{
		if (!InStructProperty || !InStructProperty->Struct || !InValuePtr)
		{
			return nullptr;
		}

		if (InStructProperty->Struct == TBaseStructure<FVector>::Get())
		{
			// VectorValue는 FVector 구조체 원본 값이다.
			const FVector& VectorValue = *static_cast<const FVector*>(InValuePtr);

			// VectorObject는 FVector를 x/y/z 필드로 표현한 JSON object다.
			TSharedRef<FJsonObject> VectorObject = MakeShared<FJsonObject>();
			VectorObject->SetNumberField(TEXT("x"), VectorValue.X);
			VectorObject->SetNumberField(TEXT("y"), VectorValue.Y);
			VectorObject->SetNumberField(TEXT("z"), VectorValue.Z);
			return MakeShared<FJsonValueObject>(VectorObject);
		}

		if (InStructProperty->Struct == TBaseStructure<FVector2D>::Get())
		{
			// VectorValue는 FVector2D 구조체 원본 값이다.
			const FVector2D& VectorValue = *static_cast<const FVector2D*>(InValuePtr);

			// VectorObject는 FVector2D를 x/y 필드로 표현한 JSON object다.
			TSharedRef<FJsonObject> VectorObject = MakeShared<FJsonObject>();
			VectorObject->SetNumberField(TEXT("x"), VectorValue.X);
			VectorObject->SetNumberField(TEXT("y"), VectorValue.Y);
			return MakeShared<FJsonValueObject>(VectorObject);
		}

		if (InStructProperty->Struct == TBaseStructure<FRotator>::Get())
		{
			// RotatorValue는 FRotator 구조체 원본 값이다.
			const FRotator& RotatorValue = *static_cast<const FRotator*>(InValuePtr);

			// RotatorObject는 FRotator를 pitch/yaw/roll 필드로 표현한 JSON object다.
			TSharedRef<FJsonObject> RotatorObject = MakeShared<FJsonObject>();
			RotatorObject->SetNumberField(TEXT("pitch"), RotatorValue.Pitch);
			RotatorObject->SetNumberField(TEXT("yaw"), RotatorValue.Yaw);
			RotatorObject->SetNumberField(TEXT("roll"), RotatorValue.Roll);
			return MakeShared<FJsonValueObject>(RotatorObject);
		}

		return nullptr;
	}

	// BuildUnsupportedSetting은 지원하지 않는 shallow setting fallback descriptor를 만든다.
	FADumpInputSettingDescriptor BuildUnsupportedSetting(const FProperty* InProperty)
	{
		// SettingDescriptor는 unsupported 타입을 나타내는 fallback 설정 항목이다.
		FADumpInputSettingDescriptor SettingDescriptor;
		SettingDescriptor.PropertyName = InProperty ? InProperty->GetName() : TEXT("unknown");
		SettingDescriptor.CppType = InProperty ? InProperty->GetCPPType() : FString();
		SettingDescriptor.ValueKind = TEXT("unsupported");
		SettingDescriptor.ValueJson = MakeShared<FJsonValueNull>();
		SettingDescriptor.ValueText = MakeBoundedValueText(InProperty ? InProperty->GetCPPType() : FString());
		SettingDescriptor.bUnsupported = true;
		return SettingDescriptor;
	}

	// TryBuildShallowSetting은 재귀 없이 안전한 scalar/enum/name/string/text/vector/object 값을 typed descriptor로 만든다.
	FADumpInputSettingDescriptor TryBuildShallowSetting(const FProperty* InProperty, const void* InContainerPtr)
	{
		// SettingDescriptor는 현재 프로퍼티의 typed setting 출력 항목이다.
		FADumpInputSettingDescriptor SettingDescriptor;
		SettingDescriptor.PropertyName = InProperty ? InProperty->GetName() : TEXT("unknown");
		SettingDescriptor.CppType = InProperty ? InProperty->GetCPPType() : FString();
		SettingDescriptor.ValueJson = MakeShared<FJsonValueNull>();
		if (!InProperty || !InContainerPtr)
		{
			SettingDescriptor.ValueKind = TEXT("unsupported");
			SettingDescriptor.bUnsupported = true;
			return SettingDescriptor;
		}

		if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty))
		{
			// ValuePtr는 현재 bool 프로퍼티가 저장된 주소다.
			const void* ValuePtr = BoolProperty->ContainerPtrToValuePtr<void>(InContainerPtr);
			SettingDescriptor.ValueKind = TEXT("bool");
			SettingDescriptor.ValueJson = MakeShared<FJsonValueBoolean>(BoolProperty->GetPropertyValue(ValuePtr));
			SettingDescriptor.ValueText = BoolProperty->GetPropertyValue(ValuePtr) ? TEXT("true") : TEXT("false");
			return SettingDescriptor;
		}

		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(InProperty))
		{
			// ValuePtr는 현재 enum 프로퍼티가 저장된 주소다.
			const void* ValuePtr = EnumProperty->ContainerPtrToValuePtr<void>(InContainerPtr);

			// RawValue는 enum underlying property에서 읽은 정수 값이다.
			const int64 RawValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr);
			SettingDescriptor.ValueKind = TEXT("enum");
			SettingDescriptor.ValueText = ResolveEnumValueText(EnumProperty->GetEnum(), RawValue);
			SettingDescriptor.ValueJson = MakeShared<FJsonValueString>(SettingDescriptor.ValueText);
			return SettingDescriptor;
		}

		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(InProperty))
		{
			// ValuePtr는 현재 byte 프로퍼티가 저장된 주소다.
			const void* ValuePtr = ByteProperty->ContainerPtrToValuePtr<void>(InContainerPtr);

			// RawValue는 byte 또는 legacy enum 값이다.
			const int64 RawValue = ByteProperty->GetPropertyValue(ValuePtr);
			if (ByteProperty->Enum)
			{
				SettingDescriptor.ValueKind = TEXT("enum");
				SettingDescriptor.ValueText = ResolveEnumValueText(ByteProperty->Enum, RawValue);
				SettingDescriptor.ValueJson = MakeShared<FJsonValueString>(SettingDescriptor.ValueText);
				return SettingDescriptor;
			}
			SettingDescriptor.ValueKind = TEXT("integer");
			SettingDescriptor.ValueJson = MakeShared<FJsonValueNumber>(static_cast<double>(RawValue));
			SettingDescriptor.ValueText = FString::Printf(TEXT("%lld"), RawValue);
			return SettingDescriptor;
		}

		if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty))
		{
			// ValuePtr는 현재 numeric 프로퍼티가 저장된 주소다.
			const void* ValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(InContainerPtr);
			if (NumericProperty->IsInteger())
			{
				// IntValue는 현재 numeric integer 값이다.
				const int64 IntValue = NumericProperty->GetSignedIntPropertyValue(ValuePtr);
				SettingDescriptor.ValueKind = TEXT("integer");
				SettingDescriptor.ValueJson = MakeShared<FJsonValueNumber>(static_cast<double>(IntValue));
				SettingDescriptor.ValueText = FString::Printf(TEXT("%lld"), IntValue);
				return SettingDescriptor;
			}
			// FloatValue는 현재 numeric floating point 값이다.
			const double FloatValue = NumericProperty->GetFloatingPointPropertyValue(ValuePtr);
			SettingDescriptor.ValueKind = TEXT("number");
			SettingDescriptor.ValueJson = MakeShared<FJsonValueNumber>(FloatValue);
			SettingDescriptor.ValueText = FString::SanitizeFloat(FloatValue);
			return SettingDescriptor;
		}

		if (const FStrProperty* StringProperty = CastField<FStrProperty>(InProperty))
		{
			// ValuePtr는 현재 string 프로퍼티가 저장된 주소다.
			const void* ValuePtr = StringProperty->ContainerPtrToValuePtr<void>(InContainerPtr);
			SettingDescriptor.ValueKind = TEXT("string");
			SettingDescriptor.ValueText = MakeBoundedValueText(StringProperty->GetPropertyValue(ValuePtr));
			SettingDescriptor.ValueJson = MakeShared<FJsonValueString>(SettingDescriptor.ValueText);
			return SettingDescriptor;
		}

		if (const FNameProperty* NameProperty = CastField<FNameProperty>(InProperty))
		{
			// ValuePtr는 현재 name 프로퍼티가 저장된 주소다.
			const void* ValuePtr = NameProperty->ContainerPtrToValuePtr<void>(InContainerPtr);
			SettingDescriptor.ValueKind = TEXT("name");
			SettingDescriptor.ValueText = NameProperty->GetPropertyValue(ValuePtr).ToString();
			SettingDescriptor.ValueJson = MakeShared<FJsonValueString>(SettingDescriptor.ValueText);
			return SettingDescriptor;
		}

		if (const FTextProperty* TextProperty = CastField<FTextProperty>(InProperty))
		{
			// ValuePtr는 현재 text 프로퍼티가 저장된 주소다.
			const void* ValuePtr = TextProperty->ContainerPtrToValuePtr<void>(InContainerPtr);
			SettingDescriptor.ValueKind = TEXT("text");
			SettingDescriptor.ValueText = MakeBoundedValueText(TextProperty->GetPropertyValue(ValuePtr).ToString());
			SettingDescriptor.ValueJson = MakeShared<FJsonValueString>(SettingDescriptor.ValueText);
			return SettingDescriptor;
		}

		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty))
		{
			// ValuePtr는 현재 object/class reference 프로퍼티가 저장된 주소다.
			const void* ValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(InContainerPtr);

			// ReferencedObject는 reflection으로 읽은 UObject reference다.
			const UObject* ReferencedObject = ObjectProperty->GetObjectPropertyValue(ValuePtr);
			SettingDescriptor.ValueKind = CastField<FClassProperty>(InProperty) ? TEXT("class_ref") : TEXT("object_ref");
			SettingDescriptor.ValueText = MakeObjectPathText(ReferencedObject);
			SettingDescriptor.ValueJson = MakeShared<FJsonValueString>(SettingDescriptor.ValueText);
			return SettingDescriptor;
		}

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
		{
			// ValuePtr는 현재 struct 프로퍼티가 저장된 주소다.
			const void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(InContainerPtr);

			// StructValue는 지원되는 vector-like struct JSON 값이다.
			const TSharedPtr<FJsonValue> StructValue = TryMakeStructJsonValue(StructProperty, ValuePtr);
			if (StructValue.IsValid())
			{
				SettingDescriptor.ValueKind = TEXT("vector");
				SettingDescriptor.ValueJson = StructValue;
				SettingDescriptor.ValueText = TEXT("<vector>");
				return SettingDescriptor;
			}
		}

		return BuildUnsupportedSetting(InProperty);
	}

	// BuildInputObjectDescriptor는 modifier/trigger UObject를 제한된 descriptor로 변환한다.
	FADumpInputObjectDescriptor BuildInputObjectDescriptor(
		const UObject* InObject,
		int32 InSourceIndex,
		FADumpInputSummary& InOutInputSummary,
		const FString& InTargetPath)
	{
		// Descriptor는 현재 modifier/trigger 출력 항목이다.
		FADumpInputObjectDescriptor Descriptor;
		Descriptor.SourceIndex = InSourceIndex;
		if (!InObject)
		{
			Descriptor.ClassName = TEXT("null");
			Descriptor.ObjectName = TEXT("null");
			return Descriptor;
		}

		Descriptor.ClassName = InObject->GetClass() ? InObject->GetClass()->GetName() : TEXT("UnknownClass");
		Descriptor.ClassPath = InObject->GetClass() ? InObject->GetClass()->GetPathName() : FString();
		Descriptor.ObjectName = InObject->GetName();

		// PropertyArray는 설정 출력을 canonical property name 순서로 고정하기 위한 목록이다.
		TArray<const FProperty*> PropertyArray;
		for (TFieldIterator<FProperty> PropertyIt(InObject->GetClass()); PropertyIt; ++PropertyIt)
		{
			// PropertyObject는 얕은 설정 추출 대상 프로퍼티다.
			const FProperty* PropertyObject = *PropertyIt;
			if (!PropertyObject)
			{
				continue;
			}

			PropertyArray.Add(PropertyObject);
		}

		PropertyArray.Sort([](const FProperty& InLeft, const FProperty& InRight)
		{
			return InLeft.GetName() < InRight.GetName();
		});

		for (const FProperty* PropertyObject : PropertyArray)
		{
			if (Descriptor.Settings.Num() >= InputSummaryMaxSettings)
			{
				Descriptor.bTruncated = true;
				AddInputSummaryWarning(
					InOutInputSummary,
					TEXT("ADUMP_INPUT_SETTING_UNSUPPORTED"),
					FString::Printf(TEXT("Input setting list was truncated at %d entries."), InputSummaryMaxSettings),
					InTargetPath);
				break;
			}

			// SettingDescriptor는 현재 프로퍼티를 typed shallow descriptor로 변환한 결과다.
			FADumpInputSettingDescriptor SettingDescriptor = TryBuildShallowSetting(PropertyObject, InObject);
			if (SettingDescriptor.bUnsupported)
			{
				AddInputSummaryWarning(
					InOutInputSummary,
					TEXT("ADUMP_INPUT_SETTING_UNSUPPORTED"),
					FString::Printf(TEXT("Unsupported input setting type: %s"), *SettingDescriptor.PropertyName),
					FString::Printf(TEXT("%s.%s"), *InTargetPath, *SettingDescriptor.PropertyName));
			}
			Descriptor.Settings.Add(MoveTemp(SettingDescriptor));
		}

		Descriptor.SettingCount = Descriptor.Settings.Num();
		return Descriptor;
	}

	// BuildModifierDescriptors는 modifier chain을 원본 순서대로 descriptor 배열로 변환한다.
	TArray<FADumpInputObjectDescriptor> BuildModifierDescriptors(
		const TArray<TObjectPtr<UInputModifier>>& InModifiers,
		FADumpInputSummary& InOutInputSummary,
		const FString& InTargetPath)
	{
		// DescriptorArray는 modifier chain descriptor 목록이다.
		TArray<FADumpInputObjectDescriptor> DescriptorArray;
		for (int32 ModifierIndex = 0; ModifierIndex < InModifiers.Num(); ++ModifierIndex)
		{
			if (DescriptorArray.Num() >= InputSummaryMaxChainEntries)
			{
				AddInputSummaryWarning(
					InOutInputSummary,
					TEXT("ADUMP_INPUT_CHAIN_TRUNCATED"),
					FString::Printf(TEXT("Input modifier chain was truncated at %d entries."), InputSummaryMaxChainEntries),
					InTargetPath);
				break;
			}
			DescriptorArray.Add(BuildInputObjectDescriptor(
				InModifiers[ModifierIndex].Get(),
				ModifierIndex,
				InOutInputSummary,
				FString::Printf(TEXT("%s.modifiers[%d]"), *InTargetPath, ModifierIndex)));
		}
		return DescriptorArray;
	}

	// BuildTriggerDescriptors는 trigger chain을 원본 순서대로 descriptor 배열로 변환한다.
	TArray<FADumpInputObjectDescriptor> BuildTriggerDescriptors(
		const TArray<TObjectPtr<UInputTrigger>>& InTriggers,
		FADumpInputSummary& InOutInputSummary,
		const FString& InTargetPath)
	{
		// DescriptorArray는 trigger chain descriptor 목록이다.
		TArray<FADumpInputObjectDescriptor> DescriptorArray;
		for (int32 TriggerIndex = 0; TriggerIndex < InTriggers.Num(); ++TriggerIndex)
		{
			if (DescriptorArray.Num() >= InputSummaryMaxChainEntries)
			{
				AddInputSummaryWarning(
					InOutInputSummary,
					TEXT("ADUMP_INPUT_CHAIN_TRUNCATED"),
					FString::Printf(TEXT("Input trigger chain was truncated at %d entries."), InputSummaryMaxChainEntries),
					InTargetPath);
				break;
			}
			DescriptorArray.Add(BuildInputObjectDescriptor(
				InTriggers[TriggerIndex].Get(),
				TriggerIndex,
				InOutInputSummary,
				FString::Printf(TEXT("%s.triggers[%d]"), *InTargetPath, TriggerIndex)));
		}
		return DescriptorArray;
	}

	// CountTruncatedDescriptors는 descriptor 중 설정이 잘린 항목 수를 계산한다.
	int32 CountTruncatedDescriptors(const TArray<FADumpInputObjectDescriptor>& InDescriptors)
	{
		// TruncatedCount는 bTruncated가 true인 descriptor 개수다.
		int32 TruncatedCount = 0;
		for (const FADumpInputObjectDescriptor& Descriptor : InDescriptors)
		{
			if (Descriptor.bTruncated)
			{
				++TruncatedCount;
			}
		}
		return TruncatedCount;
	}

	// CountDescriptorSettings는 descriptor 설정 총 개수를 계산한다.
	int32 CountDescriptorSettings(const TArray<FADumpInputObjectDescriptor>& InDescriptors)
	{
		// SettingCount는 descriptor들이 출력한 얕은 설정 개수 총합이다.
		int32 SettingCount = 0;
		for (const FADumpInputObjectDescriptor& Descriptor : InDescriptors)
		{
			SettingCount += Descriptor.SettingCount;
		}
		return SettingCount;
	}

	// ResolveMappingSettingBehaviorText는 FEnhancedActionKeyMapping의 protected 설정 enum을 reflection으로 읽는다.
	FString ResolveMappingSettingBehaviorText(const FEnhancedActionKeyMapping& InMapping)
	{
		// MappingStruct는 FEnhancedActionKeyMapping reflection 구조체다.
		const UScriptStruct* MappingStruct = FEnhancedActionKeyMapping::StaticStruct();
		if (!MappingStruct)
		{
			return TEXT("unknown");
		}

		// SettingBehaviorProperty는 protected SettingBehavior 프로퍼티를 reflection으로 찾은 결과다.
		const FProperty* SettingBehaviorProperty = MappingStruct->FindPropertyByName(TEXT("SettingBehavior"));
		if (!SettingBehaviorProperty)
		{
			return TEXT("unknown");
		}

		// SettingBehaviorDescriptor는 enum property 또는 byte property에서 읽은 typed 설정 값이다.
		const FADumpInputSettingDescriptor SettingBehaviorDescriptor = TryBuildShallowSetting(SettingBehaviorProperty, &InMapping);
		return SettingBehaviorDescriptor.ValueText.IsEmpty() ? TEXT("unknown") : SettingBehaviorDescriptor.ValueText;
	}

	// BuildDescriptorChainSignature는 mapping 정렬용 modifier/trigger class path signature를 만든다.
	FString BuildDescriptorChainSignature(const TArray<FADumpInputObjectDescriptor>& InDescriptors)
	{
		// SignaturePartArray는 source order를 보존한 class path 목록이다.
		TArray<FString> SignaturePartArray;
		for (const FADumpInputObjectDescriptor& Descriptor : InDescriptors)
		{
			SignaturePartArray.Add(Descriptor.ClassPath);
		}
		return FString::Join(SignaturePartArray, TEXT("|"));
	}

	// BuildMappingPreviewText는 mapping 한 건을 사람이 읽을 수 있는 한 줄로 만든다.
	FString BuildMappingPreviewText(const FADumpInputMappingItem& InMappingItem)
	{
		return FString::Printf(
			TEXT("%d:%s <- %s"),
			InMappingItem.SourceIndex,
			*InMappingItem.ActionName,
			*InMappingItem.KeyName);
	}

	// SortInputMappingsDeterministically는 mapping 출력 순서를 안정적으로 정렬한다.
	void SortInputMappingsDeterministically(TArray<FADumpInputMappingItem>& InOutMappings)
	{
		InOutMappings.Sort([](const FADumpInputMappingItem& InLeft, const FADumpInputMappingItem& InRight)
		{
			if (InLeft.ActionPath != InRight.ActionPath)
			{
				return InLeft.ActionPath < InRight.ActionPath;
			}
			if (InLeft.KeyName != InRight.KeyName)
			{
				return InLeft.KeyName < InRight.KeyName;
			}
			const FString LeftModifierSignature = BuildDescriptorChainSignature(InLeft.Modifiers);
			const FString RightModifierSignature = BuildDescriptorChainSignature(InRight.Modifiers);
			if (LeftModifierSignature != RightModifierSignature)
			{
				return LeftModifierSignature < RightModifierSignature;
			}
			const FString LeftTriggerSignature = BuildDescriptorChainSignature(InLeft.Triggers);
			const FString RightTriggerSignature = BuildDescriptorChainSignature(InRight.Triggers);
			if (LeftTriggerSignature != RightTriggerSignature)
			{
				return LeftTriggerSignature < RightTriggerSignature;
			}
			return InLeft.SourceIndex < InRight.SourceIndex;
		});
	}

	// BuildInputActionSummary는 UInputAction 전용 input_summary_v1 결과를 채운다.
	bool BuildInputActionSummary(const UInputAction& InInputAction, FADumpInputSummary& OutInputSummary, FADumpPerf& InOutPerf)
	{
		OutInputSummary.SchemaVersion = ADumpInput::GetSchemaVersionText();
		OutInputSummary.AssetKind = TEXT("input_action");
		OutInputSummary.bSupported = true;
		OutInputSummary.ValueType = ResolveInputSummaryValueTypeText(InInputAction.ValueType);
		OutInputSummary.AccumulationBehavior = ResolveAccumulationBehaviorText(InInputAction.AccumulationBehavior);
		OutInputSummary.ActionDescription = InInputAction.ActionDescription.ToString();
		OutInputSummary.bConsumeInput = InInputAction.bConsumeInput;
		OutInputSummary.bConsumeLegacyActionAndAxis = InInputAction.bConsumesActionAndAxisMappings;
		OutInputSummary.bReserveAllMappings = InInputAction.bReserveAllMappings;
		OutInputSummary.bTriggerWhenPaused = InInputAction.bTriggerWhenPaused;
		OutInputSummary.TriggerEventsThatConsumeLegacyKeys = InInputAction.TriggerEventsThatConsumeLegacyKeys;
		OutInputSummary.PlayerMappableSettingsPath = MakeObjectPathText(InInputAction.GetPlayerMappableKeySettings().Get());
		OutInputSummary.ActionModifiers = BuildModifierDescriptors(InInputAction.Modifiers, OutInputSummary, TEXT("input_action"));
		OutInputSummary.ActionTriggers = BuildTriggerDescriptors(InInputAction.Triggers, OutInputSummary, TEXT("input_action"));
		OutInputSummary.ModifierCount = OutInputSummary.ActionModifiers.Num();
		OutInputSummary.TriggerCount = OutInputSummary.ActionTriggers.Num();
		OutInputSummary.TruncatedEntryCount =
			CountTruncatedDescriptors(OutInputSummary.ActionModifiers)
			+ CountTruncatedDescriptors(OutInputSummary.ActionTriggers);
		if (OutInputSummary.PreviewLines.Num() < InputSummaryMaxPreviewLines)
		{
			OutInputSummary.PreviewLines.Add(FString::Printf(
				TEXT("input_action value_type=%s modifiers=%d triggers=%d"),
				*OutInputSummary.ValueType,
				OutInputSummary.ModifierCount,
				OutInputSummary.TriggerCount));
		}
		InOutPerf.PropertyCount += CountDescriptorSettings(OutInputSummary.ActionModifiers);
		InOutPerf.PropertyCount += CountDescriptorSettings(OutInputSummary.ActionTriggers);
		return true;
	}

	// BuildMappingItem은 FEnhancedActionKeyMapping을 input_summary_v1 mapping 항목으로 변환한다.
	FADumpInputMappingItem BuildMappingItem(
		const FEnhancedActionKeyMapping& InMapping,
		int32 InSourceIndex,
		FADumpInputSummary& InOutInputSummary)
	{
		// MappingItem은 현재 출력할 mapping 요약 항목이다.
		FADumpInputMappingItem MappingItem;
		MappingItem.SourceIndex = InSourceIndex;
		MappingItem.ActionPath = MakeObjectPathText(InMapping.Action.Get());
		MappingItem.ActionName = InMapping.Action ? InMapping.Action->GetName() : TEXT("None");
		MappingItem.LinkedActionValueType = InMapping.Action ? ResolveInputSummaryValueTypeText(InMapping.Action->ValueType) : FString();
		MappingItem.PlayerMappableSettingsPath = MakeObjectPathText(InMapping.GetPlayerMappableKeySettings());
		MappingItem.KeyName = InMapping.Key.IsValid() ? InMapping.Key.GetFName().ToString() : TEXT("Invalid");
		MappingItem.KeyDisplayText = InMapping.Key.IsValid() ? InMapping.Key.GetDisplayName().ToString() : FString();
		MappingItem.bKeyValid = InMapping.Key.IsValid();
		MappingItem.SettingBehavior = ResolveMappingSettingBehaviorText(InMapping);
		MappingItem.Modifiers = BuildModifierDescriptors(InMapping.Modifiers, InOutInputSummary, FString::Printf(TEXT("mappings[%d]"), InSourceIndex));
		MappingItem.Triggers = BuildTriggerDescriptors(InMapping.Triggers, InOutInputSummary, FString::Printf(TEXT("mappings[%d]"), InSourceIndex));
		MappingItem.ModifierCount = MappingItem.Modifiers.Num();
		MappingItem.TriggerCount = MappingItem.Triggers.Num();
		return MappingItem;
	}

	// BuildInputMappingContextSummary는 UInputMappingContext 전용 input_summary_v1 결과를 채운다.
	bool BuildInputMappingContextSummary(const UInputMappingContext& InInputMappingContext, FADumpInputSummary& OutInputSummary, FADumpPerf& InOutPerf)
	{
		OutInputSummary.SchemaVersion = ADumpInput::GetSchemaVersionText();
		OutInputSummary.AssetKind = TEXT("input_mapping_context");
		OutInputSummary.bSupported = true;
		OutInputSummary.ContextDescription = InInputMappingContext.ContextDescription.ToString();
		OutInputSummary.RegistrationTrackingMode = ResolveRegistrationTrackingText(InInputMappingContext.GetRegistrationTrackingMode());
		OutInputSummary.InputModeFilterOption = ResolveInputModeFilterText(InInputMappingContext.GetInputModeFilterOptions());

		// MappingArray는 InputMappingContext 기본 키 매핑 배열이다.
		const TArray<FEnhancedActionKeyMapping>& MappingArray = InInputMappingContext.GetMappings();
		OutInputSummary.TotalMappingCount = MappingArray.Num();

		for (int32 MappingIndex = 0; MappingIndex < MappingArray.Num(); ++MappingIndex)
		{
			// MappingItem은 현재 원본 index의 key mapping이다.
			const FEnhancedActionKeyMapping& MappingItem = MappingArray[MappingIndex];
			if (MappingItem.Action)
			{
				++OutInputSummary.ActionReferenceCount;
			}
			else
			{
				++OutInputSummary.NullActionCount;
				AddInputSummaryWarning(
					OutInputSummary,
					TEXT("ADUMP_INPUT_NULL_ACTION"),
					FString::Printf(TEXT("Input mapping at source index %d has no action."), MappingIndex),
					FString::Printf(TEXT("mappings[%d].action"), MappingIndex));
			}

			if (!MappingItem.Key.IsValid())
			{
				AddInputSummaryWarning(
					OutInputSummary,
					TEXT("ADUMP_INPUT_INVALID_KEY"),
					FString::Printf(TEXT("Input mapping at source index %d has an invalid key."), MappingIndex),
					FString::Printf(TEXT("mappings[%d].key"), MappingIndex));
			}

			if (OutInputSummary.Mappings.Num() >= InputSummaryMaxMappings)
			{
				if (!OutInputSummary.bMappingsTruncated)
				{
					OutInputSummary.bMappingsTruncated = true;
					AddInputSummaryWarning(
						OutInputSummary,
						TEXT("ADUMP_INPUT_MAPPING_TRUNCATED"),
						FString::Printf(TEXT("Input mappings were truncated at %d entries."), InputSummaryMaxMappings),
						TEXT("mappings"));
				}
				continue;
			}

			OutInputSummary.Mappings.Add(BuildMappingItem(MappingItem, MappingIndex, OutInputSummary));
		}

		SortInputMappingsDeterministically(OutInputSummary.Mappings);
		OutInputSummary.EmittedMappingCount = OutInputSummary.Mappings.Num();
		OutInputSummary.TruncatedEntryCount = OutInputSummary.bMappingsTruncated ? 1 : 0;

		for (const FADumpInputMappingItem& MappingItem : OutInputSummary.Mappings)
		{
			OutInputSummary.ModifierCount += MappingItem.ModifierCount;
			OutInputSummary.TriggerCount += MappingItem.TriggerCount;
			OutInputSummary.TruncatedEntryCount += CountTruncatedDescriptors(MappingItem.Modifiers);
			OutInputSummary.TruncatedEntryCount += CountTruncatedDescriptors(MappingItem.Triggers);
			InOutPerf.PropertyCount += CountDescriptorSettings(MappingItem.Modifiers);
			InOutPerf.PropertyCount += CountDescriptorSettings(MappingItem.Triggers);
		}

		for (int32 PreviewIndex = 0; PreviewIndex < OutInputSummary.Mappings.Num() && PreviewIndex < InputSummaryMaxPreviewLines; ++PreviewIndex)
		{
			OutInputSummary.PreviewLines.Add(BuildMappingPreviewText(OutInputSummary.Mappings[PreviewIndex]));
		}
		return true;
	}
}

namespace ADumpInput
{
	const TCHAR* GetSchemaVersionText()
	{
		return TEXT("input_summary_v1");
	}

	bool ExtractInputSummary(
		const FString& InAssetObjectPath,
		FADumpInputSummary& OutInputSummary,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf,
		bool bFailIfUnsupported)
	{
		OutInputSummary = FADumpInputSummary();

		// ExtractStartSeconds는 input_summary 추출 시간 측정 시작점이다.
		const double ExtractStartSeconds = FPlatformTime::Seconds();

		// AssetSoftPath는 UObject 로드를 위한 object path 래퍼다.
		const FSoftObjectPath AssetSoftPath(InAssetObjectPath);

		// AssetObject는 input_summary 대상인지 확인할 로드된 자산 객체다.
		UObject* AssetObject = AssetSoftPath.TryLoad();
		if (!AssetObject)
		{
			AddInputIssue(
				OutIssues,
				TEXT("ADUMP_INPUT_LOAD_FAIL"),
				FString::Printf(TEXT("Failed to load asset for input_summary: %s"), *InAssetObjectPath),
				EADumpIssueSeverity::Error,
				InAssetObjectPath);
			InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - ExtractStartSeconds);
			return false;
		}

		// bExtracted는 로드된 객체에서 input_summary를 만들었는지 나타낸다.
		const bool bExtracted = ExtractInputSummaryFromObject(AssetObject, OutInputSummary, OutIssues, InOutPerf, bFailIfUnsupported);
		InOutPerf.DetailsSeconds += (FPlatformTime::Seconds() - ExtractStartSeconds);
		return bExtracted;
	}

	bool ExtractInputSummaryFromObject(
		const UObject* InAssetObject,
		FADumpInputSummary& OutInputSummary,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf,
		bool bFailIfUnsupported)
	{
		OutInputSummary = FADumpInputSummary();
		if (!InAssetObject)
		{
			if (bFailIfUnsupported)
			{
				AddInputIssue(
					OutIssues,
					TEXT("ADUMP_INPUT_NULL_ASSET"),
					TEXT("input_summary requires a loaded InputAction or InputMappingContext asset."),
					EADumpIssueSeverity::Error,
					FString());
				return false;
			}
			return true;
		}

		if (const UInputAction* InputAction = Cast<UInputAction>(InAssetObject))
		{
			return BuildInputActionSummary(*InputAction, OutInputSummary, InOutPerf);
		}

		if (const UInputMappingContext* InputMappingContext = Cast<UInputMappingContext>(InAssetObject))
		{
			return BuildInputMappingContextSummary(*InputMappingContext, OutInputSummary, InOutPerf);
		}

		if (bFailIfUnsupported)
		{
			AddInputIssue(
				OutIssues,
				TEXT("ADUMP_INPUT_UNSUPPORTED_ASSET"),
				FString::Printf(TEXT("input_summary supports only InputAction and InputMappingContext assets: %s"), *InAssetObject->GetPathName()),
				EADumpIssueSeverity::Error,
				InAssetObject->GetPathName());
			return false;
		}

		return true;
	}
}
