// File: ADumpJson.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: BPDump 공통 JSON 직렬화/저장 helper 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpJson
{
	// BuildDefaultOutputFilePath는 AssetObjectPath 기준 기본 dump.json 저장 경로를 계산한다.
	FString BuildDefaultOutputFilePath(const FString& AssetObjectPath);

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
