// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProgressManager.h"
#include "ProgressSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UGameProgressManager::Initialize()
{
    LoadProgress();
}

void UGameProgressManager::SaveProgress()
{
    if (!ProgressData)
    {
        ProgressData = Cast<UProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UProgressSaveGame::StaticClass()));
    }

    // 저장 진행
    bool result = UGameplayStatics::SaveGameToSlot(ProgressData, ProgressSlot, UserIndex);
    UE_LOG(LogTemp, Error, TEXT("UGameProgressManager::SaveProgress() result - %d"), (int32)result);
}

void UGameProgressManager::LoadProgress()
{
    if (UGameplayStatics::DoesSaveGameExist(ProgressSlot, UserIndex))
    {
        ProgressData = Cast<UProgressSaveGame>(UGameplayStatics::LoadGameFromSlot(ProgressSlot, UserIndex));
        if (!ProgressData)
        {
            ProgressData = Cast<UProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UProgressSaveGame::StaticClass()));
        }
    }
    else
    {
        ProgressData = Cast<UProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UProgressSaveGame::StaticClass()));
    }
}

void UGameProgressManager::SetTutorialCompletion(bool bNewState)
{
	if (!IsValid(ProgressData)) LoadProgress();

    UE_LOG(LogTemp, Warning, TEXT("UGameProgressManager::SetTutorialCompletion(%d) #1"), (int32)bNewState);
    ProgressData->bIsTutorialDone = bNewState;
    SaveProgress();
}

bool UGameProgressManager::GetTutorialCompletion() const
{
    if (ProgressData)
    {
        return ProgressData->bIsTutorialDone;
    }
    return false;
}

void UGameProgressManager::SetFusionCellCount(int32 NewCount)
{
    if (!IsValid(ProgressData)) LoadProgress();
    ProgressData->FusionCellCount = NewCount;
    SaveProgress();
}

int32 UGameProgressManager::GetFusionCellCount() const
{
    if (ProgressData)
    {
        return ProgressData->FusionCellCount;
    }
    return false;
}

void UGameProgressManager::SetGenesiumCount(int32 NewCount)
{
    if (!IsValid(ProgressData)) LoadProgress();
    ProgressData->GenesiumCount = NewCount;
    SaveProgress();
}

int32 UGameProgressManager::GetGenesiumCount() const
{
    if (ProgressData)
    {
        return ProgressData->GenesiumCount;
    }
    return false;
}
