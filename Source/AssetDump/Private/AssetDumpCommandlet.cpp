// File: AssetDumpCommandlet.cpp
// Version: v0.2.7
// Changelog:
// - v0.2.7: LinksMeta 옵션 추가(links_only에서도 사람이 읽기 쉬운 최소 메타를 links에 포함)
// - v0.2.6: GraphName/LinksOnly/LinkKind 옵션 지원 추가
// - v0.2.5: Blueprint 링크 중복 제거(Output 핀만 기록 + 키 기반 중복 제거)
// - v0.2.3: UE 5.7의 UEdGraphNode::GetSubGraphs() 시그니처(인자 없음, TArray 반환)에 맞춰 단순/안전하게 서브그래프 수집
// - v0.2.2: GetSubGraphs 시그니처 차이를 템플릿으로 흡수하기 위해 type_traits/utility 추가
// - v0.2.0: Blueprint 그래프/노드/핀/연결을 JSON으로 덤프
// - v0.1.2: JSON 파일을 UTF-8(BOM 없음)로 강제 저장해서 서버 JSON 파싱 안정화
// - v0.1.0: Mode=list|asset|map, Output 경로 지정 지원


#include "AssetDumpCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SoftObjectPath.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

#include "EdGraphSchema_K2.h"

#include "Engine/Blueprint.h"


