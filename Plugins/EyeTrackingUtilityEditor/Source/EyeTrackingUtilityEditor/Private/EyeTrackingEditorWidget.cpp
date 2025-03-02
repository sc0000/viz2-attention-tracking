// Copyright (c) 2023 Sebastian Cyliax


#include "EyeTrackingEditorWidget.h"

#include "FileHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "PhysicsEngine/BodySetup.h"
#include "EyeTrackingUtilityEditor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ComboBoxString.h"

#include "HeatmapRT.h"
#include "HeatmapReadyActor.h"
#include "DebugHeader.h"

UEditorActorSubsystem* UEyeTrackingEditorWidget::EditorActorSubsystem = nullptr;

TArray<AStaticMeshActor*> UEyeTrackingEditorWidget::GetAllStaticMeshActorsInLevel()
{
	TArray<AStaticMeshActor*> AllStaticMeshActorsInLevel;

	if (!GetEditorActorSubsystem())
		return AllStaticMeshActorsInLevel;

	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();

	for (AActor* LevelActor : AllLevelActors)
	{
		if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(LevelActor))
			AllStaticMeshActorsInLevel.Add(StaticMeshActor);
	}

	return AllStaticMeshActorsInLevel;
}

TArray<AHeatmapReadyActor*> UEyeTrackingEditorWidget::GetAllHeatmapReadyActorsInLevel()
{
	TArray<AHeatmapReadyActor*> AllHeatmapReadyActorsInLevel;

	if (!GetEditorActorSubsystem())
		return AllHeatmapReadyActorsInLevel;

	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	
	for (AActor* LevelActor : AllLevelActors)
	{
		AHeatmapReadyActor* StaticMeshActor = Cast<AHeatmapReadyActor>(LevelActor);

		if (StaticMeshActor) AllHeatmapReadyActorsInLevel.Add(StaticMeshActor);
	}

	return AllHeatmapReadyActorsInLevel;
}

