// Fill out your copyright notice in the Description page of Project Settings.


#include "StageManagerComponent.h"
#include "StageGeneratorComponent.h"
#include "../../Character/EnemyCharacter/EnemyCharacterBase.h"
#include "../../Character/CharacterBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
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
		Cast<ACharacterBase>(playerCharacter)->OnCharacterDied.AddDynamic(this, &UStageManagerComponent::SetGameIsOver);
	}
	ChapterManagerRef = Cast<ANewChapterManagerBase>(GetOwner());
}

void UStageManagerComponent::Initialize(FChapterInfo NewChapterInfo)
{
	CurrentChapterInfo = NewChapterInfo;
}

void UStageManagerComponent::InitStage(FStageInfo NewStageInfo)
{
	if (IsValid(MainAreaRef)) ClearPreviousLevelData();
	
	//Clear previous portal
	ClearPreviousActors();

	//Init new stage
	CurrentStageInfo = NewStageInfo;

	UE_LOG(LogTemp, Warning, TEXT("=========== Init Stage ============"));
	UE_LOG(LogTemp, Warning, TEXT("> Stage Type : %d"), CurrentStageInfo.StageType);
	UE_LOG(LogTemp, Warning, TEXT("> Coordinate : %s"), *CurrentStageInfo.Coordinate.ToString());

	//Load new level
	CreateLevelInstance(NewStageInfo.MainLevelAsset, NewStageInfo.OuterLevelAsset);
	GetWorld()->GetTimerManager().SetTimer(LoadCheckTimerHandle, this, &UStageManagerComponent::CheckLoadStatusAndStartGame, 1.0f, true);
}

void UStageManagerComponent::ClearResource()
{
	ClearPreviousActors();
	ClearPreviousLevelData();
}

float UStageManagerComponent::GetStageRemainingTime()
{
	return GetOwner()->GetWorldTimerManager().GetTimerRemaining(TimeAttackTimerHandle);
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
	if (!IsValid(MainAreaRef) || !IsValid(OuterAreaRef)) return; 

	//Remove previous level instance
	MainAreaRef->SetIsRequestingUnloadAndRemoval(true);
	OuterAreaRef->SetIsRequestingUnloadAndRemoval(true);

	//Clear timer
	GetOwner()->GetWorldTimerManager().ClearTimer(LoadCheckTimerHandle);
	GetOwner()->GetWorldTimerManager().ClearTimer(TimeAttackTimerHandle);
	LoadCheckTimerHandle.Invalidate();
	TimeAttackTimerHandle.Invalidate();
}

void UStageManagerComponent::ClearPreviousActors()
{
	//Remove all spawned actors
	for (int idx = SpawnedActorList.Num() - 1; idx >= 0; idx--)
	{
		if (SpawnedActorList[idx].IsValid())
		{
			AActor* target = SpawnedActorList[idx].Get();
			SpawnedActorList[idx] = nullptr;
			
			//need refactor. weapon is not destroyed together when character is destroyed using Destroy().
			if (target->ActorHasTag("Character"))
			{
				UGameplayStatics::ApplyDamage(target, 1e8, nullptr, GetOwner(), nullptr);
			}
			else
			{
				target->Destroy();
			}
		}
	}
	SpawnedActorList.Reset();
	RemainingEnemyCount = 0;
}

void UStageManagerComponent::UpdateSpawnPointProperty()
{
	if (!IsValid(MainAreaRef)) return;

	TArray<AActor*> spawnPointList;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Point"), spawnPointList);
	const FVector zAxisCalibrationValue = FVector(0.0f, 0.0f, 50.0f);

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
			CurrentStageInfo.PortalLocationList.Add(spawnPoint->GetActorLocation() + zAxisCalibrationValue);
			checkf(spawnPoint->Tags.Num() >= 3, TEXT("Portal SpawnPoint의 Tag[2], Direction Tag가 지정되어있지않습니다."));
			CurrentStageInfo.PortalDirectionTagList.Add(spawnPoint->Tags[2]);
		}
	}
}

