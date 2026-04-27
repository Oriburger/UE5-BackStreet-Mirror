// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStreetGameInstance.h"
#include "GameAnalytics.h"
#include "../System/SaveSystem/SaveSlotManager.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UBackStreetGameInstance::Init()
{
	Super::Init();

	// Initialize World Event
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UBackStreetGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UBackStreetGameInstance::OnPostLoadMap);

	UGameAnalytics::initialize("85f49739d2aaf9bea648cac3b0050fe8", "87c9a4270e020e932955e4dd5fc4b91d7f0c61da");
}

void UBackStreetGameInstance::Shutdown()
{
	Super::Shutdown();
	OnGameInstanceShutdown();
}

void UBackStreetGameInstance::OnPreLoadMap(const FString& MapName)
{
	UE_LOG(LogSaveSystem, Log, TEXT(" "));
	UE_LOG(LogSaveSystem, Log, TEXT("============================================================================"));
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPreLoadMap - PreLoadMap called with MapName: %s"), *MapName);
	UE_LOG(LogSaveSystem, Log, TEXT("============================================================================"));
	SaveSlotManagerRef = nullptr;

	//메인메뉴로 이동 시, 데이터 초기화
	if (MapName.Contains("MainMenu"))
	{
		UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPreLoadMap - In MainMenu level, resetting save data"));
		SaveSlotInfo = FSaveSlotInfo();
		SaveSlotInfo.bIsInGame = false;
		SaveSlotInfo.bIsInNeutralZone = false;
		ProgressSaveData = FProgressSaveData();
		AchievementSaveData = FAchievementSaveData();
	}
}

void UBackStreetGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	UE_LOG(LogSaveSystem, Log, TEXT(" "));
	UE_LOG(LogSaveSystem, Log, TEXT("============================================================================"));
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPostLoadMap - PostLoadMap called with MapName: %s"), *LoadedWorld->GetMapName());
	UE_LOG(LogSaveSystem, Log, TEXT("============================================================================"));

	//메인메뉴로 이동 시, 데이터 초기화
	if (LoadedWorld->GetMapName().Contains("MainMenu"))
	{
		UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPostLoadMap - In MainMenu level, resetting save data"));
		SaveSlotInfo = FSaveSlotInfo();
		SaveSlotInfo.bIsInGame = false;
		SaveSlotInfo.bIsInNeutralZone = false;
		ProgressSaveData = FProgressSaveData();
		AchievementSaveData = FAchievementSaveData();
	}
	else if (LoadedWorld->GetMapName().Contains("Chapter"))
	{
		SaveSlotInfo.bIsInGame = true;
		SaveSlotInfo.bIsInNeutralZone = false; 
	}
	else if( LoadedWorld->GetMapName().Contains("NeutralZone"))
	{
		SaveSlotInfo.bIsInGame = false;
		SaveSlotInfo.bIsInNeutralZone = true;
	}

	// SaveSlotManager가 유효한지 체크
	bool result = IsValid(GetSafeSaveSlotManager());
	if (result)
	{
		UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPostLoadMap - SaveSlotManager spawned successfully"));
		GetSafeSaveSlotManager()->OnPostLoadMap(LoadedWorld);
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
				SaveSlotManagerRef->Initialize();
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
	if (IsValid(SaveSlotManagerRef))
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
	SaveSlotInfo.SaveSlotName = NewSaveSlotName;
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::SetCurrentSaveSlotName - SaveSlotName: %s"), *SaveSlotInfo.SaveSlotName);
}

void UBackStreetGameInstance::GetIsInGameOrNeutralZone(bool& bIsInGame, bool& bIsNeutralZone)
{
	bIsInGame = SaveSlotInfo.bIsInGame;
	bIsNeutralZone = SaveSlotInfo.bIsInNeutralZone;
}

void UBackStreetGameInstance::CacheGameData(FSaveSlotInfo NewSaveSlotInfo, FProgressSaveData NewProgressData, FAchievementSaveData NewAchievementData)
{
	if (!NewSaveSlotInfo.bIsInitialized)
	{
		UE_LOG(LogSaveSystem, Warning, TEXT("UBackStreetGameInstance::CacheGameData - NewSaveSlotInfo is not initialized"));
		return;
	}

	SaveSlotInfo = NewSaveSlotInfo;
	ProgressSaveData = NewProgressData;
	AchievementSaveData = NewAchievementData;

	//LOG
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::CacheGameData Invoked!!"));
	UE_LOG(LogSaveSystem, Log, TEXT("[Cacheded Data Preview] -------------------"));
	UE_LOG(LogSaveSystem, Log, TEXT("- ChapterID : %d"), ProgressSaveData.ChapterInfo.ChapterID);
	UE_LOG(LogSaveSystem, Log, TEXT("- SaveSlotName : %s"), *SaveSlotInfo.SaveSlotName);
	UE_LOG(LogSaveSystem, Log, TEXT("- StageCoordinate: %d"), ProgressSaveData.ChapterInfo.CurrentStageCoordinate);
	UE_LOG(LogSaveSystem, Log, TEXT("- StageInfoList: %d"), ProgressSaveData.ChapterInfo.StageInfoList.Num());
	UE_LOG(LogSaveSystem, Log, TEXT("- CurrentMapName : %s"), *ProgressSaveData.StageInfo.MainLevelAsset.ToString());
	UE_LOG(LogSaveSystem, Log, TEXT("- IsInGame : %d, IsInNeutralZone : %d"), SaveSlotInfo.bIsInGame, SaveSlotInfo.bIsInNeutralZone);
	UE_LOG(LogSaveSystem, Log, TEXT("- bIsRequiredToMoveNeutralZone : %d"), SaveSlotInfo.bIsRequiredToMoveNeutralZone);
	UE_LOG(LogSaveSystem, Log, TEXT("- TotalCoreCount InGame : %d"), ProgressSaveData.TotalCoreCountInGame);
	UE_LOG(LogSaveSystem, Log, TEXT("- InventoryCoreCount : %d"), AchievementSaveData.PermanentUpgradeInfo.CoreCount);
	UE_LOG(LogSaveSystem, Log, TEXT("------------------------------------------------"));
}

bool UBackStreetGameInstance::GetCachedSaveGameData(FSaveSlotInfo& OutSaveSlotInfo, FProgressSaveData& OutProgressData, FAchievementSaveData& OutAchievementData)
{
	if (!SaveSlotInfo.bIsInitialized)
	{
		UE_LOG(LogSaveSystem, Warning, TEXT("UBackStreetGameInstance::GetCachedSaveGameData - SaveSlotInfo is not initialized"));
		return false;
	}
	OutSaveSlotInfo = SaveSlotInfo;
	OutProgressData = ProgressSaveData;
	OutAchievementData = AchievementSaveData;

	return true;
}
