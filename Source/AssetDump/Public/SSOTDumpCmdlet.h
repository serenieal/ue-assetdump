// SSOTDumpCmdlet.h
// v0.3 - Wheel SSOT 덤프(ssot_roots.json + ssot_wheel.json)

#pragma once

#include "Commandlets/Commandlet.h"
#include "SSOTDumpCmdlet.generated.h"

UCLASS()
class USSOTDumpCmdlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// 커맨드렛 진입점
	virtual int32 Main(const FString& CmdLine) override;

private:
	// 출력 폴더
	FString OutputDir;

	// 덤프 대상
	FString DumpTarget;

	// meta에 시간 기록 여부
	bool bWriteTimeToMeta = true;

	// cfg 화이트리스트 파일명
	FString CfgWhitelistFileName = TEXT("ssot_cfg.lst");

	// roots 파일명(출력폴더 기준)
	FString RootsFileName = TEXT("ssot_roots.json");

	// 엄격 모드: FAIL 하나라도 있으면 exit code 1
	bool bStrictFail = true;

	// 커맨드라인 파싱
	void ParseArgs(const FString& CmdLine);

	// 개별 덤프
	bool DumpMeta() const;
	bool DumpAssets() const;
	bool DumpConfig() const;
	bool DumpTags() const;
	bool DumpInput() const;

	// Wheel SSOT 덤프 (v0.3)
	bool DumpWheel() const;

	// 파일 경로 유틸
	FString MakePath(const FString& FileName) const;
};