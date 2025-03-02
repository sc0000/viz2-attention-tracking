// Copyright (c) 2025 Sebastian Cyliax

#include "TakeRecorderControls.h"

#include "Recorder/TakeRecorderBlueprintLibrary.h"
#include "Recorder/TakeRecorderPanel.h"
#include "TakeRecorderSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "NiagaraSystem.h"
#include "NiagaraTypes.h" 

#include "DebugHeader.h"

void UTakeRecorderControls::StartRecording(UTakeRecorderPanel* TakeRecorderPanel)
{
	if (!TakeRecorderPanel)
	{
		DebugHeader::ShowNotifyInfo("Failed to access Take Recorder Panel");
		return;
	}

	TakeRecorderPanel->StartRecording();
}

void UTakeRecorderControls::StopRecording(UTakeRecorderPanel* TakeRecorderPanel)
{
	if (!TakeRecorderPanel)
	{
		DebugHeader::ShowNotifyInfo("Failed to access Take Recorder Panel");
		return;
	}

	TakeRecorderPanel->StopRecording();
}

void UTakeRecorderControls::SetTraceLifetime(UNiagaraSystem* System, const float NewLifetime)
{
	const FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.SoftObjectPaths.Add(FSoftObjectPath(*System->GetPathName()));

	TArray<FAssetData> AssetData;
	AssetRegistry.Get().GetAssets(Filter, AssetData);

	TArray<FNiagaraVariable> UserParameters;
	System->GetExposedParameters().GetUserParameters(UserParameters);

	// TODO: Enable more than one user parameter; maybe compare names?

	if (UserParameters.Num() == 0 ||
		UserParameters[0].GetType() != FNiagaraTypeDefinition::GetFloatDef()) return;

	System->GetExposedParameters().SetParameterValue<float>(NewLifetime, UserParameters[0]);
}

void UTakeRecorderControls::SetTakeRecorderConfigs(const FString& RootTakeSaveDir, 
	UTakePreset* TakePreset, const float CountdownSeconds)
{
	UTakeRecorderProjectSettings* TakeRecorderProjectSettings =
		GetMutableDefault<UTakeRecorderProjectSettings>(UTakeRecorderProjectSettings::StaticClass());

	if (!TakeRecorderProjectSettings) return;
	
	TakeRecorderProjectSettings->Settings.RootTakeSaveDir = FDirectoryPath{ RootTakeSaveDir };

	UTakeRecorderUserSettings* TakeRecorderUserSettings =
		GetMutableDefault<UTakeRecorderUserSettings>(UTakeRecorderUserSettings::StaticClass());

	if (!TakeRecorderProjectSettings) return;

	TakeRecorderUserSettings->Settings.CountdownSeconds = CountdownSeconds;

	UTakeRecorderPanel* Panel = UTakeRecorderBlueprintLibrary::GetTakeRecorderPanel();

	if (!Panel) return;

	Panel->SetupForRecording_TakePreset(TakePreset);
}

void UTakeRecorderControls::SetTakeRecorderSaveDir(const FString& TakeSaveDir)
{
	UTakeRecorderProjectSettings* TakeRecorderProjectSettings = 
		GetMutableDefault<UTakeRecorderProjectSettings>(UTakeRecorderProjectSettings::StaticClass());
	
	TakeRecorderProjectSettings->Settings.TakeSaveDir = TakeSaveDir;
}
