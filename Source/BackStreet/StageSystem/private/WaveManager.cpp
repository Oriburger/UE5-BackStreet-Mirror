#include "../public/WaveManager.h"
#include "../public/ResourceManager.h"
#include "../public/ChapterManagerBase.h"
#include "../public/StageData.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../public/ChapterManagerBase.h"
#include "../../Character/public/EnemyCharacterBase.h"

void AWaveManager::InitWaveManager(class AChapterManagerBase* Target)
{

	// ��������Ʈ ���ε� �۾�
	
}

void AWaveManager::InitReference(class AChapterManagerBase* TargetA, class AResourceManager* TargetB)
{
	ChapterManager = TargetA;
	ResourceManager = TargetB;
	SetOwner(TargetA);
}


bool AWaveManager::CheckAllWaveClear(class AStageData* Target)
{
	if (Target->GetWave() < Target->GetStageTypeInfo().MaxWave)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void AWaveManager::CheckWaveCategorybytype(class AStageData* Target, AEnemyCharacterBase* Enemy)
{
	if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Hades)
	{
		if (Target->GetMonsterList().IsEmpty())
		{
			//Wave üũ
			// 1) ���̺� ���� -> ���̺� �ܰ� ����, ���� ����
			// 2) ���̺� �� -> �������� �Ϸ� ( ���� ���ε� )
			// �Ʒ� �Լ� �����丵( ��������Ʈ�� ���̺� Ŭ���� üũ �� ���̺꿡�� é�� Ŭ���� �� �ϵ��� ���� ��û)
			if (CheckAllWaveClear(Target))
			{
				Target->SetIsClear(true);
				ResourceManager->SpawnRewardBox(Target);

				Cast<AChapterManagerBase>(GetOwner())->CheckChapterClear();
			}
			else
			{
				SpawnHadesWave(Target);

			}
		}
	}
	else if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Defense)
	{
		// ID�� ���� �� ����
		if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Defense)
		{
			ManageDefenseWaveMonsterCount(Target, Enemy->CharacterID, false);
		}
	}
}



void AWaveManager::SetWave(class AStageData* Target)
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

void AWaveManager::SpawnDefenseWave(class AStageData* Target)
{

}

void AWaveManager::SpawnHadesWave(class AStageData* Target)
{
	int32 spawnamount = ResourceManager->SelectSpawnMonsterAmount(Target);

	for (int32 i = 0; i < spawnamount; i++)
	{
		int32 enemyID = ResourceManager->SelectSpawnMonsterID(Target);
		FVector spawnLocation = ResourceManager->GetSpawnLocation(Target);
		ResourceManager->BindMonsterDelegate(Target, ResourceManager->SpawnMonster(Target, enemyID, spawnLocation));
	}
	Target->SetWave(Target->GetWave() + 1);
}

void AWaveManager::SetDefenseWaveTimer(class AStageData* Target, float time)
{
	// ���̺� �ð� ����
	float totalTime = *Target->GetStageTypeInfo().ClearTimeForEachWave.Find(Target->GetWave());

	FStageDataStruct newStageData = Target->GetStageInfo();
	newStageData.WaveTime = totalTime;
	Target->SetStageInfo(newStageData);

	// ���� Ÿ�̸� ����
	//GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AResourceManager::SpawnDefenseWave, 3.0f, true, 0.0f);

	// ���̺� Ÿ�̸� ����
//	GetWorldTimerManager().SetTimer(Target->GetStageInfo().WaveTimerHandle, this, &AResourceManager::CalculateWaveTime, 1.0f, false, 0.0f);

	Target->SetWave(Target->GetWave() + 1);
}

void AWaveManager::ClearDefenseWave(class AStageData* Target)
{

	ResourceManager->CleanStageMonster(Target);

	if (CheckAllWaveClear(Target))
	{
		// �������� Ŭ���� ����
		Target->SetIsClear(true);
		ResourceManager->SpawnRewardBox(Target);

		Cast<AChapterManagerBase>(GetOwner())->CheckChapterClear();
	}
	else
	{
		// ���̺� Ŭ���� ���� ���� ���̺� ����
		SetDefenseWaveTimer(Target, 0.0);

	}

}

void AWaveManager::CalculateWaveTime()
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
		// Ÿ�̸� ��ȯ
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		//		GetWorldTimerManager().ClearTimer(currentStage->GetStageInfo().WaveTimerHandle);

	}

}
//void AWaveManager::SpawnMonsterWithID(class AStageData* Target, int32 EnemyID)
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
//	// ���ε� �۾�
//	FString name = monster->GetController()->GetName();
//	Target->AIOffDelegate.AddDynamic(Cast<AAIControllerBase>(monster->GetController()), &AAIControllerBase::DeactivateAI);
//	Target->AIOnDelegate.AddDynamic(Cast<AAIControllerBase>(monster->GetController()), &AAIControllerBase::ActivateAI);
//	monster->EnemyDeathDelegate.BindUFunction(this, FName("DieMonster"));
//
//	// ���潺 ���̺��� ��� ���� ���� ���
//	if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_Defense)
//	{
//		ManageDefenseWaveMonsterCount(Target, EnemyID, true);
//	}
//}

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
			UE_LOG(LogTemp, Log, TEXT("AResourceManager:: Error"));

		}

	}
	newStageData.ExistEnemyList = enemyList;
	Target->SetStageInfo(newStageData);
}