// Fill out your copyright notice in the Description page of Project Settings.


#include "StageManagerComponent.h"
#include "StageGeneratorComponent.h"
#include "../../Character/EnemyCharacter/EnemyCharacterBase.h"
#include "../../Character/CharacterBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Item/ItemBase.h"
#include "../../Item/ItemBoxBase.h"
#include "../AISystem/AIControllerBase.h"
#include "./Stage/GateBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "SlateBasics.h"
#include "Runtime/UMG/Public/UMG.h"

// Sets default values for this component's properties
UStageManagerComponent::UStageManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UStageManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	//Bind to player death delegate
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (IsValid(playerCharacter))
	{
		Cast<ACharacterBase>(playerCharacter)->OnDeath.AddUObject(this, &UStageManagerComponent::SetGameIsOver);
	}
	ChapterManagerRef = Cast<ANewChapterManagerBase>(GetOwner());
	PlayerRef = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	OnStageLoadBegin.AddDynamic(Cast<AMainCharacterBase>(PlayerRef.Get()), &AMainCharacterBase::ResetCameraRotation);
}

void UStageManagerComponent::Initialize(FChapterInfo NewChapterInfo)
{
	CurrentChapterInfo = NewChapterInfo;
	VisitedCraftstageCount = 0;
}

void UStageManagerComponent::InitStage(FStageInfo NewStageInfo)
{
	//Clear Previous Stage Resources and Data
	ClearPreviousResource();

	//Init new stage
	CurrentStageInfo = NewStageInfo;

	UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::InitStage-------------"));
	UE_LOG(LogStage, Warning, TEXT("> Stage Type : %d"), CurrentStageInfo.StageType);
	UE_LOG(LogStage, Warning, TEXT("> Coordinate : %d"), CurrentStageInfo.Coordinate);

	//Update visited craft stage count
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Craft)
	{
		VisitedCraftstageCount += 1;
	}

	//Check level asset list is not empty	
	if (NewStageInfo.MainLevelAsset.IsNull())
	{
		UE_LOG(LogSaveSystem, Log, TEXT("UStageManagerComponent::InitStage - MainLevelAsset is Null"));
		TArray<TSoftObjectPtr<UWorld>> newWorldAssetList = CurrentChapterInfo.GetWorldList(CurrentStageInfo.StageType);
		checkf(!newWorldAssetList.IsEmpty(), TEXT("UStageManagerComponent::InitStage - newWorldAssetList가 지정되어있지않습니다."));

		// Load new level
		//	UE_LOG(LogStage, Warning, TEXT("@@@@@@@ Tuto : %d, level valid : %d"), (int32)ChapterManagerRef.Get()->GetTutorialCompletion(), (int32)CurrentChapterInfo.TutorialLevel.IsNull());
		if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Entry && !ChapterManagerRef.Get()->GetTutorialCompletion()
			&& !CurrentChapterInfo.TutorialLevel.IsNull() && CurrentChapterInfo.ChapterID == 1)
		{
			UE_LOG(LogStage, Warning, TEXT("> UStageManagerComponent::InitStage - Tutorial level will be activated"));
			CurrentStageInfo.MainLevelAsset = CurrentChapterInfo.TutorialLevel;
		}

		else if (newWorldAssetList.Num() == 1 || PreviousLevel.IsNull())
		{
			UE_LOG(LogStage, Warning, TEXT("> UStageManagerComponent::InitStage - Lack of %d's Map Count, It may cause the level reinstancing crash."), (int32)CurrentStageInfo.StageType);
			CurrentStageInfo.MainLevelAsset = newWorldAssetList[0];
		}
		else
		{
			int32 prevLevelIdx = -1;
			int32 randIdxAdder = FMath::RandRange(1, newWorldAssetList.Num() - 1);

			newWorldAssetList.Find(PreviousLevel, prevLevelIdx);
			prevLevelIdx = prevLevelIdx == INDEX_NONE ? 0 : prevLevelIdx;

			UE_LOG(LogStage, Warning, TEXT("> prev : %d, randIdx : %d, result : %d"), prevLevelIdx, randIdxAdder, (prevLevelIdx + randIdxAdder) % newWorldAssetList.Num());

			CurrentStageInfo.MainLevelAsset = newWorldAssetList[(prevLevelIdx + randIdxAdder) % newWorldAssetList.Num()];
		}
	}
	else
	{
		UE_LOG(LogSaveSystem, Log, TEXT("UStageManagerComponent::InitStage - MainLevelAsset is valid"));
	}
	
	UE_LOG(LogStage, Warning, TEXT("> MapName : %s"), *CurrentStageInfo.MainLevelAsset.ToString());
	PreviousLevel = CurrentStageInfo.MainLevelAsset;
	CreateLevelInstance(CurrentStageInfo.MainLevelAsset, CurrentStageInfo.OuterLevelAsset);
	GetWorld()->GetTimerManager().SetTimer(LoadCheckTimerHandle, this, &UStageManagerComponent::CheckLoadStatusAndStartGame, 0.25f, true);
}

