// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/ResourceManager.h"
#include "../public/StageData.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Global/public/DebuffManager.h"
#include "../public/ChapterManagerBase.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../../AISystem/public/AIControllerBase.h"
#include "../../Item/public/RewardBoxBase.h"
#include "../../Item/public/ItemBoxBase.h"
#include "../../Item/public/ItemBase.h"
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
	if (!IsValid(gameModeRef))
		return;
	gameModeRef->ClearResourceDelegate.AddDynamic(this, &AResourceManager::CleanAllResource);
	gameModeRef->ChapterClearResourceDelegate.AddDynamic(this, &AResourceManager::CleanAllResource);
	
}

void AResourceManager::SpawnStageActor(class AStageData* Target)
{
	UE_LOG(LogTemp, Log, TEXT("AResourceManager::SpawnStageActor -> Start Spawn"));
	if (!IsValid(Target))
		return;
	if (Target->GetStageCategoryType() == EStageCategoryInfo::E_Normal)
	{
			SetWave(Target);
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
	if (monsterSpawnPoint.IsEmpty())
		return;
	uint16 idx = FMath::RandRange(0, monsterSpawnPoint.Num() - 1);
	FVector spawnLocation = monsterSpawnPoint[idx];
	spawnLocation = spawnLocation + FVector(0, 0, 200);
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyCharacterBase* monster = GetWorld()->SpawnActor<AEnemyCharacterBase>(BossAssets[0], spawnLocation, FRotator::ZeroRotator, actorSpawnParameters);
	if (!IsValid(monster))
		return;
	Target->AddMonsterList(monster);
	// 수정 필요 하드코딩..
	monster->CharacterID = 1200;
	monster->InitEnemyStat();
	monster->InitAsset(1200);
	BindMonsterDelegate(Target, monster);
}

void AResourceManager::SpawnItem(class AStageData* Target)
{
	if (!IsValid(Target))
		return;
	TArray<FVector> itemSpawnPoints = Target->GetItemSpawnPoints();
	if (itemSpawnPoints.IsEmpty())
		return;
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
		if (!IsValid(item))
			return;
		item->InitItemBox(false);
		Target->AddItemBoxList(item);
		
	}
	
}

void AResourceManager::SpawnRewardBox(class AStageData* Target)
{
	if (!IsValid(Target))
		return;
	TArray<FVector> rewardBoxSpawnPoint = Target->GetRewardBoxSpawnPoint();
	if (rewardBoxSpawnPoint.IsEmpty())
		return;
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ARewardBoxBase* rewardBox;
	if (!RewardBoxAssets.IsValidIndex(0) || !rewardBoxSpawnPoint.IsValidIndex(0))
		return;
	rewardBox = GetWorld()->SpawnActor<ARewardBoxBase>(RewardBoxAssets[0], rewardBoxSpawnPoint[0], FRotator(0, 90, 0), actorSpawnParameters);
	if (!IsValid(rewardBox))
		return;
		Target->SetRewardBox(rewardBox);

}

void AResourceManager::SpawnCraftingBox(class AStageData* Target)
{
	if (!IsValid(Target))
		return;
	TArray<FVector> craftingBoxSpawnPoint = Target->GetCraftingBoxSpawnPoint();
	TArray<FRotator> craftingBoxSpawnRotatorPoint = Target->GetCraftingBoxSpawnRotatorPoint();
	if (craftingBoxSpawnPoint.IsEmpty())
		return;

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
	if (!IsValid(craftBox))
		return;
	Target->SetCraftingBox(craftBox);

}

void AResourceManager::DieMonster(AEnemyCharacterBase* Target)
{
	if (!IsValid(GetOwner())||!IsValid(Target)) return;

	AStageData* currentStage = Cast<AChapterManagerBase>(GetOwner())->GetCurrentStage();
	if (!IsValid(currentStage)) return;

	currentStage->RemoveMonsterList(Target);

	// 웨이브 타입이 하데스인 경우에만 실행
	if (currentStage->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Hades)
	{
		if (currentStage->GetMonsterList().IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("AResourceManager::DieMonster: Stage Clear BroadCast StgaeClearDelegate"));
			//Wave 체크
			// 1) 웨이브 진행 -> 웨이브 단계 증가, 몬스터 스폰
			// 2) 웨이브 끝 -> 스테이지 완료 ( 스폰 바인딩 )
			if (CheckAllWaveClear(currentStage))
			{
				currentStage->SetIsClear(true);
				SpawnRewardBox(currentStage);

				Cast<AChapterManagerBase>(GetOwner())->CheckChapterClear();
			}
			else
			{
				SpawnHadesWave(currentStage);

			}
		}
	}
	else if(currentStage->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Defense)
	{
		// ID별 몬스터 수 관리
		if (currentStage->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Defense)
		{
			ManageDefenseWaveMonsterCount(currentStage, Target->CharacterID, false);
		}
	}
}

