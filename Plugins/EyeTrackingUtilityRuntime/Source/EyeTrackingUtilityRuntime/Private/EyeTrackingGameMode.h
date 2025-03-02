// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EyeTrackingGameMode.generated.h"

UCLASS()
class AEyeTrackingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void BeginPlay() override;
};
