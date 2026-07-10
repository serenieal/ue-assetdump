// File: ADumpWidgetTree.h
// Version: v1.0
// Changelog:
// - v1.0: WidgetBlueprint Designer hierarchy summary builder 선언 추가.
//
// Purpose:
// - UWidgetBlueprint::WidgetTree->RootWidget 기준 Designer 계층구조를 summary 중간 구조로 변환한다.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

class UWidgetBlueprint;

namespace ADumpWidgetTree
{
	// BuildDesignerSummary는 WidgetBlueprint의 Designer tree를 순회해 summary의 WidgetDesigner 필드를 채운다.
	void BuildDesignerSummary(const UWidgetBlueprint& InWidgetBlueprint, FADumpSummary& OutSummary);
}
