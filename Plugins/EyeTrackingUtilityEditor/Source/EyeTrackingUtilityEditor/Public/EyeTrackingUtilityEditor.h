// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_EVENT_OneParam(FEyeTrackingUtilityEditorModule, FOnStaticMeshAddedToScene, class UStaticMeshComponent*);
DECLARE_EVENT_OneParam(FEyeTrackingUtilityEditorModule, FOnWaypointAddedToScene, class AWaypoint*);
// typedef FOnStaticMeshAddedToScene::FDelegate FOnStaticMeshAddedToSceneDelegate;

DECLARE_EVENT(FEyeTrackingUtilityEditorModule, FOnExitVrPreview);

class FEyeTrackingUtilityEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FOnStaticMeshAddedToScene OnStaticMeshAddedToScene;
	static FOnWaypointAddedToScene OnWaypointAddedToScene;
	static FOnExitVrPreview OnExitVrPreview;

private:
	static class UEditorActorSubsystem* EditorActorSubsystem;

	static bool GetEditorActorSubsystem();
	
	void OnNewActorsDropped(const TArray<UObject*>& DroppedObjects, const TArray<AActor*>& CreatedActors);
	void LoadLastHeatmapAndResetList(bool bIsSimulating);
	void RestoreMaterialsForHeatmapReadyActors();
	void ResetMaterialParameters();
};