void UEyeTrackingEditorWidget::ReplaceStaticMeshActorWithHeatmapReadyActor(AStaticMeshActor* StaticMeshActor,
	const TSubclassOf<AHeatmapReadyActor> HeatmapReadyActorClass)
{
	if (!StaticMeshActor) return;

	const UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();

	if (!StaticMeshComponent) return;

	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();

	TArray<UMaterialInterface*> Materials = StaticMeshComponent->GetMaterials();
	
	UWorld* World = GEditor->GetEditorWorldContext().World();
	
	if (!StaticMesh || !World) return;

	const AHeatmapReadyActor* HeatmapReadyActor =
		World->SpawnActor<AHeatmapReadyActor>(HeatmapReadyActorClass, StaticMeshActor->GetActorLocation(),
			StaticMeshActor->GetActorRotation());

	UStaticMeshComponent* HeatmapReadyActorMeshComponent =
		Cast<UStaticMeshComponent>(HeatmapReadyActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

	if (!HeatmapReadyActorMeshComponent) return;
	
	HeatmapReadyActorMeshComponent->SetStaticMesh(StaticMesh);
	HeatmapReadyActorMeshComponent->SetWorldScale3D(StaticMeshActor->GetStaticMeshComponent()->GetComponentScale());

	const int32 MaterialsNum = Materials.Num();
	
	for (size_t i = 0; i < MaterialsNum; ++i)
		HeatmapReadyActorMeshComponent->SetMaterial(i, Materials[i]);

	StaticMeshActor->Destroy();
}

void UEyeTrackingEditorWidget::ReplaceHeatmapReadyActorWithStaticMeshActor(AHeatmapReadyActor* HeatmapReadyActor)
{
	if (!HeatmapReadyActor) return;
	
	UWorld* World = GEditor->GetEditorWorldContext().World();

	const UStaticMeshComponent* StaticMeshComponent =
		Cast<UStaticMeshComponent>(HeatmapReadyActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

	if (!StaticMeshComponent || !World) return;
		
	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();

	if (!StaticMesh) return;

	TArray<UMaterialInterface*> CurrentMaterials = StaticMeshComponent->GetMaterials();
	TArray<UMaterialInterface*> StoredMaterials = HeatmapReadyActor->GetMaterials();

	const FTransform ActorTransform = HeatmapReadyActor->GetActorTransform();

	AActor* SpawnedActor =
		World->SpawnActor(AStaticMeshActor::StaticClass(), &ActorTransform);

	const AStaticMeshActor* SpawnedStaticMeshActor = Cast<AStaticMeshActor>(SpawnedActor);

	if (!SpawnedStaticMeshActor) return;

	UStaticMeshComponent* SpawnedStaticMeshComponent = SpawnedStaticMeshActor->GetStaticMeshComponent();

	if (!SpawnedStaticMeshComponent) return;

	SpawnedStaticMeshComponent->SetStaticMesh(StaticMesh);

	for (size_t i = 0; i < CurrentMaterials.Num(); ++i)
	{
		UMaterialInterface* CurrentMaterial = CurrentMaterials[i];
		UMaterialInterface* StoredMaterial = StoredMaterials.Num() > i ? StoredMaterials[i] : nullptr;
		
		if (CurrentMaterial->IsAsset())
		{
			SpawnedStaticMeshComponent->SetMaterial(i, CurrentMaterial);
			continue;
		}
		
		if (StoredMaterial && StoredMaterial->IsAsset())
		{
			SpawnedStaticMeshComponent->SetMaterial(i, StoredMaterial);
			continue;
		}

		DebugHeader::ShowNotifyInfo("No valid material found for slot" + FString::FromInt(i));
	}

	World->DestroyActor(HeatmapReadyActor);
}

void UEyeTrackingEditorWidget::NativeConstruct()
{
	// FEyeTrackingUtilityEditorModule::OnStaticMeshAddedToScene.AddUObject(this, 
	// 	&UEyeTrackingEditorWidget::OnStaticMeshAddedToSceneTriggered);
	//
	FEyeTrackingUtilityEditorModule::OnExitVrPreview.AddUObject(this, 
		&UEyeTrackingEditorWidget::OnExitVrPreview);
}

void UEyeTrackingEditorWidget::PrepareAllStaticMeshActorsInLevel(const TSubclassOf<AHeatmapReadyActor> HeatmapReadyActorClass)
{
	TArray<AStaticMeshActor*> StaticMeshActors = GetAllStaticMeshActorsInLevel();

	for (AStaticMeshActor* StaticMeshActor : StaticMeshActors)
	{
		ReplaceStaticMeshActorWithHeatmapReadyActor(StaticMeshActor, HeatmapReadyActorClass);
	}
}

void UEyeTrackingEditorWidget::PrepareSelectedStaticMeshActors(const TSubclassOf<AHeatmapReadyActor> HeatmapReadyActorClass)
{
	if (!GetEditorActorSubsystem())
		return;
	
	TArray<AActor*> SelectedLevelActors = EditorActorSubsystem->GetSelectedLevelActors();

	for (AActor* SelectedLevelActor : SelectedLevelActors)
	{
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(SelectedLevelActor);
		ReplaceStaticMeshActorWithHeatmapReadyActor(StaticMeshActor, HeatmapReadyActorClass);
	}
}

void UEyeTrackingEditorWidget::RevertSelectedHeatmapReadyActors()
{
	if (!GetEditorActorSubsystem())
		return;
	
	TArray<AActor*> SelectedLevelActors = EditorActorSubsystem->GetSelectedLevelActors();

	for (AActor* SelectedLevelActor : SelectedLevelActors)
	{
		AHeatmapReadyActor* HeatmapReadyActor = Cast<AHeatmapReadyActor>(SelectedLevelActor);
		ReplaceHeatmapReadyActorWithStaticMeshActor(HeatmapReadyActor);
	}
}

/*
* This is a very hacky solution for broken Rhino imports...
*/
void UEyeTrackingEditorWidget::RemoveBrokenUvChannel()
{
	if (!GetEditorActorSubsystem())
		return;
	
	TArray<AActor*> SelectedLevelActors = EditorActorSubsystem->GetSelectedLevelActors();

	for (const AActor* SelectedLevelActor : SelectedLevelActors)
	{
		const UStaticMeshComponent* StaticMeshComponent = 
			Cast<UStaticMeshComponent>(SelectedLevelActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (!StaticMeshComponent) return;

		TObjectPtr<UStaticMesh> StaticMesh = StaticMeshComponent->GetStaticMesh();

		if (!StaticMesh) return;

		uint8 NumChannels = StaticMesh->GetNumUVChannels(0);

		if (NumChannels > 1)
			StaticMesh->RemoveUVChannel(0, 0);
	}
}

void UEyeTrackingEditorWidget::RemoveBrokenUvChannelFromAllHeatmapReadyActors()
{
	TArray<AHeatmapReadyActor*> HeatmapReadyActors = GetAllHeatmapReadyActorsInLevel();

	for (AHeatmapReadyActor* HeatmapReadyActor : HeatmapReadyActors)
	{
		const UStaticMeshComponent* StaticMeshComponent =
			Cast<UStaticMeshComponent>(HeatmapReadyActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (!StaticMeshComponent) return;

		UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();

		if (!StaticMesh) return;

		uint8 NumChannels = StaticMesh->GetNumUVChannels(0);

		if (NumChannels > 1)
			StaticMesh->RemoveUVChannel(0, 0);
	}
}

void UEyeTrackingEditorWidget::LoadHeatmapRT(const FString& FileName, const float MetricsThreshold)
{
	if (!GEditor) return;
	
	UHeatmapRT::LoadHeatmap(FileName, GEditor->GetEditorWorldContext().World(), MetricsThreshold);
}

void UEyeTrackingEditorWidget::ResetHeatmapRT(UMaterialInterface* PaintBrushMaterialAsset)
{
	if (!GEditor) return;
	
	UHeatmapRT::ResetHeatmap(GEditor->GetEditorWorldContext().World(), PaintBrushMaterialAsset);
}

void UEyeTrackingEditorWidget::SetupColorsComboBox(UComboBoxString* ColorsComboBox, const TMap<FString, FColor>& Colors)
{
	if (!ColorsComboBox) return;

	for (const TTuple<FString, FColor>& Color : Colors)
		ColorsComboBox->AddOption(Color.Key);
}

void UEyeTrackingEditorWidget::SetEyesColor(const TMap<FString, FColor>& Colors, FString ColorName)
{
	for (TTuple<FString, FColor> Color : Colors)
	{
		if (Color.Key == ColorName)
		{
			AHeatmapReadyActor::SetEyesColor(Color.Value);
			return;
		}
	}
}

void UEyeTrackingEditorWidget::SetMaterialForActors(const TArray<AActor*> Actors, UMaterialInterface* Material)
{
	for (const AActor* Actor : Actors)
	{
		if (!Actor) continue;

		UStaticMeshComponent* StaticMeshComponent =
			Cast<UStaticMeshComponent>(Actor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (!StaticMeshComponent) continue;

		for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); ++i)
			StaticMeshComponent->SetMaterial(i, Material);
	}
}

void UEyeTrackingEditorWidget::SetMaterialForSelectedActors(UMaterialInterface* Material)
{
	if (!GetEditorActorSubsystem())
		return;
	
	const TArray<AActor*> SelectedLevelActors = EditorActorSubsystem->GetSelectedLevelActors();

	SetMaterialForActors(SelectedLevelActors, Material);
}

void UEyeTrackingEditorWidget::SetMaterialForHeatmapReadyActors(const UObject* WorldContextObject,
	UMaterialInterface* Material)
{
	if (!WorldContextObject)
	{
		DebugHeader::PrintError("UEyeTrackingEditorWidget::SetMaterialForHeatmapReadyActors: "
						  "WorldContextObject Ref invalid");
		return;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AHeatmapReadyActor::StaticClass(), OutActors);

	SetMaterialForActors(OutActors, Material);
}

void UEyeTrackingEditorWidget::SetMaterialForStaticMeshActors(const UObject* WorldContextObject,
	UMaterialInterface* Material)
{
	if (!WorldContextObject)
	{
		DebugHeader::PrintError("UEyeTrackingEditorWidget::SetMaterialForStaticMeshActors: "
						  "WorldContextObject Ref invalid");
		return;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AStaticMeshActor::StaticClass(), OutActors);

	SetMaterialForActors(OutActors, Material);
}

void UEyeTrackingEditorWidget::DisableCpuAccessForAllMeshes()
{
	if (!GetEditorActorSubsystem())
		return;
	
	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();

	for (const AActor* LevelActor : AllLevelActors)
	{
		const UStaticMeshComponent* StaticMeshComponent = LevelActor->GetComponentByClass<UStaticMeshComponent>();

		if (!StaticMeshComponent) continue;

		UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();

		if (!StaticMesh) continue;

		StaticMesh->bAllowCPUAccess = false;
	}
}

void UEyeTrackingEditorWidget::RestoreMaterialsForHeatmapReadyActors()
{
	if (!GetEditorActorSubsystem())
		return;
	
	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();

	for (AActor* LevelActor : AllLevelActors)
	{
		if (!LevelActor) continue;

		AHeatmapReadyActor* HeatmapReadyActor = Cast<AHeatmapReadyActor>(LevelActor);

		if (!HeatmapReadyActor) continue;

		UStaticMeshComponent* StaticMeshComponent =
			Cast<UStaticMeshComponent>(HeatmapReadyActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (!StaticMeshComponent) continue;

		const int32 NumMaterials = StaticMeshComponent->GetNumMaterials();
		const TArray<UMaterialInterface*> Materials = HeatmapReadyActor->GetMaterials();

		for (uint8 i = 0; i < NumMaterials; ++i)
		{
			if (Materials.Num() < i + 1 || !Materials[i]) continue;

			StaticMeshComponent->SetMaterial(i, Materials[i]);
		}
	}

	FEditorFileUtils::SaveCurrentLevel();
}

void UEyeTrackingEditorWidget::EnableComplexCollisionForSelectedStaticMeshActors()
{
	if (!GetEditorActorSubsystem())
		return;
	
	TArray<AActor*> SelectedLevelActors = EditorActorSubsystem->GetSelectedLevelActors();

	for (const AActor* SelectedLevelActor : SelectedLevelActors)
	{
		UStaticMeshComponent* SelectedActorMeshComponent =
			Cast<UStaticMeshComponent>(SelectedLevelActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (!SelectedActorMeshComponent) continue;

		EnableComplexCollision(SelectedActorMeshComponent);
	}
}

// TODO: Remove!
void UEyeTrackingEditorWidget::EnableComplexCollisionForAllStaticMeshActors()
{
	if (DebugHeader::ShowMsgDialog(OK_CANCEL,
		TEXT("This action can't be undone! Continue?")) == EAppReturnType::Cancel)
		return;

	TArray<AStaticMeshActor*> StaticMeshActors = GetAllStaticMeshActorsInLevel();

	for (const AStaticMeshActor* StaticMeshActor : StaticMeshActors)
	{
		UStaticMeshComponent* SelectedActorMeshComponent =
			Cast<UStaticMeshComponent>(StaticMeshActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (!SelectedActorMeshComponent) continue;

		EnableComplexCollision(SelectedActorMeshComponent);
	}
}

void UEyeTrackingEditorWidget::UpdateScrollBoxHighlight_Implementation()
{
}

bool UEyeTrackingEditorWidget::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();

	return EditorActorSubsystem != nullptr;
}

TArray<AActor*> UEyeTrackingEditorWidget::GetSelectedLevelActors()
{
	TArray<AActor*> SelectedLevelActors;
	
	if (!GetEditorActorSubsystem())
		return SelectedLevelActors;
	
	 SelectedLevelActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedLevelActors.Num() == 0)
		DebugHeader::ShowMsgDialog(OK, TEXT("No Actor selected"));

	return SelectedLevelActors;
}

void UEyeTrackingEditorWidget::PrepareStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent)
{
	if (!StaticMeshComponent) return;

	const TObjectPtr<UStaticMesh> StaticMesh = StaticMeshComponent->GetStaticMesh();
	
	if (!StaticMesh) return;

	StaticMesh->bAllowCPUAccess = true;

	const UBodySetup* BodySetup = StaticMesh->GetBodySetup();

	if (!BodySetup) return;

	const bool bHasSimpleCollision = BodySetup->AggGeom.GetElementCount() != 0;

	if (!bHasSimpleCollision) EnableComplexCollision(StaticMeshComponent);
}

void UEyeTrackingEditorWidget::EnableComplexCollision(UStaticMeshComponent* StaticMeshComponent)
{
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	StaticMeshComponent->GetBodySetup()->CollisionTraceFlag = CTF_UseComplexAsSimple;
}