int32 UAssetDumpCommandlet::Main(const FString& CommandLine)
{
	// 실행 모드: list / asset / map
	FString ModeValue;
	// 출력 파일 경로
	FString OutputFilePath;
	// 필터 경로(예: /Game/Prototype)
	FString FilterPath;
	// 에셋 경로(예: /Game/Prototype/Player/BP_PlayerPawn.BP_PlayerPawn)
	FString AssetPath;
	// 맵 에셋 경로(예: /Game/Level/TestMap.TestMap)
	FString MapAssetPath;

	if (!GetCmdValue(CommandLine, TEXT("Mode="), ModeValue))
	{
		UE_LOG(LogTemp, Error, TEXT("Missing -Mode=. Use -Mode=list|asset|map"));
		return 1;
	}

	if (!GetCmdValue(CommandLine, TEXT("Output="), OutputFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Missing -Output=. Example: -Output=C:/Temp/out.json"));
		return 1;
	}

	FString JsonText;

	if (ModeValue.Equals(TEXT("list"), ESearchCase::IgnoreCase))
	{
		GetCmdValue(CommandLine, TEXT("Filter="), FilterPath);
		if (FilterPath.IsEmpty())
		{
			FilterPath = TEXT("/Game");
		}

		if (!BuildAssetListJson(FilterPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("asset"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Asset="), AssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Prototype/Player/BP_PlayerPawn.BP_PlayerPawn"));
			return 1;
		}

		if (!BuildAssetJson(AssetPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("map"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Map="), MapAssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Map=. Example: -Map=/Game/Level/TestMap.TestMap"));
			return 1;
		}

		if (!BuildMapJson(MapAssetPath, JsonText))
		{
			return 2;
		}
	}
	else if (ModeValue.Equals(TEXT("bpgraph"), ESearchCase::IgnoreCase))
	{
		if (!GetCmdValue(CommandLine, TEXT("Asset="), AssetPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing -Asset=. Example: -Asset=/Game/Path/BP_X.BP_X"));
			return 1;
		}


		// GraphNameFilter: 그래프 이름 필터(예: EventGraph). 비어있으면 전체 그래프.
		FString GraphNameFilter;
		GetCmdValue(CommandLine, TEXT("GraphName="), GraphNameFilter);

		// bLinksOnly: true면 nodes/pins 생략하고 links만 덤프
		bool bLinksOnly = false;
		FParse::Bool(*CommandLine, TEXT("LinksOnly="), bLinksOnly);

		// LinkKindText: exec|data|all
		FString LinkKindText;
		GetCmdValue(CommandLine, TEXT("LinkKind="), LinkKindText);

		// LinkKindFilter: 문자열을 enum으로 변환
		EAssetDumpBpLinkKind LinkKindFilter = EAssetDumpBpLinkKind::All;
		if (LinkKindText.Equals(TEXT("exec"), ESearchCase::IgnoreCase))
		{
			LinkKindFilter = EAssetDumpBpLinkKind::Exec;
		}
		else if (LinkKindText.Equals(TEXT("data"), ESearchCase::IgnoreCase))
		{
			LinkKindFilter = EAssetDumpBpLinkKind::Data;
		}
		else
		{
			LinkKindFilter = EAssetDumpBpLinkKind::All;
		}


		// LinksMetaText: none|min
		FString LinksMetaText;
		GetCmdValue(CommandLine, TEXT("LinksMeta="), LinksMetaText);

		// LinksMetaLevel: 문자열을 enum으로 변환
		EAssetDumpBpLinksMetaLevel LinksMetaLevel = EAssetDumpBpLinksMetaLevel::None;
		if (LinksMetaText.Equals(TEXT("min"), ESearchCase::IgnoreCase))
		{
			LinksMetaLevel = EAssetDumpBpLinksMetaLevel::Min;
		}
		else
		{
			LinksMetaLevel = EAssetDumpBpLinksMetaLevel::None;
		}


		if (!BuildBlueprintGraphJson(AssetPath, GraphNameFilter, bLinksOnly, LinkKindFilter, LinksMetaLevel, JsonText))
		{
			return 2;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unknown -Mode=%s"), *ModeValue);
		return 1;
	}

	if (!SaveJsonToFile(OutputFilePath, JsonText))
	{
		return 3;
	}

	UE_LOG(LogTemp, Display, TEXT("Saved JSON: %s"), *OutputFilePath);
	return 0;
}

bool UAssetDumpCommandlet::BuildAssetListJson(const FString& FilterPath, FString& OutJsonText)
{
	// AssetRegistry 모듈
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	// 검색 필터
	FARFilter AssetFilter;
	AssetFilter.bRecursivePaths = true;
	AssetFilter.PackagePaths.Add(*FilterPath);

	TArray<FAssetData> FoundAssets;
	AssetRegistryModule.Get().GetAssets(AssetFilter, FoundAssets);

	// JSON 루트 오브젝트
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	// 에셋 배열
	TArray<TSharedPtr<FJsonValue>> AssetArray;

	for (const FAssetData& AssetData : FoundAssets)
	{
		// 각 에셋 오브젝트
		TSharedRef<FJsonObject> AssetObject = MakeShared<FJsonObject>();

		// 패키지명(/Game/...)
		AssetObject->SetStringField(TEXT("package_name"), AssetData.PackageName.ToString());
		// 오브젝트 경로(/Game/...Asset.Asset)
		AssetObject->SetStringField(TEXT("object_path"), AssetData.GetObjectPathString());
		// 클래스명
		AssetObject->SetStringField(TEXT("class_name"), AssetData.AssetClassPath.GetAssetName().ToString());

		AssetArray.Add(MakeShared<FJsonValueObject>(AssetObject));
	}

	RootObject->SetArrayField(TEXT("assets"), AssetArray);

	// JSON 직렬화
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}

bool UAssetDumpCommandlet::BuildAssetJson(const FString& AssetPath, FString& OutJsonText)
{
	// SoftObjectPath로 로드 시도
	FSoftObjectPath SoftPath(AssetPath);
	// 로드된 UObject
	UObject* LoadedObject = SoftPath.TryLoad();

	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *AssetPath);
		return false;
	}

	// JSON 루트
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("asset_path"), AssetPath);
	RootObject->SetStringField(TEXT("object_name"), LoadedObject->GetName());
	RootObject->SetStringField(TEXT("class_name"), LoadedObject->GetClass()->GetName());

	// AssetRegistry에서 Tag를 읽기(가능한 범위)
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));

	TSharedRef<FJsonObject> TagObject = MakeShared<FJsonObject>();
	for (const auto& TagPair : AssetData.TagsAndValues)
	{
		TagObject->SetStringField(TagPair.Key.ToString(), TagPair.Value.AsString());
	}
	RootObject->SetObjectField(TEXT("tags"), TagObject);

	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}

