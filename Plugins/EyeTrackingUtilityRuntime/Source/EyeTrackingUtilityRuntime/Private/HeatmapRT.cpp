// Copyright (c) 2025 Sebastian Cyliax

#include "HeatmapRT.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

#include "HeatmapReadyActor.h"
#include "JsonParser.h"
#include "DebugHeader.h"

AHeatmapReadyActor* UHeatmapRT::LastActorPaintedOn = nullptr;
TArray<AHeatmapReadyActor*> UHeatmapRT::HeatmapReadyActors;
FString UHeatmapRT::LastSavedOrLoadedHeatmapFileName = "";
TArray<FAttentionTrackingDataPoint> UHeatmapRT::AttentionTrackingDataCurrentlyLoaded;
TMap<FString, FAttentionMetricsEntry> UHeatmapRT::AttentionMetrics;

TArray<FTimerHandle> UHeatmapRT::HeatmapTimerHandles;
TMap<FName, FTimerHandle> UHeatmapRT::BlendParameterTimerHandles; 

void UHeatmapRT::SaveHeatmap(const FString& FileName, const TArray<FAttentionTrackingDataPoint>& AttentionTrackingData)
{
	bool bOutSuccess = false;
	FString FilePath = UJsonParser::AttentionTrackingDataFolderPath();
	FilePath.Append(FileName);

	UJsonParser::WriteAttentionTrackingDataToJsonFile(AttentionTrackingData, FilePath, bOutSuccess);
	
	DebugHeader::ShowNotifyInfoIf(!bOutSuccess, "Failed to save heatmap to " + FilePath);
}

void UHeatmapRT::LoadHeatmap(const FString& FileName, const UObject* WorldContextObject, const float MetricsThreshold)
{
	bool bOutSuccess;
	FString FilePath = UJsonParser::AttentionTrackingDataFolderPath();
	FilePath.Append(FileName);
	
	UJsonParser::ReadAttentionTrackingDataFromJsonFile(FilePath,
		AttentionTrackingDataCurrentlyLoaded, bOutSuccess);

	if (!bOutSuccess)
	{
		DebugHeader::ShowNotifyInfo("Failed to load heatmap from " + FilePath);
		return;
	}

	TMap<FString, FString> MetricsNames;
	GetMetricsNames(WorldContextObject, MetricsNames);
	CalculateAttentionMetrics(MetricsNames, MetricsThreshold);
}

void UHeatmapRT::PaintLoadedHeatmap(UObject* WorldContextObject, const bool bLoadImmediately)
{
	if (!WorldContextObject)
	{
		DebugHeader::PrintError("UHeatmapRT::LoadHeatmap: WorldContextObject Ref invalid");
		return;
	}
	
	const UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		DebugHeader::PrintError("UHeatmapRT::LoadHeatmap: World Ref invalid");
		return;
	}

	const uint64 Num = AttentionTrackingDataCurrentlyLoaded.Num();

	if (Num == 0)
	{
		DebugHeader::ShowNotifyInfo("No heatmap loaded");
		return;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AHeatmapReadyActor::StaticClass(), OutActors);
	const uint64 NumActors = OutActors.Num();

	HeatmapReadyActors.Empty();
	HeatmapReadyActors.Reserve(NumActors);
	
	for (uint64 i = 0; i < NumActors; i++)
		HeatmapReadyActors.Push(Cast<AHeatmapReadyActor>(OutActors[i]));
	
	for (uint64 i = 0; i < Num; ++i)
	{
		if (bLoadImmediately)
		{
			PaintHeatmapDataPoint(AttentionTrackingDataCurrentlyLoaded[i], WorldContextObject);
		}

		else
		{
			const float DelayTime = AttentionTrackingDataCurrentlyLoaded[i].TimePassedSinceRecordingStarted;

			HeatmapTimerHandles.Emplace(FTimerHandle());
			FTimerDelegate TimerDelegate;

			TimerDelegate.BindLambda([=]()
			{
				PaintHeatmapDataPoint(AttentionTrackingDataCurrentlyLoaded[i], WorldContextObject);
			});
			
			World->GetTimerManager().SetTimer(HeatmapTimerHandles[i], TimerDelegate, DelayTime, false);
		}
	}

	LastActorPaintedOn = nullptr;
}

void UHeatmapRT::StopTimer(UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		DebugHeader::PrintError("UHeatmapRT::LoadHeatmap: World Ref invalid");
		return;
	}

	for (FTimerHandle TimerHandle : HeatmapTimerHandles)
	{
		if (TimerHandle.IsValid())
			World->GetTimerManager().ClearTimer(TimerHandle);
	}
}

