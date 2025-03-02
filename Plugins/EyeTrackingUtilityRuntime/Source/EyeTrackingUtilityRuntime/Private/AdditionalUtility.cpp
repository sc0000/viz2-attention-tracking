// Copyright (c) 2025 Sebastian Cyliax

#include "AdditionalUtility.h"

FString UAdditionalUtility::PaddedIntegerToString(const int32& Integer, const int32 NumDigits)
{
	char Buffer[100];
	int RespCode;
	RespCode = snprintf(Buffer, 100, "%0*d", NumDigits, Integer);
	return FString(ANSI_TO_TCHAR(Buffer));
}

void UAdditionalUtility::GetAxesByLength(FVector Vector, TArray<double>& OutArray)
{
	OutArray.Empty();
	OutArray = { Vector.X, Vector.Y, Vector.Z };
	OutArray.Sort(TGreater<double>());
}
