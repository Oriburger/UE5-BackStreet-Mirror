// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveSlotManager.h"
#include "Kismet/GameplayStatics.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Global/BackStreetGameInstance.h"
#include "../../System/MapSystem/NewChapterManagerBase.h"
#include "../../System/MapSystem/StageManagerComponent.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "ProgressSaveGame.h"

ASaveSlotManager::ASaveSlotManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASaveSlotManager::OnPreLoadMap(const FString& MapName)
{
	/*
	UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPreLoadMap - %s"), *MapName);
	
	// 레벨 전환 시작 시 처리할 로직 추가
   // URL 파라미터 파싱
    FString newSaveSlotName = UGameplayStatics::ParseOption(MapName, TEXT("SaveSlot"));
    FString shouldLoad = UGameplayStatics::ParseOption(MapName, TEXT("ShouldLoad"));

    // 로그 출력 (디버깅 용도)
    UE_LOG(LogSaveSystem, Warning, TEXT("UBackStreetGameInstance::OnPreLoadMap - SaveSlot: %s, ShouldLoadData: %s"),
        *newSaveSlotName, bIsRequiredToLoad ? TEXT("True") : TEXT("False"));
	
	CurrentSaveSlotName = newSaveSlotName;
	*/
}

void ASaveSlotManager::OnPostLoadMap(UWorld* LoadedWorld)
{
	/*
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
*/
}

void ASaveSlotManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Init variables
	DefferedInitializeCount = 0;

	// Initialize references
	InitializeReference();

	// Bind to level load events
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ASaveSlotManager::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ASaveSlotManager::OnPostLoadMap);
}

void ASaveSlotManager::InitializeReference()
{
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GameInstanceRef.IsValid())
	{
		GameInstanceRef->GetCachedSaveGameData(ProgressSaveData, AchievementSaveData, InventorySaveData);
	}
	ChapterManagerRef = Cast<ANewChapterManagerBase>(GamemodeRef->GetChapterManagerRef());
	StageManagerRef = ChapterManagerRef->StageManagerComponent;
	PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	PlayerInventoryRef = Cast<UItemInventoryComponent>(PlayerCharacterRef->InventoryComponent);
	PlayerWeaponRef = Cast<UWeaponComponentBase>(PlayerCharacterRef->WeaponComponent);

	if (!GameInstanceRef.IsValid() || !GamemodeRef.IsValid() || !ChapterManagerRef.IsValid() || !StageManagerRef.IsValid() 
		|| !PlayerCharacterRef.IsValid() || !PlayerInventoryRef.IsValid() || !PlayerWeaponRef.IsValid())
	{
		DefferedInitializeReference();
		return;
	}
}

void ASaveSlotManager::DefferedInitializeReference()
{
	DefferedInitializeCount++;
	checkf(DefferedInitializeCount < 10, TEXT("ASaveSlotManager::DefferedInitializeReference - DefferedInitializeCount is overed!"));

	if (DefferedInitializeTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DefferedInitializeTimerHandle);
	}

	// Defered Initialize
	GetWorld()->GetTimerManager().SetTimer(DefferedInitializeTimerHandle, this, &ASaveSlotManager::DefferedInitializeReference, DefferedInitializeTime, false);
}

void ASaveSlotManager::FetchCachedData()
{
	if (GameInstanceRef.IsValid())
	{
		GameInstanceRef->GetCachedSaveGameData(ProgressSaveData, AchievementSaveData, InventorySaveData);
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::FetchCachedData - GameInstance is not valid"));
	}
}

void ASaveSlotManager::CacheCurrentGameState()
{
	if (GameInstanceRef.IsValid())
	{
		GameInstanceRef->CacheGameData(ProgressSaveData, AchievementSaveData, InventorySaveData);
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::CacheCurrentGameState - GameInstance is not valid"));
	}
}

void ASaveSlotManager::ApplyCachedData()
{
	if (ChapterManagerRef.IsValid())
	{
		ChapterManagerRef->OverwriteChapterInfo(ProgressSaveData.ChapterInfo, ProgressSaveData.StageInfo);
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::ApplyCachedData - ChapterManagerRef is not valid"));
	}
	if (PlayerCharacterRef.IsValid())
	{
		//PlayerCharacterRef->SetPlayerProgressData(ProgressSaveData);
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::ApplyCachedData - PlayerCharacterRef is not valid"));
	}
}
