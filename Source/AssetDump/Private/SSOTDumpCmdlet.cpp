// SSOTDumpCmdlet.cpp
// v0.3b patch (path non-fixed)
// - DetectInputRoot() with longest common prefix folder
// - -profile parser (JSON) + CLI overrides
// - ssot_detect.json output
// v0.3 - Wheel SSOT 덤프 구현
// v0.2 - cfg 화이트리스트 + GameplayTags + EnhancedInput 덤프 구현

#include "SSOTDumpCmdlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "GameplayTagsManager.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

// ---------- v0.3b: 파일 스코프 옵션(헤더 수정 없이 CPP에서만 끝내기) ----------

// 프로필 경로(-profile=)
static FString GSSOT_ProfilePath;

// 강제 input root(-input_root=)
static FString GSSOT_InputRootOverride;

// 자동탐지 사용 여부(-autodetect / -noautodetect)
static bool GSSOT_bAutoDetect = true;

// detect 결과 파일명(-detect_out=)
static FString GSSOT_DetectOutFileName = TEXT("ssot_detect.json");

// ---------- v0.3b: 프로필 구조 ----------

struct FSSOTProfileRoots
{
	// 프로필에 정의된 입력 루트(비어있으면 자동탐지/CLI로)
	FString InputRoot;
};

struct FSSOTProfile
{
	// 프로필 루트 묶음
	FSSOTProfileRoots Roots;
};

// ---------- v0.3b: 결정적 문자열 유틸 ----------

static FString SSOT_NormalizeGamePath(const FString& InPath)
{
	// /Game 경로용 정규화(슬래시/트레일링 슬래시 제거)
	FString Path = InPath;
	Path.TrimStartAndEndInline();
	Path.ReplaceInline(TEXT("\\"), TEXT("/"));

	while (Path.EndsWith(TEXT("/")))
	{
		Path.LeftChopInline(1);
	}
	return Path;
}

static void SSOT_SplitPathSegments(const FString& InPath, TArray<FString>& OutSegments)
{
	// "/Game/A/B" -> ["Game","A","B"]
	OutSegments.Reset();

	FString Clean = SSOT_NormalizeGamePath(InPath);

	TArray<FString> Raw;
	Clean.ParseIntoArray(Raw, TEXT("/"), /*CullEmpty=*/true);

	for (const FString& Seg : Raw)
	{
		if (!Seg.IsEmpty())
		{
			OutSegments.Add(Seg);
		}
	}
}

static FString SSOT_JoinPathSegments(const TArray<FString>& Segments, int32 Count)
{
	// ["Game","A","B"], Count=2 -> "/Game/A"
	if (Segments.Num() == 0 || Count <= 0)
	{
		return FString();
	}

	const int32 SafeCount = FMath::Min(Count, Segments.Num());

	FString Out = TEXT("/");
	for (int32 i = 0; i < SafeCount; ++i)
	{
		Out += Segments[i];
		if (i != SafeCount - 1)
		{
			Out += TEXT("/");
		}
	}
	return Out;
}

// ---------- v0.3b: “최장 공통 prefix 폴더” 구현(요청한 완전 코드) ----------

static bool SSOT_LongestCommonPrefixFolder(
	const TArray<FString>& FolderPaths,
	FString& OutCommonFolder)
{
	// 입력: 폴더 경로들 (예: /Game/Proj/Input)
	// 출력: 가장 긴 공통 prefix 폴더 (예: /Game/Proj)

	OutCommonFolder.Reset();

	if (FolderPaths.Num() == 0)
	{
		return false;
	}

	// 모든 경로를 세그먼트로 변환
	TArray<TArray<FString>> AllSegments;
	AllSegments.Reserve(FolderPaths.Num());

	int32 MinLen = MAX_int32;

	for (const FString& Path : FolderPaths)
	{
		TArray<FString> Segs;
		SSOT_SplitPathSegments(Path, Segs);

		if (Segs.Num() == 0)
		{
			return false;
		}

		AllSegments.Add(MoveTemp(Segs));
		MinLen = FMath::Min(MinLen, AllSegments.Last().Num());
	}

	// 공통 prefix 길이 계산
	int32 CommonLen = 0;

	for (int32 i = 0; i < MinLen; ++i)
	{
		const FString& Candidate = AllSegments[0][i];

		bool bAllMatch = true;
		for (int32 p = 1; p < AllSegments.Num(); ++p)
		{
			if (!AllSegments[p][i].Equals(Candidate))
			{
				bAllMatch = false;
				break;
			}
		}

		if (!bAllMatch)
		{
			break;
		}

		++CommonLen;
	}

	if (CommonLen <= 0)
	{
		return false;
	}

	// prefix를 다시 경로로 조립
	OutCommonFolder = SSOT_JoinPathSegments(AllSegments[0], CommonLen);
	OutCommonFolder = SSOT_NormalizeGamePath(OutCommonFolder);

	return !OutCommonFolder.IsEmpty();
}

// ---------- v0.3b: profile 로드 ----------

static FString SSOT_ResolveProfilePath(const FString& RawProfilePath)
{
	// 프로필 경로 해석(상대/절대 모두 지원)
	if (RawProfilePath.IsEmpty())
	{
		return FString();
	}

	// 1) 입력 그대로 시도
	FString FullPath = RawProfilePath;
	if (FPaths::FileExists(FullPath))
	{
		return FPaths::ConvertRelativePathToFull(FullPath);
	}

	// 2) 프로젝트 디렉터리 기준 상대경로
	FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), RawProfilePath);
	if (FPaths::FileExists(FullPath))
	{
		return FullPath;
	}

	// 3) 현재 작업 디렉터리 기준
	FullPath = FPaths::ConvertRelativePathToFull(RawProfilePath);
	if (FPaths::FileExists(FullPath))
	{
		return FullPath;
	}

	return FString();
}

