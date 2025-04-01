// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "ProgressSaveGame.h"

ASaveManager::ASaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASaveManager::BeginPlay()
{
	Super::BeginPlay();
/*
	if (DoesSaveExist())
	{
		LoadGameData();
	}
	else
	{
		CreateNewSaveGame();
		SaveGameData(); // 최초 생성 후 바로 저장
	}*/
}

void ASaveManager::SaveGameData_Implementation()
{
	if (!CachedMasterSave)
	{
		CreateNewSaveGame();
	}

	SaveProgress();
	SaveInventory();
	SaveAchievement();

	UGameplayStatics::SaveGameToSlot(CachedMasterSave, SaveSlotName, UserIndex);
}

void ASaveManager::LoadGameData_Implementation()
{
	if (!DoesSaveExist()) return;

	CachedMasterSave = Cast<UProgressSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
	if (!CachedMasterSave) return;

	LoadProgress();
	LoadInventory();
	LoadAchievement();

	bIsLoaded = true;
}

void ASaveManager::SaveProgress_Implementation()
{
	// TODO: 게임 내부 상태에서 값을 받아서 CachedMasterSave->ProgressSave->Data에 저장
}

void ASaveManager::SaveInventory_Implementation()
{
	// TODO: 인벤토리 상태 저장
}

void ASaveManager::SaveAchievement_Implementation()
{
	// TODO: 업적 상태 저장
}

void ASaveManager::LoadProgress_Implementation()
{
	// TODO: CachedMasterSave->ProgressSave->Data에서 값 꺼내 게임에 반영
}

void ASaveManager::LoadInventory_Implementation()
{
	// TODO: 인벤토리 정보 로드
}

void ASaveManager::LoadAchievement_Implementation()
{
	// TODO: 업적 정보 로드
}

bool ASaveManager::DoesSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

void ASaveManager::CreateNewSaveGame()
{
}