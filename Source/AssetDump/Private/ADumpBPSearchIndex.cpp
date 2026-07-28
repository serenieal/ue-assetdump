// File: ADumpBPSearchIndex.cpp
// Version: v0.1.0
// Changelog:
// - v0.1.0: graph/event/call/variable/class symbol, deterministic normalization/order, bounded terms와 512-symbol 제한 구현.
// Migration:
// - UObject graph를 재순회하지 않고 FADumpAssetInfo/FADumpGraph 결과만 사용한다.

#include "ADumpBPSearchIndex.h"

#include "Algo/Sort.h"
#include "Misc/PackageName.h"

namespace
{
	constexpr int32 MaxSearchSymbolCount = 512;
	constexpr int32 MaxSearchTermCount = 8;

	// IsBlueprintAssetFamily는 AssetDump canonical asset family가 Blueprint 계열인지 검사한다.
	bool IsBlueprintAssetFamily(const FString& InAssetFamily)
	{
		return InAssetFamily.EndsWith(TEXT("_blueprint"), ESearchCase::CaseSensitive);
	}

	// ResolveKindRank는 symbol kind의 deterministic registry rank를 반환한다.
	int32 ResolveKindRank(const FString& InKind)
	{
		if (InKind == TEXT("graph")) return 0;
		if (InKind == TEXT("event")) return 1;
		if (InKind == TEXT("function_call")) return 2;
		if (InKind == TEXT("interface_call")) return 3;
		if (InKind == TEXT("variable_read")) return 4;
		if (InKind == TEXT("variable_write")) return 5;
		if (InKind == TEXT("class_reference")) return 6;
		return 100;
	}

	// ResolveNodeSymbolKind는 graph node role을 search symbol kind로 변환한다.
	FString ResolveNodeSymbolKind(const FString& InPrimaryRole)
	{
		if (InPrimaryRole == TEXT("event")) return TEXT("event");
		if (InPrimaryRole == TEXT("function_call")) return TEXT("function_call");
		if (InPrimaryRole == TEXT("interface_call")) return TEXT("interface_call");
		if (InPrimaryRole == TEXT("variable_get")) return TEXT("variable_read");
		if (InPrimaryRole == TEXT("variable_set")) return TEXT("variable_write");
		return FString();
	}

	// ResolveShortClassName은 class/object path에서 검색용 짧은 이름을 추출한다.
	FString ResolveShortClassName(const FString& InClassPath)
	{
		FString Name = FPackageName::ObjectPathToObjectName(InClassPath);
		if (Name.IsEmpty())
		{
			Name = FPackageName::GetShortName(InClassPath);
		}
		if (Name.EndsWith(TEXT("_C"), ESearchCase::CaseSensitive))
		{
			Name.LeftChopInline(2);
		}
		return Name.IsEmpty() ? InClassPath : Name;
	}

	// AddSearchTerm은 case-insensitive 중복 없이 bounded term을 추가한다.
	void AddSearchTerm(TArray<FString>& InOutTerms, TSet<FString>& InOutNormalizedTerms, const FString& InTerm)
	{
		if (InOutTerms.Num() >= MaxSearchTermCount)
		{
			return;
		}

		const FString TrimmedTerm = InTerm.TrimStartAndEnd();
		if (TrimmedTerm.IsEmpty())
		{
			return;
		}

		const FString DedupKey = TrimmedTerm.ToLower();
		if (InOutNormalizedTerms.Contains(DedupKey))
		{
			return;
		}
		InOutNormalizedTerms.Add(DedupKey);
		InOutTerms.Add(TrimmedTerm);
	}

	// FinalizeSearchTerms는 contract source order로 symbol search terms를 구성한다.
	void FinalizeSearchTerms(FADumpBPSearchSymbol& InOutSymbol)
	{
		InOutSymbol.SearchTerms.Reset();
		TSet<FString> NormalizedTerms;
		AddSearchTerm(InOutSymbol.SearchTerms, NormalizedTerms, InOutSymbol.Name);
		AddSearchTerm(InOutSymbol.SearchTerms, NormalizedTerms, InOutSymbol.NormalizedName);
		AddSearchTerm(InOutSymbol.SearchTerms, NormalizedTerms, InOutSymbol.MemberName);
		AddSearchTerm(InOutSymbol.SearchTerms, NormalizedTerms, InOutSymbol.MemberParent);
		AddSearchTerm(InOutSymbol.SearchTerms, NormalizedTerms, InOutSymbol.GraphName);
		AddSearchTerm(InOutSymbol.SearchTerms, NormalizedTerms, InOutSymbol.PrimaryRole);
		AddSearchTerm(InOutSymbol.SearchTerms, NormalizedTerms, InOutSymbol.GraphType);
		AddSearchTerm(InOutSymbol.SearchTerms, NormalizedTerms, InOutSymbol.Kind);
	}

