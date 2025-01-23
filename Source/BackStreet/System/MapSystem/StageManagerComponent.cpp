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

	UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::InitStage-------------"));
	UE_LOG(LogTemp, Warning, TEXT("> Stage Type : %d"), CurrentStageInfo.StageType);
	UE_LOG(LogTemp, Warning, TEXT("> Coordinate : %s"), *CurrentStageInfo.Coordinate.ToString());

	//Update visited craft stage count
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Craft)
	{
		VisitedCraftstageCount += 1;
	}

	//Check level asset list is not empty	
	TArray<TSoftObjectPtr<UWorld>> newWorldAssetList = CurrentChapterInfo.GetWorldList(NewStageInfo.StageType);
	checkf(!newWorldAssetList.IsEmpty(), TEXT("UStageManagerComponent::InitStage - newWorldAssetList가 지정되어있지않습니다."));

	//Load new level
	if (newWorldAssetList.Num() == 1 || PreviousLevel.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("> UStageManagerComponent::InitStage - Lack of %d's Map Count, It may cause the level reinstancing crash."), (int32)NewStageInfo.StageType);
		NewStageInfo.MainLevelAsset = newWorldAssetList[0];
	}
	else
	{
		int32 prevLevelIdx = -1;
		int32 randIdxAdder = FMath::RandRange(1, newWorldAssetList.Num() - 1);

		newWorldAssetList.Find(PreviousLevel, prevLevelIdx);
		prevLevelIdx = prevLevelIdx == INDEX_NONE ? 0 : prevLevelIdx;

		UE_LOG(LogTemp, Warning, TEXT("> prev : %d, randIdx : %d, result : %d"), prevLevelIdx, randIdxAdder, (prevLevelIdx + randIdxAdder) % newWorldAssetList.Num());

		NewStageInfo.MainLevelAsset = newWorldAssetList[(prevLevelIdx + randIdxAdder) % newWorldAssetList.Num()];
	}
	UE_LOG(LogTemp, Warning, TEXT("> MapName : %s"), *NewStageInfo.MainLevelAsset.ToString());
	PreviousLevel = NewStageInfo.MainLevelAsset;
	CreateLevelInstance(NewStageInfo.MainLevelAsset, NewStageInfo.OuterLevelAsset);
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
	SpawnedActorList.Add(TargetActor);
}

void UStageManagerComponent::AddLoadingScreen()
{
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	LoadingWidgetRef = CreateWidget(GetWorld(), LoadingWidgetClass);
	if (IsValid(LoadingWidgetRef))
	{
		LoadingWidgetRef->AddToViewport();
	}
}

void UStageManagerComponent::RemoveLoadingScreen()
{
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	if (IsValid(LoadingWidgetRef))
	{
		LoadingWidgetRef->Destruct();
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
			SpawnedActorList[idx] = nullptr;
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
				SpawnedActorList[idx] = nullptr;
				target->Destroy();
			}
		}
	}
	RemainingEnemyCount = 0;
}

