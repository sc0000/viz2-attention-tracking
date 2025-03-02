// Copyright (c) 2025 Sebastian Cyliax

#include "EyeTrackingCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "Recorder/TakeRecorderBlueprintLibrary.h"
#include "Camera/CameraComponent.h"

#include "HeatmapRT.h"
#include "HeatmapReadyActor.h"
#include "DebugHeader.h"
#include "AdditionalUtility.h"

AEyeTrackingCharacter::AEyeTrackingCharacter()
	:	TakeRecorderPanelReference(nullptr),
		bIsTracking(false),
		CurrentTimeStep(0.f),
		LastActorFocussed(nullptr),
		bVR(false),
		TrackingStartTime(0.f)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEyeTrackingCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (GEditor && GEditor->IsVRPreviewActive())
	{
		bVR = true;
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
	}

	else
	{
		bVR = false;
		bUseControllerRotationPitch = true;
		bUseControllerRotationYaw = true;
	}
}

void AEyeTrackingCharacter::GazeScreenToWorld(FVector& LineTraceStart, FVector& LineTraceEnd) const
{
	const FViewport* Viewport = GEngine->GameViewport->Viewport;

	if (!Viewport)
	{
		DebugHeader::ShowNotifyInfo("Viewport not valid");
		return;
	}

	const FVector2D ViewportSize = FVector2D(Viewport->GetSizeXY());

	const float ScreenCenterX = ViewportSize.X / 2.f;
	const float ScreenCenterY = ViewportSize.Y / 2.f;

	const float ScreenPositionX = ScreenCenterX + (ScreenCenterX * EyesOffset.Y * 1.2f); // Careful, magic numbers!
	const float ScreenPositionY = ScreenCenterY - (ScreenCenterY * EyesOffset.Z * 2.f);

	const FVector2D ScreenPosition = FVector2D(ScreenPositionX, ScreenPositionY);

	FVector WorldPosition;
	FVector WorldDirection;

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!PlayerController)
	{
		DebugHeader::ShowMsgDialog(OK, "Could not access PlayerController");
		return;
	}

	UGameplayStatics::DeprojectScreenToWorld(PlayerController, ScreenPosition, WorldPosition, WorldDirection);
	
	const APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	if (!PlayerCameraManager)
	{
		DebugHeader::ShowMsgDialog(OK, "Could not access PlayerCameraManager");
		return;
	}

	LineTraceStart = PlayerCameraManager->GetCameraLocation();
	LineTraceEnd = WorldPosition + WorldDirection * 50000.f;
}

void AEyeTrackingCharacter::SetTakeRecorderPanelReference()
{
	TakeRecorderPanelReference = UTakeRecorderBlueprintLibrary::GetTakeRecorderPanel();

	if (!TakeRecorderPanelReference)
		DebugHeader::ShowNotifyInfo("Take Recorder could not be referenced");
}