void UStageManagerComponent::ClearPreviousResource()
{
	ClearPreviousActors();
	ClearPreviousLevelData();
}

float UStageManagerComponent::GetStageRemainingTime()
{
	return GetOwner()->GetWorldTimerManager().GetTimerRemaining(TimeAttackTimerHandle);
}

void UStageManagerComponent::RegisterActor(AActor* TargetActor)
{
	if (!IsValid(TargetActor)) return;
	SpawnedActorList.AddUnique(TargetActor);
}

void UStageManagerComponent::RegisterActorList(TArray<AActor*> TargetActorList)
{
	for (auto& actor : TargetActorList)
	{
		if (!IsValid(actor) || actor->IsActorBeingDestroyed()) continue;
		RegisterActor(actor);
	}
}

void UStageManagerComponent::OverwriteStageInfo(FChapterInfo NewChapterInfo, FStageInfo NewStageInfo)
{
	UE_LOG(LogSaveSystem, Log, TEXT("UStageManagerComponent::OverwriteStageInfo - Coordinate : %d"), NewStageInfo.Coordinate);
	CurrentChapterInfo = NewChapterInfo;
	CurrentStageInfo = NewStageInfo;
}

void UStageManagerComponent::AddLoadingScreen()
{
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	LoadingWidgetRef = CreateWidget(GetWorld(), LoadingWidgetClass);
	if (IsValid(LoadingWidgetRef))
	{
		LoadingWidgetRef->AddToViewport();
		bIsLoadingScreenActive = true;
	}
}

void UStageManagerComponent::RemoveLoadingScreen()
{
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	if (IsValid(LoadingWidgetRef))
	{
		LoadingWidgetRef->Destruct();
		bIsLoadingScreenActive = false;
	}
}

void UStageManagerComponent::CreateLevelInstance(TSoftObjectPtr<UWorld> MainLevel, TSoftObjectPtr<UWorld> OuterLevel)
{
	if (MainLevel.IsNull()) return;

	//Clear previous portal
	ClearPreviousActors();

	//Init loading screen
	AddLoadingScreen();

	//Start load
	bool result = false;
	LoadStatus = (OuterLevel.IsNull() ? 1 : 2);

	MainAreaRef = MainAreaRef->LoadLevelInstanceBySoftObjectPtr(GetWorld(), MainLevel, FVector(0.0f)
					, FRotator::ZeroRotator, result);
	MainAreaRef->OnLevelLoaded.AddDynamic(this, &UStageManagerComponent::UpdateLoadStatusCount);

	if (OuterLevel.IsNull()) return;
	{
		OuterAreaRef = OuterAreaRef->LoadLevelInstanceBySoftObjectPtr(GetWorld(), OuterLevel, FVector(0.0f)
			, FRotator::ZeroRotator, result);
		OuterAreaRef->OnLevelLoaded.AddDynamic(this, &UStageManagerComponent::UpdateLoadStatusCount);
	}

	//Load Begin
	OnStageLoadBegin.Broadcast();
}

