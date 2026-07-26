// File: ADumpComponentTree.h
// Version: v1.0
// Changelog:
// - v1.0: Actor Blueprint용 결정적 component_tree_v1 추출기 공개 계약을 추가.

#pragma once

#include "CoreMinimal.h"

#include "ADumpTypes.h"

namespace ADumpComponentTree
{
	// ExtractComponentTree는 Actor Blueprint의 CDO와 SCS에서 경량 컴포넌트 forest를 추출한다.
	bool ExtractComponentTree(
		const FString& InAssetObjectPath,
		FADumpComponentTree& OutComponentTree,
		TArray<FADumpIssue>& OutIssues,
		bool bInExplicitRequest);
}