static bool SSOT_LoadProfileJson(const FString& RawProfilePath, FSSOTProfile& OutProfile, FString& OutError)
{
	// 프로필 JSON 로드
	OutError.Reset();
	OutProfile = FSSOTProfile();

	const FString ProfilePath = SSOT_ResolveProfilePath(RawProfilePath);
	if (ProfilePath.IsEmpty())
	{
		OutError = TEXT("Profile path not found.");
		return false;
	}

	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *ProfilePath))
	{
		OutError = TEXT("Failed to read profile json.");
		return false;
	}

	TSharedPtr<FJsonObject> RootObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);

	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		OutError = TEXT("Failed to parse profile json.");
		return false;
	}

	// roots 섹션
	const TSharedPtr<FJsonObject>* RootsObjPtr = nullptr;
	if (RootObj->TryGetObjectField(TEXT("roots"), RootsObjPtr) && RootsObjPtr && RootsObjPtr->IsValid())
	{
		// input_root
		FString InputRoot;
		(*RootsObjPtr)->TryGetStringField(TEXT("input_root"), InputRoot);
		OutProfile.Roots.InputRoot = SSOT_NormalizeGamePath(InputRoot);
	}

	return true;
}

// ---------- v0.3b: detect 결과 저장 ----------

static bool SSOT_SaveDetectJson(
	const FString& OutputDir,
	const FString& InputRoot,
	const FString& InputRootSource,
	const TArray<FString>& ImcFolderPaths,
	const FString& Note)
{
	// detect 파일 저장 (OutputDir 아래 ssot_detect.json)
	IFileManager::Get().MakeDirectory(*OutputDir, /*Tree=*/true);

	// JSON 구성
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("schema_version"), TEXT("0.3b"));

	TSharedPtr<FJsonObject> Detected = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> InputObj = MakeShared<FJsonObject>();
	InputObj->SetStringField(TEXT("value"), InputRoot);
	InputObj->SetStringField(TEXT("source"), InputRootSource);
	InputObj->SetNumberField(TEXT("imc_folder_count"), ImcFolderPaths.Num());

	// 샘플(너무 커지는 거 방지): 최대 50개만 저장
	TArray<TSharedPtr<FJsonValue>> FolderArray;
	const int32 MaxSamples = 50;

	for (int32 i = 0; i < ImcFolderPaths.Num() && i < MaxSamples; ++i)
	{
		FolderArray.Add(MakeShared<FJsonValueString>(ImcFolderPaths[i]));
	}

	InputObj->SetArrayField(TEXT("imc_folders_sample"), FolderArray);

	Detected->SetObjectField(TEXT("input_root"), InputObj);
	Root->SetObjectField(TEXT("detected"), Detected);

	if (!Note.IsEmpty())
	{
		Root->SetStringField(TEXT("note"), Note);
	}

	// 문자열화
	FString OutJsonText;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonText);

	if (!FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
	{
		return false;
	}

	const FString FilePath = FPaths::Combine(OutputDir, GSSOT_DetectOutFileName);
	return FFileHelper::SaveStringToFile(OutJsonText, *FilePath);
}

// ---------- v0.3b: DetectInputRoot (IMC 기반) ----------

static bool SSOT_DetectInputRootByIMC(
	IAssetRegistry& AssetRegistry,
	FString& OutInputRoot,
	TArray<FString>& OutImcFolderPaths)
{
	// EnhancedInput IMC들을 찾아 폴더들로부터 LCP 폴더를 구한다.
	OutInputRoot.Reset();
	OutImcFolderPaths.Reset();

	// /Game 스캔 보장
	{
		TArray<FString> ScanPaths;
		ScanPaths.Add(TEXT("/Game"));
		AssetRegistry.ScanPathsSynchronous(ScanPaths, /*bForceRescan=*/true);
	}

	// IMC 수집: 클래스 경로 문자열만 사용(EnhancedInput 헤더 없이도 컴파일 가능)
	FARFilter Filter;

	// "/Script/EnhancedInput.InputMappingContext"
	Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/EnhancedInput"), TEXT("InputMappingContext")));
	Filter.bRecursiveClasses = true;

	// 경로는 아직 모름 → 전체 /Game에서 찾기
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;

	TArray<FAssetData> ImcAssets;
	AssetRegistry.GetAssets(Filter, ImcAssets);

	if (ImcAssets.Num() == 0)
	{
		return false;
	}

	// IMC들의 PackagePath(폴더) 수집 + 정렬(결정성)
	for (const FAssetData& A : ImcAssets)
	{
		const FString FolderPath = SSOT_NormalizeGamePath(A.PackagePath.ToString());
		if (!FolderPath.IsEmpty())
		{
			OutImcFolderPaths.AddUnique(FolderPath);
		}
	}
	OutImcFolderPaths.Sort();

	// “최장 공통 prefix 폴더” 계산
	FString CommonFolder;
	if (!SSOT_LongestCommonPrefixFolder(OutImcFolderPaths, CommonFolder))
	{
		return false;
	}

	OutInputRoot = CommonFolder;
	return true;
}

// ---------- v0.3b: 최종 input_root 해석(우선순위: CLI > profile > autodetect > fallback) ----------

static FString SSOT_ResolveInputRoot(
	IAssetRegistry& AssetRegistry,
	const FString& OutputDir,
	FString& OutSourceTag)
{
	OutSourceTag = TEXT("unknown");

	// 1) CLI override
	if (!GSSOT_InputRootOverride.IsEmpty())
	{
		OutSourceTag = TEXT("cli");
		return SSOT_NormalizeGamePath(GSSOT_InputRootOverride);
	}

	// 2) profile
	if (!GSSOT_ProfilePath.IsEmpty())
	{
		FSSOTProfile Profile;
		FString ProfileErr;

		if (SSOT_LoadProfileJson(GSSOT_ProfilePath, Profile, ProfileErr))
		{
			if (!Profile.Roots.InputRoot.IsEmpty())
			{
				OutSourceTag = TEXT("profile");
				return Profile.Roots.InputRoot;
			}
		}
		// profile이 깨져도 덤프 자체는 계속 진행(단, detect에 note 남김)
	}

	// 3) autodetect
	if (GSSOT_bAutoDetect)
	{
		TArray<FString> ImcFolders;
		FString DetectedRoot;

		if (SSOT_DetectInputRootByIMC(AssetRegistry, DetectedRoot, ImcFolders))
		{
			OutSourceTag = TEXT("auto_imc_lcp");
			SSOT_SaveDetectJson(OutputDir, DetectedRoot, OutSourceTag, ImcFolders, TEXT("autodetect succeeded"));
			return DetectedRoot;
		}
		else
		{
			// autodetect 실패 기록
			SSOT_SaveDetectJson(OutputDir, TEXT(""), TEXT("auto_failed"), ImcFolders, TEXT("autodetect failed (no IMC or no common folder)"));
		}
	}

	// 4) fallback (최후): /Game
	OutSourceTag = TEXT("fallback_game");
	return TEXT("/Game");
}