void AEyeTrackingCharacter::PaintHeatmap(uint8 UvChannel, const UCameraComponent* Camera)
{
	if (!bIsTracking || !Camera) return;
	
	if (LastActorFocussed)
		LastActorFocussed->SetFocussed(false);
	
	FVector LineTraceStart;
	FVector LineTraceEnd;

	if (bVR)
		GazeScreenToWorld(LineTraceStart, LineTraceEnd);

	else
	{
		LineTraceStart = Camera->GetComponentLocation();
		LineTraceEnd = LineTraceStart + Camera->GetForwardVector() * 10000.f;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

	FHitResult HitResult;

	if (!UKismetSystemLibrary::LineTraceSingleForObjects(this, LineTraceStart, LineTraceEnd,
		ObjectTypes, true, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true))
	{
		// DebugHeader::Print("Line Trace Failed", FColor::Red, 0.f);
		return;
	}
	
	AHeatmapReadyActor* ActorToPaint = Cast<AHeatmapReadyActor>(HitResult.HitObjectHandle.FetchActor());

	if (!ActorToPaint)
	{
		// DebugHeader::Print("HeatmapReadyActor invalid", FColor::Red, 0.f);
		return;
	}
	
	CurrentTimeStep = UGameplayStatics::GetTimeSeconds(this) - TrackingStartTime;

	FVector2D UvCoordinates;
	
	if (!UGameplayStatics::FindCollisionUV(HitResult, UvChannel, UvCoordinates))
	{
		DebugHeader::Print("FindCollisionUV() returned false", FColor::Red, 5.f);
		return;
	}

	// DebugHeader::Print(UvCoordinates.ToString(), FColor::Red, 0.f);
	
	FVector2D PaintBrushScaleDivisor = CalculateScaleDivisor(ActorToPaint, HitResult.ImpactNormal);
	
	ActorToPaint->ScalePaintBrush(PaintBrushScaleDivisor);
	ActorToPaint->PaintHeatmap(UvCoordinates);

	FAttentionTrackingDataPoint NewHeatmapDataPoint
	{
		FMath::Clamp<float>(CurrentTimeStep, 0.f, 1024.f),
		UKismetSystemLibrary::GetObjectName(ActorToPaint),
		UvCoordinates,
		PaintBrushScaleDivisor
	};
	
	HeatmapData.Add(NewHeatmapDataPoint);
}

/*
* TODO: Scaling should be calculated when loading, not when writing the heatmap data!
*/
FVector2D AEyeTrackingCharacter::CalculateScaleDivisor(AActor* HitActor, const FVector ImpactNormal)
{
	const UStaticMeshComponent* StaticMeshComponent =
		Cast<UStaticMeshComponent>(HitActor->FindComponentByClass(UStaticMeshComponent::StaticClass()));

	if (!StaticMeshComponent) return FVector2D(1.f, 1.f);

	FVector2D ScaleDivisor;

	FVector Min, Max;
	StaticMeshComponent->GetLocalBounds(Min, Max);

	// ::Print("Min: " + Min.ToString() + " Max: " + Max.ToString(), FColor::Yellow, 0.f);

	TArray<double> SortedBoundsAxes;
	UAdditionalUtility::GetAxesByLength(Max, SortedBoundsAxes);
	const float LongestAxis = SortedBoundsAxes[0] / 100.f;

	const FVector MeshScale = StaticMeshComponent->GetComponentScale() * LongestAxis;
	// const FVector MeshScale = Max * LongestAxis;
	
	const float ForwardDot = FMath::Abs(FVector::DotProduct(ImpactNormal, HitActor->GetActorForwardVector()));
	const float RightDot = FMath::Abs(FVector::DotProduct(ImpactNormal, HitActor->GetActorRightVector()));
	const float UpDot = FMath::Abs(FVector::DotProduct(ImpactNormal, HitActor->GetActorUpVector()));

	if (ForwardDot >= RightDot && ForwardDot >= UpDot)
		ScaleDivisor = FVector2D(MeshScale.Y, MeshScale.Z);
	
	else if (RightDot >= ForwardDot && RightDot >= UpDot)
		ScaleDivisor = FVector2D(MeshScale.X, MeshScale.Y);
	
	else if (UpDot >= ForwardDot && UpDot >= RightDot)
		ScaleDivisor = FVector2D(MeshScale.X, MeshScale.Y);
	
	// This should never happen...
	else ScaleDivisor = FVector2D(1.f, 1.f);
	
	return ScaleDivisor;
}

void AEyeTrackingCharacter::SetEyeTrackingState()
{
	bIsTracking = !bIsTracking;

	if (bIsTracking) HeatmapData.Empty();

	TrackingStartTime = UGameplayStatics::GetTimeSeconds(this);
}

void AEyeTrackingCharacter::PaintHeatmapDataPoint(const FAttentionTrackingDataPoint& DataPoint) const 
{
	UHeatmapRT::PaintHeatmapDataPoint(DataPoint, this);
}

void AEyeTrackingCharacter::FocusActor(const UCameraComponent* Camera)
{
	if (bIsTracking || !Camera) return;

	const FVector LineTraceStart = Camera->GetComponentLocation();
	const FVector LineTraceEnd = LineTraceStart + Camera->GetForwardVector() * 10000.f;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

	FHitResult HitResult;

	if (!UKismetSystemLibrary::LineTraceSingleForObjects(this, LineTraceStart, LineTraceEnd,
		ObjectTypes, true, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true))
	{
		if (LastActorFocussed)
			LastActorFocussed->SetFocussed(false);
		
		// DebugHeader::Print("Line Trace Failed", FColor::Red, 0.f);
		return;
	}
	
	AHeatmapReadyActor* FocussedActor = Cast<AHeatmapReadyActor>(HitResult.HitObjectHandle.FetchActor());

	if (!FocussedActor) return;

	if (LastActorFocussed)
		LastActorFocussed->SetFocussed(false);

	LastActorFocussed = FocussedActor;

	if (LastActorFocussed->bNeedsMetrics)
		LastActorFocussed->SetFocussed(true);
}

// Called every frame
void AEyeTrackingCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEyeTrackingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