bool UStageManagerComponent::TryUpdateSpawnPointProperty()
{
	if (!IsValid(MainAreaRef)) return false;

	TArray<AActor*> spawnPointList;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Point"), spawnPointList);
	const FVector zAxisCalibrationValue = FVector(0.0f, 0.0f, 50.0f);

	if (spawnPointList.IsEmpty()) return false;

	for (AActor*& spawnPoint : spawnPointList)
	{
		if (!IsValid(spawnPoint) || !spawnPoint->Tags.IsValidIndex(1)) continue;
		
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
	return true;
}

void UStageManagerComponent::SpawnEnemy()
{
	//Basic condition (stage type check and data count check
	if (CurrentStageInfo.StageType != EStageCategoryInfo::E_Entry
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_TimeAttack
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Exterminate
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Escort
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_EliteTimeAttack
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_EliteExterminate
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_EliteEscort
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Boss) return;

	if (CurrentStageInfo.EnemyCompositionInfo.CompositionNameList.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("UStageManagerComponent::SpawnEnemy : Not enough enemy composition"));
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
		UE_LOG(LogTemp, Error, TEXT("UStageManagerComponent::SpawnEnemy : composition for stage type %d is empty"), (int32)CurrentStageInfo.StageType);
		return;
	}

	for (int32 idx = 0; idx < CurrentStageInfo.EnemySpawnLocationList.Num(); idx++)
	{
		const int32 randomIdx = UKismetMathLibrary::RandomInteger(compositionNameList.Num());
		const FName targetName = compositionNameList[randomIdx];
		FEnemyCompositionInfo enemyComposition;

		if (!CurrentChapterInfo.EnemyCompositionMap.Contains(targetName))
		{
			UE_LOG(LogTemp, Error, TEXT("UStageManagerComponent::SpawnEnemy : composition \"%s\" is not found"), *targetName.ToString());
			continue;
		}

		TArray<int32> keyArray = {};
		FVector spawnLocation = CurrentStageInfo.EnemySpawnLocationList[idx];
		enemyComposition = CurrentChapterInfo.EnemyCompositionMap[targetName];
		enemyComposition.EnemySet.GenerateKeyArray(keyArray);

		UE_LOG(LogTemp, Warning, TEXT("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
		UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::SpawnEnemy : composition - %s %d"), *targetName.ToString(), keyArray.Num());

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
	if (CurrentStageInfo.StageType != EStageCategoryInfo::E_Entry
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_TimeAttack
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Exterminate
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Escort
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_EliteTimeAttack
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_EliteExterminate
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_EliteEscort
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Boss) return;
	
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
		UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::SpawnPortal -> Not Enough Portal Spawn Point!"));
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
			
			FName directionTag = CurrentStageInfo.PortalDirectionTagList[idx];
			FVector2D direction = ((directionTag == FName("Up")) ? FVector2D(1, 0)
								: (directionTag == FName("Down")) ? FVector2D(0, 1) : FVector2D(0));
			checkf(direction != FVector2D(0), TEXT("Gate spawn point direction tag is invalid"));
			if (!stageGeneratorRef->GetIsCoordinateInBoundary(CurrentStageInfo.Coordinate + direction)
				|| ChapterManagerRef.Get()->GetIsStageBlocked(CurrentStageInfo.Coordinate + direction))
			{
				newGate->Destroy();
				continue;
			}

			FStageInfo nextStageInfo = ChapterManagerRef.Get()->GetStageInfoWithCoordinate(CurrentStageInfo.Coordinate + direction);
			newGate->InitGate(direction, nextStageInfo);

			//temp
			newGate->ActivateGate();
			newGate->OnEnterRequestReceived.BindUFunction(GetOwner(), FName("MoveStage"));
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
		UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame : Update Spawn Point"));
		bool result = TryUpdateSpawnPointProperty();

		if (result)
		{
			//Clear timer and invalidate
			GetWorld()->GetTimerManager().ClearTimer(LoadCheckTimerHandle);
			LoadCheckTimerHandle.Invalidate();

			//Clear Prev Actors
			UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame : Clear Previous Actors"));
			ClearPreviousActors();

			//Start stage with delay
			UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame() : Load Done, StartStage"));
			FTimerHandle gameStartDelayHandle;
			GetWorld()->GetTimerManager().SetTimer(gameStartDelayHandle, this, &UStageManagerComponent::StartStage, 2.0f, false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame() : Post load task (UpdateSpawnPointProperty) Failed, Retry..."));
			GetWorld()->GetTimerManager().ClearTimer(LoadCheckTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(LoadCheckTimerHandle, this, &UStageManagerComponent::CheckLoadStatusAndStartGame, 2.0f, false);
		}
	}
}

void UStageManagerComponent::StartStage()
{
	//Visit Check
	if (CurrentStageInfo.bIsVisited) return; 
	CurrentStageInfo.bIsVisited = true;

	//Load End Broadcast (Safe with latent delay)
	OnStageLoadDone.Broadcast();

	//Remove loading screen 
	UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::StartStage : Remove loading screen"));
	RemoveLoadingScreen();

	//spawn enemy and portal
	UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::StartStage : SpawnEnemy"));
	SpawnEnemy();

	//spawn ItemBox
	UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::StartStage : ItemBox"));
	SpawnItemBox();

	//Check next stage's reward validation
	EnsureUniqueStageRewards(CurrentStageInfo.Coordinate);

	//Move player to start location
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (IsValid(playerCharacter))
	{
		playerCharacter->SetActorLocation(CurrentStageInfo.PlayerStartLocation);
		playerCharacter->SetActorRotation(CurrentStageInfo.PlayerStartRotation);
		playerCharacter->GetController()->SetControlRotation(CurrentStageInfo.PlayerStartRotation);
	}

	//Start timer if stage type if timeattack
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteTimeAttack)
	{
		FTimerDelegate stageOverDelegate;
		stageOverDelegate.BindUFunction(this, FName("FinishStage"), false);

		float extraTime = !PlayerRef.IsValid() ? 0.0f : PlayerRef.Get()->GetCharacterGameplayInfo().ExtraStageTime;
		GetOwner()->GetWorldTimerManager().SetTimer(TimeAttackTimerHandle, stageOverDelegate, 1.0f, false, CurrentStageInfo.TimeLimitValue + extraTime);
		
		//UI Event
		OnTimeAttackStageBegin.Broadcast();
	}
	else if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Craft
			|| CurrentStageInfo.StageType == EStageCategoryInfo::E_MiniGame
			|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Gatcha)
	{
		FinishStage(true);
	}
}

void UStageManagerComponent::FinishStage(bool bStageClear)
{
	if (CurrentStageInfo.bIsFinished) return;
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

	if (bStageClear &&
		(CurrentStageInfo.StageType == EStageCategoryInfo::E_Entry
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Exterminate
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteExterminate))
	{
		//Stage Clear UI Update using delegate
		OnStageCleared.Broadcast();

		//Stage Reward
		GrantStageRewards();
	}
	else if (CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteTimeAttack)
	{
		if (bStageClear)
		{
			//Stage Clear UI Update using delegate
			OnStageCleared.Broadcast();

			//Stage Reward
			GrantStageRewards();
		}
		else
		{
			//Time Out using delegate
			OnTimeIsOver.Broadcast();	

		}
		//TimeAttack UI Event
		OnTimeAttackStageEnd.Broadcast();
	}
	
	//StageFinished Delegate
	OnStageFinished.Broadcast(CurrentStageInfo);
}

void UStageManagerComponent::EnsureUniqueStageRewards(FVector2D Coordinate)
{
	UE_LOG(LogTemp, Warning, TEXT("===========UStageManagerComponent::EnsureUniqueStageRewards(%d, %d)==================="), (int32)Coordinate.Y, (int32)Coordinate.X);

	const TArray<FVector2D> directions = { {0.0f, 1.0f}, {1.0f, 0.0f} }; //좌, 우 검색
	TArray<int32> rewardIDList; //{Left, Right} Stage
	
	//현재 Component가 살아있다 -> Owner가 살아있음이 보장 -> GeneratorComponent도 살아있음
	UStageGeneratorComponent* stageGeneratorRef = ChapterManagerRef.Get()->StageGeneratorComponent;

	//좌, 우 존재 유무 검사
	for (int32 idx = 0; idx < directions.Num(); idx++)
	{
		FVector2D nextCoordinate = Coordinate + directions[idx];
		bool result = !stageGeneratorRef->GetIsCoordinateInBoundary(nextCoordinate) 
					|| ChapterManagerRef.Get()->GetStageInfoWithCoordinate(nextCoordinate).bIsBlocked;
		if (result)
		{
			UE_LOG(LogTemp, Warning, TEXT("> One side is blocked, return the function."));
			return;  //한쪽이 Blocked 되어있거나 맵 경계 밖이라면 수정을 할 필요가 없음
		}
	}
	
	FStageInfo& leftStageInfo = ChapterManagerRef.Get()->GetStageInfoWithCoordinate(Coordinate + directions[0]);
	FStageInfo& rightStageInfo = ChapterManagerRef.Get()->GetStageInfoWithCoordinate(Coordinate + directions[1]);

	if (!leftStageInfo.RewardInfoList.IsEmpty()
		&& leftStageInfo.RewardInfoList.Num() == rightStageInfo.RewardInfoList.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("> first comp - %d, %d"), leftStageInfo.RewardInfoList[0].ItemID, rightStageInfo.RewardInfoList[0].ItemID);

		//정렬 되어있음을 가정
		for (int32 idx = 0; idx < leftStageInfo.RewardInfoList.Num(); idx++)
		{
			if (leftStageInfo.RewardInfoList[idx].ItemID != rightStageInfo.RewardInfoList[idx].ItemID)
			{
				UE_LOG(LogTemp, Warning, TEXT("> Difference is detected, return the function"));
				return;
			}
		}
		
		//새로운 보상으로 지정한다.
		for (auto& rewardInfo : rightStageInfo.RewardInfoList)//convert to id list
			rewardIDList.Add(rewardInfo.ItemID);
		leftStageInfo.RewardInfoList = stageGeneratorRef->GetRewardListFromCandidates(leftStageInfo.StageType, rewardIDList);
		leftStageInfo.StageIcon = leftStageInfo.RewardInfoList[0].ItemImage;
		UE_LOG(LogTemp, Warning, TEXT("> after change - %d, %d"), leftStageInfo.RewardInfoList[0].ItemID, rightStageInfo.RewardInfoList[0].ItemID);

	}
}

void UStageManagerComponent::GrantStageRewards()
{
	AMainCharacterBase* playerCharacter = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!IsValid(playerCharacter)) return;

	TArray<FItemInfoDataStruct> rewardInfoList = GetCurrentStageInfo().RewardInfoList;
	for (FItemInfoDataStruct& rewardItemInfo : rewardInfoList)
	{
		if (rewardItemInfo.bIsActorItem)
		{
			SpawnedActorList.Add(SpawnItemActor(rewardItemInfo));
		}
		else
		{
			playerCharacter->ItemInventory->AddItem(rewardItemInfo.ItemID, rewardItemInfo.ItemAmount);
		}
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
	if (CheckStageClearStatus())
	{
		FinishStage(true);
	}
}

bool UStageManagerComponent::CheckStageClearStatus()
{
	//Basic condition (stage type check)
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Entry
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Exterminate
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteExterminate
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Escort
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteEscort
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Boss
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteExterminate)
	{
		return RemainingEnemyCount == 0;
	}
	else if (CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteTimeAttack)
	{
		bool bIsTimeLeft = GetRemainingTime() > 0.0f;
		return RemainingEnemyCount == 0 && bIsTimeLeft;
	}

	return false;
}

void UStageManagerComponent::SetGameIsOver()
{
	CurrentStageInfo.bIsClear = false;
	CurrentStageInfo.bIsGameOver = true;
	FinishStage(false);
}
