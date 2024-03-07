// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/ResourceManager.h"
#include "../public/StageData.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../public/ChapterManagerBase.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../../AISystem/public/AIControllerBase.h"
#include "../../Item/public/RewardBoxBase.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "../public/WaveManager.h"
#include "../../Item/public/ItemBoxBase.h"
#include "../../Item/public/ItemBase.h"
#include "Engine/DamageEvents.h"
#include "../../CraftingSystem/public/CraftBoxBase.h"
#include "Engine/LevelStreaming.h"

AResourceManager::AResourceManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AResourceManager::BeginPlay()
{
	Super::BeginPlay();

	ABackStreetGameModeBase* gameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	ensure(gameModeRef != nullptr);

	gameModeRef->ClearResourceDelegate.AddDynamic(this, &AResourceManager::CleanAllResource);	
}

void AResourceManager::InitReference(class AWaveManager* Target)
{
	WaveManager = Target;
}

void AResourceManager::SpawnStageActor(class AStageData* Target)
{
	UE_LOG(LogTemp, Log, TEXT("AResourceManager::SpawnStageActor -> Start Spawn"));
	checkf(Target != nullptr,TEXT("Target Spawn Stage Is InValid"));

	if (Target->GetStageCategoryType() == EStageCategoryInfo::E_Normal)
	{
			WaveManager->SetWave(Target);
			SpawnItem(Target);
			SpawnCraftingBox(Target);

	}
	else if (Target->GetStageCategoryType() == EStageCategoryInfo::E_Boss)
	{
			SpawnBossMonster(Target);
			SpawnItem(Target);
			SpawnCraftingBox(Target);
	}
	UE_LOG(LogTemp, Log, TEXT("AResourceManager::SpawnStageActor -> Finish Spawn"));

}

void AResourceManager::SpawnBossMonster(class AStageData* Target)
{
	if (!IsValid(Target))
		return;
	FActorSpawnParameters actorSpawnParameters;
	TArray<FVector> monsterSpawnPoint = Target->GetMonsterSpawnPoints();
	ensure(!monsterSpawnPoint.IsEmpty());

	uint16 idx = FMath::RandRange(0, monsterSpawnPoint.Num() - 1);
	FVector spawnLocation = monsterSpawnPoint[idx];
	spawnLocation = spawnLocation + FVector(0, 0, 200);
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyCharacterBase* monster = GetWorld()->SpawnActor<AEnemyCharacterBase>(BossAssets[0], spawnLocation, FRotator::ZeroRotator, actorSpawnParameters);
	ensure(monster != nullptr);

	Target->AddMonsterList(monster);
	// 수정 필요 하드코딩..
	monster->CharacterID = 1200;
	monster->InitEnemyCharacter(1200);
	monster->InitAsset(1200);
	monster->SwitchToNextWeapon();
	BindMonsterDelegate(Target, monster);
}

void AResourceManager::SpawnItem(class AStageData* Target)
{
	ensure(Target != nullptr);

	TArray<FVector> itemSpawnPoints = Target->GetItemSpawnPoints();
	ensure(!itemSpawnPoints.IsEmpty());

	for (int i = 0; i < 100; i++)
	{
		int32 selectidxA = FMath::RandRange(0, itemSpawnPoints.Num() - 1);
		int32 selectidxB = FMath::RandRange(0, itemSpawnPoints.Num() - 1);
		FVector temp;

		temp = itemSpawnPoints[selectidxA];
		itemSpawnPoints[selectidxA] = itemSpawnPoints[selectidxB];
		itemSpawnPoints[selectidxB] = temp;

	}

	int8 spawnMax = FMath::RandRange(MIN_ITEM_SPAWN, MAX_ITEM_SPAWN);
	for (int8 i = 0; i < spawnMax; i++)
	{
		int8 type = FMath::RandRange(0, ItemBoxAssets.Num() - 1);
		FVector spawnLocation = itemSpawnPoints[i];
		spawnLocation = spawnLocation + FVector(0, 0, 200);
		AItemBoxBase* item = GetWorld()->SpawnActor<AItemBoxBase>(ItemBoxAssets[type], spawnLocation, FRotator::ZeroRotator);
		ensure(item != nullptr);
		item->InitItemBox(false);
		Target->AddItemBoxList(item);
		
	}
	
}