void UStageManagerComponent::ClearPreviousLevelData()
{
	//Clear timer
	GetOwner()->GetWorldTimerManager().ClearTimer(LoadCheckTimerHandle);
	GetOwner()->GetWorldTimerManager().ClearTimer(TimeAttackTimerHandle);
	LoadCheckTimerHandle.Invalidate();
	TimeAttackTimerHandle.Invalidate();

	//Clear Spawn Location Info
	CurrentStageInfo.ClearSpawnLocationInfo();

	//Remove previous level instance
	if (IsValid(MainAreaRef))
	{
		MainAreaRef->SetIsRequestingUnloadAndRemoval(true);
	} 
	if (IsValid(OuterAreaRef))
	{
		OuterAreaRef->SetIsRequestingUnloadAndRemoval(true);
	}
}

void UStageManagerComponent::ClearPreviousActors()
{
	//Remove all spawned actors
	for (int idx = SpawnedActorList.Num() - 1; idx >= 0; idx--)
	{
		if (SpawnedActorList[idx].IsValid())
		{
			AActor* target = SpawnedActorList[idx].Get();
			target->Destroy();
		}
	}
	SpawnedActorList.Reset();
	RemainingEnemyCount = 0;
}

void UStageManagerComponent::ClearPreviousActorsWithTag(FName Tag)
{
	//Remove all spawned actors
	for (int idx = SpawnedActorList.Num() - 1; idx >= 0; idx--)
	{
		if (SpawnedActorList[idx].IsValid())
		{
			AActor* target = SpawnedActorList[idx].Get();

			if(!IsValid(target) || target->IsActorBeingDestroyed()) SpawnedActorList[idx] = nullptr;

			//need refactor. weapon is not destroyed together when character is destroyed using Destroy().
			else if (target->ActorHasTag(Tag))
			{
				target->Destroy();
				SpawnedActorList.RemoveAtSwap(idx);
			}
		}
	}
	RemainingEnemyCount = 0;
}

TArray<AActor*> UStageManagerComponent::TryUpdateSpawnPointProperty()
{
	if (!IsValid(MainAreaRef)) return {};

	TArray<AActor*> spawnPointList;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Point"), spawnPointList);
	const FVector zAxisCalibrationValue = FVector(0.0f, 0.0f, 50.0f);

	UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::TryUpdateSpawnPointProperty() ---------------"));

	if (spawnPointList.IsEmpty()) return {};

	UE_LOG(LogStage, Warning, TEXT("> SpawnPointList -------------"));
	
	//Clear Spawn Location Info
	//동적 추가가 필요하면 이 구문을 지우기.
	CurrentStageInfo.ClearSpawnLocationInfo();

	for (AActor*& spawnPoint : spawnPointList)
	{
		UE_LOG(LogStage, Warning, TEXT(" - %s"), *UKismetSystemLibrary::GetDisplayName(spawnPoint));
		if (!IsValid(spawnPoint) || spawnPoint->IsActorBeingDestroyed() || !spawnPoint->Tags.IsValidIndex(1)) continue;
		UE_LOG(LogStage, Warning, TEXT("   ㄴ> Tag : %s"), *spawnPoint->Tags[1].ToString());
		UE_LOG(LogStage, Warning, TEXT("   ㄴ> Location : %s"), *spawnPoint->GetActorLocation().ToCompactString());

		if (spawnPoint->Tags[1] == FName("Enemy"))
		{
			CurrentStageInfo.EnemySpawnLocationList.Add(spawnPoint->GetActorLocation() + zAxisCalibrationValue);
		}
		else if (spawnPoint->Tags[1] == FName("Boss"))
		{
			CurrentStageInfo.BossSpawnLocation = spawnPoint->GetActorLocation() + zAxisCalibrationValue;
			CurrentStageInfo.BossSpawnRotation = spawnPoint->GetActorRotation();
		}
		else if (spawnPoint->Tags[1] == FName("PlayerStart"))
		{
			CurrentStageInfo.PlayerStartLocation = spawnPoint->GetActorLocation() + zAxisCalibrationValue;
			CurrentStageInfo.PlayerStartRotation = spawnPoint->GetActorRotation();
		}
		else if (spawnPoint->Tags[1] == FName("Gate"))
		{
			CurrentStageInfo.PortalTransformList.Add(spawnPoint->GetActorTransform());
			checkf(spawnPoint->Tags.Num() >= 3, TEXT("UStageManagerComponent::TryUpdateSpawnPointProperty() - Portal SpawnPoint의 Tag[2], Direction Tag가 지정되어있지않습니다."));
			CurrentStageInfo.PortalDirectionTagList.Add(spawnPoint->Tags[2]);
		}
		else if (spawnPoint->Tags[1] == FName("Reward"))
		{
			CurrentStageInfo.RewardSpawnLocation = spawnPoint->GetActorLocation();
		}
		else if (spawnPoint->Tags[1] == FName("ItemBox"))
		{
			CurrentStageInfo.ItemBoxSpawnLocationList.Add(spawnPoint->GetActorLocation());
		}
	}
	return spawnPointList;
}

