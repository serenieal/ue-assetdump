// File: AssetDumpCommandlet.h
// Version: v0.2.7
// Changelog:
// - v0.2.7: LinksOnly에서도 사람이 읽기 쉽도록 links에 최소 메타(노드 타이틀/핀 이름/카테고리) 옵션 추가(LinksMeta)
// - v0.2.6: bpgraph 옵션 추가(GraphName / LinksOnly / LinkKind)
// - v0.2.5: 링크 중복 제거(Output 핀만 기록 + 키 기반 중복 제거)
// - v0.2.0: Blueprint 그래프(노드/핀/링크) 덤프 기능 추가
// - v0.1.0: AssetRegistry 기반 목록/에셋 덤프/맵 액터 덤프를 JSON으로 출력

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "AssetDumpCommandlet.generated.h"

UCLASS()
class UAssetDumpCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Run() 함수: 커맨드렛 진입점
	virtual int32 Main(const FString& CommandLine) override;

private:
	// Enum: EAssetDumpBpLinkKind
	// Purpose: bpgraph 링크 필터 종류
	enum class EAssetDumpBpLinkKind : uint8
	{
		All,
		Exec,
		Data
	};

	// Enum: EAssetDumpBpLinksMetaLevel
	// Purpose: links_only에서도 사람이 읽기 쉬운 최소 메타를 links에 포함할지 여부
	enum class EAssetDumpBpLinksMetaLevel : uint8
	{
		None,
		Min
	};

	// 에셋 목록을 AssetRegistry로 뽑아 JSON 문자열로 만든다(로딩 최소화).
	bool BuildAssetListJson(const FString& FilterPath, FString& OutJsonText);

	// 특정 에셋(블루프린트 포함)을 로드해서 기본 정보/태그를 JSON 문자열로 만든다.
	bool BuildAssetJson(const FString& AssetPath, FString& OutJsonText);

	// 특정 맵(UWorld)을 로드해서 액터 목록/트랜스폼 등을 JSON 문자열로 만든다.
	bool BuildMapJson(const FString& MapAssetPath, FString& OutJsonText);

	// JSON 문자열을 파일로 저장한다.
	bool SaveJsonToFile(const FString& OutputFilePath, const FString& JsonText);

	// CommandLine 파싱 유틸: -Key=Value 형태를 읽는다.
	bool GetCmdValue(const FString& CommandLine, const TCHAR* Key, FString& OutValue);

	// BuildBlueprintGraphJson: 블루프린트 그래프/노드/핀/링크를 JSON으로 생성(옵션 포함)
	bool BuildBlueprintGraphJson(
		const FString& BlueprintAssetPath,
		const FString& GraphNameFilter,
		bool bLinksOnly,
		EAssetDumpBpLinkKind LinkKindFilter,
		EAssetDumpBpLinksMetaLevel LinksMetaLevel,
		FString& OutJsonText
	);


	// AppendPinLinks: 한 핀의 연결 정보를 links 배열에 추가(중복 제거 + LinkKind + LinksMeta)
	void AppendPinLinks(
		class UEdGraphPin* FromPin,
		const FString& FromNodeGuid,
		TArray<TSharedPtr<FJsonValue>>& InOutLinks,
		TSet<FString>& InOutUniqueLinkKeys,
		EAssetDumpBpLinkKind LinkKindFilter,
		EAssetDumpBpLinksMetaLevel LinksMetaLevel
	);


};
