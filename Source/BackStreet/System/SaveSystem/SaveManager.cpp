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
		SaveGameData(); // ���� ���� �� �ٷ� ����
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
	// TODO: ���� ���� ���¿��� ���� �޾Ƽ� CachedMasterSave->ProgressSave->Data�� ����
}

void ASaveManager::SaveInventory_Implementation()
{
	// TODO: �κ��丮 ���� ����
}

void ASaveManager::SaveAchievement_Implementation()
{
	// TODO: ���� ���� ����
}

void ASaveManager::LoadProgress_Implementation()
{
	// TODO: CachedMasterSave->ProgressSave->Data���� �� ���� ���ӿ� �ݿ�
}

void ASaveManager::LoadInventory_Implementation()
{
	// TODO: �κ��丮 ���� �ε�
}

void ASaveManager::LoadAchievement_Implementation()
{
	// TODO: ���� ���� �ε�
}

bool ASaveManager::DoesSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

void ASaveManager::CreateNewSaveGame()
{
}