FString UHeatmapRT::GetLastSavedOrLoadedHeatmapFileName()
{
	return LastSavedOrLoadedHeatmapFileName;
}

void UHeatmapRT::SetLastSavedOrLoadedHeatmapFileName(const FString& FileName)
{
	LastSavedOrLoadedHeatmapFileName = FileName;
}

void UHeatmapRT::ResetHeatmap(UObject* WorldContextObject, UMaterialInterface* PaintBrushMaterialAsset)
{
	if (!WorldContextObject)
	{
		DebugHeader::PrintError("UHeatmapRT::ResetHeatmap: WorldContextObject Ref invalid");
		return;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AHeatmapReadyActor::StaticClass(), OutActors);

	for (AActor* OutActor : OutActors)
	{
		AHeatmapReadyActor* HeatmapReadyActor = Cast<AHeatmapReadyActor>(OutActor);

		if (!HeatmapReadyActor) continue;

		HeatmapReadyActor->SetupMaterials(PaintBrushMaterialAsset);
	}
}

void UHeatmapRT::CalculateAttentionMetrics(const TMap<FString, FString>& MetricsNames, const float Threshold)
{
	AttentionMetrics.Empty();
	
	int AttentionSequenceIndex = 0;
	float CurrentAttentionTime = 0.f;
	float FirstAttentionTime = 0.f;
	
	const FAttentionTrackingDataPoint* PreviousDataPoint = nullptr;

	if (AttentionTrackingDataCurrentlyLoaded.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo("No heatmap loaded");
		return;
	}
	
	for (FAttentionTrackingDataPoint& DataPoint : AttentionTrackingDataCurrentlyLoaded)
	{
		if (!PreviousDataPoint)
		{
			PreviousDataPoint = &DataPoint;
			FirstAttentionTime = DataPoint.TimePassedSinceRecordingStarted;
			continue;
		}

		if (DataPoint.ObjectName == PreviousDataPoint->ObjectName)
		{
			CurrentAttentionTime +=
				DataPoint.TimePassedSinceRecordingStarted - PreviousDataPoint->TimePassedSinceRecordingStarted;

			PreviousDataPoint = &DataPoint;
			continue;
		}
		
		if (DataPoint.ObjectName != PreviousDataPoint->ObjectName)
		{
			const float FirstAttentionTimeToStore = FirstAttentionTime;
			FirstAttentionTime = DataPoint.TimePassedSinceRecordingStarted;
			
			const float CurrentAttentionTimeToStore = CurrentAttentionTime;
			CurrentAttentionTime = 0.f;

			// DebugHeader::PrintLog("CurrentAttentionTimeToStore: " + FString::SanitizeFloat(CurrentAttentionTimeToStore));
			
			if (CurrentAttentionTimeToStore < Threshold)
			{
				PreviousDataPoint = &DataPoint;
				continue;
			}
			
			const FString& ObjectName = PreviousDataPoint->ObjectName;

			if (!MetricsNames.Contains(ObjectName))
			{
				PreviousDataPoint = &DataPoint;
				continue;
			}

			const FString& MetricsName = MetricsNames[ObjectName];
			// DebugHeader::PrintLog("Metrics Name: " + MetricsName);
			
			if (!AttentionMetrics.Contains(MetricsName))
			{
				FAttentionMetricsEntry NewEntry = { CurrentAttentionTimeToStore,
					CurrentAttentionTimeToStore,
					FirstAttentionTimeToStore,
					1,
					{ AttentionSequenceIndex } };
				
				AttentionMetrics.Add(MetricsName, NewEntry);

				++AttentionSequenceIndex;
				PreviousDataPoint = &DataPoint;
				continue;
			}

			FAttentionMetricsEntry& CurrentEntry = AttentionMetrics[MetricsName];
			
			CurrentEntry.AttentionSequenceIndices.Add(AttentionSequenceIndex);
			CurrentEntry.TotalAttentionTime += CurrentAttentionTimeToStore;
			CurrentEntry.AverageAttentionTime =
				CurrentEntry.TotalAttentionTime / static_cast<float>(CurrentEntry.AttentionSequenceIndices.Num());
			++CurrentEntry.TimesFocussed;
			++AttentionSequenceIndex;
			
			PreviousDataPoint = &DataPoint;
		}
	}

	// Update the last focused object if PreviousDataPoint is valid
	if (PreviousDataPoint)
	{
		const float CurrentAttentionTimeToStore = CurrentAttentionTime;
		// DebugHeader::PrintLog("CurrentAttentionTimeToStore: " + FString::SanitizeFloat(CurrentAttentionTimeToStore));
	
		const FString& ObjectName = PreviousDataPoint->ObjectName;
		
		if (!MetricsNames.Contains(ObjectName)) return;
		
		const FString& MetricsName = MetricsNames[ObjectName];

		if (!AttentionMetrics.Contains(MetricsName))
		{
			FAttentionMetricsEntry NewEntry = { CurrentAttentionTime,
				CurrentAttentionTimeToStore,
				FirstAttentionTime,
				1,
				{ AttentionSequenceIndex } };
				
			AttentionMetrics.Add(MetricsName, NewEntry);
			++AttentionSequenceIndex;
			return;
		}
		
		FAttentionMetricsEntry& CurrentEntry = AttentionMetrics[MetricsName];
		CurrentEntry.AttentionSequenceIndices.Add(AttentionSequenceIndex);
		CurrentEntry.TotalAttentionTime += CurrentAttentionTimeToStore;
		CurrentEntry.AverageAttentionTime =
			CurrentEntry.TotalAttentionTime / static_cast<float>(CurrentEntry.AttentionSequenceIndices.Num());
		++CurrentEntry.TimesFocussed;
	}
}

