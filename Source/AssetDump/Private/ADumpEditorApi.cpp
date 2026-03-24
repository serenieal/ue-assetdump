		FString SelectedAssetObjectPath;
	FString SelectedDisplayName;
	if (!GetSelectedBlueprintObjectPath(SelectedAssetObjectPath, SelectedDisplayName, OutMessage))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[ADumpEditorApi] StartDumpSelectedBlueprint SelectedAsset='%s' Display='%s' RawOutput='%s'"), *SelectedAssetObjectPath, *SelectedDisplayName, *OutputFilePath);

	const FADumpRunOpts DumpRunOpts = BuildRunOpts(

