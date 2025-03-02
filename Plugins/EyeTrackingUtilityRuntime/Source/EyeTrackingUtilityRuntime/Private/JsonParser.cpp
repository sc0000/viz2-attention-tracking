// Copyright (c) 2025 Sebastian Cyliax

#include "JsonParser.h"

#include "HeatmapRT.h"
#include "DebugHeader.h"

void UJsonParser::ReadAttentionTrackingDataFromJsonFile(const FString& FilePath, TArray<FAttentionTrackingDataPoint>& AttentionTrackingData, 
	bool& bOutSuccess)
{
	AttentionTrackingData.Empty();
	
	const FString JsonString = ReadStringFromFile(FilePath, bOutSuccess);
	TArray<TSharedPtr<FJsonValue>> JsonRootArray;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonRootArray))
	{
		DebugHeader::ShowMsgDialog(OK, TEXT("JSON deserialization failed: " + FilePath));

		bOutSuccess = false;
		return;
	}

	uint32 Index = 0;

	for (const TSharedPtr<FJsonValue>& Entry : JsonRootArray)
	{
		const TSharedPtr<FJsonObject> JsonObject = Entry->AsObject();

		FAttentionTrackingDataPoint RenderTargetCoordinatesData;

		double OutNumber;

		if (!JsonObject->TryGetNumberField("TimePassedSinceRecordingStarted", OutNumber))
		{
			DebugHeader::ShowNotifyInfo(TEXT("Retrieval of TimePassedSinceRecordingStarted field failed at index "
				+ FString::FromInt(Index)));
			continue;
		}

		RenderTargetCoordinatesData.TimePassedSinceRecordingStarted = static_cast<float>(OutNumber);

		if (!JsonObject->TryGetStringField("ObjectName", RenderTargetCoordinatesData.ObjectName))
		{
			DebugHeader::ShowNotifyInfo(TEXT("Retrieval of ObjectName field failed at index "
				+ FString::FromInt(Index)));
			continue;
		}

		if (!JsonObject->TryGetNumberField("U", OutNumber))
		{
			DebugHeader::ShowNotifyInfo(TEXT("Retrieval of UVX field failed at index "
				+ FString::FromInt(Index)));
			continue;
		}

		RenderTargetCoordinatesData.Coordinates.X = static_cast<float>(OutNumber);

		if (!JsonObject->TryGetNumberField("V", OutNumber))
		{
			DebugHeader::ShowNotifyInfo(TEXT("Retrieval of UVY field failed at index "
				+ FString::FromInt(Index)));
			continue;
		}

		RenderTargetCoordinatesData.Coordinates.Y = static_cast<float>(OutNumber);

		if (!JsonObject->TryGetNumberField("PaintBrushScaleDivisorX", OutNumber))
		{
			DebugHeader::ShowNotifyInfo(TEXT("Retrieval of PaintBrushScaleDivisorX field failed at index "
				+ FString::FromInt(Index)));
			continue;
		}

		RenderTargetCoordinatesData.PaintBrushScaleDivisor.X = static_cast<float>(OutNumber);

		if (!JsonObject->TryGetNumberField("PaintBrushScaleDivisorY", OutNumber))
		{
			DebugHeader::ShowNotifyInfo(TEXT("Retrieval of PaintBrushScaleDivisorY field failed at index "
				+ FString::FromInt(Index)));
			continue;
		}

		RenderTargetCoordinatesData.PaintBrushScaleDivisor.Y = static_cast<float>(OutNumber);

		AttentionTrackingData.Add(RenderTargetCoordinatesData);

		++Index;
	}
}

void UJsonParser::WriteAttentionTrackingDataToJsonFile(const TArray<FAttentionTrackingDataPoint>& AttentionTrackingDataArray,
                                                       const FString& FilePath, bool& bOutSuccess)
{
	TArray<TSharedPtr<FJsonValue>> RootArray;
	
	for (const FAttentionTrackingDataPoint& AttentionTrackingDataPoint : AttentionTrackingDataArray)
	{
		const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetNumberField("TimePassedSinceRecordingStarted", AttentionTrackingDataPoint.TimePassedSinceRecordingStarted);
		JsonObject->SetStringField("ObjectName", AttentionTrackingDataPoint.ObjectName);
		JsonObject->SetNumberField("U", AttentionTrackingDataPoint.Coordinates.X);
		JsonObject->SetNumberField("V", AttentionTrackingDataPoint.Coordinates.Y);
		JsonObject->SetNumberField("PaintBrushScaleDivisorX", AttentionTrackingDataPoint.PaintBrushScaleDivisor.X);
		JsonObject->SetNumberField("PaintBrushScaleDivisorY", AttentionTrackingDataPoint.PaintBrushScaleDivisor.Y);

		TSharedPtr<FJsonValueObject> RtcValueObject = MakeShareable(new FJsonValueObject(JsonObject));
		RootArray.Add(RtcValueObject);
	}

	WriteJson(RootArray, FilePath, bOutSuccess);
	DebugHeader::Print(FilePath + ": " + FString::FromInt(bOutSuccess), FColor::Red, 5.f);
}

FString UJsonParser::AttentionTrackingDataFolderPath()
{
	return FPaths::ProjectDir() + "Data/";
}