void UHeatmapRT::GetAttentionTrackingDataCurrentlyLoaded(TArray<FAttentionTrackingDataPoint>& OutData)
{
	OutData = AttentionTrackingDataCurrentlyLoaded;
}

void UHeatmapRT::GetAttentionMetrics(TMap<FString, FAttentionMetricsEntry>& OutMetrics)
{
	OutMetrics = AttentionMetrics;
}

void UHeatmapRT::SortAttentionMetrics(ESortMode SortMode, bool bAscending)
{
	TArray<TPair<FString, FAttentionMetricsEntry>> Array = AttentionMetrics.Array();

	Array.Sort([SortMode, bAscending](const TPair<FString, FAttentionMetricsEntry>& A,
		const TPair<FString, FAttentionMetricsEntry>& B)
	{
		switch (SortMode)
		{
		case ESortMode::ESM_Name:
			return bAscending ? A.Key < B.Key : A.Key > B.Key;

		case ESortMode::ESM_First:
			return bAscending ?
				A.Value.FirstAttentionAfter < B.Value.FirstAttentionAfter :
					A.Value.FirstAttentionAfter > B.Value.FirstAttentionAfter;
			
		case ESortMode::ESM_Total:
			return bAscending ?
				A.Value.TotalAttentionTime < B.Value.TotalAttentionTime :
					A.Value.TotalAttentionTime > B.Value.TotalAttentionTime;

		case ESortMode::ESM_Times:
			return bAscending ?
				A.Value.TimesFocussed < B.Value.TimesFocussed :
					A.Value.TimesFocussed > B.Value.TimesFocussed;
		
		case ESortMode::ESM_Average:
			return bAscending ?
				A.Value.AverageAttentionTime < B.Value.AverageAttentionTime :
					A.Value.AverageAttentionTime > B.Value.AverageAttentionTime;

		case ESortMode::ESM_MAX:
			return true;
		}
		
		return true;
	});

	AttentionMetrics.Empty();

	for (const TPair<FString, FAttentionMetricsEntry>& Entry : Array)
		AttentionMetrics.Add(Entry.Key, Entry.Value);
}

void UHeatmapRT::PaintHeatmapDataPoint(const FAttentionTrackingDataPoint& DataPoint,
                                       const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		DebugHeader::ShowNotifyInfo("From UHeatmapRT::PaintHeatmapDataPoint(): World invalid");
		return;
	}
	
	if (LastActorPaintedOn && DataPoint.ObjectName == UKismetSystemLibrary::GetObjectName(LastActorPaintedOn))
	{
		LastActorPaintedOn->ScalePaintBrush(DataPoint.PaintBrushScaleDivisor);
		LastActorPaintedOn->PaintHeatmap(DataPoint.Coordinates);
	}
	
	else
	{
		for (AHeatmapReadyActor* Actor : HeatmapReadyActors)
		{
			if (!Actor || DataPoint.ObjectName != UKismetSystemLibrary::GetObjectName(Actor))
				continue;

			LastActorPaintedOn = Actor;
				
			if (!LastActorPaintedOn)
				continue;

			LastActorPaintedOn->ScalePaintBrush(DataPoint.PaintBrushScaleDivisor);
			LastActorPaintedOn->PaintHeatmap(DataPoint.Coordinates);

			break;
		}
	}
}