bool AResourceManager::CheckAllWaveClear(class AStageData* Target)
{
	if (Target->GetWave()<Target->GetStageTypeInfo().MaxWave)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void AResourceManager::SetDefenseWaveTimer(class AStageData* Target, float time)
{
	// 웨이브 시간 세팅
	float totalTime = *Target->GetStageTypeInfo().ClearTimeForEachWave.Find(Target->GetWave());

	FStageDataStruct newStageData = Target->GetStageInfo();
	newStageData.WaveTime = totalTime;
	Target->SetStageInfo(newStageData);

	// 스폰 타이머 세팅
	//GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AResourceManager::SpawnDefenseWave, 3.0f, true, 0.0f);

	// 웨이브 타이머 세팅
//	GetWorldTimerManager().SetTimer(Target->GetStageInfo().WaveTimerHandle, this, &AResourceManager::CalculateWaveTime, 1.0f, false, 0.0f);

	Target->SetWave(Target->GetWave() + 1);
}

void AResourceManager::ClearDefenseWave(class AStageData* Target)
{
	// 몬스터 삭제
	if (!IsValid(Target)) return;
	for (int32 idx = Target->GetMonsterList().Num() - 1; idx >= 0; idx--)
	{
		TWeakObjectPtr<class AEnemyCharacterBase> target = Target->GetMonsterIdx(idx);
		if (target.IsValid())
		{
			GetWorld()->GetTimerManager().ClearAllTimersForObject(target.Get());
			target->TakeDamage(100000.0f, FDamageEvent(), nullptr, this);
		}
	}

	if (CheckAllWaveClear(Target))
	{
		// 스테이지 클리어 로직
		Target->SetIsClear(true);
		SpawnRewardBox(Target);

		Cast<AChapterManagerBase>(GetOwner())->CheckChapterClear();
	}
	else
	{
		// 웨이브 클리어 로직 다음 웨이브 시작
		SetDefenseWaveTimer(Target, 0.0);
		
	}

}

void AResourceManager::CalculateWaveTime()
{
	AStageData* currentStage = Cast<AChapterManagerBase>(GetOwner())->GetCurrentStage();
	if (!IsValid(currentStage)) return;

	FStageDataStruct newStageData = currentStage->GetStageInfo();
	int32 time = currentStage->GetStageInfo().WaveTime;
	time--;

	newStageData.WaveTime = time;
	currentStage->SetStageInfo(newStageData);

	if (time < 0)
	{
		ClearDefenseWave(currentStage);
		// 타이머 반환
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
//		GetWorldTimerManager().ClearTimer(currentStage->GetStageInfo().WaveTimerHandle);

	}

}

void AResourceManager::SetWave(class AStageData* Target)
{
	switch (Target->GetStageTypeInfo().WaveType)
	{
		case EWaveCategoryInfo::E_Hades:
			SpawnHadesWave(Target);
			break;
		case EWaveCategoryInfo::E_Defense:
			SpawnDefenseWave(Target);
			break;
	default:
		break;
	}
	
}

void AResourceManager::SpawnDefenseWave(class AStageData* Target)
{

}

void AResourceManager::SpawnHadesWave(class AStageData* Target)
{
	int32 spawnamount = SelectSpawnMonsterAmount(Target);

	for (int32 i = 0; i < spawnamount; i++)
	{
		int32 enemyID = SelectSpawnMonsterID(Target);
		FVector spawnLocation = GetSpawnLocation(Target);
		BindMonsterDelegate(Target,SpawnMonster(Target,enemyID, spawnLocation));
	}
	Target->SetWave(Target->GetWave() + 1);
}

//void AResourceManager::SpawnMonsterWithID(class AStageData* Target, int32 EnemyID)
//{
//	TArray<FVector> monsterSpawnPoint = Target->GetMonsterSpawnPoints();
//	int32 idx = GetSpawnPointIdx(Target);
//
//	FActorSpawnParameters actorSpawnParameters;
//	FVector spawnLocation = monsterSpawnPoint[idx];
//	spawnLocation = spawnLocation + FVector(0, 0, 200);
//	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
//
//	AEnemyCharacterBase* monster = GetWorld()->SpawnActor<AEnemyCharacterBase>(EnemyAssets[0], spawnLocation, FRotator::ZeroRotator, actorSpawnParameters);
//	if (!IsValid(monster))
//		return;
//	Target->AddMonsterList(monster);
//	UE_LOG(LogTemp, Log, TEXT("AResourceManager::SpawnMonster SetEnemyID %d"), EnemyID);
//	monster->CharacterID = EnemyID;
//	monster->InitEnemyStat();
//
//
//	// 바인딩 작업
//	FString name = monster->GetController()->GetName();
//	Target->AIOffDelegate.AddDynamic(Cast<AAIControllerBase>(monster->GetController()), &AAIControllerBase::DeactivateAI);
//	Target->AIOnDelegate.AddDynamic(Cast<AAIControllerBase>(monster->GetController()), &AAIControllerBase::ActivateAI);
//	monster->EnemyDeathDelegate.BindUFunction(this, FName("DieMonster"));
//
//	// 디펜스 웨이브의 경우 몬스터 숫자 기록
//	if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Defense)
//	{
//		ManageDefenseWaveMonsterCount(Target, EnemyID, true);
//	}
//}

void AResourceManager::ManageDefenseWaveMonsterCount(class AStageData* Target, int32 EnemyID, bool IsSpawn)
{
	TMap<int32, int32> enemyList = Target->GetStageInfo().ExistEnemyList;
	FStageDataStruct newStageData = Target->GetStageInfo();

	if (IsSpawn)
	{
		if (enemyList.FindKey(EnemyID)!=nullptr)
		{
			int32 num = *enemyList.FindKey(EnemyID);
			num++;
			enemyList.Add(EnemyID, num);
		}
		else
		{
			enemyList.Add(EnemyID, 1);
		}

	}
	else 
	{
		if (enemyList.FindKey(EnemyID) != nullptr)
		{
			int32 num = *enemyList.FindKey(EnemyID);
			num--;
			enemyList.Add(EnemyID, num);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("AResourceManager:: Error"));

		}

	}
	newStageData.ExistEnemyList = enemyList;
	Target->SetStageInfo(newStageData);
}

