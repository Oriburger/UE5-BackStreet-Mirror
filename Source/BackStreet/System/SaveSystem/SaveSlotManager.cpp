// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveSlotManager.h"
#include "Kismet/GameplayStatics.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Global/BackStreetGameInstance.h"
#include "../../System/MapSystem/NewChapterManagerBase.h"
#include "../../System/MapSystem/StageManagerComponent.h"
#include "../../System/AbilitySystem/AbilityManagerBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "ProgressSaveGame.h"

ASaveSlotManager::ASaveSlotManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASaveSlotManager::Initialize()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::Initialize - Initialize"));

	// Initialize GameInstance reference
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Init variables
	DefferedInitializeCount = 0;

	// Initialize references
	InitializeReference(GameInstanceRef->GetIsInGame());

	SetSaveSlotName("");
}

void ASaveSlotManager::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::BeginPlay - BeginPlay"));

	// Initialize GameInstance reference
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Init variables
	DefferedInitializeCount = 0;

	// Bind to level load events
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ASaveSlotManager::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ASaveSlotManager::OnPostLoadMap);
}

void ASaveSlotManager::InitializeReference(bool bIsInGame)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference %d - Initialize references"), bIsInGame);

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

	FetchCachedData();
	ApplyCachedData(); //SaveCount를 추가해서 NewSlot인지 보기
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
	SaveSlotInfo.bIsInitialized = true;
	FetchGameData();
}

bool ASaveSlotManager::GetIsInitialized() const
{
	return SaveSlotInfo.bIsInitialized;
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

void ASaveSlotManager::FetchGameData()
{
	// Switch to main weapon
	if (PlayerCharacterRef.IsValid())
	{
		PlayerCharacterRef->SwitchWeapon(false, true); // Force switch to main weapon
		UE_LOG(LogSaveSystem, Warning, TEXT("Current Weapon ID : %d"), PlayerCharacterRef->WeaponComponent->WeaponID);
	}

	// Fetch game data from GameInstance
	ProgressSaveData.ChapterInfo = ChapterManagerRef->GetCurrentChapterInfo();
	ProgressSaveData.StageInfo = StageManagerRef->GetCurrentStageInfo();
	ProgressSaveData.CharacterInfo = PlayerCharacterRef->GetCharacterGameplayInfo();
	ProgressSaveData.WeaponStatInfo = PlayerCharacterRef->WeaponComponent->GetWeaponStat();
	ProgressSaveData.WeaponStateInfo = PlayerCharacterRef->WeaponComponent->GetWeaponState();
	ProgressSaveData.AbilityManagerInfo = PlayerCharacterRef->AbilityManagerComponent->GetAbilityManagerInfo();
	ProgressSaveData.InventoryInfo = PlayerCharacterRef->ItemInventory->GetInventoryInfoData();
	SaveSlotInfo.bIsInGame = GameInstanceRef->GetIsInGame();
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::FetchGameData - Fetch game data from GameInstance / bIsChapterInitialized : %d"), (int32)ProgressSaveData.ChapterInfo.bIsChapterInitialized);
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
		// Cache current game state
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::CacheCurrentGameState - Cache current game state"));
		GameInstanceRef->CacheGameData(ProgressSaveData, AchievementSaveData, InventorySaveData);
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::CacheCurrentGameState - GameInstance is not valid"));
	}
}

void ASaveSlotManager::ApplyCachedData()
{
	//LOG
	UE_LOG(LogSaveSystem, Log, TEXT("[ASaveSlotManager::Apply Data Preview] -------------------"));
	UE_LOG(LogSaveSystem, Log, TEXT("- ChapterID : %d"), ProgressSaveData.ChapterInfo.ChapterID);
	UE_LOG(LogSaveSystem, Log, TEXT("- SaveSlotName : %d"), *SaveSlotInfo.SaveSlotName);
	UE_LOG(LogSaveSystem, Log, TEXT("- StageCoordinate: %s"), *ProgressSaveData.ChapterInfo.CurrentStageCoordinate.ToString());
	UE_LOG(LogSaveSystem, Log, TEXT("- StageInfoList: %d"), ProgressSaveData.ChapterInfo.StageInfoList.Num());
	UE_LOG(LogSaveSystem, Log, TEXT("- CurrentMapName : %s"), *ProgressSaveData.StageInfo.MainLevelAsset.ToString());

	if (!ChapterManagerRef.IsValid() || !PlayerCharacterRef.IsValid())
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::ApplyCachedData - Ref is not valid - [ChapterManagerRef : %d], [PlayerCharacterRef : %d]"),
			ChapterManagerRef.IsValid(), PlayerCharacterRef.IsValid());

		SaveSlotInfo.bIsInitialized = !GameInstanceRef->GetIsInGame();
		OnInitializeDone.Broadcast(!GameInstanceRef->GetIsInGame());
		return;
	}
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - Apply Cached Data"));
	
	// Apply cached data to ChapterManager and PlayerCharacter
	if (SaveSlotInfo.bIsInitialized)
	{
		if (ProgressSaveData.ChapterInfo.bIsChapterInitialized)
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - Apply Cached Data to ChapterManager"));
			ChapterManagerRef->OverwriteChapterInfo(ProgressSaveData.ChapterInfo, ProgressSaveData.StageInfo);
		}
		if (PlayerCharacterRef.IsValid())
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - Apply Cached Data to PlayerCharacter"));
			PlayerCharacterRef->SetCharacterGameplayInfo(ProgressSaveData.CharacterInfo);
			PlayerCharacterRef->ItemInventory->InitInventory(ProgressSaveData.InventoryInfo);
			PlayerCharacterRef->WeaponComponent->InitWeapon(ProgressSaveData.WeaponStatInfo.WeaponID);
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - WeaponID : %d"), ProgressSaveData.WeaponStatInfo.WeaponID);
			PlayerCharacterRef->WeaponComponent->SetWeaponStat(ProgressSaveData.WeaponStatInfo);
			PlayerCharacterRef->WeaponComponent->SetWeaponState(ProgressSaveData.WeaponStateInfo);
			PlayerCharacterRef->AbilityManagerComponent->InitAbilityManager(PlayerCharacterRef.Get(), ProgressSaveData.AbilityManagerInfo);
		}
	}
	
	SaveSlotInfo.bIsInitialized = true;
	OnInitializeDone.Broadcast(true);
}
