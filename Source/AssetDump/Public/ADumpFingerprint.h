// File: ADumpFingerprint.h
// Version: v0.1.0
// Changelog:
// - v0.1.0: 2차 개선안 Phase 2 기준으로 options hash, asset fingerprint, manifest fingerprint 조회 helper를 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpFingerprint
{
	// BuildManifestFilePath는 dump.json 옆에 저장되는 manifest.json 경로를 계산한다.
	FString BuildManifestFilePath(const FString& InOutputFilePath);

	// BuildOptionsHash는 요청 옵션만 반영한 안정 해시 문자열을 계산한다.
	FString BuildOptionsHash(const FADumpRequestInfo& InRequestInfo);

	// BuildAssetFingerprint는 자산 상태와 요청 옵션을 함께 반영한 최신성 판정용 fingerprint를 계산한다.
	FString BuildAssetFingerprint(
		const FString& InAssetObjectPath,
		const FADumpRequestInfo& InRequestInfo,
		const FString& InSchemaVersion,
		const FString& InExtractorVersion);

	// TryReadManifestFingerprint는 기존 manifest.json에서 fingerprint를 읽어온다.
	bool TryReadManifestFingerprint(const FString& InOutputFilePath, FString& OutFingerprint, FString& OutManifestFilePath);
}
