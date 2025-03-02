// Copyright (c) 2025 Sebastian Cyliax

#include "Waypoint.h"

AWaypoint::AWaypoint()
	: Index(0)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWaypoint::BeginPlay()
{
	Super::BeginPlay();
}

void AWaypoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
