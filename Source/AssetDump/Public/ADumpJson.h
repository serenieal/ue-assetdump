// File: ADumpJson.h
// Version: v0.7.0
// Changelog:
// - v0.7.0: request metadata용 candidate output path와 실제 writable output path 해석을 분리.
// - v0.6.0: mutation 없는 기본 경로 계산과 실제 writable 준비 helper를 분리해 explicit output 실행의 선제 디렉터리 생성을 제거.
// - v0.5.0: ASSETDUMP_OUTPUT_ROOT, writable PluginRoot/Dumped와 Project/Saved/AssetDump fallback의 source-aware 기본 출력 helper로 확장.
// - v0.4.0: Saved 대신 플러그인 Dumped 루트를 기본 출력 경로로 쓰는 helper를 추가.
// - v0.3.0: dump 저장 시 manifest/digest/section sidecar 파일도 함께 저장하는 2차 Phase 1 경로를 추가.
// - v0.2.0: 출력 폴더/파일 경로 해석과 자산별 파일명 규칙 helper 추가.
// - v0.1.0: BPDump 공통 JSON 직렬화/저장 helper 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpJson
{
	// BuildDefaultDumpRootDirectory는 환경 변수 또는 legacy Plugin Dumped 기본 후보를 mutation 없이 계산한다.
	FString BuildDefaultDumpRootDirectory(FString* OutSource = nullptr);

	// ResolveWritableDefaultDumpRootDirectory는 실제 기본 출력이 필요할 때 env, Plugin Dumped와 Project Saved를 write probe해 결정한다.
	FString ResolveWritableDefaultDumpRootDirectory(FString* OutSource = nullptr);

	// BuildDefaultOutputFilePathCandidate는 파일시스템 변경 없이 기본 dump.json 후보 경로를 계산한다.
	FString BuildDefaultOutputFilePathCandidate(const FString& AssetObjectPath);

	// BuildDefaultOutputFilePath는 AssetObjectPath 기준 writable 기본 dump.json 저장 경로를 계산한다.
	FString BuildDefaultOutputFilePath(const FString& AssetObjectPath);

	// ResolveOutputFilePathCandidate는 request metadata용 최종 후보 경로를 mutation 없이 계산한다.
	FString ResolveOutputFilePathCandidate(const FString& UserOutputPath, const FString& AssetObjectPath);

	// ResolveOutputFilePath는 실제 저장용 경로를 계산하며 명시 경로가 없으면 writable 기본 루트를 준비한다.
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
