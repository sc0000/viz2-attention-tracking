// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeatmapRT.generated.h"

UENUM(BlueprintType)
enum class ESortMode : uint8
{
	ESM_Name UMETA(DisplayName = "Name"),
	ESM_First UMETA(DisplayName = "FirstAttentionAfter"),
	ESM_Total UMETA(DisplayName = "TotalAttentionTime"),
	ESM_Times UMETA(DisplayName = "TimesFocussed"),
	ESM_Average UMETA(DisplayName = "AverageAttentionTime"),
	ESM_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType, Category = "RenderTargetCoordinatesData")
struct FAttentionTrackingDataPoint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RenderTargetCoordinatesData")
	float TimePassedSinceRecordingStarted = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "RenderTargetCoordinatesData")
	FString ObjectName = "";

	UPROPERTY(BlueprintReadWrite, Category = "RenderTargetCoordinatesData")
	FVector2D Coordinates = FVector2D(0.0, 0.0);

	UPROPERTY(BlueprintReadWrite, Category = "RenderTargetCoordinatesData")
	FVector2D PaintBrushScaleDivisor = FVector2D(1.0, 1.0);
};

USTRUCT(BlueprintType, Category = "AttentionMetrics")
struct FAttentionMetricsEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "AttentionMetrics")
	float TotalAttentionTime = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "AttentionMetrics")
	float AverageAttentionTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "AttentionMetrics")
	float FirstAttentionAfter = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "AttentionMetrics")
	int32 TimesFocussed = 0;

	// Obsolete?
	UPROPERTY(BlueprintReadOnly, Category = "AttentionMetrics")
	TArray<int> AttentionSequenceIndices = {};
};

UCLASS()
class EYETRACKINGUTILITYRUNTIME_API UHeatmapRT : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	static void SaveHeatmap(const FString& FileName, const TArray<FAttentionTrackingDataPoint>& AttentionTrackingData);

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	static void LoadHeatmap(const FString& FileName, const UObject* WorldContextObject, const float MetricsThreshold = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Painting")
	static void PaintLoadedHeatmap(UObject* WorldContextObject, const bool bLoadImmediately);

	UFUNCTION(BlueprintCallable, Category = "Painting")
	static void StopTimer(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Saving and Loading")
	static FString GetLastSavedOrLoadedHeatmapFileName();

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	static void SetLastSavedOrLoadedHeatmapFileName(const FString& FileName);
	
	UFUNCTION(BlueprintCallable, Category = "Reset")
	static void ResetHeatmap(UObject* WorldContextObject, UMaterialInterface* PaintBrushMaterialAsset);

	UFUNCTION(BlueprintCallable, Category = "Attention Metrics")
	static void CalculateAttentionMetrics(const TMap<FString, FString>& MetricsNames, const float Threshold = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Attention Tracking Data")
	static void GetAttentionTrackingDataCurrentlyLoaded(TArray<FAttentionTrackingDataPoint>& OutData);

	UFUNCTION(BlueprintPure, Category = "Attention Metrics")
	static void GetAttentionMetrics(TMap<FString, FAttentionMetricsEntry>& OutMetrics);

	UFUNCTION(BlueprintCallable, Category = "Attention Metrics")
	static void GetMetricsNames(const UObject* WorldContextObject, TMap<FString, FString>& OutMetricsNames);

	UFUNCTION(BlueprintCallable, Category = "Attention Metrics")
	static void SortAttentionMetrics(ESortMode SortMode, bool bAscending);
	
	static void PaintHeatmapDataPoint(const FAttentionTrackingDataPoint& DataPoint, const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Visualization")
	static void BlendMaterialParameter(const UObject* WorldContextObject,
		const UMaterialParameterCollection* ParameterCollection, const FName ParameterName);

	UFUNCTION(BlueprintCallable, Category = "Visualization")
	static void ClearAllTimers(bool bOnEndPieMode);
	
	
private:
	static class AHeatmapReadyActor* LastActorPaintedOn;
	static TArray<AHeatmapReadyActor*> HeatmapReadyActors;
	
	static FString LastSavedOrLoadedHeatmapFileName;
	
	static TArray<FAttentionTrackingDataPoint> AttentionTrackingDataCurrentlyLoaded;
	
	
	static TMap<FString, FAttentionMetricsEntry> AttentionMetrics;

	static void LogAttentionMetrics();

	static TArray<FTimerHandle> HeatmapTimerHandles;
	static TMap<FName, FTimerHandle> BlendParameterTimerHandles; 
};
