#include "../public/WaveManager.h"
#include "../public/ResourceManager.h"
#include "../public/ChapterManagerBase.h"
#include "../public/StageData.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../public/ChapterManagerBase.h"
#include "../../Character/public/EnemyCharacterBase.h"

void AWaveManager::InitWaveManager(class AChapterManagerBase* Target)
{

	// 델리게이트 바인딩 작업
	// SpawnRewardBox 스폰
	// SpawnMonster
	// CheckChapterClear
	
}

void AWaveManager::InitReference(class AChapterManagerBase* TargetA, class AResourceManager* TargetB)
{
	ChapterManager = TargetA;
	ResourceManager = TargetB;
	SetOwner(TargetA);
}


bool AWaveManager::CheckAllWaveClear(class AStageData* Target)
{
	if (Target->GetCurrentWaveLevel() < Target->GetStageTypeInfo().MaxWave)
		return false;
	else
		return true;
}

void AWaveManager::CheckWaveCategoryByType(class AStageData* Target, AEnemyCharacterBase* Enemy)
{
	if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Hades)
	{
		if (Target->GetMonsterList().IsEmpty())
		{
			Target->SetCurrentWaveLevel(Target->GetCurrentWaveLevel() + 1);

			if (CheckAllWaveClear(Target))
			{
				Target->SetIsClear(true);
				ResourceManager->SpawnRewardBox(Target);

				Cast<AChapterManagerBase>(GetOwner())->CheckChapterClear();
			}
			else
				SpawnHadesWave(Target);
		}
	}
	else if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Defense)
			ManageDefenseWaveMonsterCount(Target, Enemy->CharacterID, false);
		
}


void AWaveManager::SetWave(class AStageData* Target)
{
	UE_LOG(LogTemp, Log, TEXT("AWaveManager::SetWave -> Start"));
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

void AWaveManager::SpawnHadesWave(class AStageData* Target)
{
	int32 spawnamount = ResourceManager->SelectSpawnMonsterAmount(Target);

	for (int32 i = 0; i < spawnamount; i++)
	{
		int32 enemyID = ResourceManager->SelectSpawnMonsterID(Target);
		ResourceManager->SpawnMonster(Target, enemyID);
	}
}

void AWaveManager::SpawnDefenseWave(class AStageData* Target)
{
	Target->CloseAllGate();
	SetDefenseWaveClearTimer(Target);
	SetDefenseWaveSpawnTimer(Target);

}

void AWaveManager::SetDefenseWaveClearTimer(class AStageData* Target)
{
	// Get Wave Clear Time
	float totalTime = *Target->GetStageTypeInfo().ClearTimeForEachWave.Find(Target->GetCurrentWaveLevel() + 1);
	GetWorldTimerManager().SetTimer(Target->GetDefenseWaveClearTimeTimerHandle(), this, &AWaveManager::ClearDefenseWave, 60.0f, false);
}

void AWaveManager::SetDefenseWaveSpawnTimer(class AStageData* Target)
{
	// Get Spawn Interval Time
	float intervalTime;
	// Temporary) Need To Fix
	intervalTime = 3.0f;
	GetWorldTimerManager().SetTimer(Target->GetDefenseWaveSpawnTimerHandle(), this, &AWaveManager::SpawnDefenseWaveMonster, intervalTime , true, 0.0f);
}

void AWaveManager::ClearDefenseWaveTimer(class AStageData* Target)
{
	GetWorldTimerManager().ClearTimer(Target->GetDefenseWaveClearTimeTimerHandle());
	GetWorldTimerManager().ClearTimer(Target->GetDefenseWaveSpawnTimerHandle());
}

void AWaveManager::SpawnDefenseWaveMonster()
{
	UE_LOG(LogTemp, Log, TEXT("AWaveManager::SpawnDefenseWaveMonster"));
	AStageData* stage = Cast<AChapterManagerBase>(GetOwner())->GetCurrentStage();

	// Select Spawn EnemyID
	int32 enemyID = 1001;

	ResourceManager->SpawnMonster(stage, enemyID);
	ManageDefenseWaveMonsterCount(stage, enemyID, true);
}

void AWaveManager::ClearDefenseWave()
{
	AStageData* stage = Cast<AChapterManagerBase>(GetOwner())->GetCurrentStage();

	ClearDefenseWaveTimer(stage);
	ResourceManager->CleanStageMonster(stage);
	stage->SetCurrentWaveLevel(stage->GetCurrentWaveLevel() + 1);

	UE_LOG(LogTemp, Log, TEXT("AWaveManager::ClearDefenseWave"));

	if (CheckAllWaveClear(stage))
	{
		// 스테이지 클리어 로직
		stage->SetIsClear(true);
		ResourceManager->SpawnRewardBox(stage);

		Cast<AChapterManagerBase>(GetOwner())->CheckChapterClear();
		stage->OpenAllGate();

	}
	else
		SpawnDefenseWave(stage);

}

void AWaveManager::ManageDefenseWaveMonsterCount(class AStageData* Target, int32 EnemyID, bool IsSpawn)
{
	TMap<int32, int32> enemyList = Target->GetStageInfo().ExistEnemyList;
	FStageDataStruct newStageData = Target->GetStageInfo();

	if (IsSpawn)
	{
		if (enemyList.FindKey(EnemyID) != nullptr)
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
			UE_LOG(LogTemp, Log, TEXT("AWaveManager:: ManageDefenseWaveMonsterCount Error"));

		}

	}
	newStageData.ExistEnemyList = enemyList;
	Target->SetStageInfo(newStageData);
}