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
#include "../../Character/Component/BoosterchipManagerComponent.h"
#include "../../Character/Component/BattleApplicationManager.h"
#include "../../Character/Component/PermanentUpgradeManager.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "../../Character/Component/SkillManagerComponentBase.h"	

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
	GameInstanceRef->GetIsInGameOrNeutralZone(SaveSlotInfo.bIsInGame, SaveSlotInfo.bIsInNeutralZone);
	InitializeReference();
}

void ASaveSlotManager::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::BeginPlay - BeginPlay"));

	// Initialize GameInstance reference
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Init variables			
	DefferedInitializeCount = 0; 

	// Bind to load done event
	OnLoadDone.AddDynamic(this, &ASaveSlotManager::OnLoadFinished);

	// Bind to level load events
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ASaveSlotManager::OnPreLoadMap);

	// Check if we are in the main menu level
	bIsInMainMenuLevel = GetWorld() && GetWorld()->GetMapName().Contains("MainMenu");
}

void ASaveSlotManager::InitializeReference()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - Ingame ? : %d , In NeutralZone? : %d"), SaveSlotInfo.bIsInGame, SaveSlotInfo.bIsInNeutralZone);

	// Initialize references
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// 플레이어 캐릭터와 인벤토리, 무기 컴포넌트 참조 초기화
	PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	PlayerInventoryRef = PlayerCharacterRef.IsValid() ? PlayerCharacterRef->ItemInventory : nullptr;
	PlayerWeaponRef = PlayerCharacterRef.IsValid() ? PlayerCharacterRef->WeaponComponent : nullptr;

	// 인게임인지 체크
	if (SaveSlotInfo.bIsInGame)
	{
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - In game mode"));
		GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		ChapterManagerRef = Cast<ANewChapterManagerBase>(GamemodeRef->GetChapterManagerRef());
		StageManagerRef = ChapterManagerRef.IsValid() ? ChapterManagerRef->StageManagerComponent : nullptr;
		
		if (!GameInstanceRef.IsValid() || !GamemodeRef.IsValid() || !ChapterManagerRef.IsValid() || !StageManagerRef.IsValid()
			|| !PlayerCharacterRef.IsValid() || !PlayerInventoryRef.IsValid() || !PlayerWeaponRef.IsValid())
		{
			UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::InitializeReference - One of the references is not valid"));
			UE_LOG(LogSaveSystem, Error, TEXT("[GameInstanceRef : %d], [GamemodeRef : %d], [ChapterManagerRef : %d], [StageManagerRef : %d], [PlayerCharacterRef : %d], [PlayerInventoryRef : %d], [PlayerWeaponRef : %d]"),
				GameInstanceRef.IsValid(), GamemodeRef.IsValid(), ChapterManagerRef.IsValid(), StageManagerRef.IsValid(), PlayerCharacterRef.IsValid(), PlayerInventoryRef.IsValid(), PlayerWeaponRef.IsValid());
			DefferedInitializeReference();
			return;
		}
		else
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - All references are valid"));
		}

		// Bind to events
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - Bind to events"));
		ChapterManagerRef->OnChapterStarted.AddDynamic(this, &ASaveSlotManager::OnChapterStarted);
		ChapterManagerRef->OnChapterOvered.AddDynamic(this, &ASaveSlotManager::OnChapterOvered);
		ChapterManagerRef->OnChapterCleared.AddDynamic(this, &ASaveSlotManager::OnChapterCleared);
		StageManagerRef->OnStageLoadDone.AddDynamic(this, &ASaveSlotManager::SaveGameData);
		PlayerInventoryRef->OnItemAdded.AddDynamic(this, &ASaveSlotManager::OnItemAdded);
	}
	//인게임 체크
	bool result = TryFetchCachedData();
	if (!result)		
	{
		ProgressSaveData = FProgressSaveData();
		AchievementSaveData = FAchievementSaveData();
		InitializeSaveSlotName(); //불러올 정보가 없다면, 세이브슬롯명 지정(BP)
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::InitializeReference - No cached data found, initializing with default values"));
	}
	SaveSlotInfo.bIsInitialized = true;

	CacheCurrentGameState();

	ApplyCachedData();

	OnInitializeDone.Broadcast(true);
}