static FString NormalizePathSeparators(const FString& InPath)
{
	// 경로 구분자 통일
	FString OutPath = InPath;
	OutPath.ReplaceInline(TEXT("\\"), TEXT("/"));
	return OutPath;
}

void USSOTDumpCmdlet::ParseArgs(const FString& CmdLine)
{


	// 기본 출력 폴더
	OutputDir = TEXT("Document/SSOT");

	// 기본 덤프 타겟
	DumpTarget = TEXT("all");

	// 기본 시간 기록
	bWriteTimeToMeta = true;

	// -out=, -target=, -notime 지원
	FParse::Value(*CmdLine, TEXT("-out="), OutputDir);
	FParse::Value(*CmdLine, TEXT("-target="), DumpTarget);
	
	// v0.3b: profile / input_root / autodetect / detect_out 파싱
	
	// 프로필 경로(-profile=)
	FParse::Value(*CmdLine, TEXT("-profile="), GSSOT_ProfilePath);

	// input_root 강제(-input_root=)
	FParse::Value(*CmdLine, TEXT("-input_root="), GSSOT_InputRootOverride);
	GSSOT_InputRootOverride = SSOT_NormalizeGamePath(GSSOT_InputRootOverride);

	// autodetect on/off
	if (FParse::Param(*CmdLine, TEXT("noautodetect")))
	{
		GSSOT_bAutoDetect = false;
	}
	if (FParse::Param(*CmdLine, TEXT("autodetect")))
	{
		GSSOT_bAutoDetect = true;
	}

	// detect 파일명 지정(-detect_out=)  ※파일명만 받는 걸 권장(경로는 OutputDir)
	FParse::Value(*CmdLine, TEXT("-detect_out="), GSSOT_DetectOutFileName);
	if (GSSOT_DetectOutFileName.IsEmpty())
	{
		GSSOT_DetectOutFileName = TEXT("ssot_detect.json");
	}

	if (FParse::Param(*CmdLine, TEXT("notime")))
	{
		bWriteTimeToMeta = false;
	}

	OutputDir = NormalizePathSeparators(OutputDir);
}

FString USSOTDumpCmdlet::MakePath(const FString& FileName) const
{
	// 상대 → 절대 경로
	const FString AbsDir = FPaths::ConvertRelativePathToFull(OutputDir);
	return FPaths::Combine(AbsDir, FileName);
}

int32 USSOTDumpCmdlet::Main(const FString& CmdLine)
{
	// 인자 파싱
	ParseArgs(CmdLine);

	// 출력 폴더 생성
	IFileManager::Get().MakeDirectory(*OutputDir, true);

	// 타겟 선택 헬퍼
	const auto Want = [&](const TCHAR* Name)
		{
			return DumpTarget.Equals(TEXT("all")) || DumpTarget.Equals(Name);
		};

	bool bOk = true;

	// meta는 항상 권장
	bOk &= DumpMeta();

	if (Want(TEXT("assets"))) bOk &= DumpAssets();
	if (Want(TEXT("cfg")))    bOk &= DumpConfig();
	if (Want(TEXT("tags")))   bOk &= DumpTags();
	if (Want(TEXT("input")))  bOk &= DumpInput();
	if (Want(TEXT("wheel")))  bOk &= DumpWheel();

	return bOk ? 0 : 1;
}

bool USSOTDumpCmdlet::DumpMeta() const
{
	// 스키마 버전
	const FString SchemaVersion = TEXT("0.2");

	// 프로젝트명
	const FString ProjectName = FApp::GetProjectName();

	// UE 버전
	const FString UEVersion = FEngineVersion::Current().ToString();

	// JSON 루트
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("schema_version"), SchemaVersion);
	Root->SetStringField(TEXT("project"), ProjectName);
	Root->SetStringField(TEXT("ue_version"), UEVersion);

	if (bWriteTimeToMeta)
	{
		// 결정성 필요 시 -notime 사용
		Root->SetStringField(TEXT("dump_time_utc"), FDateTime::UtcNow().ToIso8601());
	}

	FString OutJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	if (!FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(OutJson, *MakePath(TEXT("ssot_meta.json")));
}

