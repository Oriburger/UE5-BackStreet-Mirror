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

void ASaveSlotManager::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::BeginPlay - BeginPlay"));

	// Initialize GameInstance reference
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Init variables
	DefferedInitializeCount = 0;

	// Initialize references
	InitializeReference(GameInstanceRef->GetIsInGame());

	// Bind to level load events
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ASaveSlotManager::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ASaveSlotManager::OnPostLoadMap);
}

void ASaveSlotManager::InitializeReference(bool bIsInGame)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - Initialize references"));

	// Initialize references
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GameInstanceRef.IsValid())
	{
		GameInstanceRef->GetCachedSaveGameData(ProgressSaveData, AchievementSaveData, InventorySaveData);
	}

	// 인게임인지 체크
	if (bIsInGame)
	{
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - In game mode"));
		GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		ChapterManagerRef = Cast<ANewChapterManagerBase>(GamemodeRef->GetChapterManagerRef());
		StageManagerRef = ChapterManagerRef.IsValid() ? ChapterManagerRef->StageManagerComponent : nullptr;
		PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		PlayerInventoryRef = PlayerCharacterRef.IsValid() ? PlayerCharacterRef->ItemInventory : nullptr;
		PlayerWeaponRef = PlayerCharacterRef.IsValid() ? PlayerCharacterRef->WeaponComponent : nullptr;

		if (!GameInstanceRef.IsValid() || !GamemodeRef.IsValid() || !ChapterManagerRef.IsValid() || !StageManagerRef.IsValid()
			|| !PlayerCharacterRef.IsValid() || !PlayerInventoryRef.IsValid() || !PlayerWeaponRef.IsValid())
		{
			UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::InitializeReference - One of the references is not valid"));
			UE_LOG(LogSaveSystem, Error, TEXT("[GameInstanceRef : %d], [GamemodeRef : %d], [ChapterManagerRef : %d], [StageManagerRef : %d], [PlayerCharacterRef : %d], [PlayerInventoryRef : %d], [PlayerWeaponRef : %d]"),
				GameInstanceRef.IsValid(), GamemodeRef.IsValid(), ChapterManagerRef.IsValid(), StageManagerRef.IsValid(), PlayerCharacterRef.IsValid(), PlayerInventoryRef.IsValid(), PlayerWeaponRef.IsValid());
			DefferedInitializeReference();
			return;
		}

		// Bind to events
		if (ChapterManagerRef.IsValid() && StageManagerRef.IsValid())
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - Bind to events"));
			ChapterManagerRef->OnChapterCleared.AddDynamic(this, &ASaveSlotManager::SaveGameData);
			StageManagerRef->OnStageLoadDone.AddDynamic(this, &ASaveSlotManager::SaveGameData);
		}
	}
	else
	{
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - Not in game mode"));
	}
}

void ASaveSlotManager::DefferedInitializeReference()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::DefferedInitializeReference - DefferedInitializeCount: %d"), DefferedInitializeCount);

	// Check if the references are valid
	DefferedInitializeCount++;
	checkf(DefferedInitializeCount < 10, TEXT("ASaveSlotManager::DefferedInitializeReference - DefferedInitializeCount is overed!"));

	if (DefferedInitializeTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DefferedInitializeTimerHandle);
	}

    // Defered Initialize using Timer Delegate
    FTimerDelegate timerDelegate;
	timerDelegate.BindUObject(this, &ASaveSlotManager::InitializeReference, GameInstanceRef->GetIsInGame());
    GetWorld()->GetTimerManager().SetTimer(DefferedInitializeTimerHandle, timerDelegate, DefferedInitializeTime, false);
}

void ASaveSlotManager::SaveGameData_Implementation()
{

}

FString ASaveSlotManager::GetSaveSlotName() const
{
	return GameInstanceRef.IsValid() ? GameInstanceRef->GetCurrentSaveSlotName() : "";
}

void ASaveSlotManager::SetSaveSlotName(FString NewSaveSlotName)
{
	if (!GameInstanceRef.IsValid())
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::SetSaveSlotName - GameInstance is not valid"));
		return;
	}
	GameInstanceRef->SetCurrentSaveSlotName(NewSaveSlotName);
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
