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
	
	// ���� ��ȯ �̺�Ʈ ���ε�
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
	
	// ���� ��ȯ ���� �� ó���� ���� �߰�
   // URL �Ķ���� �Ľ�
    FString newSaveSlotName = UGameplayStatics::ParseOption(MapName, TEXT("SaveSlot"));
    FString shouldLoad = UGameplayStatics::ParseOption(MapName, TEXT("ShouldLoad"));
	
	// SaveSlotName�� ����Ǿ��ų� ShouldLoadData�� true�� ��쿡�� �ε� �ʿ�
	bIsRequiredToLoad = bIsRequiredToLoad || (CurrentSaveSlotName != newSaveSlotName) || (shouldLoad == TEXT("true"));

    // �α� ��� (����� �뵵)
    UE_LOG(LogSaveSystem, Warning, TEXT("UBackStreetGameInstance::OnPreLoadMap - SaveSlot: %s, ShouldLoadData: %s"),
        *newSaveSlotName, bIsRequiredToLoad ? TEXT("True") : TEXT("False"));
	
	CurrentSaveSlotName = newSaveSlotName;
}

void UBackStreetGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPostLoadMap - %s"), *LoadedWorld->GetName());

	// ���� ��ȯ �Ϸ� �� ó���� ���� �߰�
	if (bIsRequiredToLoad)
	{
		bIsRequiredToLoad = false;

		// ���� ���� �����Ͱ� �ε�� �Ŀ� �ʿ��� �۾� ����
		// ���� ��ȯ ��, SaveManager�� �����Ͱ� ���ǵǱ� ������ ������ ���־���Ѵ�.
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