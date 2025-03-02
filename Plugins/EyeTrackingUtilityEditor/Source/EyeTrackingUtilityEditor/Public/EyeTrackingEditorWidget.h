// Copyright (c) 2023 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"

#include "EditorUtilityWidget.h"

#include "EyeTrackingEditorWidget.generated.h"

class AHeatmapReadyActor;

UCLASS()
class EYETRACKINGUTILITYEDITOR_API UEyeTrackingEditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

protected:
	// EVENT HANDLING

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnStaticMeshAddedToSceneTriggered(UStaticMeshComponent* StaticMeshComponent);

	UFUNCTION(BlueprintImplementableEvent)
	void OnExitVrPreview();

	// RENDER TARGETS HEATMAP INTERFACE

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void PrepareAllStaticMeshActorsInLevel(const TSubclassOf<AHeatmapReadyActor> HeatmapReadyActorClass);

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void PrepareSelectedStaticMeshActors(const TSubclassOf<AHeatmapReadyActor> HeatmapReadyActorClass);

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void RevertSelectedHeatmapReadyActors();

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void RemoveBrokenUvChannel();

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void RemoveBrokenUvChannelFromAllHeatmapReadyActors();

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void LoadHeatmapRT(const FString& FileName, const float MetricsThreshold = 0.f);
	
	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void ResetHeatmapRT(UMaterialInterface* PaintBrushMaterialAsset);

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void SetupColorsComboBox(class UComboBoxString* ColorsComboBox, const TMap<FString, FColor>& Colors);

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void SetEyesColor(const TMap<FString, FColor>& Colors, FString ColorName);

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void SetMaterialForActors(const TArray<AActor*> Actors, UMaterialInterface* Material);
	
	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void SetMaterialForSelectedActors(UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void SetMaterialForHeatmapReadyActors(const UObject* WorldContextObject, UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void SetMaterialForStaticMeshActors(const UObject* WorldContextObject, UMaterialInterface* Material);
	
	UFUNCTION(BlueprintCallable, Category = "CPU Access")
	static void DisableCpuAccessForAllMeshes();

	// COLLISION INTERFACE

	UFUNCTION(BlueprintCallable, Category = "Collision Setup")
	static void EnableComplexCollisionForSelectedStaticMeshActors();

	// TODO: Remove!
	UFUNCTION(BlueprintCallable, Category = "Collision Setup")
	static void EnableComplexCollisionForAllStaticMeshActors();

	// GENERAL PURPOSE PROPS

public:
	UFUNCTION(BlueprintCallable, Category = "Render Targets Painting Interface")
	static void RestoreMaterialsForHeatmapReadyActors();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Update ScrollBox Highlight")
	void UpdateScrollBoxHighlight();
	void UpdateScrollBoxHighlight_Implementation();

	UPROPERTY(BlueprintReadWrite, Category = "Path")
	FString CurrentDirectory;

	UPROPERTY(BlueprintReadWrite, Category = "Path")
	FString CurrentFile;

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Render Targets Painting Interface")
	bool bLoadImmediately = false;
	
	UPROPERTY(BlueprintReadWrite, Category = "Config Data")
	bool bShowHeatmap = false;

	UPROPERTY(BlueprintReadWrite, Category = "Config Data")
	bool bRemoveColors = false;

private:
	static class UEditorActorSubsystem* EditorActorSubsystem;

	static bool GetEditorActorSubsystem();
	
	static TArray<AActor*> GetSelectedLevelActors();
	static TArray<AStaticMeshActor*> GetAllStaticMeshActorsInLevel();
	static TArray<AHeatmapReadyActor*> GetAllHeatmapReadyActorsInLevel();

	static void ReplaceStaticMeshActorWithHeatmapReadyActor(AStaticMeshActor* StaticMeshActor, 
		TSubclassOf<AHeatmapReadyActor> HeatmapReadyActorClass);

	static void ReplaceHeatmapReadyActorWithStaticMeshActor(AHeatmapReadyActor* HeatmapReadyActor);

	static void PrepareStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent);
	static void EnableComplexCollision(UStaticMeshComponent* StaticMeshComponent);
};