void AResourceManager::SpawnRewardBox(class AStageData* Target)
{
	ensure(Target != nullptr);

	TArray<FVector> rewardBoxSpawnPoint = Target->GetRewardBoxSpawnPoint();
	ensure(!rewardBoxSpawnPoint.IsEmpty());
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ARewardBoxBase* rewardBox;
	if (!RewardBoxAssets.IsValidIndex(0) || !rewardBoxSpawnPoint.IsValidIndex(0))
		return;
	rewardBox = GetWorld()->SpawnActor<ARewardBoxBase>(RewardBoxAssets[0], rewardBoxSpawnPoint[0], FRotator(0, 90, 0), actorSpawnParameters);
	ensure(rewardBox != nullptr);
	Target->SetRewardBox(rewardBox);

}

void AResourceManager::SpawnCraftingBox(class AStageData* Target)
{
	ensure(Target != nullptr);

	TArray<FVector> craftingBoxSpawnPoint = Target->GetCraftingBoxSpawnPoint();
	TArray<FRotator> craftingBoxSpawnRotatorPoint = Target->GetCraftingBoxSpawnRotatorPoint();
	ensure(!craftingBoxSpawnRotatorPoint.IsEmpty());


	for (int i = 0; i < 100; i++)
	{
		int32 selectidxA = FMath::RandRange(0, craftingBoxSpawnPoint.Num() - 1);
		int32 selectidxB = FMath::RandRange(0, craftingBoxSpawnPoint.Num() - 1);
		FVector tempVector;
		FRotator tempRotator;

		tempVector = craftingBoxSpawnPoint[selectidxA];
		craftingBoxSpawnPoint[selectidxA] = craftingBoxSpawnPoint[selectidxB];
		craftingBoxSpawnPoint[selectidxB] = tempVector;

		tempRotator = craftingBoxSpawnRotatorPoint[selectidxA];
		craftingBoxSpawnRotatorPoint[selectidxA] = craftingBoxSpawnRotatorPoint[selectidxB];
		craftingBoxSpawnRotatorPoint[selectidxB] = tempRotator;

	}

	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ACraftBoxBase* craftBox;
	if (!CraftingBoxAssets.IsValidIndex(0) || !craftingBoxSpawnPoint.IsValidIndex(0))
		return;
	craftBox = GetWorld()->SpawnActor<ACraftBoxBase>(CraftingBoxAssets[0], craftingBoxSpawnPoint[0], craftingBoxSpawnRotatorPoint[0], actorSpawnParameters);
	ensure(craftBox != nullptr);
	Target->SetCraftingBox(craftBox);

}

void AResourceManager::RemoveMonsterFromList(AEnemyCharacterBase* Target)
{
	if (!IsValid(GetOwner())||!IsValid(Target)) return;

	AStageData* currentStage = Cast<AChapterManagerBase>(GetOwner())->GetCurrentStage();
	if (!IsValid(currentStage)) return;

	currentStage->RemoveMonsterList(Target);

	WaveManager->CheckWaveCategoryByType(currentStage, Target);
}

AEnemyCharacterBase* AResourceManager::SpawnMonster(class AStageData* Target, int32 EnemyID)
{
	FVector spawnLocation = GetSpawnLocation(Target);
	AEnemyCharacterBase* monster = CreateMonster(Target, EnemyID, spawnLocation);
	BindMonsterDelegate(Target, monster);

	return monster;
}


AEnemyCharacterBase* AResourceManager::CreateMonster(class AStageData* Target, int32 EnemyID, FVector Location)
{
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyCharacterBase* monster = GetWorld()->SpawnActor<AEnemyCharacterBase>(EnemyAssets[0], Location, FRotator::ZeroRotator, actorSpawnParameters);
	Target->AddMonsterList(monster);

	monster->CharacterID = EnemyID;
	monster->InitEnemyCharacter(EnemyID);
	monster->InitAsset(EnemyID);
	monster->SwitchToNextWeapon();
	WaveManager->ManageWaveMonsterCount(Target, monster->CharacterID, true);


	return monster;

}

void AResourceManager::BindMonsterDelegate(class AStageData* Target, class AEnemyCharacterBase* Monster)
{
	ensure(Target != nullptr);

	FString name = Monster->GetController()->GetName();
	Target->AIOffDelegate.AddDynamic(Cast<AAIControllerBase>(Monster->GetController()), &AAIControllerBase::DeactivateAI);
	Target->AIOnDelegate.AddDynamic(Cast<AAIControllerBase>(Monster->GetController()), &AAIControllerBase::ActivateAI);
	Monster->EnemyDeathDelegate.BindUFunction(this, FName("RemoveMonsterFromList"));
	
}