	// AddClassReferenceCandidate는 canonical class path 기준으로 unique class symbol 후보를 추가한다.
	void AddClassReferenceCandidate(
		const FString& InClassPath,
		TSet<FString>& InOutClassPathSet,
		TArray<FADumpBPSearchSymbol>& InOutSymbols)
	{
		const FString ClassPath = InClassPath.TrimStartAndEnd();
		if (ClassPath.IsEmpty() || InOutClassPathSet.Contains(ClassPath))
		{
			return;
		}
		InOutClassPathSet.Add(ClassPath);

		FADumpBPSearchSymbol Symbol;
		Symbol.Kind = TEXT("class_reference");
		Symbol.Name = ResolveShortClassName(ClassPath);
		Symbol.NormalizedName = ADumpBPSearchIndex::NormalizeSearchName(Symbol.Name);
		Symbol.MemberParent = ClassPath;
		FinalizeSearchTerms(Symbol);
		InOutSymbols.Add(MoveTemp(Symbol));
	}

	// SortSearchSymbols는 contract sort keys로 symbol 후보를 정렬한다.
	void SortSearchSymbols(TArray<FADumpBPSearchSymbol>& InOutSymbols)
	{
		Algo::Sort(InOutSymbols, [](const FADumpBPSearchSymbol& Left, const FADumpBPSearchSymbol& Right)
		{
			const int32 LeftRank = ResolveKindRank(Left.Kind);
			const int32 RightRank = ResolveKindRank(Right.Kind);
			if (LeftRank != RightRank) return LeftRank < RightRank;
			if (Left.NormalizedName != Right.NormalizedName) return Left.NormalizedName < Right.NormalizedName;
			if (Left.Name != Right.Name) return Left.Name < Right.Name;
			if (Left.GraphName != Right.GraphName) return Left.GraphName < Right.GraphName;
			if (Left.NodeId != Right.NodeId) return Left.NodeId < Right.NodeId;
			return Left.MemberParent < Right.MemberParent;
		});
	}

	// RecountSearchSymbolKinds는 emitted symbol 종류별 summary count를 계산한다.
	void RecountSearchSymbolKinds(FADumpBPSearchIndex& InOutIndex)
	{
		InOutIndex.GraphSymbolCount = 0;
		InOutIndex.EventSymbolCount = 0;
		InOutIndex.FunctionCallSymbolCount = 0;
		InOutIndex.InterfaceCallSymbolCount = 0;
		InOutIndex.VariableReadSymbolCount = 0;
		InOutIndex.VariableWriteSymbolCount = 0;
		InOutIndex.ClassReferenceSymbolCount = 0;
		for (const FADumpBPSearchSymbol& Symbol : InOutIndex.Symbols)
		{
			if (Symbol.Kind == TEXT("graph")) ++InOutIndex.GraphSymbolCount;
			else if (Symbol.Kind == TEXT("event")) ++InOutIndex.EventSymbolCount;
			else if (Symbol.Kind == TEXT("function_call")) ++InOutIndex.FunctionCallSymbolCount;
			else if (Symbol.Kind == TEXT("interface_call")) ++InOutIndex.InterfaceCallSymbolCount;
			else if (Symbol.Kind == TEXT("variable_read")) ++InOutIndex.VariableReadSymbolCount;
			else if (Symbol.Kind == TEXT("variable_write")) ++InOutIndex.VariableWriteSymbolCount;
			else if (Symbol.Kind == TEXT("class_reference")) ++InOutIndex.ClassReferenceSymbolCount;
		}
	}
}

