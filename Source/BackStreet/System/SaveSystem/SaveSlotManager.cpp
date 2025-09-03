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
		
		ProgressSaveData = FProgressSaveData();
		AchievementSaveData = FAchievementSaveData();
		InventorySaveData = FInventorySaveData();
	}
	if (MapName.Contains("NeutralZone"))
	{
		bIsReadyToMoveNeutralZone = true;
		ProgressSaveData.ChapterInfo = FChapterInfo();
		ProgressSaveData.StageInfo = FStageInfo();
	}

	CacheCurrentGameState();
}

void ASaveSlotManager::OnPostLoadMap_Implementation(UWorld* LoadedWorld)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnPostLoadMap - PostLoadMap %s"), *LoadedWorld->GetMapName());

	//NeutralZone로 전환 시의 상태를 초기화
	if (LoadedWorld && LoadedWorld->GetMapName().Contains("NeutralZone"))
	{
		SaveGameData();
	}
}

void ASaveSlotManager::OnChapterCleared()
{
	if (ChapterManagerRef.IsValid())
	{
		ProgressSaveData.ChapterClearTimeList.Add(ChapterManagerRef->GetChapterClearTime());
		UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::OnChapterCleared - Chapter #%d cleared at time %f"), ChapterManagerRef->GetCurrentChapterInfo().ChapterID, ChapterManagerRef->GetChapterClearTime());
	}
	CacheCurrentGameState();
}

void ASaveSlotManager::OnItemAdded(const FItemInfoDataStruct& NewItemInfo, const int32 AddCount)
{
	//Gear id : 1, Core id : 2
	const int32 gearID = 1;
	const int32 coreID = 2;

	if (NewItemInfo.ItemID == gearID)
	{
		ProgressSaveData.TotalGearCountInGame += AddCount;
	}
	else if (NewItemInfo.ItemID == coreID)
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
	}
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
		ProgressSaveData.CharacterInfo = PlayerCharacterRef->GetCharacterGameplayInfo();
		ProgressSaveData.WeaponStatInfo = PlayerCharacterRef->WeaponComponent->GetWeaponStat();
		ProgressSaveData.WeaponStateInfo = PlayerCharacterRef->WeaponComponent->GetWeaponState();
		ProgressSaveData.AbilityManagerInfo = PlayerCharacterRef->AbilityManagerComponent->GetAbilityManagerInfo();
		ProgressSaveData.InventoryInfo = PlayerCharacterRef->ItemInventory->GetInventoryInfoData();
		ProgressSaveData.SkillManagerInfo = PlayerCharacterRef->SkillManagerComponent->GetSkillManagerInfo();	
		
		if (bIsReadyToMoveNeutralZone)
		{
			// 영구재화 Handling 
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::FetchGameData - Fetching permanent data from PlayerCharacterRef"));
			InventorySaveData.CoreCount += ProgressSaveData.TotalCoreCountInGame;

			//최고기록 Handling
			if (ProgressSaveData.ChapterClearTimeList.Num() > 4)
			{
				SaveSlotInfo.BestChapterClearTimeList = SaveSlotInfo.BestChapterClearTimeList.IsEmpty() ? ProgressSaveData.ChapterClearTimeList
					: (ProgressSaveData.ChapterClearTimeList[3] < SaveSlotInfo.BestChapterClearTimeList[3] ? ProgressSaveData.ChapterClearTimeList : SaveSlotInfo.BestChapterClearTimeList);
			}
		}
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::FetchGameData - InventorySaveData.CoreCount : %d"), InventorySaveData.CoreCount);
	}
}

bool ASaveSlotManager::TryFetchCachedData()
{
	bool result = false;
	if (GameInstanceRef.IsValid())
	{
		result = GameInstanceRef->GetCachedSaveGameData(SaveSlotInfo, ProgressSaveData, AchievementSaveData, InventorySaveData);

		if (result)
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::FetchCachedData - Cached data fetched successfully"));

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
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::FetchCachedData - GameInstance is not valid"));
	}
	return result;
}

void ASaveSlotManager::CacheCurrentGameState()
{
	if (SaveSlotInfo.bIsInGame)
	{
		GameInstanceRef->CacheGameData(SaveSlotInfo, ProgressSaveData, AchievementSaveData, InventorySaveData);
	}
	else if (SaveSlotInfo.bIsInNeutralZone)
	{
		ProgressSaveData = FProgressSaveData();
		GameInstanceRef->CacheGameData(SaveSlotInfo, ProgressSaveData, AchievementSaveData, InventorySaveData);
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

	if (!ChapterManagerRef.IsValid() || !PlayerCharacterRef.IsValid())
	{
		UE_LOG(LogSaveSystem, Error, TEXT("ASaveSlotManager::ApplyCachedData - Ref is not valid - [ChapterManagerRef : %d], [PlayerCharacterRef : %d]"),
			ChapterManagerRef.IsValid(), PlayerCharacterRef.IsValid());
		return;
	}
	UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - Apply Cached Data"));
	
	// Apply cached data to ChapterManager and PlayerCharacter
	if (SaveSlotInfo.bIsInGame)
	{
		if (ProgressSaveData.ChapterInfo.bIsChapterInitialized)
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - Apply Cached Data to ChapterManager"));
			ChapterManagerRef->OverwriteChapterInfo(ProgressSaveData.ChapterInfo, ProgressSaveData.StageInfo);
		}
		if (PlayerCharacterRef.IsValid() && ProgressSaveData.CharacterInfo.CharacterID != 0)
		{
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - Apply Cached Data to PlayerCharacter"));
			PlayerCharacterRef->SetCharacterGameplayInfo(ProgressSaveData.CharacterInfo);
			PlayerCharacterRef->ItemInventory->InitInventory(ProgressSaveData.InventoryInfo);
			PlayerCharacterRef->WeaponComponent->InitWeapon(ProgressSaveData.WeaponStatInfo.WeaponID);
			UE_LOG(LogSaveSystem, Log, TEXT("ASaveSlotManager::ApplyCachedData - WeaponID : %d"), ProgressSaveData.WeaponStatInfo.WeaponID);
			PlayerCharacterRef->WeaponComponent->SetWeaponStat(ProgressSaveData.WeaponStatInfo);
			PlayerCharacterRef->WeaponComponent->SetWeaponState(ProgressSaveData.WeaponStateInfo);
			PlayerCharacterRef->AbilityManagerComponent->InitAbilityManager(PlayerCharacterRef.Get(), ProgressSaveData.AbilityManagerInfo);
			PlayerCharacterRef->SkillManagerComponent->InitSkillManager(ProgressSaveData.SkillManagerInfo);
		}
	}
}