void UStageManagerComponent::SpawnEnemy()
{
	//Basic condition (stage type check and data count check
	if (CurrentStageInfo.StageType != EStageCategoryInfo::E_Entry
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Exterminate
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_TimeAttack
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Combat
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Boss) return;

	if (CurrentStageInfo.EnemyCompositionInfo.CompositionList.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::SpawnEnemy : Not enough enemy composition"));
		return;
	}

	//Check data is valid
	if (CurrentStageInfo.EnemyCompositionInfo.CompositionList.Num() > CurrentStageInfo.EnemySpawnLocationList.Num())
	{
		UE_LOG(LogGameMode, Warning, TEXT("------- UStageManagerComponent::SpawnEnemy() -------"));
		UE_LOG(LogGameMode, Warning, TEXT("Spawn point num is less than enemy composition list num."));
		UE_LOG(LogGameMode, Warning, TEXT("There can be collapse among enemy characters."));
	}

	//Init enemy count to zero
	RemainingEnemyCount = 0;

	//--------------Spawn enemy to world and add to SpawnedActorList-------------------------------
	if (CurrentStageInfo.EnemySpawnLocationList.Num() > 0)
	{
		for (int32 idx = 0; idx < CurrentStageInfo.EnemyCompositionInfo.CompositionList.Num(); idx++)
		{
			FEnemyGroupInfo& composition = CurrentStageInfo.EnemyCompositionInfo.CompositionList[idx];

			TArray<int32> keyArray = {};
			FVector spawnLocation = CurrentStageInfo.EnemySpawnLocationList[idx % CurrentStageInfo.EnemySpawnLocationList.Num()];
			composition.EnemySet.GenerateKeyArray(keyArray);

			for (int32& enemyID : keyArray)
			{
				int32 count = composition.EnemySet[enemyID];

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
						RemainingEnemyCount += 1;
					}
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

void UStageManagerComponent::SpawnPortal(int32 GateCount)
{
	if (!ChapterManagerRef.IsValid()) return;
	if (CurrentStageInfo.PortalLocationList.Num() < GateCount
		|| CurrentStageInfo.PortalDirectionTagList.Num() < GateCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::SpawnPortal -> Not Enough Portal Spawn Point!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::SpawnPortal, spawn %d portal"), GateCount);

	UStageGeneratorComponent* stageGeneratorRef = ChapterManagerRef.Get()->StageGeneratorComponent;
	for (int32 idx = 0; idx < GateCount; idx++)
	{
		AGateBase* newGate = GetWorld()->SpawnActor<AGateBase>(GateClass, CurrentStageInfo.PortalLocationList[idx], FRotator::ZeroRotator);
		if (IsValid(newGate))
		{
			SpawnedActorList.Add(newGate);
			
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
			UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::SpawnPortal, spawn %s portal"), *directionTag.ToString());

			EStageCategoryInfo nextStageType = ChapterManagerRef.Get()->GetStageInfoWithCoordinate(CurrentStageInfo.Coordinate + direction).StageType;
			newGate->InitGate(direction, ChapterManagerRef.Get()->GetStageTypeName(nextStageType));

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
		UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::CheckLoadStatusAndStartGame() : Load Done, StartStage"));

		//Clear timer and invalidate
		GetWorld()->GetTimerManager().ClearTimer(LoadCheckTimerHandle);
		LoadCheckTimerHandle.Invalidate();

		//Start stage with delay
		FTimerHandle gameStartDelayHandle;
		GetWorld()->GetTimerManager().SetTimer(gameStartDelayHandle, this, &UStageManagerComponent::StartStage, 2.0f, false);
	}
}

void UStageManagerComponent::StartStage()
{
	//Visit Check
	if (CurrentStageInfo.bIsVisited) return; 
	CurrentStageInfo.bIsVisited = true;

	//Load End
	OnStageLoadEnd.Broadcast();

	//Update spawn points
	UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::StartStage : Update Spawn Point"));
	UpdateSpawnPointProperty();

	//Remove loading screen 
	UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::StartStage : Remove loading screen"));
	RemoveLoadingScreen();

	//spawn enemy and portal
	UE_LOG(LogTemp, Warning, TEXT("UStageManagerComponent::StartStage : SpawnEnemy"));
	SpawnEnemy();

	//Move player to start location
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (IsValid(playerCharacter))
	{
		playerCharacter->SetActorLocation(CurrentStageInfo.PlayerStartLocation);
		playerCharacter->SetActorRotation(CurrentStageInfo.PlayerStartRotation);
		playerCharacter->GetController()->SetControlRotation(CurrentStageInfo.PlayerStartRotation);
	}

	//Start timer if stage type if timeattack
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack)
	{
		FTimerDelegate stageOverDelegate;
		stageOverDelegate.BindUFunction(this, FName("FinishStage"), false);
		GetOwner()->GetWorldTimerManager().SetTimer(TimeAttackTimerHandle, stageOverDelegate, 1.0f, false, CurrentStageInfo.TimeLimitValue);
		
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
	ClearPreviousActors();

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
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Combat))
	{
		//Stage Clear UI Update using delegate
		OnStageCleared.Broadcast();

		//Stage Reward
		GrantStageRewards();

		//=========Temporary code for bic===========================
		if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Exterminate)
		{
			ACharacterBase* playerRef = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

			if(IsValid(playerRef))	
			{
				float healValue = playerRef->GetCharacterStat().DefaultHP * 0.1f;
				UE_LOG(LogTemp, Warning, TEXT("Take Heal %.2lf"), healValue);
				playerRef->TakeHeal(healValue);
			}
		}
	}
	else if (CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack)
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

void UStageManagerComponent::GrantStageRewards()
{
	AMainCharacterBase* playerCharacter = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!IsValid(playerCharacter)) return;

	FStageRewardInfoList* rewardInfoList = CurrentChapterInfo.StageRewardInfoMap.Find(CurrentStageInfo.StageType);
	if (rewardInfoList)
	{
		for (auto& rewardCandidateInfo : rewardInfoList->RewardCandidateInfoList)
		{
			if (rewardCandidateInfo.RewardItemIDList.Num() != rewardCandidateInfo.RewardItemProbabilityList.Num()) continue;

			// calculate total probability
			float totalProbability = 0.0f;
			for (float probability : rewardCandidateInfo.RewardItemProbabilityList)
			{
				totalProbability += probability;
			}

			// generate random value
			float randomValue = FMath::FRandRange(0.0f, totalProbability);

			// select reward by cumulative probability
			float cumulativeProbability = 0.0f;
			for (int32 candidateIdx = 0; candidateIdx < rewardCandidateInfo.RewardItemIDList.Num(); ++candidateIdx)
			{
				cumulativeProbability += rewardCandidateInfo.RewardItemProbabilityList[candidateIdx];

				if (randomValue <= cumulativeProbability)
				{
					// selected reward item id
					int32 selectedRewardItemID = rewardCandidateInfo.RewardItemIDList[candidateIdx];

					//add item to inventory
					playerCharacter->ItemInventory->AddItem(selectedRewardItemID, 1);

					UE_LOG(LogTemp, Warning, TEXT("Reward Granted %d!@@@@@@@@@@"), selectedRewardItemID);
					break;
				}
			}
		}
	}
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
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Exterminate
	|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Entry
	|| CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack
	|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Boss
	|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Combat)
	{
		return RemainingEnemyCount == 0;
	}
	else if (CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack)
	{
		bool bIsTimeLeft = GetRemainingTime() == 0.0f;
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
