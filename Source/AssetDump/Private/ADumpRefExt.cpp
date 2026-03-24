// File: ADumpRefExt.cpp
// Version: v0.1.0
// Changelog:
// - v0.1.0: Details 섹션에 기록된 실제 값만 사용해 hard/soft 직접 참조를 수집하는 구현 추가.

#include "ADumpRefExt.h"

namespace
{
	// AddRefIssue는 reference 추출기에서 공통 issue 기록을 단순화한다.
	void AddRefIssue(
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

	// NormalizeRefText는 공백/None/nullptr 같은 비어 있는 값을 제거한다.
	FString NormalizeRefText(const FString& InText)
	{
		FString NormalizedText = InText;
		NormalizedText.TrimStartAndEndInline();
		if (NormalizedText.IsEmpty()
			|| NormalizedText.Equals(TEXT("None"), ESearchCase::IgnoreCase)
			|| NormalizedText.Equals(TEXT("nullptr"), ESearchCase::IgnoreCase)
			|| NormalizedText.Equals(TEXT("null"), ESearchCase::IgnoreCase))
		{
			return FString();
		}
		return NormalizedText;
	}

	// IsReferenceKind는 property value kind가 reference 계열인지 판별한다.
	bool IsReferenceKind(EADumpValueKind InValueKind)
	{
		return InValueKind == EADumpValueKind::ObjectRef
			|| InValueKind == EADumpValueKind::ClassRef
			|| InValueKind == EADumpValueKind::SoftObjectRef
			|| InValueKind == EADumpValueKind::SoftClassRef;
	}

	// IsSoftReferenceKind는 reference 계열 중 soft reference인지 판별한다.
	bool IsSoftReferenceKind(EADumpValueKind InValueKind)
	{
		return InValueKind == EADumpValueKind::SoftObjectRef
			|| InValueKind == EADumpValueKind::SoftClassRef;
	}

	// LooksLikeAssetPath는 문자열이 자산/오브젝트 경로로 보이는지 최소 판별한다.
	bool LooksLikeAssetPath(const FString& InText)
	{
		return InText.Contains(TEXT("/")) || InText.Contains(TEXT(".")) || InText.Contains(TEXT(":"));
	}

	// AppendRefItem은 중복 없이 hard/soft 참조 배열에 실제 값을 추가한다.
	void AppendRefItem(
		const FADumpPropertyItem& InPropertyItem,
		const FString& InSource,
		TArray<FADumpRefItem>& InOutRefItems,
		TSet<FString>& InOutUniqueKeys,
		FADumpPerf& InOutPerf)
	{
		const FString NormalizedPath = NormalizeRefText(InPropertyItem.ValueText);
		if (NormalizedPath.IsEmpty())
		{
			return;
		}

		if (!LooksLikeAssetPath(NormalizedPath))
		{
			return;
		}

		const FString UniqueKey = FString::Printf(TEXT("%s|%s|%s"), *NormalizedPath, *InSource, *InPropertyItem.PropertyPath);
		if (InOutUniqueKeys.Contains(UniqueKey))
		{
			return;
		}
		InOutUniqueKeys.Add(UniqueKey);

		FADumpRefItem NewRefItem;
		NewRefItem.Path = NormalizedPath;
		NewRefItem.ClassName = InPropertyItem.CppType;
		NewRefItem.Source = InSource;
		NewRefItem.SourcePath = InPropertyItem.PropertyPath;
		InOutRefItems.Add(MoveTemp(NewRefItem));
		InOutPerf.ReferenceCount++;
	}

	// CollectRefsFromPropertyItems는 property 배열에서 reference 계열 값만 읽어 hard/soft 참조를 누적한다.
	void CollectRefsFromPropertyItems(
		const TArray<FADumpPropertyItem>& InPropertyItems,
		const FString& InSource,
		TArray<FADumpRefItem>& InOutHardRefs,
		TArray<FADumpRefItem>& InOutSoftRefs,
		TSet<FString>& InOutUniqueHardKeys,
		TSet<FString>& InOutUniqueSoftKeys,
		FADumpPerf& InOutPerf)
	{
		for (const FADumpPropertyItem& PropertyItem : InPropertyItems)
		{
			if (!IsReferenceKind(PropertyItem.ValueKind))
			{
				continue;
			}

			if (IsSoftReferenceKind(PropertyItem.ValueKind))
			{
				AppendRefItem(PropertyItem, InSource, InOutSoftRefs, InOutUniqueSoftKeys, InOutPerf);
			}
			else
			{
				AppendRefItem(PropertyItem, InSource, InOutHardRefs, InOutUniqueHardKeys, InOutPerf);
			}
		}
	}
}

namespace ADumpRefExt
{
	bool ExtractReferences(
		const FString& AssetObjectPath,
		const FADumpDetails& InDetails,
		FADumpReferences& OutReferences,
		TArray<FADumpIssue>& OutIssues,
		FADumpPerf& InOutPerf)
	{
		OutReferences.Hard.Reset();
		OutReferences.Soft.Reset();

		TSet<FString> UniqueHardKeys;
		TSet<FString> UniqueSoftKeys;

		CollectRefsFromPropertyItems(
			InDetails.ClassDefaults,
			TEXT("details.class_defaults"),
			OutReferences.Hard,
			OutReferences.Soft,
			UniqueHardKeys,
			UniqueSoftKeys,
			InOutPerf);

		for (const FADumpComponentItem& ComponentItem : InDetails.Components)
		{
			const FString ComponentSource = FString::Printf(TEXT("details.components.%s"), *ComponentItem.ComponentName);
			CollectRefsFromPropertyItems(
				ComponentItem.Properties,
				ComponentSource,
				OutReferences.Hard,
				OutReferences.Soft,
				UniqueHardKeys,
				UniqueSoftKeys,
				InOutPerf);
		}

		if (OutReferences.Hard.Num() == 0 && OutReferences.Soft.Num() == 0)
		{
			AddRefIssue(
				OutIssues,
				TEXT("REFERENCES_EMPTY"),
				TEXT("No direct references were found from recorded details values."),
				EADumpIssueSeverity::Info,
				EADumpPhase::References,
				AssetObjectPath);
		}

		return true;
	}
}
