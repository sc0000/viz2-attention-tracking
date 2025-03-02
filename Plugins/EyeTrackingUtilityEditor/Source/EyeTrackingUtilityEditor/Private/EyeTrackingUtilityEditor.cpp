// Copyright Epic Games, Inc. All Rights Reserved.

#include "EyeTrackingUtilityEditor.h"

#include "FileHelpers.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Engine/StaticMeshActor.h"

#include "EyeTrackingCharacter.h"
#include "EyeTrackingEditorWidget.h"
#include "HeatmapReadyActor.h"
#include "Waypoint.h"

#define LOCTEXT_NAMESPACE "FEyeTrackingUtilityEditorModule"

FOnStaticMeshAddedToScene FEyeTrackingUtilityEditorModule::OnStaticMeshAddedToScene;
FOnWaypointAddedToScene FEyeTrackingUtilityEditorModule::OnWaypointAddedToScene;
FOnExitVrPreview FEyeTrackingUtilityEditorModule::OnExitVrPreview;
UEditorActorSubsystem* FEyeTrackingUtilityEditorModule::EditorActorSubsystem = nullptr;

void FEyeTrackingUtilityEditorModule::StartupModule()
{
	// BindOnStaticMeshAssetAdded();
	FEditorDelegates::OnNewActorsDropped.AddRaw(this, &FEyeTrackingUtilityEditorModule::OnNewActorsDropped);
	FEditorDelegates::EndPIE.AddRaw(this, &FEyeTrackingUtilityEditorModule::LoadLastHeatmapAndResetList);
	FCoreDelegates::OnEnginePreExit.AddRaw(this, &FEyeTrackingUtilityEditorModule::ResetMaterialParameters);
}

void FEyeTrackingUtilityEditorModule::ShutdownModule()
{
	FEditorDelegates::OnNewActorsDropped.RemoveAll(this);
	FEditorDelegates::EndPIE.RemoveAll(this);
}

bool FEyeTrackingUtilityEditorModule::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();

	return EditorActorSubsystem != nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeStatic (used as callback)
void FEyeTrackingUtilityEditorModule::OnNewActorsDropped(const TArray<UObject*>& DroppedObjects, 
                                                         const TArray<AActor*>& CreatedActors)
{
	for (AActor* Actor : CreatedActors)
	{
		if (const AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
		{
			UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();

			// StaticMeshComponent->GetStaticMesh()->bAllowCPUAccess = true;
			OnStaticMeshAddedToScene.Broadcast(StaticMeshComponent);
		}

		else if (AWaypoint* Waypoint = Cast<AWaypoint>(Actor))
		{
			if (!GetEditorActorSubsystem())
				return;

			TArray<AActor*> LevelActors = EditorActorSubsystem->GetAllLevelActors();
			TArray<AWaypoint*> ExistingWaypoints;
			
			for (AActor* LevelActor : LevelActors)
			{
				AWaypoint* ExistingWaypoint = Cast<AWaypoint>(LevelActor);

				if (!ExistingWaypoint) continue;

				ExistingWaypoints.Emplace(ExistingWaypoint);
			}

			ExistingWaypoints.Sort([](const AWaypoint& A, const AWaypoint& B)
			{
				return A.Index < B.Index;
			});

			const int32 ExistingWaypointsNum = ExistingWaypoints.Num();

			// Indexing starts at 1, for convenience reasons...
			for (int32 i = 0; i < ExistingWaypointsNum; ++i)
				ExistingWaypoints[i]->Index = i; 

			Waypoint->Index = ExistingWaypointsNum;
			OnWaypointAddedToScene.Broadcast(Waypoint);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic (used as callback)
void FEyeTrackingUtilityEditorModule::LoadLastHeatmapAndResetList(bool bIsSimulating)
{
	FEyeTrackingUtilityEditorModule::OnExitVrPreview.Broadcast();
}

// ReSharper disable once CppMemberFunctionMayBeStatic (used as callback)
void FEyeTrackingUtilityEditorModule::RestoreMaterialsForHeatmapReadyActors()
{
	UEyeTrackingEditorWidget::RestoreMaterialsForHeatmapReadyActors();
	
	if (!GetEditorActorSubsystem())
		return;

	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();

	for (AActor* LevelActor : AllLevelActors)
	{
		if (!LevelActor) continue;

		AHeatmapReadyActor* HeatmapReadyActor = Cast<AHeatmapReadyActor>(LevelActor);

		if (!HeatmapReadyActor) continue;

		UStaticMeshComponent* StaticMeshComponent =
			Cast<UStaticMeshComponent>(HeatmapReadyActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (!StaticMeshComponent) continue;

		const int32 NumMaterials = StaticMeshComponent->GetNumMaterials();
		const TArray<UMaterialInterface*> Materials = HeatmapReadyActor->GetMaterials();

		for (uint8 i = 0; i < NumMaterials; ++i)
		{
			if (Materials.Num() < i + 1 || !Materials[i]) continue;

			StaticMeshComponent->SetMaterial(i, Materials[i]);
		}
	}

	FEditorFileUtils::SaveCurrentLevel();
}

// ReSharper disable once CppMemberFunctionMayBeStatic (used as callback)
void FEyeTrackingUtilityEditorModule::ResetMaterialParameters()
{
	constexpr FConfigData ConfigData = { 0.f, 1.f };
	bool bOutSuccess = false;
	UJsonParser::WriteConfigDataToJsonFile(ConfigData, UJsonParser::ConfigDataFilePath(), bOutSuccess);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEyeTrackingUtilityEditorModule, EyeTrackingUtilityEditor)