void UHeatmapRT::BlendMaterialParameter(const UObject* WorldContextObject, const UMaterialParameterCollection* ParameterCollection, const FName ParameterName)
{
	if (!WorldContextObject || !ParameterCollection) return;
	
	const UWorld* World = WorldContextObject->GetWorld();

	if (!World) return;
	
	UMaterialParameterCollectionInstance* ParameterCollectionInstance = World->GetParameterCollectionInstance(ParameterCollection); 
	
	if (!ParameterCollectionInstance) return;

	float CurrentValue = 0.f;
	ParameterCollectionInstance->GetScalarParameterValue(ParameterName, CurrentValue);

	const float TargetValue = UKismetMathLibrary::Round(1.f - CurrentValue);

	FTimerHandle& TimerHandle = BlendParameterTimerHandles.FindOrAdd(ParameterName);

	if (World->GetTimerManager().IsTimerActive(TimerHandle))
		World->GetTimerManager().ClearTimer(TimerHandle);
	
	FTimerDelegate TimerDelegate;
	
	TimerDelegate.BindLambda([World, TimerHandle, ParameterCollectionInstance, ParameterName, CurrentValue, TargetValue]() mutable
	{
		const float NewValue = FMath::Lerp(CurrentValue, TargetValue, 0.1f);
		ParameterCollectionInstance->SetScalarParameterValue(ParameterName, NewValue);

		if (FMath::IsNearlyEqual(NewValue, TargetValue, 0.01f))
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
			ParameterCollectionInstance->SetScalarParameterValue(ParameterName, UKismetMathLibrary::Round(TargetValue));

			FConfigData ConfigData;
			bool bOutSuccess = false;

			ParameterCollectionInstance->GetScalarParameterValue(FName("ShowTextures"), ConfigData.ShowTextures);
			ParameterCollectionInstance->GetScalarParameterValue(FName("ShowHeatmap"), ConfigData.ShowHeatmap);

			UJsonParser::WriteConfigDataToJsonFile(ConfigData, UJsonParser::ConfigDataFilePath(), bOutSuccess);
		}
		
		else CurrentValue = NewValue;
	});
			
	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.01f, true);
}

void UHeatmapRT::ClearAllTimers(const bool bOnEndPieMode)
{
	const UWorld* World = bOnEndPieMode ? GEditor->PlayWorld : GEditor->EditorWorld;
	
	if (!World) return;
	
	for (auto Timer : BlendParameterTimerHandles)
	{
		if (World->GetTimerManager().IsTimerActive(Timer.Value))
			World->GetTimerManager().ClearTimer(Timer.Value);
	}			
}

void UHeatmapRT::LogAttentionMetrics()
{
	static int TimesCalled = 0;
	++TimesCalled;

	FString ToLog = "Times called: " + FString::FromInt(TimesCalled);
	
	if (AttentionMetrics.Num() == 0)
	{
		ToLog.Append(" Metrics array empty");
		DebugHeader::PrintLog(ToLog);
		return;
	}
	
	for (const auto Entry : AttentionMetrics)
	{
		ToLog.Append(" " + Entry.Key);
	}

	DebugHeader::PrintLog(ToLog);
}

void UHeatmapRT::GetMetricsNames(const UObject* WorldContextObject, TMap<FString, FString>& OutMetricsNames)
{
	if (!WorldContextObject)
	{
		DebugHeader::PrintLog("UHeatmapRT::GetMetricsNames(): WorldContextObject invalid");
		return;
	}

	// Use a local temporary map to collect data
	TMap<FString, FString> LocalMetricsNames;

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AHeatmapReadyActor::StaticClass(), OutActors);

	for (AActor* OutActor : OutActors)
	{
		if (!OutActor)
		{
			DebugHeader::PrintLog("UHeatmapRT::GetMetricsNames(): OutActor invalid");
			continue;
		}

		const AHeatmapReadyActor* HeatmapReadyActor = Cast<AHeatmapReadyActor>(OutActor);

		if (!HeatmapReadyActor || !HeatmapReadyActor->bNeedsMetrics)
			continue;

		const FString& ObjectName = UKismetSystemLibrary::GetObjectName(HeatmapReadyActor);
		const FString& MetricsName = HeatmapReadyActor->MetricsName;

		if (MetricsName.IsEmpty() || MetricsName == "unset")
			continue;
		
		LocalMetricsNames.Add(ObjectName, MetricsName);
	}
	
	OutMetricsNames = LocalMetricsNames;
}
