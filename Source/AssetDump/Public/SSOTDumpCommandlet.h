// SSOTDumpCommandlet.h
// v0.3b
// 목적: -run=SSOTDump 가 찾는 "USSOTDumpCommandlet" 클래스를 제공한다.

#pragma once

#include "SSOTDumpCmdlet.h"
#include "SSOTDumpCommandlet.generated.h"

UCLASS()
class USSOTDumpCommandlet : public USSOTDumpCmdlet
{
	GENERATED_BODY()
	// 구현 없음: 부모(USSOTDumpCmdlet)의 Main()을 그대로 사용
};