void ASaveSlotManager::DefferedInitializeReference()
{
	if(GetWorld()->GetMapName().Contains("MainMenu"))
	{
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::DefferedInitializeReference - In MainMenu level, skipping Deffered Initialize"));
		SaveSlotInfo = FSaveSlotInfo();
		return;
	}

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
	timerDelegate.BindUObject(this, &ASaveSlotManager::InitializeReference);
    GetWorld()->GetTimerManager().SetTimer(DefferedInitializeTimerHandle, timerDelegate, DefferedInitializeTime, false);
}

void ASaveSlotManager::OnPreLoadMap_Implementation(const FString& MapName)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnPreLoadMap - PreLoadMap: %s"), *MapName);

	// 메인메뉴 및 중립구역으로 전환 시의 상태를 초기화
	if (MapName.Contains("MainMenu"))
	{
		bIsReadyToMoveNeutralZone = true;
		UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::OnPreLoadMap - MainMenu detected, SaveSlotManagerRef reset"));
		
		SaveSlotInfo = FSaveSlotInfo();
		ProgressSaveData = FProgressSaveData();
		AchievementSaveData = FAchievementSaveData();
	}
	if (MapName.Contains("NeutralZone"))
	{
		bIsReadyToMoveNeutralZone = true;
		ProgressSaveData.ChapterInfo = FChapterInfo();
		ProgressSaveData.StageInfo = FStageInfo();

		//도중에 중립구역으로 나갈 시, 진행 정보를 다 지움
		if (!bIsChapterOvered && !bIsChapterCleared)
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnPreLoadMap - Moving to NeutralZone Manually, resetting ProgressSaveData"));
			ProgressSaveData = FProgressSaveData();
		}
		AchievementSaveData.PermanentUpgradeInfo.CoreCount += ProgressSaveData.TotalCoreCountInGame;
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnPreLoadMap - bIsChapterOvered: %d, bIsChapterCleared: %d"), bIsChapterOvered, bIsChapterCleared);
	}

	CacheCurrentGameState();
}

void ASaveSlotManager::OnPostLoadMap_Implementation(UWorld* LoadedWorld)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnPostLoadMap - PostLoadMap %s"), *LoadedWorld->GetMapName());

	//NeutralZone로 전환 시의 상태를 초기화
	if (LoadedWorld && LoadedWorld->GetMapName().Contains("NeutralZone"))
	{
		//print core count
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnPostLoadMap - Moved to NeutralZone, TotalCoreCountInGame: %d"), ProgressSaveData.TotalCoreCountInGame);
		ProgressSaveData = FProgressSaveData();
		SaveSlotInfo.bIsRequiredToMoveNeutralZone = false;
		SaveGameData();
	}
}

void ASaveSlotManager::OnChapterStarted()
{
	bIsChapterCleared = bIsChapterOvered = false;
	CacheCurrentGameState();
	SaveGameData();
}

void ASaveSlotManager::OnChapterCleared()
{
	if (ChapterManagerRef.IsValid())
	{
		ProgressSaveData.ChapterClearTimeList.Add(ChapterManagerRef->GetChapterClearTime());
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnChapterCleared - Chapter #%d cleared at time %f"), ChapterManagerRef->GetCurrentChapterInfo().ChapterID, ChapterManagerRef->GetChapterClearTime());
	}
	const bool bIsMaxChapter = ChapterManagerRef.IsValid() ? ChapterManagerRef.Get()->GetIsMaxChapter() : false;
	SaveSlotInfo.bIsRequiredToMoveNeutralZone = bIsMaxChapter;
	bIsChapterCleared = true;
	
	CacheCurrentGameState();
	
	if (bIsMaxChapter)
	{
		SaveGameData();
	}
}

void ASaveSlotManager::OnChapterOvered()
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnChapterOvered - Chapter Overed"));
	SaveSlotInfo.bIsRequiredToMoveNeutralZone = true;
	
	bIsChapterOvered = true;

	CacheCurrentGameState();
	SaveGameData();
}

void ASaveSlotManager::OnItemAdded(const FItemInfoDataStruct& NewItemInfo, const int32 AddCount)
{
	if (NewItemInfo.ItemID == GEAR_ITEM_ID)
	{
		ProgressSaveData.TotalGearCountInGame += AddCount;
	}
	else if (NewItemInfo.ItemID == CORE_ITEM_ID)
	{
		ProgressSaveData.TotalCoreCountInGame += AddCount;
	}
}

void ASaveSlotManager::SaveGameData_Implementation()
{	
	OnSaveBegin.Broadcast(true);
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::SaveGameData - SaveGameData called"));
}

bool ASaveSlotManager::GetIsInitialized() const
{
	return SaveSlotInfo.bIsInitialized;
}