namespace ADumpBPSearchIndex
{
	FString NormalizeSearchName(const FString& InName)
	{
		FString Result = InName.TrimStartAndEnd().ToLower();
		for (const TCHAR Separator : { TEXT('.'), TEXT('/'), TEXT('\\'), TEXT(':'), TEXT('_'), TEXT('-') })
		{
			Result.ReplaceCharInline(Separator, TEXT(' '));
		}

		TArray<FString> TokenArray;
		Result.ParseIntoArrayWS(TokenArray);
		return FString::Join(TokenArray, TEXT(" "));
	}

	bool BuildSearchIndex(
		const FADumpAssetInfo& InAssetInfo,
		const TArray<FADumpGraph>& InGraphs,
		bool bInLinksOnly,
		bool bInExplicitRequest,
		FADumpBPSearchIndex& OutSearchIndex)
	{
		OutSearchIndex = FADumpBPSearchIndex();

		const bool bIsBlueprint = IsBlueprintAssetFamily(InAssetInfo.AssetFamily);
		if (!bIsBlueprint)
		{
			if (bInExplicitRequest)
			{
				OutSearchIndex.SchemaVersion = TEXT("bp_search_index_v1");
				OutSearchIndex.bSupported = false;
				OutSearchIndex.UnsupportedReason = TEXT("unsupported_asset_class");
			}
			return true;
		}

		OutSearchIndex.SchemaVersion = TEXT("bp_search_index_v1");
		OutSearchIndex.MaxSymbols = MaxSearchSymbolCount;
		if (bInLinksOnly)
		{
			OutSearchIndex.bSupported = false;
			OutSearchIndex.UnsupportedReason = TEXT("links_only");
			return true;
		}

		OutSearchIndex.bSupported = true;
		TArray<FADumpBPSearchSymbol> Candidates;
		TSet<FString> ClassPathSet;

		for (const FADumpGraph& Graph : InGraphs)
		{
			FADumpBPSearchSymbol GraphSymbol;
			GraphSymbol.Kind = TEXT("graph");
			GraphSymbol.Name = Graph.GraphName;
			GraphSymbol.NormalizedName = NormalizeSearchName(GraphSymbol.Name);
			GraphSymbol.GraphName = Graph.GraphName;
			GraphSymbol.GraphType = ToString(Graph.GraphType);
			FinalizeSearchTerms(GraphSymbol);
			Candidates.Add(MoveTemp(GraphSymbol));

			for (const FADumpGraphNode& Node : Graph.Nodes)
			{
				const FString Kind = ResolveNodeSymbolKind(Node.Role.Primary);
				if (!Kind.IsEmpty())
				{
					FADumpBPSearchSymbol Symbol;
					Symbol.Kind = Kind;
					Symbol.Name = Node.MemberName.IsEmpty() ? Node.NodeClass : Node.MemberName;
					Symbol.NormalizedName = NormalizeSearchName(Symbol.Name);
					Symbol.GraphName = Graph.GraphName;
					Symbol.GraphType = ToString(Graph.GraphType);
					Symbol.NodeId = Node.NodeId;
					Symbol.PrimaryRole = Node.Role.Primary;
					Symbol.MemberParent = Node.MemberParent;
					Symbol.MemberName = Node.MemberName;
					FinalizeSearchTerms(Symbol);
					Candidates.Add(MoveTemp(Symbol));
				}

				AddClassReferenceCandidate(Node.MemberParent, ClassPathSet, Candidates);
			}
		}

		AddClassReferenceCandidate(InAssetInfo.ParentClassPath, ClassPathSet, Candidates);
		AddClassReferenceCandidate(InAssetInfo.GeneratedClassPath, ClassPathSet, Candidates);

		SortSearchSymbols(Candidates);
		const int32 CandidateCount = Candidates.Num();
		if (CandidateCount > MaxSearchSymbolCount)
		{
			Candidates.SetNum(MaxSearchSymbolCount);
			OutSearchIndex.bTruncated = true;
			OutSearchIndex.OmittedSymbolCount = CandidateCount - MaxSearchSymbolCount;
		}

		for (int32 SymbolIndex = 0; SymbolIndex < Candidates.Num(); ++SymbolIndex)
		{
			Candidates[SymbolIndex].SymbolId = FString::Printf(TEXT("symbol_%03d"), SymbolIndex);
		}
		OutSearchIndex.Symbols = MoveTemp(Candidates);
		OutSearchIndex.SymbolCount = OutSearchIndex.Symbols.Num();
		RecountSearchSymbolKinds(OutSearchIndex);
		return true;
	}
}