bool USSOTDumpCmdlet::DumpAssets() const
{
	// AssetRegistry 모듈 로드
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// /Game 스캔 보장
	TArray<FString> ScanPaths;
	ScanPaths.Add(TEXT("/Game"));
	AssetRegistry.ScanPathsSynchronous(ScanPaths, true);

	// 모든 에셋 수집
	TArray<FAssetData> AllAssets;
	AssetRegistry.GetAllAssets(AllAssets, true);

	// 결정성: ObjectPath 정렬
	AllAssets.Sort([](const FAssetData& A, const FAssetData& B)
		{
			return A.GetObjectPathString() < B.GetObjectPathString();
		});

	// JSON 배열 생성
	TArray<TSharedPtr<FJsonValue>> AssetArray;
	AssetArray.Reserve(AllAssets.Num());

	for (const FAssetData& AssetData : AllAssets)
	{
		TSharedPtr<FJsonObject> Item = MakeShared<FJsonObject>();
		Item->SetStringField(TEXT("object_path"), AssetData.GetObjectPathString());
		Item->SetStringField(TEXT("class"), AssetData.AssetClassPath.ToString());
		Item->SetStringField(TEXT("package_path"), AssetData.PackagePath.ToString());

		AssetArray.Add(MakeShared<FJsonValueObject>(Item));
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("assets"), AssetArray);

	FString OutJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	if (!FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(OutJson, *MakePath(TEXT("ssot_assets.json")));
}

// -------------------------
// cfg 화이트리스트 덤프 (v0.2)
// -------------------------

struct FCfgRule
{
	// ini 파일명 (DefaultEngine.ini 등)
	FString IniName;

	// 섹션명
	FString SectionName;

	// 키명 (또는 *)
	FString KeyName;
};

static bool ParseCfgRules(
	const FString& RuleText,
	TArray<FCfgRule>& OutRules)
{
	// 라인 분리
	TArray<FString> Lines;
	RuleText.ParseIntoArrayLines(Lines);

	for (FString Line : Lines)
	{
		Line.TrimStartAndEndInline();

		// 주석/빈줄 스킵
		if (Line.IsEmpty() || Line.StartsWith(TEXT("#")))
		{
			continue;
		}

		TArray<FString> Parts;
		Line.ParseIntoArray(Parts, TEXT("|"), true);

		if (Parts.Num() != 3)
		{
			// 규칙 포맷 불량
			return false;
		}

		FCfgRule Rule;
		Rule.IniName = Parts[0].TrimStartAndEnd();
		Rule.SectionName = Parts[1].TrimStartAndEnd();
		Rule.KeyName = Parts[2].TrimStartAndEnd();

		OutRules.Add(Rule);
	}

	// 결정성: 룰 정렬
	OutRules.Sort([](const FCfgRule& A, const FCfgRule& B)
		{
			if (A.IniName != B.IniName) return A.IniName < B.IniName;
			if (A.SectionName != B.SectionName) return A.SectionName < B.SectionName;
			return A.KeyName < B.KeyName;
		});

	return true;
}

static FString ResolveIniPathByName(const FString& IniName)
{
	// 최소 구현: DefaultEngine.ini / DefaultGame.ini / DefaultInput.ini / DefaultEditor.ini
	// UE의 “Config Cache”는 보통 아래로 resolve 가능
	// - DefaultEngine.ini  -> GEngineIni
	// - DefaultGame.ini    -> GGameIni
	// - DefaultInput.ini   -> GInputIni
	// - DefaultEditor.ini  -> GEditorIni

	if (IniName.Equals(TEXT("DefaultEngine.ini"))) return GEngineIni;
	if (IniName.Equals(TEXT("DefaultGame.ini")))   return GGameIni;
	if (IniName.Equals(TEXT("DefaultInput.ini")))  return GInputIni;
	if (IniName.Equals(TEXT("DefaultEditor.ini"))) return GEditorIni;

	// 기타는 프로젝트 필요 시 확장
	return FString();
}

bool USSOTDumpCmdlet::DumpConfig() const
{
	// 화이트리스트 파일 읽기
	const FString WhitelistPath = MakePath(CfgWhitelistFileName);

	FString RuleText;
	if (!FFileHelper::LoadFileToString(RuleText, *WhitelistPath))
	{
		// 화이트리스트가 없으면 실패 처리 (SSOT는 “무엇을 덤프하는지”가 중요)
		return false;
	}

	// 룰 파싱
	TArray<FCfgRule> Rules;
	if (!ParseCfgRules(RuleText, Rules))
	{
		return false;
	}

	// 출력 구조: (ini, section)별 pairs 묶기
	// 결정성: ini/section/pairs 모두 정렬
	struct FSectionDump
	{
		FString IniName;
		FString SectionName;
		TMap<FString, FString> PairMap;
	};

	TArray<FSectionDump> SectionDumps;

	auto FindOrAddSectionDump = [&](const FString& IniName, const FString& SectionName) -> FSectionDump&
		{
			for (FSectionDump& Dump : SectionDumps)
			{
				if (Dump.IniName == IniName && Dump.SectionName == SectionName)
				{
					return Dump;
				}
			}

			FSectionDump NewDump;
			NewDump.IniName = IniName;
			NewDump.SectionName = SectionName;
			SectionDumps.Add(NewDump);
			return SectionDumps.Last();
		};

	for (const FCfgRule& Rule : Rules)
	{
		// ini resolve
		const FString IniPath = ResolveIniPathByName(Rule.IniName);
		if (IniPath.IsEmpty())
		{
			return false;
		}

		// 섹션 전체 덤프
		if (Rule.KeyName.Equals(TEXT("*")))
		{
			TArray<FString> AllPairs;
			if (GConfig->GetSection(*Rule.SectionName, AllPairs, IniPath))
			{
				FSectionDump& Dump = FindOrAddSectionDump(Rule.IniName, Rule.SectionName);

				for (const FString& PairLine : AllPairs)
				{
					FString Key, Value;
					if (PairLine.Split(TEXT("="), &Key, &Value))
					{
						Key.TrimStartAndEndInline();
						Value.TrimStartAndEndInline();
						Dump.PairMap.Add(Key, Value);
					}
				}
			}
			continue;
		}

		// 키 단일 덤프
		FString Value;
		if (GConfig->GetString(*Rule.SectionName, *Rule.KeyName, Value, IniPath))
		{
			FSectionDump& Dump = FindOrAddSectionDump(Rule.IniName, Rule.SectionName);
			Dump.PairMap.Add(Rule.KeyName, Value);
		}
		else
		{
			// 룰에 있는데 값이 없으면 FAIL: SSOT 기준이 깨진 것
			return false;
		}
	}

	// 결정성: 섹션 정렬
	SectionDumps.Sort([](const FSectionDump& A, const FSectionDump& B)
		{
			if (A.IniName != B.IniName) return A.IniName < B.IniName;
			return A.SectionName < B.SectionName;
		});

	// JSON 구성
	TArray<TSharedPtr<FJsonValue>> CfgArray;

	for (const FSectionDump& Dump : SectionDumps)
	{
		TSharedPtr<FJsonObject> SectionObj = MakeShared<FJsonObject>();
		SectionObj->SetStringField(TEXT("ini"), Dump.IniName);
		SectionObj->SetStringField(TEXT("section"), Dump.SectionName);

		// pairs 정렬 출력
		TArray<FString> Keys;
		Dump.PairMap.GetKeys(Keys);
		Keys.Sort();

		TArray<TSharedPtr<FJsonValue>> PairArray;
		for (const FString& Key : Keys)
		{
			TSharedPtr<FJsonObject> PairObj = MakeShared<FJsonObject>();
			PairObj->SetStringField(TEXT("key"), Key);
			PairObj->SetStringField(TEXT("value"), Dump.PairMap[Key]);
			PairArray.Add(MakeShared<FJsonValueObject>(PairObj));
		}

		SectionObj->SetArrayField(TEXT("pairs"), PairArray);
		CfgArray.Add(MakeShared<FJsonValueObject>(SectionObj));
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("cfg"), CfgArray);

	FString OutJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	if (!FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(OutJson, *MakePath(TEXT("ssot_cfg.json")));
}

// SSOTDumpCmdlet.cpp
// v0.2.1 - UE5.7 RequestAllGameplayTags 시그니처 대응

// ... 상단 include 그대로 ...
#include "GameplayTagsManager.h"
#include "GameplayTagContainer.h"  // ✅ 필요 시 추가

bool USSOTDumpCmdlet::DumpTags() const
{
	// [v0.2.1] UE5.7: RequestAllGameplayTags는 FGameplayTagContainer를 받음

	// 모든 태그를 담을 컨테이너
	FGameplayTagContainer AllTagContainer;

	// 태그 요청 (OnlyIncludeDictionaryTags=true)
	UGameplayTagsManager::Get().RequestAllGameplayTags(
		AllTagContainer,
		/*OnlyIncludeDictionaryTags=*/true
	);

	// 문자열 리스트로 변환
	TArray<FString> TagStrings;
	TagStrings.Reserve(AllTagContainer.Num());

	for (const FGameplayTag& Tag : AllTagContainer)
	{
		TagStrings.Add(Tag.ToString());
	}

	// 결정성 보장: 정렬
	TagStrings.Sort();

	// JSON 배열 생성
	TArray<TSharedPtr<FJsonValue>> TagArray;
	TagArray.Reserve(TagStrings.Num());

	for (const FString& TagStr : TagStrings)
	{
		TagArray.Add(MakeShared<FJsonValueString>(TagStr));
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("tags"), TagArray);

	FString OutJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);

	if (!FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(OutJson, *MakePath(TEXT("ssot_tags.json")));
}

// -------------------------
// Enhanced Input 덤프 (v0.2)
// -------------------------

static void CollectAssetsByClass(
	IAssetRegistry& AssetRegistry,
	const FTopLevelAssetPath& ClassPath,
	const FString& PackagePathFilter,
	TArray<FAssetData>& OutAssets)
{
	FARFilter Filter;
	Filter.ClassPaths.Add(ClassPath);
	Filter.bRecursiveClasses = true;

	// /Game 하위 특정 폴더만 보고 싶으면 필터링
	if (!PackagePathFilter.IsEmpty())
	{
		Filter.PackagePaths.Add(*PackagePathFilter);
		Filter.bRecursivePaths = true;
	}

	AssetRegistry.GetAssets(Filter, OutAssets);

	// 결정성: ObjectPath 정렬
	OutAssets.Sort([](const FAssetData& A, const FAssetData& B)
		{
			return A.GetObjectPathString() < B.GetObjectPathString();
		});
}

bool USSOTDumpCmdlet::DumpInput() const
{
	// AssetRegistry 로딩
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// /Game 스캔 보장
	TArray<FString> ScanPaths;
	ScanPaths.Add(TEXT("/Game"));
	AssetRegistry.ScanPathsSynchronous(ScanPaths, true);

	// v0.3b: input_root를 고정하지 않고 해석(Detect + Profile + CLI)
	// OutputDir는 Commandlet의 출력 폴더(상대/절대 상관없지만 절대면 더 안전)
	FString InputRootSource;
	const FString ResolvedInputRoot = SSOT_ResolveInputRoot(AssetRegistry, OutputDir, InputRootSource);

	// v0.3c: detect 덮어쓰기 방지
	// 이미 autodetect가 ssot_detect.json을 저장했으면(파일 존재) 다시 쓰지 않는다.
	{
		const FString DetectFilePath = FPaths::Combine(OutputDir, GSSOT_DetectOutFileName);

		if (!FPaths::FileExists(DetectFilePath))
		{
			TArray<FString> EmptyFolders;
			SSOT_SaveDetectJson(OutputDir, ResolvedInputRoot, InputRootSource, EmptyFolders, TEXT("resolved via cli/profile"));
		}
	}

	// 이후 InputRoot 대신 ResolvedInputRoot를 필터로 사용
	// 예: CollectAssetsByClass(..., PackagePathFilter = ResolvedInputRoot);

	// IA 수집
	TArray<FAssetData> InputActions;
	CollectAssetsByClass(
		AssetRegistry,
		FTopLevelAssetPath(TEXT("/Script/EnhancedInput"), TEXT("InputAction")),
		ResolvedInputRoot,
		InputActions);

	// IMC 수집
	TArray<FAssetData> InputContexts;
	CollectAssetsByClass(
		AssetRegistry,
		FTopLevelAssetPath(TEXT("/Script/EnhancedInput"), TEXT("InputMappingContext")),
		ResolvedInputRoot,
		InputContexts);

	// IA JSON
	TArray<TSharedPtr<FJsonValue>> IaArray;
	for (const FAssetData& IaData : InputActions)
	{
		TSharedPtr<FJsonObject> IaObj = MakeShared<FJsonObject>();
		IaObj->SetStringField(TEXT("object_path"), IaData.GetObjectPathString());
		IaArray.Add(MakeShared<FJsonValueObject>(IaObj));
	}

	// IMC JSON
	TArray<TSharedPtr<FJsonValue>> ImcArray;

	for (const FAssetData& ImcData : InputContexts)
	{
		// 실제 로드해서 매핑 읽기
		UInputMappingContext* Imc = Cast<UInputMappingContext>(ImcData.GetAsset());
		if (!Imc)
		{
			return false;
		}

		TSharedPtr<FJsonObject> ImcObj = MakeShared<FJsonObject>();
		ImcObj->SetStringField(TEXT("object_path"), ImcData.GetObjectPathString());

		// 매핑 수집
		struct FMapRow
		{
			FString ActionPath;
			FString KeyStr;
			TArray<FString> TriggerClasses;
			TArray<FString> ModifierClasses;
		};

		TArray<FMapRow> Rows;

		for (const FEnhancedActionKeyMapping& Mapping : Imc->GetMappings())
		{
			// Action이 없으면 스킵(혹은 FAIL 처리 - 팀 스타일)
			if (!Mapping.Action)
			{
				continue;
			}

			FMapRow Row;
			Row.ActionPath = Mapping.Action->GetPathName();
			Row.KeyStr = Mapping.Key.ToString();

			// Trigger 클래스들
			for (const UInputTrigger* Trigger : Mapping.Triggers)
			{
				if (Trigger)
				{
					Row.TriggerClasses.Add(Trigger->GetClass()->GetPathName());
				}
			}

			// Modifier 클래스들
			for (const UInputModifier* Modifier : Mapping.Modifiers)
			{
				if (Modifier)
				{
					Row.ModifierClasses.Add(Modifier->GetClass()->GetPathName());
				}
			}

			// 결정성: trigger/modifier 정렬
			Row.TriggerClasses.Sort();
			Row.ModifierClasses.Sort();

			Rows.Add(Row);
		}

		// 결정성: (ActionPath, Key) 정렬
		Rows.Sort([](const FMapRow& A, const FMapRow& B)
			{
				if (A.ActionPath != B.ActionPath) return A.ActionPath < B.ActionPath;
				return A.KeyStr < B.KeyStr;
			});

		// JSON 변환
		TArray<TSharedPtr<FJsonValue>> MappingArray;

		for (const FMapRow& Row : Rows)
		{
			TSharedPtr<FJsonObject> MapObj = MakeShared<FJsonObject>();
			MapObj->SetStringField(TEXT("action_path"), Row.ActionPath);
			MapObj->SetStringField(TEXT("key"), Row.KeyStr);

			// triggers
			TArray<TSharedPtr<FJsonValue>> TrigArray;
			for (const FString& ClassPath : Row.TriggerClasses)
			{
				TrigArray.Add(MakeShared<FJsonValueString>(ClassPath));
			}
			MapObj->SetArrayField(TEXT("triggers"), TrigArray);

			// modifiers
			TArray<TSharedPtr<FJsonValue>> ModArray;
			for (const FString& ClassPath : Row.ModifierClasses)
			{
				ModArray.Add(MakeShared<FJsonValueString>(ClassPath));
			}
			MapObj->SetArrayField(TEXT("modifiers"), ModArray);

			MappingArray.Add(MakeShared<FJsonValueObject>(MapObj));
		}

		ImcObj->SetArrayField(TEXT("mappings"), MappingArray);
		ImcArray.Add(MakeShared<FJsonValueObject>(ImcObj));
	}

	// 최종 JSON
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("input_actions"), IaArray);
	Root->SetArrayField(TEXT("input_contexts"), ImcArray);

	FString OutJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	if (!FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(OutJson, *MakePath(TEXT("ssot_input.json")));
}


static FString ToTickGroupString(const ETickingGroup TickGroup)
{
	// TickGroup 문자열화(결정적)
	switch (TickGroup)
	{
	case TG_PrePhysics:   return TEXT("PrePhysics");
	case TG_StartPhysics: return TEXT("StartPhysics");
	case TG_DuringPhysics:return TEXT("DuringPhysics");
	case TG_EndPhysics:   return TEXT("EndPhysics");
	case TG_PostPhysics:  return TEXT("PostPhysics");
	default:              return TEXT("Unknown");
	}
}

static bool LoadJsonObjectFromFile(const FString& FilePath, TSharedPtr<FJsonObject>& OutObj)
{
	// 파일 로드
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *FilePath))
	{
		return false;
	}

	// JSON 파싱
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
	return FJsonSerializer::Deserialize(Reader, OutObj) && OutObj.IsValid();
}

