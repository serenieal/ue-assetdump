// File: AssetDumpCommandlet.h
// Version: v0.2.8
// Changelog:
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
