// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Global/BackStreetGameInstance.h"
#include "../../System/MapSystem/NewChapterManagerBase.h"
#include "../../System/MapSystem/StageManagerComponent.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "ProgressSaveGame.h"

ASaveManager::ASaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASaveManager::BeginPlay()
{
	Super::BeginPlay();
}

void ASaveManager::Initialize_Implementation()
{
	//Ref 지정
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GamemodeRef.IsValid())
	{
		ChapterManagerRef = GamemodeRef.Get()->GetChapterManagerRef();
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveManager::BeginPlay - Gamemode is not valid"));
	}

	//GameInstanceRef 지정
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	//MainCharacterRef 지정
	PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	//델리게이트 바인딩
	if (ChapterManagerRef.IsValid() && IsValid(ChapterManagerRef.Get()->StageManagerComponent))
	{
		ChapterManagerRef.Get()->OnChapterCleared.AddDynamic(this, &ASaveManager::SaveProgress);
		ChapterManagerRef.Get()->StageManagerComponent->OnStageLoadDone.AddDynamic(this, &ASaveManager::SaveProgress);
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveManager::Initialize - ChapterManagerRef is not valid"));
	}
}

void ASaveManager::SaveGameData_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::SaveGameData - Called"));
}

void ASaveManager::LoadGameData_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::LoadGameData - Called"));
}

void ASaveManager::SaveProgress_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::SaveProgress - Called"));

	FetchProgressData();
}

void ASaveManager::SaveInventory_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::SaveInventory - Called"));

	FetchInventoryData();
}

void ASaveManager::SaveAchievement_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::SaveAchievement - Called"));

	FetchAchievementData();
}

void ASaveManager::LoadProgress_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::LoadProgress - Called"));
}

void ASaveManager::LoadInventory_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::LoadInventory - Called"));
}

void ASaveManager::LoadAchievement_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::LoadAchievement - Called"));
}

void ASaveManager::RequestLoadGame()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::RequestLoadGame - Called"));

	//Load Game Data
	LoadGameData();

	//Set GameInstance to not load data
	GameInstanceRef.Get()->SetIsRequiredToLoad(false);
}

void ASaveManager::RestoreGameDataFromCache()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::RestoreGameData - Called"));
	
	//Restore Game Data
	GameInstanceRef.Get()->GetCachedSaveGameData(ProgressSaveData, AchievementSaveData, InventorySaveData);
	ChapterManagerRef.Get()->OverwriteChapterInfo(ProgressSaveData.ChapterInfo, ProgressSaveData.StageInfo);

	//LOG
	UE_LOG(LogSaveSystem, Log, TEXT("[Loaded Data Preview] -------------------"));
	UE_LOG(LogSaveSystem, Log, TEXT("- ChapterID : %d"), ProgressSaveData.ChapterInfo.ChapterID);
	UE_LOG(LogSaveSystem, Log, TEXT("- StageCoordinate: %s"), *ProgressSaveData.ChapterInfo.CurrentStageCoordinate.ToString());
	UE_LOG(LogSaveSystem, Log, TEXT("- StageInfoList: %d"), ProgressSaveData.ChapterInfo.StageInfoList.Num());
	UE_LOG(LogSaveSystem, Log, TEXT("- CurrentMapName : %s"), *ProgressSaveData.StageInfo.MainLevelAsset.ToString());

	//Set GameInstance to not load data
	GameInstanceRef.Get()->SetIsRequiredToLoad(false);
	
	//Notify Load Done
	OnLoadDone.Broadcast(true);
}