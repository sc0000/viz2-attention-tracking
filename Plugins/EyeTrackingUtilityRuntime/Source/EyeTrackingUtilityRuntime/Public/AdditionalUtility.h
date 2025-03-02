// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AdditionalUtility.generated.h"

UCLASS()
class EYETRACKINGUTILITYRUNTIME_API UAdditionalUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AdditionalUtility")
	static FString PaddedIntegerToString(const int32& Integer, const int32 NumDigits = 2);

	UFUNCTION()
	static void GetAxesByLength(FVector Vector, TArray<double>& OutArray);
};