FString ASaveSlotManager::GetSaveSlotName() const
{
	return GameInstanceRef.IsValid() ? GameInstanceRef->GetCurrentSaveSlotName() : "";
}

void ASaveSlotManager::SetTutorialCompletion(bool bCompleted)
{
	SaveSlotInfo.bIsTutorialDone = bCompleted;
	GameInstanceRef->SetCurrentSaveSlotInfo(SaveSlotInfo);
}

void ASaveSlotManager::SetSaveSlotName(FString NewSaveSlotName)
{
	if (!GameInstanceRef.IsValid())
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::SetSaveSlotName - GameInstance is not valid"));
		return;
	}
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::SetSaveSlotName - SaveSlotName : %s"), *NewSaveSlotName);
	SaveSlotInfo.SaveSlotName = NewSaveSlotName;
	GameInstanceRef->SetCurrentSaveSlotName(NewSaveSlotName);
}

void ASaveSlotManager::OnLoadFinished(bool bIsSuccess)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnLoadFinished - Load finished with success: %d"), bIsSuccess);

	//MainMenu -> NeutralZone or InGame
	if (bIsSuccess)
	{
		SaveSlotInfo.bIsInitialized = true;
		GameInstanceRef->SetCurrentSaveSlotInfo(SaveSlotInfo);

		ApplyCachedData();
		OnInitializeDone.Broadcast(SaveSlotInfo.bIsInGame);

		if (SaveSlotInfo.bIsRequiredToMoveNeutralZone)
		{
			GamemodeRef->RequestOpenLevel(FName("NeutralZonePersistent"));
		}
	}
}

void ASaveSlotManager::FetchGameData()
{
	UE_LOG(LogSaveSystem, Warning, TEXT("ASaveSlotManager::FetchGameData() - FetchGameData called"));

	// Switch to main weapon
	if (PlayerCharacterRef.IsValid())
	{
		PlayerCharacterRef->SwitchWeapon(false, true); // Force switch to main weapon
		UE_LOG(LogSaveSystem, Warning, TEXT("Current Weapon ID : %d"), PlayerCharacterRef->WeaponComponent->WeaponID);
	}

	// Fetch game data from GameInstance
	if (ChapterManagerRef.IsValid() && StageManagerRef.IsValid()) // in ingame
	{
		ProgressSaveData.ChapterInfo = ChapterManagerRef->GetCurrentChapterInfo();
		ProgressSaveData.StageInfo = StageManagerRef->GetCurrentStageInfo();
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::FetchGameData - Map Name : %s ,  Is Valid : %d"), *ProgressSaveData.StageInfo.MainLevelAsset.ToString(), (ProgressSaveData.StageInfo.MainLevelAsset.IsValid()));
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::FetchGameData - Fetch game data from GameInstance / bIsChapterInitialized : %d"), (int32)ProgressSaveData.ChapterInfo.bIsChapterInitialized);
	}
	else
	{
		UE_LOG(LogSaveSystem, Warning, TEXT("ASaveSlotManager::FetchGameData - Not in ingame mode, using default chapter and stage info"));
		ProgressSaveData.ChapterInfo = FChapterInfo();
		ProgressSaveData.StageInfo = FStageInfo();
	}
	if (PlayerCharacterRef.IsValid()) //also in neutral zone
	{
		ProgressSaveData.WeaponStatInfo = PlayerCharacterRef->WeaponComponent->GetWeaponStat();
		AchievementSaveData.PermanentUpgradeInfo = PlayerCharacterRef->PermanentUpgradeManager->GetPermanentUpgradeManagerInfo();

		if (SaveSlotInfo.bIsInGame)
		{
			ProgressSaveData.CharacterInfo = PlayerCharacterRef->GetCharacterGameplayInfo();
			ProgressSaveData.WeaponStateInfo = PlayerCharacterRef->WeaponComponent->GetWeaponState();
			ProgressSaveData.BoosterchipManagerInfo = PlayerCharacterRef->BoosterchipManagerComponent->GetBoosterchipManagerInfo();
			ProgressSaveData.BattleApplicationManagerInfo = PlayerCharacterRef->BattleApplicationManager->GetBattleApplicationManagerInfo();
			//print battleapp info 

			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::FetchGameData - BattleApp Count : %d"), ProgressSaveData.BattleApplicationManagerInfo.AbilityManagerInfo.ActiveAbilityInfoList.Num());
			for (auto& abilityInfo : ProgressSaveData.BattleApplicationManagerInfo.AbilityManagerInfo.ActiveAbilityInfoList)
			{
				UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::FetchGameData - BattleApp Active Ability ID : %d"), abilityInfo.AbilityId);
			}

			ProgressSaveData.InventoryInfo = PlayerCharacterRef->ItemInventory->GetInventoryInfoData();
			ProgressSaveData.SkillManagerInfo = PlayerCharacterRef->SkillManagerComponent->GetSkillManagerInfo();
		} 

		FCharacterGameplayInfo tempInfo = ProgressSaveData.CharacterInfo;
		tempInfo.ResetAllAbilityStatInfo();

		if (bIsReadyToMoveNeutralZone && SaveSlotInfo.bIsInGame)
		{
			//최고기록 Handling
			if (ProgressSaveData.ChapterClearTimeList.Num() > 4)
			{
				SaveSlotInfo.BestChapterClearTimeList = SaveSlotInfo.BestChapterClearTimeList.IsEmpty() ? ProgressSaveData.ChapterClearTimeList
					: (ProgressSaveData.ChapterClearTimeList[3] < SaveSlotInfo.BestChapterClearTimeList[3] ? ProgressSaveData.ChapterClearTimeList : SaveSlotInfo.BestChapterClearTimeList);
			}			
		}
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::FetchGameData - InventorySaveData.CoreCount : %d"), AchievementSaveData.PermanentUpgradeInfo.CoreCount);
	}
}

