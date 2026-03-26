// File: ADumpEditorApi.h
// Version: v0.4.1
// Changelog:
// - v0.4.1: Compile Before Dump, Skip If Up To Date 옵션을 공통 실행 옵션으로 전달하도록 API 시그니처를 확장.
// - v0.4.0: 단계 실행형 덤프 시작/진행/취소/상태 조회 API 추가.
// - v0.3.0: 수동 덤프에서 그래프 필터 옵션(GraphNameFilter/LinksOnly/LinkKind)을 전달할 수 있도록 API를 확장.
// - v0.2.0: 옵션 기반 공통 에디터 dump API 추가.
// - v0.1.0: Editor Utility Widget가 호출할 수 있는 BPDump 에디터용 API 추가.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "ADumpEditorApi.generated.h"

UCLASS()
class UADumpEditorApi : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// GetSelectedBlueprintObjectPath는 현재 에디터에서 선택된 첫 Blueprint의 오브젝트 경로를 반환한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 에디터에서 선택된 첫 Blueprint의 오브젝트 경로와 표시 이름을 반환합니다."))
	static bool GetSelectedBlueprintObjectPath(FString& OutAssetObjectPath, FString& OutDisplayName, FString& OutMessage);

	// StartDumpSelectedBlueprint는 현재 선택된 Blueprint에 대해 단계 실행형 덤프를 시작한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 선택된 Blueprint에 대해 진행률/취소가 가능한 단계 실행형 덤프를 시작합니다."))
	static bool StartDumpSelectedBlueprint(
		const FString& OutputFilePath,
		bool bIncludeSummary,
		bool bIncludeDetails,
		bool bIncludeGraphs,
		bool bIncludeReferences,
		bool bCompileBeforeDump,
		bool bSkipIfUpToDate,
		const FString& GraphNameFilter,
		bool bLinksOnly,
		const FString& LinkKindText,
		FString& OutMessage);

	// TickActiveDump는 현재 단계 실행형 덤프의 다음 단계를 한 번 진행한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 실행 중인 덤프의 다음 단계를 한 번 진행합니다."))
	static bool TickActiveDump(FString& OutMessage);

	// CancelActiveDump는 현재 실행 중인 덤프 취소를 요청한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 실행 중인 덤프 취소를 요청합니다."))
	static void CancelActiveDump();

	// IsDumpRunning은 현재 덤프가 실행 중인지 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 덤프가 실행 중인지 반환합니다."))
	static bool IsDumpRunning();

	// GetDumpProgressPercent01은 현재 진행률(0~1)을 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 덤프 진행률을 0.0 ~ 1.0 범위로 반환합니다."))
	static float GetDumpProgressPercent01();

	// GetDumpPhaseText는 현재 단계 표시용 문자열을 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 덤프 단계 표시용 문자열을 반환합니다."))
	static FString GetDumpPhaseText();

	// GetDumpDetailText는 현재 세부 상태 문자열을 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 덤프 세부 상태 문자열을 반환합니다."))
	static FString GetDumpDetailText();

	// GetDumpWarningCount는 누적 warning 개수를 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 덤프의 warning 개수를 반환합니다."))
	static int32 GetDumpWarningCount();

	// GetDumpErrorCount는 누적 error 개수를 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 덤프의 error 개수를 반환합니다."))
	static int32 GetDumpErrorCount();

	// GetDumpResolvedOutputPath는 마지막 또는 현재 덤프의 출력 파일 경로를 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "마지막 또는 현재 덤프의 출력 파일 경로를 반환합니다."))
	static FString GetDumpResolvedOutputPath();

	// GetDumpStatusMessage는 현재 상태 메시지를 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 덤프 상태 메시지를 반환합니다."))
	static FString GetDumpStatusMessage();

	// GetDumpLogText는 자동 줄바꿈 대상이 되는 전체 로그 문자열을 반환한다.
	UFUNCTION(BlueprintPure, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 덤프의 전체 로그 문자열을 반환합니다."))
	static FString GetDumpLogText();

	// DumpSelectedBlueprint는 현재 선택된 Blueprint를 옵션 기반으로 동기 덤프한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 선택된 Blueprint를 옵션 기반으로 동기 덤프하고 dump.json 저장 경로를 반환합니다."))
	static bool DumpSelectedBlueprint(
		const FString& OutputFilePath,
		bool bIncludeSummary,
		bool bIncludeDetails,
		bool bIncludeGraphs,
		bool bIncludeReferences,
		bool bCompileBeforeDump,
		bool bSkipIfUpToDate,
		const FString& GraphNameFilter,
		bool bLinksOnly,
		const FString& LinkKindText,
		FString& OutResolvedOutputFilePath,
		FString& OutMessage);

	// DumpBlueprintByPath는 지정한 Blueprint 오브젝트 경로를 옵션 기반으로 동기 덤프한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "Blueprint 오브젝트 경로를 받아 옵션 기반 dump.json을 동기 생성합니다."))
	static bool DumpBlueprintByPath(
		const FString& AssetObjectPath,
		const FString& OutputFilePath,
		bool bIncludeSummary,
		bool bIncludeDetails,
		bool bIncludeGraphs,
		bool bIncludeReferences,
		bool bCompileBeforeDump,
		bool bSkipIfUpToDate,
		const FString& GraphNameFilter,
		bool bLinksOnly,
		const FString& LinkKindText,
		FString& OutResolvedOutputFilePath,
		FString& OutMessage);

	// DumpSelectedBlueprintSummary는 현재 선택된 Blueprint에 대해 Summary 기반 dump.json을 생성한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "현재 선택된 Blueprint를 Summary 기반으로 동기 덤프하고 dump.json 저장 경로를 반환합니다."))
	static bool DumpSelectedBlueprintSummary(const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage);

	// DumpBlueprintSummaryByPath는 지정한 Blueprint 오브젝트 경로에 대해 Summary 기반 dump.json을 생성한다.
	UFUNCTION(BlueprintCallable, Category = "AssetDump|Editor",
		meta = (ToolTip = "Blueprint 오브젝트 경로를 받아 Summary 기반 dump.json을 동기 생성합니다."))
	static bool DumpBlueprintSummaryByPath(const FString& AssetObjectPath, const FString& OutputFilePath, FString& OutResolvedOutputFilePath, FString& OutMessage);
};
