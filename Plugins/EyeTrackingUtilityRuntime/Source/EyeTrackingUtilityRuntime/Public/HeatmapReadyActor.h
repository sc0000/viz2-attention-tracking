// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeatmapReadyActor.generated.h"

UCLASS()
class EYETRACKINGUTILITYRUNTIME_API AHeatmapReadyActor : public AActor
{
	GENERATED_BODY()

public:
	AHeatmapReadyActor();

	// UFUNCTION(BlueprintCallable, Category = "Setup")
	// void Setup(UMaterialInterface* CanvasMaterialAsset, UMaterialInterface* PaintBrushMaterialAsset);

	UFUNCTION(BlueprintCallable, Category = "Setup")
	void SetupMaterials(UMaterialInterface* PaintBrushMaterialAsset);

	UFUNCTION(BlueprintCallable, Category = "PaintHeatmap")
	void ScalePaintBrush(FVector2D ScaleDivisor) const;

	UFUNCTION(BlueprintCallable, Category = "PaintHeatmap")
	void PaintHeatmap(const FVector2D UV);

	UFUNCTION(BlueprintCallable, Category = "Materials")
	TArray<UMaterialInterface*> GetMaterials();

	UFUNCTION(BlueprintCallable, Category = "PaintHeatmap")
	static void SetEyesColor(const FColor Color);

	UFUNCTION(BlueprintCallable, Category = "Metrics")
	void SetFocussed(bool bFocussed) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metrics")
	bool bNeedsMetrics = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metrics", meta = (EditCondition = "bNeedsMetrics"))
	FString MetricsName = "unset";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scale Divisor Override")
	bool bOverrideScaleDivisor = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scale Divisor Override", meta = (EditCondition = "bOverrideScaleDivisor"))
	FVector2D ScaleDivisorOverride = { 1.0, 1.0 };

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Mesh")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Canvas")
	UMaterialInstanceDynamic* Canvas;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Canvas")
	TArray<UMaterialInstanceDynamic*> CanvasInstances;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PaintBrushMaterial")
	UMaterialInstanceDynamic* PaintBrushMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PaintBrushMaterial")
	UTextureRenderTarget2D* RenderTarget;

private:
	UPROPERTY()
	TArray<UMaterialInterface*> Materials;

	static FColor EyesColor;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