void UStageManagerComponent::SpawnEnemy()
{
	//Basic condition (stage type check and data count check
	if (!CurrentStageInfo.GetIsCombatStage()) return;

	if (CurrentStageInfo.EnemyCompositionInfo.CompositionNameList.Num() == 0)
	{
		UE_LOG(LogStage, Error, TEXT("UStageManagerComponent::SpawnEnemy : Not enough enemy composition"));
		return;
	}

	//Check data is valid
	if (CurrentStageInfo.EnemySpawnLocationList.Num() == 0)
	{
		UE_LOG(LogGameMode, Error, TEXT("------- UStageManagerComponent::SpawnEnemy() -------"));
		UE_LOG(LogGameMode, Error, TEXT("Spawn point num is not enough. Mob will not be spawned"));
	}
	//Init enemy count to zero
	RemainingEnemyCount = 0;

	//--------------Spawn enemy to world and add to SpawnedActorList-------------------------------
	TArray<FName> compositionNameList = CurrentChapterInfo.StageEnemyCompositionInfoMap[CurrentStageInfo.StageType].CompositionNameList;
	
	if (compositionNameList.IsEmpty())
	{
		UE_LOG(LogStage, Error, TEXT("UStageManagerComponent::SpawnEnemy : composition for stage type %d is empty"), (int32)CurrentStageInfo.StageType);
		return;
	}

	for (int32 idx = 0; idx < CurrentStageInfo.EnemySpawnLocationList.Num(); idx++)
	{
		const int32 randomIdx = UKismetMathLibrary::RandomInteger(compositionNameList.Num());
		const FName targetName = compositionNameList[randomIdx];
		FEnemyCompositionInfo enemyComposition;

		if (!CurrentChapterInfo.EnemyCompositionMap.Contains(targetName))
		{
			UE_LOG(LogStage, Error, TEXT("UStageManagerComponent::SpawnEnemy : composition \"%s\" is not found"), *targetName.ToString());
			continue;
		}

		TArray<int32> keyArray = {};
		FVector spawnLocation = CurrentStageInfo.EnemySpawnLocationList[idx];
		enemyComposition = CurrentChapterInfo.EnemyCompositionMap[targetName];
		enemyComposition.EnemySet.GenerateKeyArray(keyArray);

		UE_LOG(LogStage, Warning, TEXT("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
		UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::SpawnEnemy : composition - %s %d"), *targetName.ToString(), keyArray.Num());

		for (int32& enemyID : keyArray)
		{
			int32 count = enemyComposition.EnemySet[enemyID];

			while (count--)
			{
				AEnemyCharacterBase* newEnemy = GetWorld()->SpawnActor<AEnemyCharacterBase>(EnemyCharacterClass
					, spawnLocation + FVector(0.0f, 0.0f, 200.0f), FRotator::ZeroRotator);
				if (IsValid(newEnemy))
				{
					newEnemy->CharacterID = enemyID;
					newEnemy->InitEnemyCharacter(enemyID);
					newEnemy->InitAsset(enemyID);
					newEnemy->SpawnDefaultController();
					newEnemy->EnemyDeathDelegate.BindUFunction(this, FName("UpdateEnemyCountAndCheckClear"));
					Cast<AAIControllerBase>(newEnemy->GetController())->ActivateAI();

					SpawnedActorList.Add(newEnemy);
					if (OnActorSpawned.IsBound())
					{
						OnActorSpawned.Broadcast(spawnLocation, newEnemy);
					}
					RemainingEnemyCount += 1;
				}
			}
		}
	}
	
	//--------------Spawn boss character to world -----------------------------
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Boss)
	{
		checkf(IsValid(CurrentChapterInfo.BossCharacterClass), TEXT("UStageManagerComponent::현재 챕터에 보스가 지정되어있지 않습니다. "));
		
		AEnemyCharacterBase* boss = GetWorld()->SpawnActor<AEnemyCharacterBase>(CurrentChapterInfo.BossCharacterClass
			, CurrentStageInfo.BossSpawnLocation, CurrentStageInfo.BossSpawnRotation);
		if (IsValid(boss))
		{
			boss->InitEnemyCharacter(boss->CharacterID);
			boss->InitAsset(boss->CharacterID);
			boss->EnemyDeathDelegate.BindUFunction(this, FName("UpdateEnemyCountAndCheckClear"));
			Cast<AAIControllerBase>(boss->GetController())->ActivateAI();
			SpawnedActorList.Add(boss);
			RemainingEnemyCount += 1;
		}
	}
}

void UStageManagerComponent::SpawnCraftbox()
{
	//Spawn craft box~~
}

void UStageManagerComponent::SpawnItemBox()
{
	//Basic condition (stage type check and data count check
	if (!IsValid(ItemBoxClass)) return;
	if (!CurrentStageInfo.GetIsCombatStage()) return;
	
	//Check data is valid
	if (!CurrentChapterInfo.MaxItemBoxSpawnCountMap.Contains(CurrentStageInfo.StageType))
	{
		UE_LOG(LogGameMode, Error, TEXT("----- UStageManagerComponent::SpawnItemBox() ----------"));
		UE_LOG(LogGameMode, Error, TEXT("> MaxItemBoxSpawnCountMap is not defined for type #%d"), (int32)CurrentStageInfo.StageType);
		return;
	}
	if (CurrentChapterInfo.MaxItemBoxSpawnCountMap[CurrentStageInfo.StageType] > CurrentStageInfo.ItemBoxSpawnLocationList.Num())
	{
		UE_LOG(LogGameMode, Error, TEXT("----- UStageManagerComponent::SpawnItemBox() ----------"));
		UE_LOG(LogGameMode, Error, TEXT("> Spawn point num is less than MaxItemBoxSpanwCount num."));
		UE_LOG(LogGameMode, Error, TEXT("> There can be collapse among item boxes."));
	}
	//Set itembox spawn property
	int32 spawnItemboxCount = UKismetMathLibrary::RandomInteger(CurrentChapterInfo.MaxItemBoxSpawnCountMap[CurrentStageInfo.StageType]) + 1;
	int32 randomIdxOffset = UKismetMathLibrary::RandomInteger(CurrentStageInfo.ItemBoxSpawnLocationList.Num());

	//--------------Spawn item to world and add to SpawnedActorList-------------------------------
	if (CurrentStageInfo.ItemBoxSpawnLocationList.Num() > 0)
	{
		for (int32 idx = 0; idx < spawnItemboxCount; idx++)
		{
			TArray<int32> keyArray = {};
			FVector spawnLocation = CurrentStageInfo.ItemBoxSpawnLocationList[(idx + randomIdxOffset) % CurrentStageInfo.ItemBoxSpawnLocationList.Num()];

			AItemBoxBase* newItemBox = GetWorld()->SpawnActor<AItemBoxBase>(ItemBoxClass
										, spawnLocation + FVector(0.0f, 0.0f, 200.0f), FRotator::ZeroRotator);
			
			SpawnedActorList.Add(newItemBox);
			if (OnActorSpawned.IsBound())
			{
				//OnActorSpawned.Broadcast(spawnLocation, newItemBox);
			}

			if (IsValid(newItemBox))
			{
				newItemBox->ActivateProjectileMovement();
				newItemBox->ActivateItem();
			}
		}
	}
}

void UStageManagerComponent::SpawnPortal(int32 GateCount)
{
	if (!ChapterManagerRef.IsValid()) return;
	if (CurrentStageInfo.PortalTransformList.Num() < GateCount
		|| CurrentStageInfo.PortalDirectionTagList.Num() < GateCount)
	{
		UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::SpawnPortal -> Not Enough Portal Spawn Point!"));
		return;
	}

	UStageGeneratorComponent* stageGeneratorRef = ChapterManagerRef.Get()->StageGeneratorComponent;
	for (int32 idx = 0; idx < GateCount; idx++)
	{
		const FVector spawnLocation = CurrentStageInfo.PortalTransformList[idx].GetLocation();
		const FRotator spawnRotation = CurrentStageInfo.PortalTransformList[idx].GetRotation().Rotator();

		AGateBase* newGate = GetWorld()->SpawnActor<AGateBase>(GateClass, spawnLocation, spawnRotation);
		if (IsValid(newGate))
		{
			SpawnedActorList.Add(newGate);
			if (OnActorSpawned.IsBound())
			{
				//OnActorSpawned.Broadcast(spawnLocation, newGate);
			}

			FStageInfo nextStageInfo = ChapterManagerRef.Get()->GetStageInfo(CurrentStageInfo.Coordinate + 1);
			newGate->InitGate(idx, nextStageInfo);

			//temp
			newGate->ActivateGate();
			newGate->OnEnterRequestReceived.BindUFunction(GetOwner(), FName("MoveStage"));

			//다음 스테이지가 보스나 제작 스테이지라면, 포탈을 1개만 생성한다
			if (nextStageInfo.StageType == EStageCategoryInfo::E_Boss
				|| nextStageInfo.StageType == EStageCategoryInfo::E_Craft) break;
		}
	}
}

void UStageManagerComponent::CheckLoadStatusAndStartGame()
{
	if (GetWorld()->GetTimerManager().GetTimerElapsed(LoadCheckTimerHandle) > LoadTimeOut) return;

	if (GetLoadIsDone())
	{
		//Update spawn points 
		//LoadDone이 떠도, 
		UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame : Update Spawn Point"));
		TArray<AActor*> resultList = TryUpdateSpawnPointProperty();

		if (!resultList.IsEmpty())
		{
			//Clear timer and invalidate
			GetWorld()->GetTimerManager().ClearTimer(LoadCheckTimerHandle);
			LoadCheckTimerHandle.Invalidate();

			//Clear Prev Actors
			UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame : Clear Previous Actors"));
			ClearPreviousActors();

			//Register Points
			RegisterActorList(resultList);

			//Start stage with delay
			UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame() : Load Done, StartStage"));
			FTimerHandle gameStartDelayHandle;
			
			GetWorld()->GetTimerManager().SetTimer(gameStartDelayHandle, this, &UStageManagerComponent::StartStage, 2.0f, false);
		}
		else
		{
			UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame() : Post load task (UpdateSpawnPointProperty) Failed, Retry..."));
			GetWorld()->GetTimerManager().ClearTimer(LoadCheckTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(LoadCheckTimerHandle, this, &UStageManagerComponent::CheckLoadStatusAndStartGame, 2.0f, false);
		}
	}
}

void UStageManagerComponent::StartStage()
{
	//Load End Broadcast (Safe with latent delay)
	OnStageLoadDone.Broadcast();

	//Remove loading screen 
	UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::StartStage : Remove loading screen"));
	RemoveLoadingScreen();

	//spawn enemy and portal
	UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::StartStage : SpawnEnemy"));
	SpawnEnemy();

	//spawn ItemBox
	UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::StartStage : ItemBox"));
	SpawnItemBox();

	//Move player to start location
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (IsValid(playerCharacter))
	{
		playerCharacter->SetActorLocation(CurrentStageInfo.PlayerStartLocation);
		playerCharacter->SetActorRotation(CurrentStageInfo.PlayerStartRotation);
		playerCharacter->GetController()->SetControlRotation(CurrentStageInfo.PlayerStartRotation);
	}


	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Craft)
	{
		FinishStage(true);
	}
}

void UStageManagerComponent::FinishStage(bool bStageClear)
{
	if (CurrentStageInfo.bIsFinished)
	{
		UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::FinishStage - Stage is already finished"));
		return;
	}
	CurrentStageInfo.bIsClear = bStageClear;
	CurrentStageInfo.bIsFinished = true;

	//Clear all remaining actors
	ClearPreviousActorsWithTag("Point");

	//Clear time attack timer handle
	GetOwner()->GetWorldTimerManager().ClearTimer(TimeAttackTimerHandle);
	TimeAttackTimerHandle.Invalidate();

	//if this is not end stage, then spawn the portal
	if (!CurrentStageInfo.bIsGameOver && CurrentStageInfo.StageType != EStageCategoryInfo::E_Boss)
	{
		SpawnPortal(2);
	}

	if (bStageClear && CurrentStageInfo.GetIsCombatStage(false))
	{
		//Stage Clear UI Update using delegate
		OnStageCleared.Broadcast();

		//Stage Reward
		GrantStageRewards();

		if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Entry && !ChapterManagerRef.Get()->GetTutorialCompletion())
		{
			UE_LOG(LogStage, Warning, TEXT("> UStageManagerComponent::FinishStage - Tutorial level will be activated"));
		}
	}

	UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::FinishStage - Stage Finished!"));
	OnStageFinished.Broadcast(CurrentStageInfo);
}

