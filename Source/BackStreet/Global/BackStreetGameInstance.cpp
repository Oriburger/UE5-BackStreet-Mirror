// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStreetGameInstance.h"
#include "../System/SaveSystem/GameProgressManager.h"
#include "../System/SaveSystem/ProgressSaveGame.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UBackStreetGameInstance::Init()
{
	Super::Init();
	
	// 레벨 전환 이벤트 바인딩
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
	
	// 레벨 전환 시작 시 처리할 로직 추가
}

void UBackStreetGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPostLoadMap - Level load done"));
	// 레벨 전환 완료 시 처리할 로직 추가
}