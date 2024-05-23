// Fill out your copyright notice in the Description page of Project Settings.


#include "StageManagerComponent.h"
#include "../../Character/EnemyCharacter/EnemyCharacterBase.h"
#include "../../Character/CharacterBase.h"
#include "../AISystem/AIControllerBase.h"
#include "./Stage/GateBase.h"
#include "SlateBasics.h"
#include "Runtime/UMG/Public/UMG.h"

// Sets default values for this component's properties
UStageManagerComponent::UStageManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Init blueprint class
	static ConstructorHelpers::FClassFinder<AGateBase> gateInventoryClassFinder(TEXT("/Game/System/StageManager/Blueprint/BP_Gate"));
	checkf(gateInventoryClassFinder.Succeeded(), TEXT("UStageManagerComponent::Gate 클래스 탐색에 실패했습니다."));
	GateClass = gateInventoryClassFinder.Class;

	static ConstructorHelpers::FClassFinder<AEnemyCharacterBase> enemyCharacterClassFinder(TEXT("/Game/Character/EnemyCharacter/Blueprint/BP_EnemyCharacter"));
	checkf(enemyCharacterClassFinder.Succeeded(), TEXT("UStageManagerComponent::EnemyCharacter 클래스 탐색에 실패했습니다."));
	EnemyCharacterClass = enemyCharacterClassFinder.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> loadingWidgetClassFinder(TEXT("/Game/UI/Global/Blueprint/WB_LoadingScreenSimple"));
	checkf(loadingWidgetClassFinder.Succeeded(), TEXT("UStageManagerComponent::loadingWidget 클래스 탐색에 실패했습니다."));
	LoadingWidgetClass = loadingWidgetClassFinder.Class;
}


// Called when the game starts
void UStageManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ..
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
	//UE_LOG(LogTemp, Warning, TEXT("> Level Name : %s"), *CurrentStageInfo.LevelAssetName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("> Coordinate : %d"), CurrentStageInfo.TilePos.X);

	//Load new level
	CreateLevelInstance(NewStageInfo.MainLevelAsset, NewStageInfo.OuterLevelAsset);
	GetWorld()->GetTimerManager().SetTimer(LoadCheckTimerHandle, this, &UStageManagerComponent::CheckLoadStatusAndStartGame, 1.0f, true);
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

	for (AActor*& spawnPoint : spawnPointList)
	{
		if (!IsValid(spawnPoint) || !spawnPoint->Tags.IsValidIndex(1)) continue;
		
		if (spawnPoint->Tags[1] == FName("Enemy"))
		{
			CurrentStageInfo.EnemySpawnLocationList.Add(spawnPoint->GetActorLocation());
		}
		else if (spawnPoint->Tags[1] == FName("PlayerStart"))
		{
			CurrentStageInfo.PlayerStartLocation = spawnPoint->GetActorLocation();
		}
		else if (spawnPoint->Tags[1] == FName("Gate"))
		{
			CurrentStageInfo.PortalLocationList.Add(spawnPoint->GetActorLocation());
		}
	}
}

void UStageManagerComponent::SpawnEnemy()
{
	//Basic condition (stage type check and data count check
	if (CurrentStageInfo.StageType != EStageCategoryInfo::E_Entry
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Combat
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_TimeAttack
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_EliteCombat
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_EliteTimeAttack
		&& CurrentStageInfo.StageType != EStageCategoryInfo::E_Boss) return;

	if (CurrentStageInfo.EnemyCompositionInfo.CompositionList.Num() == 0
		|| CurrentStageInfo.EnemyCompositionInfo.CompositionList.Num() == 0) return;

	//Check data is valid
	if (CurrentStageInfo.EnemyCompositionInfo.CompositionList.Num() > CurrentStageInfo.EnemySpawnLocationList.Num())
	{
		UE_LOG(LogGameMode, Warning, TEXT("------- UStageManagerComponent::SpawnEnemy() -------"));
		UE_LOG(LogGameMode, Warning, TEXT("Spawn point num is less than enemy composition list num."));
		UE_LOG(LogGameMode, Warning, TEXT("There can be collapse among enemy characters."));
	}

	//Init enemy count to zero
	RemainingEnemyCount = 0;

	//Spawn enemy to world and add to SpawnedActorList
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
					newEnemy->SwitchToNextWeapon();
					newEnemy->EnemyDeathDelegate.BindUFunction(this, FName("UpdateEnemyCountAndCheckClear"));
					Cast<AAIControllerBase>(newEnemy->GetController())->ActivateAI();

					SpawnedActorList.Add(newEnemy);
					RemainingEnemyCount += 1;
				}
			}
		}
	}
}

