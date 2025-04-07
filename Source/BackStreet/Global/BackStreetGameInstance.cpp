// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStreetGameInstance.h"
#include "../System/SaveSystem/GameProgressManager.h"
#include "../System/SaveSystem/ProgressSaveGame.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UBackStreetGameInstance::Init()
{
	Super::Init();
	
	// ���� ��ȯ �̺�Ʈ ���ε�
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UBackStreetGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UBackStreetGameInstance::OnPostLoadMap);
}

void UBackStreetGameInstance::SaveGameData(FProgressSaveData NewProgressData, FAchievementSaveData NewAchievementData, FInventorySaveData NewInventoryData)
{
	ProgressSaveData = NewProgressData;
	AchievementSaveData = NewAchievementData;
	InventorySaveData = NewInventoryData;
}

void UBackStreetGameInstance::GetSaveGameData(FProgressSaveData& OutProgressData, FAchievementSaveData& OutAchievementData, FInventorySaveData& OutInventoryData)
{
	OutProgressData = ProgressSaveData;
	OutAchievementData = AchievementSaveData;
	OutInventoryData = InventorySaveData;
}

void UBackStreetGameInstance::OnPreLoadMap(const FString& MapName)
{
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPreLoadMap - %s"), *MapName);
	
	// ���� ��ȯ ���� �� ó���� ���� �߰�
}

void UBackStreetGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPostLoadMap - Level load done"));
	// ���� ��ȯ �Ϸ� �� ó���� ���� �߰�
}