void UJsonParser::WriteAttentionMetricsToJsonFile(const TMap<FString, FAttentionMetricsEntry>& AttentionMetrics,
                                                  const FString& FilePath, bool& bOutSuccess)
{
	TArray<TSharedPtr<FJsonValue>> RootArray;
	
	for (const TPair<FString, FAttentionMetricsEntry>& Entry : AttentionMetrics)
	{
		const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

		JsonObject->SetStringField("ObjectName", Entry.Key);
		JsonObject->SetNumberField("TotalAttentionTime", Entry.Value.TotalAttentionTime);
		JsonObject->SetNumberField("AverageAttentionTime", Entry.Value.AverageAttentionTime);

		TArray<TSharedPtr<FJsonValue>> AttentionSequenceIndices;
		
		for (const int Index : Entry.Value.AttentionSequenceIndices)
		{
			TSharedPtr<FJsonValue> JsonValue =
				MakeShareable(new FJsonValueNumber(static_cast<double>(Index)));
			
			AttentionSequenceIndices.Add(JsonValue);
		}

		JsonObject->SetArrayField("AttentionSequenceIndices", AttentionSequenceIndices);

		TSharedPtr<FJsonValueObject> RtcValueObject = MakeShareable(new FJsonValueObject(JsonObject));
		RootArray.Add(RtcValueObject);
	}

	WriteJson(RootArray, FilePath, bOutSuccess);
}
 
FString UJsonParser::AttentionMetricsFolderPath()
{
	return FPaths::ProjectDir() + "Metrics/";
}

FConfigData UJsonParser::ReadConfigDataFromJsonFile(const FString& FilePath, bool& bOutSuccess)
{
	const FString JsonString = ReadStringFromFile(FilePath, bOutSuccess);
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	TSharedPtr<FJsonValue> ReadJsonValue;

	if (!FJsonSerializer::Deserialize(Reader, ReadJsonValue))
	{
		DebugHeader::ShowMsgDialog(OK, TEXT("JSON deserialization failed: " + FilePath));

		bOutSuccess = false;
		return FConfigData{ 0.f, 0.f };
	}

	const TSharedPtr<FJsonObject> JsonObject = ReadJsonValue->AsObject();

	FConfigData ConfigData;

	if (!JsonObject->TryGetNumberField("ShowHeatmap", ConfigData.ShowHeatmap) ||
		!JsonObject->TryGetNumberField("ShowTextures", ConfigData.ShowTextures))
	{
		DebugHeader::ShowMsgDialog(OK,
			TEXT("Retrieval of one or more material overlay config entries failed: " + FilePath));

		return FConfigData{ false, false };
	}

	// DebugHeader::ShowNotifyInfo(TEXT("Successfully read material overlay config: " + FilePath));

	return ConfigData;
}

void UJsonParser::WriteConfigDataToJsonFile(const FConfigData ConfigData,
	const FString& FilePath, bool& bOutSuccess)
{
	const TSharedPtr<FJsonObject> MocJsonObject = MakeShareable(new FJsonObject);

	MocJsonObject->SetNumberField("ShowHeatmap", ConfigData.ShowHeatmap);
	MocJsonObject->SetNumberField("ShowTextures", ConfigData.ShowTextures);

	WriteJson(MocJsonObject, FilePath, bOutSuccess);
}

FString UJsonParser::ConfigDataFilePath()
{
	return FPaths::ProjectDir() + "Config_AT/AttentionTrackingConfig.json";
}

FString UJsonParser::ReadStringFromFile(const FString& FilePath, bool& bOutSuccess)
{
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		DebugHeader::ShowMsgDialog(OK, TEXT("File does not exist " + FilePath));

		bOutSuccess = false;
		return "";
	}

	FString String = "";

	if (!FFileHelper::LoadFileToString(String, *FilePath))
	{
		DebugHeader::ShowMsgDialog(OK, TEXT("File could not be read " + FilePath));

		bOutSuccess = false;
		return "";
	}

	bOutSuccess = true;
	return String;
}

void UJsonParser::WriteStringToFile(const FString& String, const FString& FilePath, bool& bOutSuccess)
{
	if (!FFileHelper::SaveStringToFile(String, *FilePath))
	{
		DebugHeader::ShowMsgDialog(OK, TEXT("Failed to write to file " + FilePath));

		bOutSuccess = false;
		return;
	}
	
	bOutSuccess = true;
}

TSharedPtr<FJsonObject> UJsonParser::ReadJson(const FString& FilePath, bool& bOutSuccess)
{
	const FString JsonString = ReadStringFromFile(FilePath, bOutSuccess);

	if (!bOutSuccess) return nullptr;

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		DebugHeader::ShowMsgDialog(OK, TEXT("JSON deserialization failed: " + FilePath));

		bOutSuccess = false;
		return nullptr;
	}

	// DebugHeader::ShowNotifyInfo(TEXT("Deserialized JSON successfully from: " + FilePath));

	bOutSuccess = false;
	return JsonObject;
}

void UJsonParser::WriteJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FilePath, bool& bOutSuccess)
{
	FString JsonString;

	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString, 0);

	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
	{
		DebugHeader::ShowMsgDialog(OK, TEXT("JSON serialization failed: " + FilePath));

		bOutSuccess = false;
		return;
	}

	WriteStringToFile(JsonString, FilePath, bOutSuccess);

	if (!bOutSuccess) return;

	// DebugHeader::ShowNotifyInfo(TEXT("Serialized JSON successfully and written to: " + FilePath));

	bOutSuccess = true;
}

void UJsonParser::WriteJson(const TArray<TSharedPtr<FJsonValue>>& JsonObjectArray,
	const FString& FilePath, bool& bOutSuccess)
{
	FString JsonString;

	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString, 0);

	if (!FJsonSerializer::Serialize(JsonObjectArray, JsonWriter))
	{
		DebugHeader::ShowMsgDialog(OK, TEXT("JSON serialization failed: " + FilePath));

		bOutSuccess = false;
		return;
	}

	WriteStringToFile(JsonString, FilePath, bOutSuccess);

	if (!bOutSuccess) return;
	
	bOutSuccess = true;
}
