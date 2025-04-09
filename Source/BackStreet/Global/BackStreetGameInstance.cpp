// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStreetGameInstance.h"
#include "../System/SaveSystem/GameProgressManager.h"
#include "../System/SaveSystem/ProgressSaveGame.h"
#include "../System/SaveSystem/SaveManager.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UBackStreetGameInstance::Init()
{
	Super::Init();
	
	// 레벨 전환 이벤트 바인딩
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UBackStreetGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UBackStreetGameInstance::OnPostLoadMap);
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

void UBackStreetGameInstance::OnPreLoadMap(const FString& MapName)
{
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPreLoadMap - %s"), *MapName);
	
	// 레벨 전환 시작 시 처리할 로직 추가
   // URL 파라미터 파싱
    FString newSaveSlotName = UGameplayStatics::ParseOption(MapName, TEXT("SaveSlot"));
    FString shouldLoad = UGameplayStatics::ParseOption(MapName, TEXT("ShouldLoad"));
	
	// SaveSlotName이 변경되었거나 ShouldLoadData가 true인 경우에만 로드 필요
	bIsRequiredToLoad = bIsRequiredToLoad || (CurrentSaveSlotName != newSaveSlotName) || (shouldLoad == TEXT("true"));

    // 로그 출력 (디버깅 용도)
    UE_LOG(LogSaveSystem, Warning, TEXT("UBackStreetGameInstance::OnPreLoadMap - SaveSlot: %s, ShouldLoadData: %s"),
        *newSaveSlotName, bIsRequiredToLoad ? TEXT("True") : TEXT("False"));
	
	CurrentSaveSlotName = newSaveSlotName;
}

void UBackStreetGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPostLoadMap - %s"), *LoadedWorld->GetName());

	// 레벨 전환 완료 시 처리할 로직 추가
	if (bIsRequiredToLoad)
	{
		bIsRequiredToLoad = false;

		// 게임 진행 데이터가 로드된 후에 필요한 작업 수행
		// 월드 전환 시, SaveManager의 데이터가 유실되기 때문에 복원을 해주어야한다.
		ASaveManager* saveManager = GetWorld()->GetAuthGameMode<ABackStreetGameModeBase>()->GetSaveManager();
		if (saveManager)
		{
			saveManager->RestoreGameDataFromCache();
		}
		else
		{
			UE_LOG(LogSaveSystem, Error, TEXT("UBackStreetGameInstance::OnPostLoadMap - SaveManager is not valid"));
		}
	}
}