void UStageManagerComponent::GrantStageRewards()
{
	AMainCharacterBase* playerCharacter = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!IsValid(playerCharacter)) return;

	TArray<FItemInfoDataStruct> rewardInfoList = GetCurrentStageInfo().RewardInfoList;
	if(rewardInfoList.IsEmpty())
	{
		UE_LOG(LogStage, Warning, TEXT("UStageManagerComponent::GrantStageRewards - No reward info found for stage %d"), CurrentStageInfo.Coordinate);
		return;
	}
	for (FItemInfoDataStruct& rewardItemInfo : rewardInfoList)
	{
		UE_LOG(LogStage, Log, TEXT("UStageManagerComponent::GrantStageRewards - ItemID : %d, Amount : %d, bIsActorItem : %d"), 
			rewardItemInfo.ItemID, rewardItemInfo.ItemAmount, rewardItemInfo.bIsActorItem);
		if (rewardItemInfo.bIsActorItem)
		{
			SpawnedActorList.Add(SpawnItemActor(rewardItemInfo));
		}
		else
		{
			playerCharacter->ItemInventory->AddItem(rewardItemInfo.ItemID, rewardItemInfo.ItemAmount);
		}
		break;
	}
}

AActor* UStageManagerComponent::SpawnItemActor(FItemInfoDataStruct ItemInfo)
{
	if (ItemInfo.ItemID == 0 || !ItemInfo.bIsActorItem || ItemInfo.ItemClass == nullptr) return nullptr;

	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//Spawn location will be edited.
	FVector spawnLocation = CurrentStageInfo.RewardSpawnLocation + FVector(0, 0, 100);
	AItemBase* newItem = Cast<AItemBase>(GetWorld()->SpawnActor(ItemInfo.ItemClass, &spawnLocation, nullptr, actorSpawnParameters));
	
	if (IsValid(newItem))
	{
		newItem->SetOwner(GetOwner());
		newItem->InitItem(ItemInfo.ItemID, ItemInfo);
		RegisterActor(newItem);
	}
	return newItem;
}

void UStageManagerComponent::UpdateEnemyCountAndCheckClear()
{
	UpdateRemainingEnemyCount();
	UE_LOG(LogStage, Log, TEXT("UStageManagerComponent::UpdateEnemyCountAndCheckClear - Remaining Enemy Count : %d"), RemainingEnemyCount);
	
	if (CheckStageClearStatus())
	{
		UE_LOG(LogStage, Log, TEXT("UStageManagerComponent::UpdateEnemyCountAndCheckClear - Stage Clear!"));
		FinishStage(true);
	}
}

bool UStageManagerComponent::CheckStageClearStatus()
{
	//Basic condition (stage type check)
	if (CurrentStageInfo.GetIsCombatStage())
	{
		return RemainingEnemyCount == 0;
	}
	return false;
}

void UStageManagerComponent::SetGameIsOver()
{
	CurrentStageInfo.bIsClear = false;
	CurrentStageInfo.bIsGameOver = true;
	FinishStage(false);
}
