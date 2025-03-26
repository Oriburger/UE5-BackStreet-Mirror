// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStreetGameInstance.h"
#include "../System/SaveSystem/GameProgressManager.h"
#include "../System/SaveSystem/ProgressSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UBackStreetGameInstance::Init()
{
	Super::Init();
	
	//----- 게임 진행 매니저 생성 및 초기화 -------
	GameProgressManager = NewObject<UGameProgressManager>(this);
	GameProgressManager->Initialize();
	LoadGameData();

	UE_LOG(LogTemp, Warning, TEXT("UBackStreetGameInstance::Init(), Initial value - %d"), (int32)GameProgressManager->GetProgressData()->bIsTutorialDone);
}

void UBackStreetGameInstance::SaveGameData()
{
	if (IsValid(GameProgressManager))
	{
		GameProgressManager->SaveProgress();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UBackStreetGameInstance::SaveGameData() - GameProgressManager is not valid"));
	}
}

void UBackStreetGameInstance::LoadGameData()
{
	if (IsValid(GameProgressManager))
	{
		GameProgressManager->LoadProgress();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UBackStreetGameInstance::LoadGameData() - GameProgressManager is not valid"));
	}
}

void UBackStreetGameInstance::SetTutorialCompletion(bool bCompleted)
{
	if (IsValid(GameProgressManager))
	{
		GameProgressManager->SetTutorialCompletion(bCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UBackStreetGameInstance::SetTutorialCompletion(%d) - GameProgressManager is not valid"), (int32)bCompleted);
	}
}

bool UBackStreetGameInstance::GetTutorialCompletion()
{
	if (IsValid(GameProgressManager))
	{
		return GameProgressManager->GetTutorialCompletion();
	}
	UE_LOG(LogTemp, Error, TEXT("UBackStreetGameInstance::GetTutorialCompletion() - GameProgressManager is not valid"));
	return false;
}

void UBackStreetGameInstance::SetFusionCellCount(int32 NewCount)
{
	if (IsValid(GameProgressManager))
	{
		GameProgressManager->SetFusionCellCount(NewCount);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UBackStreetGameInstance::SetFusionCellCount(%d) - GameProgressManager is not valid"), NewCount);
	}
}

int32 UBackStreetGameInstance::GetFusionCellCount() const
{
	if (IsValid(GameProgressManager))
	{
		return GameProgressManager->GetFusionCellCount();
	}
	UE_LOG(LogTemp, Error, TEXT("UBackStreetGameInstance::GetFusionCellCount() - GameProgressManager is not valid"));
	return false;
}

void UBackStreetGameInstance::SetGenesiumCount(int32 NewCount)
{
	if (IsValid(GameProgressManager))
	{
		GameProgressManager->SetGenesiumCount(NewCount);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UBackStreetGameInstance::SetGenesiumCount(%d) - GameProgressManager is not valid"), NewCount);
	}
}

int32 UBackStreetGameInstance::GetGenesiumCount() const
{
	if (IsValid(GameProgressManager))
	{
		return GameProgressManager->GetGenesiumCount();
	}
	UE_LOG(LogTemp, Error, TEXT("UBackStreetGameInstance::GetGenesiumCount() - GameProgressManager is not valid"));
	return false;
}
