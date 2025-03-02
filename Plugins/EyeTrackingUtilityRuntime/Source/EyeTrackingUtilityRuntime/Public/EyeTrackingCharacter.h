// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "JsonParser.h"

#include "EyeTrackingCharacter.generated.h"

UCLASS()
class EYETRACKINGUTILITYRUNTIME_API AEyeTrackingCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEyeTrackingCharacter();

public:
	UFUNCTION(BlueprintCallable, Category = "Get Heatmap Data")
	void GetHeatmapData(TArray<FAttentionTrackingDataPoint>& Data) { Data = HeatmapData; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "EyeTracking Math")
	void GazeScreenToWorld(FVector& LineTraceStart, FVector& LineTraceEnd) const;

	UFUNCTION(BlueprintCallable, Category = "TakeRecorder")
	void SetTakeRecorderPanelReference();

	UPROPERTY(BlueprintReadOnly)
	class UTakeRecorderPanel* TakeRecorderPanelReference;

	UFUNCTION(BlueprintCallable, Category = "Painting the Heatmap")
	void PaintHeatmap(uint8 UvChannel, const class UCameraComponent* Camera = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Painting the Heatmap")
	static FVector2D CalculateScaleDivisor(AActor* HitActor, const FVector ImpactNormal);

	UFUNCTION(BlueprintCallable, Category = "Eye-Tracking State")
	void SetEyeTrackingState();

	UFUNCTION()
	void PaintHeatmapDataPoint(const FAttentionTrackingDataPoint& DataPoint) const;

	UFUNCTION(BlueprintCallable, Category = "Metrics")
	void FocusActor(const UCameraComponent* Camera);

	UPROPERTY(BlueprintReadWrite, Category = "Eyes Offset")
	FVector EyesOffset;

	UPROPERTY(BlueprintReadWrite, Category = "Eye-Tracking State")
	bool bIsTracking;

	UPROPERTY(BlueprintReadOnly, Category = "Time" )
	float CurrentTimeStep;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Heatmap")
	FString NewHeatmapName;

	UPROPERTY(BlueprintReadWrite, Category = "Heatmap")
	FString LastSavedOrLoadedHeatmap;

	UPROPERTY(BlueprintReadOnly, Category = "Last Actor Focussed")
	AHeatmapReadyActor* LastActorFocussed;

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	bool bVR;

private:
	float TrackingStartTime;

	TArray<FAttentionTrackingDataPoint> HeatmapData;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
