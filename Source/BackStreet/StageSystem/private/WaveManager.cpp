#include "../public/WaveManager.h"
#include "../public/ResourceManager.h"
#include "../public/ChapterManagerBase.h"
#include "../public/StageData.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../public/ChapterManagerBase.h"
#include "../../Character/public/EnemyCharacterBase.h"

AWaveManager::AWaveManager()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTable(TEXT("/Game/System/StageManager/Data/D_WaveEnemyDataTable.D_WaveEnemyDataTable"));
	if (DataTable.Succeeded())
		WaveEnemyDataTable = DataTable.Object;
	else
		UE_LOG(LogTemp, Warning, TEXT("AWaveManager::InitWaveManager) DataTable is not found!"));
}

void AWaveManager::InitWaveManager(class AChapterManagerBase* Target)
{
	/*WaveEnemyDataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/System/StageManager/Data/D_WaveEnemyDataTable.D_WaveEnemyDataTable'"));
	if (WaveEnemyDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWaveManager::InitWaveManager) DataTable is not found!"));
	}*/

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
	if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_NomalWave)
	{
		ManageWaveMonsterCount(Target, Enemy->CharacterID, false);
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
	else if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_TimeLimitWave)
			ManageWaveMonsterCount(Target, Enemy->CharacterID, false);
		
}


void AWaveManager::SetWave(class AStageData* Target)
{
	UE_LOG(LogTemp, Log, TEXT("AWaveManager::SetWave -> Start"));
	switch (Target->GetStageTypeInfo().WaveType)
	{
	case EWaveCategoryInfo::E_NomalWave:
		SpawnHadesWave(Target);
		break;
	case EWaveCategoryInfo::E_TimeLimitWave:
		SpawnDefenseWave(Target);
		break;
	default:
		break;
	}

}

void AWaveManager::SpawnHadesWave(class AStageData* Target)
{
	int32 spawnamount = GetTotalNumberOfEnemySpawn(Target);

	for (int32 i = 0; i < spawnamount; i++)
	{
		int32 enemyID = SelectEnemyID();
		if (enemyID == -1) return;
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
	float intervalTime = Target->GetStageTypeInfo().SpawnInterval;
	GetWorldTimerManager().SetTimer(Target->GetDefenseWaveSpawnTimerHandle(), this, &AWaveManager::SpawnDefenseWaveMonster, intervalTime , true, 0.0f);
}

void AWaveManager::ClearDefenseWaveTimer(class AStageData* Target)
{
	GetWorldTimerManager().ClearTimer(Target->GetDefenseWaveClearTimeTimerHandle());
	GetWorldTimerManager().ClearTimer(Target->GetDefenseWaveSpawnTimerHandle());
}

void AWaveManager::SpawnDefenseWaveMonster()
{
	AStageData* stage = Cast<AChapterManagerBase>(GetOwner())->GetCurrentStage();

	// Select Spawn EnemyID
	int32 enemyID = SelectEnemyID();
	UE_LOG(LogTemp, Log, TEXT("AWaveManager::SpawnDefenseWaveMonster ID: %d"),enemyID);
	if (enemyID == -1) return;
	ResourceManager->SpawnMonster(stage, enemyID);
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

void AWaveManager::ManageWaveMonsterCount(class AStageData* Target, int32 EnemyID, bool IsSpawn)
{
	TMap<int32, int32> enemyList = Target->GetStageInfo().ExistEnemyList;
	FStageDataStruct newStageData = Target->GetStageInfo();

	if (IsSpawn)
	{
		if (enemyList.Contains(EnemyID))
		{
			int32 num = *enemyList.Find(EnemyID);
			num++;
			enemyList[EnemyID] = num;
			UE_LOG(LogTemp, Log, TEXT("AWaveManager:: ManageWaveMonsterCount Spawn ID: %d enemyList[%d] = %d"), EnemyID, EnemyID,enemyList[EnemyID]);
		}
		else
		{
			enemyList.Add(EnemyID, 1);
			UE_LOG(LogTemp, Log, TEXT("AWaveManager:: ManageWaveMonsterCount Spawn ID: %d enemyList[%d] = %d"), EnemyID, EnemyID, enemyList[EnemyID]);

		}

	}
	else
	{
		if (enemyList.Contains(EnemyID))
		{
			int32 num = *enemyList.Find(EnemyID);
			num--;
			enemyList[EnemyID] = num;
			UE_LOG(LogTemp, Log, TEXT("AWaveManager:: ManageWaveMonsterCount Die ID: %d enemyList[%d] = %d"), EnemyID, EnemyID, enemyList[EnemyID]);

		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("AWaveManager:: ManageWaveMonsterCount Error"));

		}

	}
	newStageData.ExistEnemyList = enemyList;
	Target->SetStageInfo(newStageData);
}

int32 AWaveManager::SelectEnemyID()
{
	AStageData* stage = Cast<AChapterManagerBase>(GetOwner())->GetCurrentStage();
	TArray<TPair<int32,int32>> existEnemy = stage->GetStageInfo().ExistEnemyList.Array();
	UE_LOG(LogTemp, Log, TEXT("AWaveManager:: SelectEnemyID WaveLevel %d"), stage->GetCurrentWaveLevel());

	int32 dataTableID = stage->GetStageTypeInfo().WaveComposition[stage->GetCurrentWaveLevel()];
	UE_LOG(LogTemp, Log, TEXT("AWaveManager:: dataTableID %d"), dataTableID);
	FString rowName = FString::FromInt(dataTableID);
	FWaveEnemyStruct* waveComposition = WaveEnemyDataTable->FindRow<FWaveEnemyStruct>(FName(rowName), rowName);
	TArray<TPair<int32, int32>> waveCompositionEnemy = waveComposition->EnemyList.Array();
	for (TPair<int32, int32> pair : waveCompositionEnemy)
	{
		bool IsFindpair = false;
		UE_LOG(LogTemp, Log, TEXT("AWaveManager:: waveCompositionEnemy pair ID: %d num : %d"), pair.Key,pair.Value);

		for (TPair<int32, int32> exist : existEnemy)
		{
			if (pair.Key == exist.Key)
			{
				IsFindpair = true;
				if (pair.Value > exist.Value) return pair.Key;
			}
			
		}
		if (!IsFindpair) return pair.Key;
	}

	return -1;

}

int32 AWaveManager::GetTotalNumberOfEnemySpawn(class AStageData* Target)
{
	int32 dataTableID = Target->GetStageTypeInfo().WaveComposition[Target->GetCurrentWaveLevel()];
	FString rowName = FString::FromInt(dataTableID);
	FWaveEnemyStruct* waveComposition = WaveEnemyDataTable->FindRow<FWaveEnemyStruct>(FName(rowName), rowName);
	TArray<TPair<int32, int32>> waveCompositionEnemy = waveComposition->EnemyList.Array();
	int32 totalNumber = 0;
	for (TPair<int32, int32> pair : waveCompositionEnemy)
		totalNumber += pair.Value;
	
	return totalNumber;
}