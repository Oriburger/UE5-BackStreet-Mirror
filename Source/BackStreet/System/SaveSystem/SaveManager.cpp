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
}

void ASaveManager::SaveGameData_Implementation()
{
}

void ASaveManager::LoadGameData_Implementation()
{
}

void ASaveManager::SaveProgress_Implementation()
{
	
}

void ASaveManager::SaveInventory_Implementation()
{
	
}

void ASaveManager::SaveAchievement_Implementation()
{
	

	
}

void ASaveManager::LoadProgress_Implementation()
{
	
}

void ASaveManager::LoadInventory_Implementation()
{
	
}

void ASaveManager::LoadAchievement_Implementation()
{
	
}

void ASaveManager::RequestLoadGame()
{

}

void ASaveManager::RestoreGameDataFromCache()
{
}