void UStageManagerComponent::SpawnCraftbox()
{
	//Spawn craft box~~
}

void UStageManagerComponent::SpawnPortal(int32 GateCount)
{
	//Temporary code for linear stage system. (GateCount will not be used til BIC)
	AGateBase* newGate = GetWorld()->SpawnActor<AGateBase>(GateClass, CurrentStageInfo.PortalLocationList[0], FRotator::ZeroRotator);
	if (IsValid(newGate))
	{
		SpawnedActorList.Add(newGate);
		newGate->InitGate({ 1, 0 });
		
		//temp
		newGate->ActivateGate();
		newGate->OnEnterRequestReceived.BindUFunction(GetOwner(), FName("MoveStage"));
	}
}

void UStageManagerComponent::CheckLoadStatusAndStartGame()
{
	if (GetWorld()->GetTimerManager().GetTimerElapsed(LoadCheckTimerHandle) > LoadTimeOut) return;

	if (GetLoadIsDone())
	{
		//Post load events
		UpdateSpawnPointProperty();

		//Clear timer and invalidate
		GetWorld()->GetTimerManager().ClearTimer(LoadCheckTimerHandle);
		LoadCheckTimerHandle.Invalidate();

		//Start stage with delay
		FTimerHandle gameStartDelayHandle;
		GetWorld()->GetTimerManager().SetTimer(gameStartDelayHandle, this, &UStageManagerComponent::StartStage, 1.0f, false, 1.5f);
	}
}

void UStageManagerComponent::StartStage()
{
	//Remove loading screen 
	RemoveLoadingScreen();

	//spawn enemy and portal
	SpawnEnemy();

	//Move player to start location
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (IsValid(playerCharacter))
	{
		playerCharacter->SetActorLocation(CurrentStageInfo.PlayerStartLocation);
	}

	//Start timer if stage type if timeattack
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack
		|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteTimeAttack)
	{
		FTimerDelegate stageOverDelegate;
		stageOverDelegate.BindUFunction(this, FName("FinishStage"), false, false);
		GetOwner()->GetWorldTimerManager().SetTimer(TimeAttackTimerHandle, stageOverDelegate, 1.0f, false, CurrentStageInfo.TimeLimitValue);
	}
	else if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Craft
			|| CurrentStageInfo.StageType == EStageCategoryInfo::E_MiniGame
			|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Gatcha)
	{
		FinishStage(false, false);
	}
}

void UStageManagerComponent::FinishStage(bool bIsGameOver, bool bPayReward)
{
	if (CurrentStageInfo.bIsClear) return;
	CurrentStageInfo.bIsClear = true;

	//Clear all remaining actors
	ClearPreviousActors();

	//StageFinished Delegate
	OnStageFinished.Broadcast(CurrentStageInfo);

	//Clear time attack timer handle
	GetOwner()->GetWorldTimerManager().ClearTimer(TimeAttackTimerHandle);
	TimeAttackTimerHandle.Invalidate();

	//Spawn gate 
	if (!bIsGameOver)
	{
		SpawnPortal();
	}
	else if(bIsGameOver)
	{
		//create game over widget
	}
	else if(bPayReward)
	{
		//Pay reward to player
	}
	else
	{
		//create timeatk stage over widget
	}
}

void UStageManagerComponent::UpdateEnemyCountAndCheckClear()
{
	UpdateRemainingEnemyCount();
	if (CheckStageClearStatus())
	{
		FinishStage(false, true);
	}
}

bool UStageManagerComponent::CheckStageClearStatus()
{
	//Basic condition (stage type check)
	if (CurrentStageInfo.StageType == EStageCategoryInfo::E_Combat
	|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Entry
	|| CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack
	|| CurrentStageInfo.StageType == EStageCategoryInfo::E_Boss)
	{
		return RemainingEnemyCount == 0;
	}
	else if (CurrentStageInfo.StageType == EStageCategoryInfo::E_TimeAttack
	|| CurrentStageInfo.StageType == EStageCategoryInfo::E_EliteTimeAttack)
	{
		bool bIsTimeLeft = GetRemainingTime() == 0.0f;
		return RemainingEnemyCount == 0 && bIsTimeLeft;
	}

	return false;
}