AEnemyCharacterBase* AResourceManager::SpawnMonster(class AStageData* Target, int32 EnemyID, FVector Location)
{
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyCharacterBase* monster = GetWorld()->SpawnActor<AEnemyCharacterBase>(EnemyAssets[0], Location, FRotator::ZeroRotator, actorSpawnParameters);
	Target->AddMonsterList(monster);
	UE_LOG(LogTemp, Log, TEXT("AResourceManager::SpawnMonster SetEnemyID %d"), EnemyID);
	monster->CharacterID = EnemyID;
	monster->InitEnemyStat();
	monster->InitAsset(EnemyID);

	return monster;

}

void AResourceManager::BindMonsterDelegate(class AStageData* Target, class AEnemyCharacterBase* Monster)
{
	if (!IsValid(Target))
		return;

	FString name = Monster->GetController()->GetName();
	Target->AIOffDelegate.AddDynamic(Cast<AAIControllerBase>(Monster->GetController()), &AAIControllerBase::DeactivateAI);
	Target->AIOnDelegate.AddDynamic(Cast<AAIControllerBase>(Monster->GetController()), &AAIControllerBase::ActivateAI);
	Monster->EnemyDeathDelegate.BindUFunction(this, FName("DieMonster"));
	
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

int32 AResourceManager::SelectSpawnMonsterID(class AStageData* Target)
{
	if (!IsValid(Target))
		return -1;
	FStageInfoStruct stageType = Target->GetStageTypeInfo();
	TArray<int32> enemyIDList = stageType.NormalEnemyIDList;
	int32 enemyIDIdx = FMath::RandRange(0, enemyIDList.Num() - 1);

	return enemyIDList[enemyIDIdx];
}

int32 AResourceManager::SelectSpawnMonsterAmount(class AStageData* Target)
{
	if (!IsValid(Target))
		return -1;
	FStageInfoStruct stageType = Target->GetStageTypeInfo();
	int32 spawnNum = FMath::RandRange(stageType.MinSpawnEnemy, stageType.MaxSpawnEnemy);
	
	return spawnNum;
}

void AResourceManager::CleanAllResource()
{
	UE_LOG(LogTemp, Log, TEXT("AStageManagerBase::CleanAllResource"));

	TArray<AStageData*> stages = Cast<AChapterManagerBase>(GetOwner())->GetStages();
	if (stages.IsEmpty()) return;
	for (int32 idx = stages.Num()-1; idx>=0; idx--)
	{
		AStageData* stage = stages[idx];
		if (IsValid(stage))
		{
			CleanStage(stage);
		}
	}
	ABackStreetGameModeBase* gamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (IsValid(gamemodeRef))
	{
		gamemodeRef->GetGlobalDebuffManagerRef()->ClearAllDebuffTimer();
	}

	if(IsValid(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
		Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->ClearAllTimerHandle();
}

void AResourceManager::CleanStage(class AStageData* Target)
{
	if (!IsValid(Target)) return;
	UE_LOG(LogTemp, Log, TEXT("AStageManagerBase::CleanStage"));

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
	if (!IsValid(Target)) return;
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
	if (!IsValid(Target)) return;
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


