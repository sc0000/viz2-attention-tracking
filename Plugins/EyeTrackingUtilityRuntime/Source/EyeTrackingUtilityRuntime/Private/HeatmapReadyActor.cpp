// Copyright (c) 2025 Sebastian Cyliax

#include "HeatmapReadyActor.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "DebugHeader.h"

FColor AHeatmapReadyActor::EyesColor;

AHeatmapReadyActor::AHeatmapReadyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}

// void AHeatmapReadyActor::Setup(UMaterialInterface* CanvasMaterialAsset, 
// 	UMaterialInterface* PaintBrushMaterialAsset)
// {
// 	UStaticMeshComponent* StaticMeshComponent =
// 		Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
//
// 	if (!StaticMeshComponent) return;
//
// 	Canvas = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, CanvasMaterialAsset);
// 	PaintBrushMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, PaintBrushMaterialAsset);
//
// 	StaticMeshComponent->SetMaterial(0, Canvas);
//
// 	RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this);
//
// 	Canvas->SetTextureParameterValue(FName("EyesAlpha"), RenderTarget);
// }

void AHeatmapReadyActor::SetupMaterials(UMaterialInterface* PaintBrushMaterialAsset)
{
	UStaticMeshComponent* StaticMeshComponent =
		Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));

	if (!StaticMeshComponent) return;

	CanvasInstances.Empty();

	PaintBrushMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, PaintBrushMaterialAsset);

	RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, 1024, 1024);

	TArray<UMaterialInterface*> CurrentMaterials = StaticMeshComponent->GetMaterials();

	const int64 MaterialsNum = Materials.Num();
	const int64 CurrentMaterialsNum = CurrentMaterials.Num();

	for (int64 i = 0; i < CurrentMaterialsNum; ++i)
	{
		UMaterialInterface* CurrentMaterial = CurrentMaterials[i];
		
		if (!CurrentMaterial || !CurrentMaterial->IsAsset())
			continue;
		
		if (MaterialsNum < i + 1)
			Materials.Add(CurrentMaterial);

		else Materials[i] = CurrentMaterial;
		
	}

	for (int64 i = 0; i < MaterialsNum; ++i)
	{
		if (!Materials[i]) continue;

		UMaterialInstanceDynamic* CanvasInstance =
			UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Materials[i]);

		StaticMeshComponent->SetMaterial(i, CanvasInstance);

		CanvasInstance->SetTextureParameterValue(FName("HeatmapAlpha"), RenderTarget);
		CanvasInstances.Add(CanvasInstance);
	}
}

void AHeatmapReadyActor::BeginPlay()
{
	Super::BeginPlay();
}

void AHeatmapReadyActor::ScalePaintBrush(FVector2D ScaleDivisor) const
{
	if (!PaintBrushMaterial) return;
	
	if (bOverrideScaleDivisor && ScaleDivisorOverride.X != 0.0 && ScaleDivisorOverride.Y != 0.0)
		ScaleDivisor = ScaleDivisorOverride;
	
	const FLinearColor NewScaleDivisorValue = FLinearColor(ScaleDivisor.X, ScaleDivisor.Y, 0.f, 1.f);

	PaintBrushMaterial->SetVectorParameterValue(FName("ScaleDivisor"), NewScaleDivisorValue);
}

void AHeatmapReadyActor::PaintHeatmap(const FVector2D UV)
{
	if (!PaintBrushMaterial)
	{
		DebugHeader::Print("PaintBrushMaterial invalid", FColor::Red, 5.f);
		return;
	}

	const FLinearColor NewPositionValue = FLinearColor(UV.X, UV.Y, 0.f, 2.f);

	PaintBrushMaterial->SetVectorParameterValue(FName("Position"), NewPositionValue);

	if (!RenderTarget) DebugHeader::Print("RenderTarget invalid", FColor::Red, 2.f);
	
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RenderTarget, PaintBrushMaterial);
}

TArray<UMaterialInterface*> AHeatmapReadyActor::GetMaterials()
{
	return Materials;
}

void AHeatmapReadyActor::SetEyesColor(const FColor Color)
{
	EyesColor = Color;
}

void AHeatmapReadyActor::SetFocussed(const bool bFocussed) const
{
	UStaticMeshComponent* StaticMeshComponent = GetComponentByClass<UStaticMeshComponent>();

	if (!StaticMeshComponent) return;

	StaticMeshComponent->SetRenderCustomDepth(bFocussed);
}

void AHeatmapReadyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHeatmapReadyActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHeatmapReadyActor, MetricsName))
	{
		if (MetricsName == "unset" || MetricsName.IsEmpty())
				DebugHeader::ShowNotifyInfo("Displaying Metrics requires a set name");
	}
}
