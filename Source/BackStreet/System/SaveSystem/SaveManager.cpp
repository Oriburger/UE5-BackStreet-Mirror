// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "../../Global/BackStreetGameModeBase.h"
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
	if(GamemodeRef.IsValid())
	{
		ChapterManagerRef = GamemodeRef.Get()->GetChapterManagerRef();
	}
	else 
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveManager::BeginPlay - Gamemode is not valid"));
	}
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
}

void ASaveManager::SaveInventory_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::SaveInventory - Called"));
}

void ASaveManager::SaveAchievement_Implementation()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveManager::SaveAchievement - Called"));
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

bool ASaveManager::DoesSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

void ASaveManager::CreateNewSaveGame()
{
}