static FString GetStringFieldSafe(const TSharedPtr<FJsonObject>& Obj, const FString& Key, const FString& DefaultValue)
{
	// 문자열 필드 안전 읽기
	FString Value = DefaultValue;
	if (Obj.IsValid())
	{
		Obj->TryGetStringField(Key, Value);
	}
	return Value;
}

static double GetNumberFieldSafe(const TSharedPtr<FJsonObject>& Obj, const FString& Key, const double DefaultValue)
{
	// 숫자 필드 안전 읽기
	double Value = DefaultValue;
	if (Obj.IsValid())
	{
		Obj->TryGetNumberField(Key, Value);
	}
	return Value;
}

static bool HasPropertyByName(UClass* ClassType, const FName PropertyName)
{
	// UPROPERTY 존재 여부 확인
	if (!ClassType) return false;
	return FindFProperty<FProperty>(ClassType, PropertyName) != nullptr;
}

static bool TryGetFloatDefault(UObject* CDO, const FName PropertyName, float& OutValue)
{
	// float 기본값 읽기
	if (!CDO) return false;

	FProperty* Prop = FindFProperty<FProperty>(CDO->GetClass(), PropertyName);
	if (!Prop) return false;

	FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop);
	if (!FloatProp) return false;

	OutValue = FloatProp->GetPropertyValue_InContainer(CDO);
	return true;
}

