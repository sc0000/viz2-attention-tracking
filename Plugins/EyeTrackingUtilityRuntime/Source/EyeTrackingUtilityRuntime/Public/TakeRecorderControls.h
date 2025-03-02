// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TakeRecorderControls.generated.h"

UCLASS()
class EYETRACKINGUTILITYRUNTIME_API UTakeRecorderControls : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

protected:

	UFUNCTION(BlueprintCallable, Category = "TakeRecorder")
	static void StartRecording(class UTakeRecorderPanel* TakeRecorderPanel);

	UFUNCTION(BlueprintCallable, Category = "TakeRecorder")
	static void StopRecording(UTakeRecorderPanel* TakeRecorderPanel);

	UFUNCTION(BlueprintCallable, Category = "TakeRecorder")
	static void SetTraceLifetime(UNiagaraSystem* System, const float NewLifetime);

	UFUNCTION(BlueprintCallable, Category = "TakeRecorder")
	static void SetTakeRecorderSaveDir(const FString& TakeSaveDir);

	UFUNCTION(BlueprintCallable, Category = "TakeRecorder")
	static void SetTakeRecorderConfigs(const FString& RootTakeSaveDir, 
		UTakePreset* TakePreset, const float CountdownSeconds = 0.f);

private:
	static class UTakeRecorder* TakeRecorder;
	
};