bool ASaveSlotManager::TryFetchCachedData()
{
	bool result = false;
	if (GameInstanceRef.IsValid())
	{
		result = GameInstanceRef->GetCachedSaveGameData(SaveSlotInfo, ProgressSaveData, AchievementSaveData);

		if (result)
		{
			//LOG
			UE_LOG(LogSaveSystem, Log, TEXT(" "));
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::TryFetchCachedData() Invoked!!"));
			UE_LOG(LogSaveSystem, Log, TEXT("[Fetched Data Preview] -------------------"));
			UE_LOG(LogSaveSystem, Log, TEXT("- ChapterID : %d"), ProgressSaveData.ChapterInfo.ChapterID);
			UE_LOG(LogSaveSystem, Log, TEXT("- SaveSlotName : %s"), *SaveSlotInfo.SaveSlotName);
			UE_LOG(LogSaveSystem, Log, TEXT("- StageCoordinate: %d"), ProgressSaveData.ChapterInfo.CurrentStageCoordinate);
			UE_LOG(LogSaveSystem, Log, TEXT("- StageInfoList: %d"), ProgressSaveData.ChapterInfo.StageInfoList.Num());
			UE_LOG(LogSaveSystem, Log, TEXT("- CurrentMapName : %s"), *ProgressSaveData.StageInfo.MainLevelAsset.ToString());
			UE_LOG(LogSaveSystem, Log, TEXT("- IsInGame : %d, IsInNeutralZone : %d"), SaveSlotInfo.bIsInGame, SaveSlotInfo.bIsInNeutralZone);
			UE_LOG(LogSaveSystem, Log, TEXT("- bIsRequiredToMoveNeutralZone : %d"), SaveSlotInfo.bIsRequiredToMoveNeutralZone);
			UE_LOG(LogSaveSystem, Log, TEXT("- TotalCoreCount InGame : %d"), ProgressSaveData.TotalCoreCountInGame);
			UE_LOG(LogSaveSystem, Log, TEXT("- InventoryCoreCount : %d"), AchievementSaveData.PermanentUpgradeInfo.CoreCount);
			UE_LOG(LogSaveSystem, Log, TEXT("------------------------------------------------\n"));

			// Apply cached data to ChapterManager and PlayerCharacter
			ApplyCachedData();
		}
		else
		{
			UE_LOG(LogSaveSystem, Warning, TEXT("ASaveSlotManager::FetchCachedData - Failed to fetch cached data"));
		}
	}
	else
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::FetchCachedData - GameInstance is noAchievementSaveData.PermanentUpgradeInfo.CoreCountt valid"));
	}
	return result;
}

void ASaveSlotManager::CacheCurrentGameState()
{
	if (SaveSlotInfo.bIsInGame)
	{
		GameInstanceRef->CacheGameData(SaveSlotInfo, ProgressSaveData, AchievementSaveData);
	}
	else if (SaveSlotInfo.bIsInNeutralZone)
	{
		GameInstanceRef->CacheGameData(SaveSlotInfo, ProgressSaveData, AchievementSaveData);
	}
	
}