static bool HasFunctionByName(UClass* ClassType, const FName FunctionName)
{
	// UFUNCTION 존재 여부 확인(블루프린트 함수도 포함)
	if (!ClassType) return false;
	return ClassType->FindFunctionByName(FunctionName) != nullptr;
}

static void CollectComponentNames(UObject* CDO, TArray<FString>& OutAnchorNames, TArray<FString>& OutMeshNames)
{
	// CDO의 컴포넌트 이름 수집 (휠 anchor/mesh 식별)
	if (!CDO) return;

	TInlineComponentArray<UActorComponent*> Components;
	if (AActor* ActorCDO = Cast<AActor>(CDO))
	{
		ActorCDO->GetComponents(Components);
	}

	for (UActorComponent* Comp : Components)
	{
		if (!Comp) continue;

		const FString CompName = Comp->GetName();

		// Anchor: SceneComponent + 이름 패턴
		if (Comp->IsA<USceneComponent>() && CompName.StartsWith(TEXT("Wheel_Anchor_")))
		{
			OutAnchorNames.Add(CompName);
		}

		// Mesh: StaticMeshComponent + 이름 패턴
		if (Comp->IsA<UStaticMeshComponent>() && CompName.StartsWith(TEXT("Wheel_Mesh_")))
		{
			OutMeshNames.Add(CompName);
		}
	}

	// 결정성: 정렬
	OutAnchorNames.Sort();
	OutMeshNames.Sort();
}

