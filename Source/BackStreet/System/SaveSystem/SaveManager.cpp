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
	
	if (GameInstanceRef.IsValid())
	{
		FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ASaveManager::OnPreLoadMap);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ASaveManager::OnPostLoadMap);
	}
}

void ASaveManager::Initialize_Implementation()
{
	//MainCharacterRef 지정
	PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	//델리게이트 바인딩
	if (ChapterManagerRef.IsValid())
	{
		ChapterManagerRef.Get()->OnChapterCleared.AddDynamic(this, &ASaveManager::SaveProgress);
		ChapterManagerRef.Get()->StageManagerComponent->OnStageCleared.AddDynamic(this, &ASaveManager::SaveProgress);
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

void ASaveManager::OnPreLoadMap_Implementation(const FString& MapName)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::OnPreLoadMap - %s"), *MapName);

	if (PlayerCharacterRef.IsValid())
	{
		
	}
	
}

void ASaveManager::OnPostLoadMap_Implementation(UWorld* LoadedWorld)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::OnPostLoadMap - Level load done"));
	// 레벨 전환 완료 시 처리할 로직 추가
	
}

bool ASaveManager::DoesSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

void ASaveManager::CreateNewSaveGame()
{
}