bool UAssetDumpCommandlet::BuildMapJson(const FString& MapAssetPath, FString& OutJsonText)
{
	// SoftObjectPath로 월드 로드
	FSoftObjectPath SoftPath(MapAssetPath);
	UObject* LoadedObject = SoftPath.TryLoad();

	UWorld* LoadedWorld = Cast<UWorld>(LoadedObject);
	if (!LoadedWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load map(UWorld): %s"), *MapAssetPath);
		return false;
	}

	// JSON 루트
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("map_path"), MapAssetPath);

	// 액터 배열
	TArray<TSharedPtr<FJsonValue>> ActorArray;

	for (AActor* Actor : LoadedWorld->GetCurrentLevel()->Actors)
	{
		if (!Actor)
		{
			continue;
		}

		// 액터 JSON
		TSharedRef<FJsonObject> ActorObject = MakeShared<FJsonObject>();
		ActorObject->SetStringField(TEXT("actor_name"), Actor->GetName());
		ActorObject->SetStringField(TEXT("class_name"), Actor->GetClass()->GetName());

		// 트랜스폼
		const FTransform ActorTransform = Actor->GetActorTransform();
		TSharedRef<FJsonObject> TransformObject = MakeShared<FJsonObject>();
		TransformObject->SetStringField(TEXT("location"), ActorTransform.GetLocation().ToString());
		TransformObject->SetStringField(TEXT("rotation"), ActorTransform.GetRotation().Rotator().ToString());
		TransformObject->SetStringField(TEXT("scale"), ActorTransform.GetScale3D().ToString());
		ActorObject->SetObjectField(TEXT("transform"), TransformObject);

		ActorArray.Add(MakeShared<FJsonValueObject>(ActorObject));
	}

	RootObject->SetArrayField(TEXT("actors"), ActorArray);

	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}

// Function: SaveJsonToFile
// Version: v0.1.2
// Changelog:
// - v0.1.2: JSON 파일을 UTF-8(BOM 없음)로 강제 저장해서 서버 JSON 파싱 안정화

bool UAssetDumpCommandlet::SaveJsonToFile(const FString& OutputFilePath, const FString& JsonText)
{
	// NormalizedPath: 절대 경로로 정규화
	const FString NormalizedPath = FPaths::ConvertRelativePathToFull(OutputFilePath);

	// EncodingOption: UTF-8(BOM 없음)로 강제 (비ASCII 포함돼도 인코딩 흔들리지 않게)
	const FFileHelper::EEncodingOptions EncodingOption = FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM;

	return FFileHelper::SaveStringToFile(JsonText, *NormalizedPath, EncodingOption);
}


bool UAssetDumpCommandlet::GetCmdValue(const FString& CommandLine, const TCHAR* Key, FString& OutValue)
{
	// -Key=Value 형태 파싱
	return FParse::Value(*CommandLine, Key, OutValue);
}

// Function: CollectSubGraphsFromNode
// Version: v0.2.3
// Changelog:
// - v0.2.3: UE 5.7의 UEdGraphNode::GetSubGraphs() 시그니처(인자 없음, TArray 반환)에 맞춰 단순/안전하게 서브그래프 수집