FVector AResourceManager::GetSpawnLocation(class AStageData* Target)
{
	TArray<FVector> monsterSpawnPoint = Target->GetMonsterSpawnPoints();
	int32 idx = GetSpawnPointIdx(Target);
	FVector spawnLocation = monsterSpawnPoint[idx];
	spawnLocation = spawnLocation + FVector(0, 0, 30);

	return spawnLocation;
}

int32 AResourceManager::GetSpawnPointIdx(class AStageData* Target)
{
	int32 maxIdx = Target->GetMonsterSpawnPoints().Num();
	int32 currentIdx = Target->GetStageInfo().MonsterSpawnPointOrderIdx;
	FStageDataStruct newStageData = Target->GetStageInfo();

	currentIdx++;

	if (currentIdx < maxIdx)
	{
		newStageData.MonsterSpawnPointOrderIdx = currentIdx;
	}
	else
	{
		newStageData.MonsterSpawnPointOrderIdx = 0;
		currentIdx = 0;
	}

	Target->SetStageInfo(newStageData);
	return currentIdx;

}
//
//int32 AResourceManager::SelectSpawnMonsterID(class AStageData* Target)
//{
//	ensure(Target != nullptr);
//	FStageInfoStruct stageType = Target->GetStageTypeInfo();
//	TArray<int32> enemyIDList = stageType.NormalEnemyIDList;
//	int32 enemyIDIdx = FMath::RandRange(0, enemyIDList.Num() - 1);
//
//	return enemyIDList[enemyIDIdx];
//}

//int32 AResourceManager::SelectSpawnMonsterAmount(class AStageData* Target)
//{
//	ensure(Target != nullptr);
//	FStageInfoStruct stageType = Target->GetStageTypeInfo();
//	int32 spawnNum = FMath::RandRange(stageType.MinSpawnEnemy, stageType.MaxSpawnEnemy);
//	
//	return spawnNum;
//}

void AResourceManager::CleanAllResource()
{
	TArray<AStageData*> stages = Cast<AChapterManagerBase>(GetOwner())->GetStages();
	ensure(!stages.IsEmpty());

	for (int32 idx = stages.Num()-1; idx>=0; idx--)
	{
		AStageData* stage = stages[idx];
		if (IsValid(stage))
		{
			CleanStage(stage);
		}
	}
	//ABackStreetGameModeBase* gamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	//if (IsValid(gamemodeRef))
	//{
	//	gamemodeRef->GetGlobalDebuffManagerRef()->ClearDebuffManagerTimer();
	//}

	if(IsValid(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
		Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->ClearAllTimerHandle();
}

void AResourceManager::CleanStage(class AStageData* Target)
{
	ensure(Target != nullptr);

	CleanStageMonster(Target);
	CleanStageItem(Target);
	if (IsValid(Target->GetRewardBox()))
	{
		Target->GetRewardBox()->Destroy();
	}
	if (IsValid(Target->GetCraftingBox()))
	{
		Target->GetCraftingBox()->Destroy();
	}

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void AResourceManager::CleanStageMonster(class AStageData* Target)
{
	ensure(Target != nullptr);
	for (int32 idx = Target->GetMonsterList().Num() - 1; idx >= 0; idx--)
	{
		TWeakObjectPtr<class AEnemyCharacterBase> target = Target->GetMonsterIdx(idx);
		if (target.IsValid())
		{
			GetWorld()->GetTimerManager().ClearAllTimersForObject(target.Get());
			target->TakeDamage(100000.0f, FDamageEvent(), nullptr, this);
		}
	}
}

void AResourceManager::CleanStageItem(class AStageData* Target)
{
	ensure(Target != nullptr);
	for (int32 idx = Target->GetItemBoxList().Num() - 1; idx >= 0; idx--)
	{
		TWeakObjectPtr<class AItemBoxBase> target = Target->GetItemBoxIdx(idx);
		if (target.IsValid() && !target->IsActorBeingDestroyed())
		{
			GetWorld()->GetTimerManager().ClearAllTimersForObject(target.Get());
			target->Destroy();
		}
	}

	for (int32 idx = Target->GetItemList().Num() - 1; idx >= 0; idx--)
	{
		TWeakObjectPtr<class AItemBase> target = Target->GetItemIdx(idx);
		if (target.IsValid() && !target->IsActorBeingDestroyed())
		{
			GetWorld()->GetTimerManager().ClearAllTimersForObject(target.Get());
			target->Destroy();
		}
	}
}


