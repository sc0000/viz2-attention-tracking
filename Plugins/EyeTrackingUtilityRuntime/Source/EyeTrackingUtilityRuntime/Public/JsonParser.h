// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeatmapRT.h"

#include "JsonParser.generated.h"

#define DEFAULT 0

class FJsonObject;

USTRUCT(BlueprintType, Category = "ConfigData")
struct FConfigData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ConfigData")
	float ShowHeatmap = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "ConfigData")
	float ShowTextures = 0.f;
};

UCLASS()
class EYETRACKINGUTILITYRUNTIME_API UJsonParser : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AttentionTrackingData")
	static void ReadAttentionTrackingDataFromJsonFile(const FString& FilePath, TArray<FAttentionTrackingDataPoint>& AttentionTrackingData, bool& bOutSuccess);

	UFUNCTION(BlueprintCallable, Category = "AttentionTrackingData")
	static void WriteAttentionTrackingDataToJsonFile(const TArray<FAttentionTrackingDataPoint>& AttentionTrackingDataArray,
	                                                 const FString& FilePath, bool& bOutSuccess);

	UFUNCTION(BlueprintPure, Category = "AttentionTrackingData")
	static FString AttentionTrackingDataFolderPath();

	/******/
	// UFUNCTION(BlueprintCallable, Category = "AttentionMetrics")
	// static void ReadAttentionMetricsFromJsonFile(const FString& FilePath, TArray<FAttentionMetricsEntry>& AttentionMetrics, bool& bOutSuccess);

	UFUNCTION(BlueprintCallable, Category = "AttentionMetrics")
	static void WriteAttentionMetricsToJsonFile(const TMap<FString, FAttentionMetricsEntry>& AttentionMetrics, const FString& FilePath, bool& bOutSuccess);

	UFUNCTION(BlueprintPure, Category = "AttentionMetrics")
	static FString AttentionMetricsFolderPath();
	
	/******/

	UFUNCTION(BlueprintCallable, Category = "ConfigData")
	static FConfigData ReadConfigDataFromJsonFile(const FString& FilePath, bool& bOutSuccess);

	UFUNCTION(BlueprintCallable, Category = "ConfigData")
	static void WriteConfigDataToJsonFile(const FConfigData ConfigData, const FString& FilePath, bool& bOutSuccess);

	UFUNCTION(BlueprintPure, Category = "ConfigData")
	static FString ConfigDataFilePath();

private:

	static FString ReadStringFromFile(const FString& FilePath, bool& bOutSuccess);
	static void WriteStringToFile(const FString& String, const FString& FilePath, bool& bOutSuccess);
	static TSharedPtr<FJsonObject> ReadJson(const FString& FilePath, bool& bOutSuccess);
	static void WriteJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FilePath, bool& bOutSuccess);
	static void WriteJson(const TArray<TSharedPtr<FJsonValue>>& JsonObjectArray, const FString& FilePath, bool& bOutSuccess);
};
