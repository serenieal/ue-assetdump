// File: ADumpJson.h
// Version: v0.4.0
// Changelog:
// - v0.4.0: Saved 대신 플러그인 Dumped 루트를 기본 출력 경로로 쓰는 helper를 추가.
// - v0.3.0: dump 저장 시 manifest/digest/section sidecar 파일도 함께 저장하는 2차 Phase 1 경로를 추가.
// - v0.2.0: 출력 폴더/파일 경로 해석과 자산별 파일명 규칙 helper 추가.
// - v0.1.0: BPDump 공통 JSON 직렬화/저장 helper 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpJson
{
	// BuildDefaultDumpRootDirectory는 AssetDump 플러그인 아래 Dumped 기본 출력 루트를 계산한다.
	FString BuildDefaultDumpRootDirectory();

	// BuildDefaultOutputFilePath는 AssetObjectPath 기준 기본 dump.json 저장 경로를 계산한다.
	FString BuildDefaultOutputFilePath(const FString& AssetObjectPath);

	// ResolveOutputFilePath는 사용자 입력이 폴더면 자산별 파일명을 조합하고, 파일이면 그대로 사용한다.
	FString ResolveOutputFilePath(const FString& UserOutputPath, const FString& AssetObjectPath);

	// BuildTempOutputFilePath는 원자적 교체를 위한 임시 파일 경로를 계산한다.
	FString BuildTempOutputFilePath(const FString& FinalOutputFilePath);

	// MakeResultObject는 FADumpResult를 JSON object 트리로 변환한다.
	TSharedRef<FJsonObject> MakeResultObject(const FADumpResult& InDumpResult);

	// SerializeResult는 결과 구조를 JSON 문자열로 직렬화한다.
	bool SerializeResult(const FADumpResult& InDumpResult, FString& OutJsonText, bool bPrettyPrint = true);

	// SaveJsonTextToFile은 BOM 없는 UTF-8로 temp -> final 교체 저장을 수행한다.
	bool SaveJsonTextToFile(const FString& FinalOutputFilePath, const FString& JsonText, FString& OutErrorMessage);

	// SaveResultToFile은 결과 구조 직렬화와 저장을 한 번에 수행한다.
	bool SaveResultToFile(const FString& FinalOutputFilePath, const FADumpResult& InDumpResult, FString& OutErrorMessage);
}
