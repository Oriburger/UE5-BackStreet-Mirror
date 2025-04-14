// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStreetGameInstance.h"
#include "../System/SaveSystem/GameProgressManager.h"
#include "../System/SaveSystem/ProgressSaveGame.h"
#include "../System/SaveSystem/SaveManager.h"
#include "../System/SaveSystem/SaveSlotManager.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UBackStreetGameInstance::Init()
{
	Super::Init();

	// Initialize World Event
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UBackStreetGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UBackStreetGameInstance::OnPostLoadMap);
}

void UBackStreetGameInstance::OnPreLoadMap(const FString& MapName)
{
	// 레벨 전환 시작 시 기존 SaveSlotManager 제거
	SaveSlotManagerRef = nullptr;

	// 인게임인지 여부 체크
	bIsInGame = MapName.Contains(TEXT("Chapter"));
}

void UBackStreetGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	bool result = TrySpawnAndInitializeSaveSlotManager();
	if (result)
	{
		UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPostLoadMap - SaveSlotManager spawned successfully"));
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("UBackStreetGameInstance::OnPostLoadMap - Failed to spawn SaveSlotManager"));
	}
}

bool UBackStreetGameInstance::TrySpawnAndInitializeSaveSlotManager()
{
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::TrySpawnAndInitializeSaveSlotManager - Attempting to spawn SaveSlotManager"));
	if (SaveSlotManagerClassRef)
	{
		if (!SaveSlotManagerRef)
		{
			SaveSlotManagerRef = GetWorld()->SpawnActor<ASaveSlotManager>(SaveSlotManagerClassRef);
			if (SaveSlotManagerRef)
			{
				// SaveSlotManager 초기화
				SaveSlotManagerRef->FetchCachedData();
				return true;
			}
			else
			{
				UE_LOG(LogSaveSystem, Error, TEXT("UBackStreetGameInstance::TrySpawnAndInitializeSaveSlotManager - Failed to spawn SaveSlotManager"));
			}
		}
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("UBackStreetGameInstance::TrySpawnAndInitializeSaveSlotManager - SaveSlotManagerClassRef is not valid"));
	}
	return false;
}

ASaveSlotManager* UBackStreetGameInstance::GetSafeSaveSlotManager()
{
	if (SaveSlotManagerRef)
	{
		return SaveSlotManagerRef;
	}
	if (TrySpawnAndInitializeSaveSlotManager())
	{
		UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::GetSafeSaveSlotManager - SaveSlotManager spawned successfully"));
		return SaveSlotManagerRef;
	}
	UE_LOG(LogSaveSystem, Error, TEXT("UBackStreetGameInstance::GetSafeSaveSlotManager - SaveSlotManager is not valid"));
	return nullptr;
}

void UBackStreetGameInstance::SetCurrentSaveSlotName(FString NewSaveSlotName)
{
	if (NewSaveSlotName.IsEmpty())
	{
		UE_LOG(LogSaveSystem, Error, TEXT("UBackStreetGameInstance::SetCurrentSaveSlotName - NewSaveSlotName is empty"));
		return;
	}
	CurrentSaveSlotName = NewSaveSlotName;
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::SetCurrentSaveSlotName - SaveSlotName: %s"), *CurrentSaveSlotName);
}

void UBackStreetGameInstance::CacheGameData(FProgressSaveData NewProgressData, FAchievementSaveData NewAchievementData, FInventorySaveData NewInventoryData)
{
	ProgressSaveData = NewProgressData;
	AchievementSaveData = NewAchievementData;
	InventorySaveData = NewInventoryData;

	//LOG
	UE_LOG(LogSaveSystem, Log, TEXT("[Cacheded Data Preview] -------------------"));
	UE_LOG(LogSaveSystem, Log, TEXT("- ChapterID : %d"), ProgressSaveData.ChapterInfo.ChapterID);
	UE_LOG(LogSaveSystem, Log, TEXT("- StageCoordinate: %s"), *ProgressSaveData.ChapterInfo.CurrentStageCoordinate.ToString());
	UE_LOG(LogSaveSystem, Log, TEXT("- StageInfoList: %d"), ProgressSaveData.ChapterInfo.StageInfoList.Num());
	UE_LOG(LogSaveSystem, Log, TEXT("- CurrentMapName : %s"), *ProgressSaveData.StageInfo.MainLevelAsset.ToString());
}

void UBackStreetGameInstance::GetCachedSaveGameData(FProgressSaveData& OutProgressData, FAchievementSaveData& OutAchievementData, FInventorySaveData& OutInventoryData)
{
	OutProgressData = ProgressSaveData;
	OutAchievementData = AchievementSaveData;
	OutInventoryData = InventorySaveData;
}