static void CollectSubGraphsFromNode(UEdGraphNode* SourceNode, TArray<UEdGraph*>& OutSubGraphs)
{
	// SourceNode: 서브그래프를 가진 노드(콜랩스 그래프 등)
	if (!SourceNode)
	{
		return;
	}

	// SubGraphsFromNode: UE 5.7 기준 GetSubGraphs()는 인자 없이 TArray를 반환
	const TArray<UEdGraph*> SubGraphsFromNode = SourceNode->GetSubGraphs();

	// OutSubGraphs: 결과 배열에 추가
	OutSubGraphs.Append(SubGraphsFromNode);
}


// Function: BuildBlueprintGraphJson
// Version: v0.2.6
// Changelog:
// - v0.2.6: GraphName/LinksOnly/LinkKind 옵션 지원 추가
// - v0.2.5: 링크 중복 제거 유지
// - v0.2.2: UEdGraphNode::GetSubGraphs 시그니처가 UE 버전마다 다른 문제를 SFINAE로 흡수
bool UAssetDumpCommandlet::BuildBlueprintGraphJson(
	const FString& BlueprintAssetPath,
	const FString& GraphNameFilter,
	bool bLinksOnly,
	EAssetDumpBpLinkKind LinkKindFilter,
	EAssetDumpBpLinksMetaLevel LinksMetaLevel,
	FString& OutJsonText
) {
	// SoftPath: 블루프린트 에셋 경로
	FSoftObjectPath SoftPath(BlueprintAssetPath);

	// LoadedObject: 로드된 UObject
	UObject* LoadedObject = SoftPath.TryLoad();
	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *BlueprintAssetPath);
		return false;
	}

	// LoadedBlueprint: UBlueprint 캐스팅
	UBlueprint* LoadedBlueprint = Cast<UBlueprint>(LoadedObject);
	if (!LoadedBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("Asset is not a UBlueprint: %s (class=%s)"),
			*BlueprintAssetPath,
			*LoadedObject->GetClass()->GetName());
		return false;
	}

	// AllGraphs: 블루프린트가 들고 있는 그래프들을 직접 수집한 목록
	TArray<UEdGraph*> AllGraphs;

	// UniqueGraphs: 중복 제거용 Set
	TSet<UEdGraph*> UniqueGraphs;

	// [1] UBlueprint 기본 그래프들 수집
	for (UEdGraph* Graph : LoadedBlueprint->UbergraphPages)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	for (UEdGraph* Graph : LoadedBlueprint->FunctionGraphs)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	for (UEdGraph* Graph : LoadedBlueprint->MacroGraphs)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	for (UEdGraph* Graph : LoadedBlueprint->DelegateSignatureGraphs)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	// IntermediateGeneratedGraphs는 컴파일 중간 그래프라 없을 수도 있음(있으면 포함)
	for (UEdGraph* Graph : LoadedBlueprint->IntermediateGeneratedGraphs)
	{
		if (Graph && !UniqueGraphs.Contains(Graph))
		{
			UniqueGraphs.Add(Graph);
			AllGraphs.Add(Graph);
		}
	}

	// [2] 노드가 들고 있는 SubGraph(콜랩스 그래프 등)도 추가 수집(버전/구성 따라 중요)
	bool bAddedNewGraph = true;
	while (bAddedNewGraph)
	{
		bAddedNewGraph = false;

		for (UEdGraph* ParentGraph : AllGraphs)
		{
			if (!ParentGraph)
			{
				continue;
			}

			for (UEdGraphNode* Node : ParentGraph->Nodes)
			{
				if (!Node)
				{
					continue;
				}

				// SubGraphs: 노드가 가진 하위 그래프들
				TArray<UEdGraph*> SubGraphs;

				// CollectSubGraphsFromNode: 엔진 버전에 맞는 GetSubGraphs 호출로 수집
				CollectSubGraphsFromNode(Node, SubGraphs);

				for (UEdGraph* SubGraph : SubGraphs)
				{
					if (SubGraph && !UniqueGraphs.Contains(SubGraph))
					{
						UniqueGraphs.Add(SubGraph);
						AllGraphs.Add(SubGraph);
						bAddedNewGraph = true;
					}
				}
			}
		}
	}


	// RootObject: JSON 루트
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("asset_path"), BlueprintAssetPath);
	RootObject->SetStringField(TEXT("blueprint_name"), LoadedBlueprint->GetName());

	// GraphArray: graphs 배열
	TArray<TSharedPtr<FJsonValue>> GraphArray;

	// bHasGraphFilter: 그래프 필터 사용 여부
	const bool bHasGraphFilter = !GraphNameFilter.IsEmpty();

	// GraphNameNormalized: 비교용(트림)
	const FString GraphNameNormalized = GraphNameFilter.TrimStartAndEnd();

	for (UEdGraph* Graph : AllGraphs)
	{
		if (!Graph)
		{
			continue;
		}

		// GraphName: 현재 그래프 이름
		const FString GraphName = Graph->GetName();

		// GraphNameFilter 적용(대소문자 무시)
		if (bHasGraphFilter && !GraphName.Equals(GraphNameNormalized, ESearchCase::IgnoreCase))
		{
			continue;
		}

		// GraphObject: 단일 그래프 JSON
		TSharedRef<FJsonObject> GraphObject = MakeShared<FJsonObject>();
		GraphObject->SetStringField(TEXT("graph_name"), Graph->GetName());
		GraphObject->SetStringField(TEXT("graph_class"), Graph->GetClass()->GetName());
		GraphObject->SetBoolField(TEXT("links_only"), bLinksOnly);
		GraphObject->SetStringField(TEXT("link_kind"), (LinkKindFilter == EAssetDumpBpLinkKind::Exec) ? TEXT("exec") :
			(LinkKindFilter == EAssetDumpBpLinkKind::Data) ? TEXT("data") : TEXT("all"));
		// LinksMetaTextOut: JSON에 기록할 links_meta 값
		const FString LinksMetaTextOut = (LinksMetaLevel == EAssetDumpBpLinksMetaLevel::Min) ? TEXT("min") : TEXT("none");
		GraphObject->SetStringField(TEXT("links_meta"), LinksMetaTextOut);


		// NodeArray: nodes 배열
		TArray<TSharedPtr<FJsonValue>> NodeArray;
		// LinkArray: links 배열 (핀 연결 관계)
		TArray<TSharedPtr<FJsonValue>> LinkArray;
		// UniqueLinkKeySet: 링크 중복 제거용 키 Set(그래프 단위)
		TSet<FString> UniqueLinkKeySet;
		// bWriteNodesAndPins: true면 nodes/pins를 JSON에 포함
		const bool bWriteNodesAndPins = !bLinksOnly;


		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (!Node)
			{
				continue;
			}

			// NodeGuidText: 노드 GUID 문자열
			const FString NodeGuidText = Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens);

			// NodeObject: 노드 JSON
			TSharedRef<FJsonObject> NodeObject = MakeShared<FJsonObject>();
			NodeObject->SetStringField(TEXT("node_guid"), NodeGuidText);
			NodeObject->SetStringField(TEXT("node_class"), Node->GetClass()->GetName());
			NodeObject->SetStringField(TEXT("node_title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
			NodeObject->SetNumberField(TEXT("pos_x"), Node->NodePosX);
			NodeObject->SetNumberField(TEXT("pos_y"), Node->NodePosY);

			// PinArray: pins 배열
			TArray<TSharedPtr<FJsonValue>> PinArray;

			// [1] links는 links_only 여부와 무관하게 수집해야 함
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (!Pin)
				{
					continue;
				}

				// PinObject: 핀 JSON
				TSharedRef<FJsonObject> PinObject = MakeShared<FJsonObject>();
				PinObject->SetStringField(TEXT("pin_id"), Pin->PinId.ToString(EGuidFormats::DigitsWithHyphens));
				PinObject->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
				PinObject->SetStringField(TEXT("direction"), (Pin->Direction == EGPD_Input) ? TEXT("Input") : TEXT("Output"));

				// PinTypeObject: 핀 타입 JSON (카테고리/서브카테고리/오브젝트 타입)
				TSharedRef<FJsonObject> PinTypeObject = MakeShared<FJsonObject>();
				PinTypeObject->SetStringField(TEXT("category"), Pin->PinType.PinCategory.ToString());
				PinTypeObject->SetStringField(TEXT("sub_category"), Pin->PinType.PinSubCategory.ToString());
				PinTypeObject->SetStringField(TEXT("sub_category_object"),
					Pin->PinType.PinSubCategoryObject.IsValid() ? Pin->PinType.PinSubCategoryObject->GetName() : TEXT(""));

				PinTypeObject->SetBoolField(TEXT("is_array"), Pin->PinType.IsArray());
				PinTypeObject->SetBoolField(TEXT("is_ref"), Pin->PinType.bIsReference);

				PinObject->SetObjectField(TEXT("pin_type"), PinTypeObject);

				// DefaultValue: 기본값(문자열)
				PinObject->SetStringField(TEXT("default_value"), Pin->DefaultValue);

				PinArray.Add(MakeShared<FJsonValueObject>(PinObject));

				// AppendPinLinks 호출(링크 필터 적용됨)
				AppendPinLinks(Pin, NodeGuidText, LinkArray, UniqueLinkKeySet, LinkKindFilter, LinksMetaLevel);
			}

			// [2] nodes/pins JSON은 옵션일 때만 생성
			if (!bWriteNodesAndPins)
			{
				continue;
			}

			NodeObject->SetArrayField(TEXT("pins"), PinArray);
			NodeArray.Add(MakeShared<FJsonValueObject>(NodeObject));
		}

		GraphObject->SetArrayField(TEXT("nodes"), NodeArray);
		GraphObject->SetArrayField(TEXT("links"), LinkArray);

		GraphArray.Add(MakeShared<FJsonValueObject>(GraphObject));
	}

	RootObject->SetArrayField(TEXT("graphs"), GraphArray);

	// JsonWriter: JSON 직렬화
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	return FJsonSerializer::Serialize(RootObject, JsonWriter);
}


