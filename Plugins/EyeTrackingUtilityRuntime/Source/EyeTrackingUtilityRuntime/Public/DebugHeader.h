// Copyright (c) 2025 Sebastian Cyliax

#pragma once

#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define OK EAppMsgType::Ok
#define OK_CANCEL EAppMsgType::OkCancel

namespace DebugHeader
{
	static void Print(const FString& Message, const FColor& Color, float Duration)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, Duration, Color, FString::Printf(TEXT("%s"), *Message));
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
	}

	static void PrintWarning(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	static void PrintError(const FString& Message)
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
	}

	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, 
		bool bShowMsgAsWarning = true)
	{
		FText MsgTitle = bShowMsgAsWarning ? FText::FromString("Warning") : FText::FromString("Message");

		return FMessageDialog::Open(MsgType, FText::FromString(Message), MsgTitle);
	}

	static EAppReturnType::Type ShowMsgDialogIf(bool bCondition, EAppMsgType::Type MsgType, 
		const FString& Message, bool bShowMsgAsWarning = true)
	{
		if (!bCondition) return EAppReturnType::Ok;

		FText MsgTitle = bShowMsgAsWarning ? FText::FromString("Warning") : FText::FromString("Message");

		return FMessageDialog::Open(MsgType, FText::FromString(Message), MsgTitle);
	}

	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));

		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}

	static void ShowNotifyInfoIf(bool bCondition, const FString& Message)
	{
		if (!bCondition) return;

		FNotificationInfo NotifyInfo(FText::FromString(Message));

		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}
