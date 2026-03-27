// File: AssetDumpCommandlet.h
// Version: v0.3.4
// Changelog:
// - v0.3.4: 골든 샘플 덤프와 기본 산출물 검증을 한 번에 실행하는 validate 모드를 추가.
// - v0.3.3: 폴더 단위 batchdump 실행과 run_report.json 생성용 commandlet 모드를 추가.
// - v0.3.2: 저장된 BPDump 결과를 스캔해 index.json / dependency_index.json 을 생성하는 index 모드를 추가.
// - v0.3.1: bpdump 모드에서 -Output 필수 조건을 완화.
// - v0.3.0: bpdump commandlet 경로를 공통 서비스 기반으로 정리.
// - v0.2.8: asset_details 모드 선언을 추가하고, 깨진 주석을 UTF-8로 읽히는 텍스트로 정리함.
// - v0.2.7: links_only 그래프 출력용 LinksMeta 옵션 추가.
// - v0.2.6: bpgraph 옵션(GraphName / LinksOnly / LinkKind) 추가.
// - v0.2.5: 중복 링크 필터링 개선.
// - v0.2.0: Blueprint 그래프 덤프 지원 추가.
// - v0.1.0: list / asset / map JSON 덤프 지원 추가.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "AssetDumpCommandlet.generated.h"

UCLASS()
class UAssetDumpCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Main은 AssetDump 커맨드렛의 진입점을 실행한다.
	virtual int32 Main(const FString& CommandLine) override;

private:
	// EAssetDumpBpLinkKind는 그래프 링크를 의미 단위로 필터링한다.
	enum class EAssetDumpBpLinkKind : uint8
	{
		All,
		Exec,
		Data
	};

	// EAssetDumpBpLinksMetaLevel은 links_only 출력에 붙일 메타데이터 양을 제어한다.
	enum class EAssetDumpBpLinksMetaLevel : uint8
	{
		None,
		Min
	};

	// BuildAssetListJson은 모든 에셋 오브젝트를 로드하지 않고 AssetRegistry 검색 결과를 직렬화한다.
	bool BuildAssetListJson(const FString& FilterPath, FString& OutJsonText);

	// BuildAssetJson은 기본 에셋 정보와 registry 태그를 직렬화한다.
	bool BuildAssetJson(const FString& AssetPath, FString& OutJsonText);

	// BuildAssetDetailsJson은 Blueprint CDO 기본값과 컴포넌트 목록을 직렬화한다.
	bool BuildAssetDetailsJson(const FString& AssetPath, FString& OutJsonText);

	// BuildMapJson은 맵의 액터 이름과 트랜스폼을 직렬화한다.
	bool BuildMapJson(const FString& MapAssetPath, FString& OutJsonText);

	// BuildDumpIndexFiles는 저장된 BPDump 폴더를 스캔해 index.json / dependency_index.json 을 생성한다.
	bool BuildDumpIndexFiles(const FString& DumpRootPath, FString& OutIndexFilePath, FString& OutDependencyIndexFilePath);

	// BuildValidationJson은 대표 샘플 자산을 덤프하고 산출물 기본 검증 결과를 JSON으로 직렬화한다.
	bool BuildValidationJson(const FString& CommandLine, FString& OutJsonText, int32& OutFailureCount);

	// SaveJsonToFile은 BOM 없는 UTF-8로 JSON 텍스트를 디스크에 저장한다.
	bool SaveJsonToFile(const FString& OutputFilePath, const FString& JsonText);

	// GetCmdValue는 -Key=Value 형식의 커맨드라인 값을 읽는다.
	bool GetCmdValue(const FString& CommandLine, const TCHAR* Key, FString& OutValue);

	// BuildBlueprintGraphJson은 Blueprint 그래프, 노드, 핀, 링크를 직렬화한다.
	bool BuildBlueprintGraphJson(
		const FString& BlueprintAssetPath,
		const FString& GraphNameFilter,
		bool bLinksOnly,
		EAssetDumpBpLinkKind LinkKindFilter,
		EAssetDumpBpLinksMetaLevel LinksMetaLevel,
		FString& OutJsonText
	);

	// AppendPinLinks는 하나의 출력 핀에 대한 중복 제거 링크를 추가한다.
	void AppendPinLinks(
		class UEdGraphPin* FromPin,
		const FString& FromNodeGuid,
		TArray<TSharedPtr<FJsonValue>>& InOutLinks,
		TSet<FString>& InOutUniqueLinkKeys,
		EAssetDumpBpLinkKind LinkKindFilter,
		EAssetDumpBpLinksMetaLevel LinksMetaLevel
	);
};