// Function: AppendPinLinks
// Version: v0.2.7
// Changelog:
// - v0.2.7: LinksMeta=min일 때 links에 최소 메타(노드 타이틀/핀 이름/카테고리) 인라인 포함
// - v0.2.6: LinkKindFilter(exec/data/all) 지원 추가
// - v0.2.5: 링크 중복 제거
//		1) Output 핀에서만 링크를 기록해 방향을 고정(Output -> Input)
//		2) (FromNodeGuid|FromPinId|ToNodeGuid|ToPinId) 키로 TSet 중복 제거
// - v0.2.0: Pin->LinkedTo 기반 링크(From/To) JSON 생성
void UAssetDumpCommandlet::AppendPinLinks(
	UEdGraphPin* FromPin,
	const FString& FromNodeGuid,
	TArray<TSharedPtr<FJsonValue>>& InOutLinks,
	TSet<FString>& InOutUniqueLinkKeys,
	EAssetDumpBpLinkKind LinkKindFilter,
	EAssetDumpBpLinksMetaLevel LinksMetaLevel
)
{
	// FromPin: 링크를 시작하는 핀(기준 핀)
	if (!FromPin)
	{
		return;
	}

	// Output 핀에서만 기록(중복 방향 생성 방지)
	if (FromPin->Direction != EGPD_Output)
	{
		return;
	}

	// bIsExecLink: exec 링크인지 판별(FromPin 기준)
	const bool bIsExecLink = (FromPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec);

	// LinkKindFilter 적용
	if (LinkKindFilter == EAssetDumpBpLinkKind::Exec && !bIsExecLink)
	{
		return;
	}
	if (LinkKindFilter == EAssetDumpBpLinkKind::Data && bIsExecLink)
	{
		return;
	}

	// FromPinIdText: 시작 핀 ID 문자열
	const FString FromPinIdText = FromPin->PinId.ToString(EGuidFormats::DigitsWithHyphens);

	// FromNode: 시작 핀의 소유 노드
	UEdGraphNode* FromNode = FromPin->GetOwningNode();

	for (UEdGraphPin* ToPin : FromPin->LinkedTo)
	{
		// ToPin: 연결된 상대 핀
		if (!ToPin || !ToPin->GetOwningNode())
		{
			continue;
		}

		// Input 핀으로만 연결을 기록(방향 일관성)
		if (ToPin->Direction != EGPD_Input)
		{
			continue;
		}

		// ToNode: 상대 핀의 소유 노드
		UEdGraphNode* ToNode = ToPin->GetOwningNode();

		// ToNodeGuidText: 상대 노드 GUID 문자열
		const FString ToNodeGuidText = ToNode->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens);

		// ToPinIdText: 상대 핀 ID 문자열
		const FString ToPinIdText = ToPin->PinId.ToString(EGuidFormats::DigitsWithHyphens);

		// LinkKey: 링크 중복 제거용 키
		const FString LinkKey = FString::Printf(
			TEXT("%s|%s|%s|%s"),
			*FromNodeGuid,
			*FromPinIdText,
			*ToNodeGuidText,
			*ToPinIdText
		);

		// 이미 추가된 링크면 스킵
		if (InOutUniqueLinkKeys.Contains(LinkKey))
		{
			continue;
		}

		// UniqueLinkKeySet에 등록
		InOutUniqueLinkKeys.Add(LinkKey);

		// LinkObject: JSON 링크 오브젝트
		TSharedRef<FJsonObject> LinkObject = MakeShared<FJsonObject>();
		LinkObject->SetStringField(TEXT("from_node_guid"), FromNodeGuid);
		LinkObject->SetStringField(TEXT("from_pin_id"), FromPinIdText);
		LinkObject->SetStringField(TEXT("to_node_guid"), ToNodeGuidText);
		LinkObject->SetStringField(TEXT("to_pin_id"), ToPinIdText);

		// LinksMetaLevel == Min일 때: 사람이 읽기 쉬운 최소 메타를 인라인 포함
		if (LinksMetaLevel == EAssetDumpBpLinksMetaLevel::Min)
		{
			// FromNodeTitle: 시작 노드 타이틀
			const FString FromNodeTitle = FromNode
				? FromNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString()
				: TEXT("");

			// ToNodeTitle: 상대 노드 타이틀
			const FString ToNodeTitle = ToNode
				? ToNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString()
				: TEXT("");

			// FromPinName: 시작 핀 이름
			const FString FromPinName = FromPin->PinName.ToString();

			// ToPinName: 상대 핀 이름
			const FString ToPinName = ToPin->PinName.ToString();

			// FromPinCategory: 시작 핀 카테고리(exec 등)
			const FString FromPinCategory = FromPin->PinType.PinCategory.ToString();

			// ToPinCategory: 상대 핀 카테고리
			const FString ToPinCategory = ToPin->PinType.PinCategory.ToString();

			LinkObject->SetStringField(TEXT("from_node_title"), FromNodeTitle);
			LinkObject->SetStringField(TEXT("from_pin_name"), FromPinName);
			LinkObject->SetStringField(TEXT("from_pin_category"), FromPinCategory);

			LinkObject->SetStringField(TEXT("to_node_title"), ToNodeTitle);
			LinkObject->SetStringField(TEXT("to_pin_name"), ToPinName);
			LinkObject->SetStringField(TEXT("to_pin_category"), ToPinCategory);
		}

		InOutLinks.Add(MakeShared<FJsonValueObject>(LinkObject));
	}
}



