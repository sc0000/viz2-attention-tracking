// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waypoint.generated.h"

UCLASS()
class EYETRACKINGUTILITYRUNTIME_API AWaypoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaypoint();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Index")
	int32 Index;
	
	virtual void Tick(float DeltaTime) override;
};