void ASaveSlotManager::ApplyCachedData()
{
	if (!SaveSlotInfo.bIsInitialized || bIsInMainMenuLevel)
	{
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - invoke failed, reson ( SaveSlotInfo.bIsInitialized : %d, bIsInMainMenuLevel : %d )"),
			SaveSlotInfo.bIsInitialized, bIsInMainMenuLevel);	
		return;
	}

	//Log
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData Invoked!!"));
	UE_LOG(LogSaveSystem, Log, TEXT("[ASaveSlotManager::Apply Data Preview] -------------------"));
	UE_LOG(LogSaveSystem, Log, TEXT("- ChapterID : %d"), ProgressSaveData.ChapterInfo.ChapterID);
	UE_LOG(LogSaveSystem, Log, TEXT("- SaveSlotName : %s"), *SaveSlotInfo.SaveSlotName);
	UE_LOG(LogSaveSystem, Log, TEXT("- StageCoordinate: %d"), ProgressSaveData.ChapterInfo.CurrentStageCoordinate);
	UE_LOG(LogSaveSystem, Log, TEXT("- StageInfoList: %d"), ProgressSaveData.ChapterInfo.StageInfoList.Num());
	UE_LOG(LogSaveSystem, Log, TEXT("- CurrentMapName : %s"), *ProgressSaveData.StageInfo.MainLevelAsset.ToString());
	UE_LOG(LogSaveSystem, Log, TEXT("- IsInGame : %d, IsInNeutralZone : %d"), SaveSlotInfo.bIsInGame, SaveSlotInfo.bIsInNeutralZone);
	UE_LOG(LogSaveSystem, Log, TEXT("- bIsRequiredToMoveNeutralZone : %d"), SaveSlotInfo.bIsRequiredToMoveNeutralZone);
	UE_LOG(LogSaveSystem, Log, TEXT("- TotalCoreCount InGame : %d"), ProgressSaveData.TotalCoreCountInGame);
	UE_LOG(LogSaveSystem, Log, TEXT("- InventoryCoreCount : %d"), AchievementSaveData.PermanentUpgradeInfo.CoreCount);

	if (PlayerCharacterRef.IsValid())
	{
		PlayerCharacterRef->PermanentUpgradeManager->InitPermanentUpgradeManager(PlayerCharacterRef.Get(), AchievementSaveData.PermanentUpgradeInfo);
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - PermanentUpgradeInfo.CoreCount : %d"), AchievementSaveData.PermanentUpgradeInfo.CoreCount);
	}

	if (!ChapterManagerRef.IsValid() || !PlayerCharacterRef.IsValid())
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::ApplyCachedData - Ref is not valid - [ChapterManagerRef : %d], [PlayerCharacterRef : %d]"),
			ChapterManagerRef.IsValid(), PlayerCharacterRef.IsValid());
		return;
	}
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - Apply Cached Data"));

	if (SaveSlotInfo.bIsInGame)
	{
		if (ProgressSaveData.ChapterInfo.bIsChapterInitialized)
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - Apply Cached Data to ChapterManager"));
			ChapterManagerRef->OverwriteChapterInfo(ProgressSaveData.ChapterInfo, ProgressSaveData.StageInfo);
		}
		if (PlayerCharacterRef.IsValid() && ProgressSaveData.CharacterInfo.CharacterID != 0)
		{
			PlayerCharacterRef->SetCharacterGameplayInfo(ProgressSaveData.CharacterInfo);
			
			PlayerCharacterRef->WeaponComponent->InitWeapon(ProgressSaveData.WeaponStatInfo.WeaponID);
			PlayerCharacterRef->ItemInventory->InitInventory(ProgressSaveData.InventoryInfo);
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - WeaponID : %d"), ProgressSaveData.WeaponStatInfo.WeaponID);
			PlayerCharacterRef->WeaponComponent->SetWeaponStat(ProgressSaveData.WeaponStatInfo);
			PlayerCharacterRef->WeaponComponent->SetWeaponState(ProgressSaveData.WeaponStateInfo);
			PlayerCharacterRef->BoosterchipManagerComponent->InitBoosterchipManager(PlayerCharacterRef.Get(), ProgressSaveData.BoosterchipManagerInfo);
			PlayerCharacterRef->SkillManagerComponent->InitSkillManager(ProgressSaveData.SkillManagerInfo);
			PlayerCharacterRef->BattleApplicationManager->InitBattleApplicationManager(PlayerCharacterRef.Get(), ProgressSaveData.BattleApplicationManagerInfo);
		}
	}
}