bool USSOTDumpCmdlet::DumpWheel() const
{
	// roots 파일 로드
	const FString RootsPath = MakePath(RootsFileName);

	TSharedPtr<FJsonObject> RootsObj;
	if (!LoadJsonObjectFromFile(RootsPath, RootsObj))
	{
		return false;
	}

	// roots 읽기
	const TSharedPtr<FJsonObject>* RootsSectionPtr = nullptr;
	RootsObj->TryGetObjectField(TEXT("roots"), RootsSectionPtr);

	const TSharedPtr<FJsonObject>* WheelSectionPtr = nullptr;
	RootsObj->TryGetObjectField(TEXT("wheel_ssot"), WheelSectionPtr);

	const TSharedPtr<FJsonObject> RootsSection = RootsSectionPtr ? *RootsSectionPtr : nullptr;
	const TSharedPtr<FJsonObject> WheelSection = WheelSectionPtr ? *WheelSectionPtr : nullptr;

	// Canonical root
	const FString VehicleRoot = GetStringFieldSafe(RootsSection, TEXT("vehicle_root"), TEXT("/Game"));

	// BP 이름
	const FString VehicleBpName = GetStringFieldSafe(WheelSection, TEXT("vehicle_bp_name"), TEXT("BP_ModularVehicle"));

	// 기대 TickGroup
	const FString ExpectedTickGroup = GetStringFieldSafe(WheelSection, TEXT("expected_tick_group"), TEXT("PostPhysics"));

	// 기대 우측 Yaw
	const double ExpectedRightYawDeg = GetNumberFieldSafe(WheelSection, TEXT("expected_right_yaw_deg"), 180.0);

	// AssetRegistry로 BP 찾기
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// /Game 스캔 보장
	TArray<FString> ScanPaths;
	ScanPaths.Add(TEXT("/Game"));
	AssetRegistry.ScanPathsSynchronous(ScanPaths, true);

	// BP 이름으로 후보 검색: (간단/안전) 모든 BP 중 이름 매칭
	// 초보자 단계에서는 “이름으로 찾고, 결과를 ssot_wheel에 기록”하는 게 디버깅이 쉽다.
	TArray<FAssetData> AllAssets;
	AssetRegistry.GetAllAssets(AllAssets, true);

	FAssetData FoundBp;
	bool bFound = false;

	for (const FAssetData& A : AllAssets)
	{
		// Blueprint 계열만 빠르게 필터 (정확한 class path는 프로젝트/버전마다 차이가 있어 이름 위주로 진행)
		if (A.AssetName.ToString().Equals(VehicleBpName))
		{
			FoundBp = A;
			bFound = true;
			break;
		}
	}

	// FAIL: BP 못 찾음
	TArray<FString> Fails;
	if (!bFound)
	{
		Fails.Add(TEXT("BP_ModularVehicle not found by name."));
	}

	// 결과 JSON 구성
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> WheelObj = MakeShared<FJsonObject>();

	// 기본값
	bool bInVehicleRoot = false;
	bool bStartEnabled = false;
	FString TickGroupStr = TEXT("Unknown");

	TArray<FString> AnchorNames;
	TArray<FString> MeshNames;

	float RightWheelYawDeg = 0.0f;
	float SuspensionOffsetSign = 0.0f;
	float WheelSpinSign = 0.0f;

	bool bHasChaosWheeledMovement = false;
	bool bHasUpdateFromSnapshot = false;

	if (bFound)
	{
		const FString ObjectPath = FoundBp.GetObjectPathString();
		WheelObj->SetStringField(TEXT("bp_object_path"), ObjectPath);

		// Canonical root 안에 있는지 판정
		bInVehicleRoot = ObjectPath.StartsWith(VehicleRoot);
		WheelObj->SetBoolField(TEXT("bp_in_vehicle_root"), bInVehicleRoot);

		if (!bInVehicleRoot)
		{
			Fails.Add(FString::Printf(TEXT("BP is outside vehicle_root. root=%s path=%s"), *VehicleRoot, *ObjectPath));
		}

		// BP 로드 → CDO 검사
		UObject* AssetObj = FoundBp.GetAsset();
		UBlueprintGeneratedClass* BpClass = nullptr;

		// Blueprint asset일 수도, generated class로 resolve 필요할 수도 있음.
		// 여기서는 “CDO를 얻을 수 있으면 검사” 정도로 최소 구현.
		if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetObj))
		{
			BpClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);
		}
		else
		{
			// 다른 형태면 클래스 캐스팅 시도
			BpClass = Cast<UBlueprintGeneratedClass>(AssetObj);
		}

		if (!BpClass)
		{
			Fails.Add(TEXT("Failed to resolve BlueprintGeneratedClass."));
		}
		else
		{
			UObject* CDO = BpClass->GetDefaultObject();
			AActor* ActorCDO = Cast<AActor>(CDO);

			if (!ActorCDO)
			{
				Fails.Add(TEXT("CDO is not an Actor."));
			}
			else
			{
				// Tick 검사
				bStartEnabled = ActorCDO->PrimaryActorTick.bStartWithTickEnabled;
				TickGroupStr = ToTickGroupString(ActorCDO->PrimaryActorTick.TickGroup);

				TSharedPtr<FJsonObject> TickObj = MakeShared<FJsonObject>();
				TickObj->SetBoolField(TEXT("start_enabled"), bStartEnabled);
				TickObj->SetStringField(TEXT("tick_group"), TickGroupStr);
				WheelObj->SetObjectField(TEXT("tick"), TickObj);

				if (!bStartEnabled)
				{
					Fails.Add(TEXT("StartWithTickEnabled is false."));
				}
				if (!TickGroupStr.Equals(ExpectedTickGroup))
				{
					Fails.Add(FString::Printf(TEXT("TickGroup mismatch. expected=%s actual=%s"), *ExpectedTickGroup, *TickGroupStr));
				}

				// 컴포넌트 이름 수집
				CollectComponentNames(CDO, AnchorNames, MeshNames);

				TSharedPtr<FJsonObject> CompObj = MakeShared<FJsonObject>();

				// anchors_found
				TArray<TSharedPtr<FJsonValue>> AnchorArray;
				for (const FString& N : AnchorNames) AnchorArray.Add(MakeShared<FJsonValueString>(N));
				CompObj->SetArrayField(TEXT("anchors_found"), AnchorArray);

				// meshes_found
				TArray<TSharedPtr<FJsonValue>> MeshArray;
				for (const FString& N : MeshNames) MeshArray.Add(MakeShared<FJsonValueString>(N));
				CompObj->SetArrayField(TEXT("meshes_found"), MeshArray);

				WheelObj->SetObjectField(TEXT("components"), CompObj);

				// 기대 컴포넌트 8개 체크(문서 기준)
				const TArray<FString> NeedAnchors = { TEXT("Wheel_Anchor_FL"),TEXT("Wheel_Anchor_FR"),TEXT("Wheel_Anchor_RL"),TEXT("Wheel_Anchor_RR") };
				const TArray<FString> NeedMeshes = { TEXT("Wheel_Mesh_FL"),TEXT("Wheel_Mesh_FR"),TEXT("Wheel_Mesh_RL"),TEXT("Wheel_Mesh_RR") };

				for (const FString& Need : NeedAnchors)
				{
					if (!AnchorNames.Contains(Need)) Fails.Add(FString::Printf(TEXT("Missing anchor component: %s"), *Need));
				}
				for (const FString& Need : NeedMeshes)
				{
					if (!MeshNames.Contains(Need)) Fails.Add(FString::Printf(TEXT("Missing mesh component: %s"), *Need));
				}

				// 변수 기본값 검사
				bool bHasRightYaw = TryGetFloatDefault(CDO, TEXT("RightWheelYawDeg"), RightWheelYawDeg);
				if (!bHasRightYaw)
				{
					Fails.Add(TEXT("Missing float property: RightWheelYawDeg"));
				}
				else
				{
					if (FMath::Abs(RightWheelYawDeg - (float)ExpectedRightYawDeg) > 0.01f)
					{
						Fails.Add(FString::Printf(TEXT("RightWheelYawDeg mismatch. expected=%.2f actual=%.2f"), ExpectedRightYawDeg, RightWheelYawDeg));
					}
				}

				// Snapshot 핫픽스 문서 기준 변수들 존재/기본값 검사
				bHasChaosWheeledMovement = HasPropertyByName(BpClass, TEXT("ChaosWheeledMovement"));
				bHasUpdateFromSnapshot = HasFunctionByName(BpClass, TEXT("UpdateWheelVisuals_FromSnapshot"));

				if (!bHasChaosWheeledMovement)
				{
					Fails.Add(TEXT("Snapshot SSOT missing: ChaosWheeledMovement property not found."));
				}
				if (!bHasUpdateFromSnapshot)
				{
					Fails.Add(TEXT("Snapshot SSOT missing: UpdateWheelVisuals_FromSnapshot function not found."));
				}

				// 기본값(선택): SuspensionOffsetSign, WheelSpinSign
				// 문서의 기본값 -1.0 기준
				if (TryGetFloatDefault(CDO, TEXT("SuspensionOffsetSign"), SuspensionOffsetSign))
				{
					if (FMath::Abs(SuspensionOffsetSign - (-1.0f)) > 0.01f)
					{
						Fails.Add(FString::Printf(TEXT("SuspensionOffsetSign default not -1. actual=%.2f"), SuspensionOffsetSign));
					}
				}
				else
				{
					Fails.Add(TEXT("Snapshot SSOT missing: SuspensionOffsetSign property not found."));
				}

				if (TryGetFloatDefault(CDO, TEXT("WheelSpinSign"), WheelSpinSign))
				{
					if (FMath::Abs(WheelSpinSign - (-1.0f)) > 0.01f)
					{
						Fails.Add(FString::Printf(TEXT("WheelSpinSign default not -1. actual=%.2f"), WheelSpinSign));
					}
				}
				else
				{
					Fails.Add(TEXT("Snapshot SSOT missing: WheelSpinSign property not found."));
				}

				// defaults 기록
				TSharedPtr<FJsonObject> DefObj = MakeShared<FJsonObject>();
				DefObj->SetNumberField(TEXT("RightWheelYawDeg"), RightWheelYawDeg);
				DefObj->SetNumberField(TEXT("SuspensionOffsetSign"), SuspensionOffsetSign);
				DefObj->SetNumberField(TEXT("WheelSpinSign"), WheelSpinSign);
				WheelObj->SetObjectField(TEXT("defaults"), DefObj);

				// snapshot_ready 기록
				TSharedPtr<FJsonObject> SnapObj = MakeShared<FJsonObject>();
				SnapObj->SetBoolField(TEXT("has_ChaosWheeledMovement"), bHasChaosWheeledMovement);
				SnapObj->SetBoolField(TEXT("has_UpdateWheelVisuals_FromSnapshot"), bHasUpdateFromSnapshot);
				WheelObj->SetObjectField(TEXT("snapshot_ready"), SnapObj);
			}
		}
	}

	Root->SetObjectField(TEXT("wheel"), WheelObj);

	// verdict
	const bool bPass = (Fails.Num() == 0);

	TSharedPtr<FJsonObject> VerdictObj = MakeShared<FJsonObject>();
	VerdictObj->SetBoolField(TEXT("pass"), bPass);

	TArray<TSharedPtr<FJsonValue>> FailArray;
	for (const FString& F : Fails) FailArray.Add(MakeShared<FJsonValueString>(F));
	VerdictObj->SetArrayField(TEXT("fails"), FailArray);

	Root->SetObjectField(TEXT("verdict"), VerdictObj);

	// 저장
	FString OutJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	if (!FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
	{
		return false;
	}

	const bool bSaved = FFileHelper::SaveStringToFile(OutJson, *MakePath(TEXT("ssot_wheel.json")));

	// strict 모드면 FAIL을 exit code로 반영하려고 Main에서 bOk에 포함시켜야 함.
	return bSaved && (!bStrictFail || bPass);
}
