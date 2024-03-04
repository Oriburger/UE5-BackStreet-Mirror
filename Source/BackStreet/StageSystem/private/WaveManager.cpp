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
	AStageData* stage = Cast<AChapterManagerBase>(GetOwner())->GetCurrentStage();
	WaveUpdateDelegate.AddDynamic(Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode()), &ABackStreetGameModeBase::UpdateWaveInfoUI);

	UpdateWaveUI(stage);
	
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
				Target->DoStageClearTask();
			else
			{
				if (WaveIntervalTimerDelegate.IsBound())
					WaveIntervalTimerDelegate.Unbind();

				WaveIntervalTimerDelegate.BindUFunction(this, FName("SpawnHadesWave"), Target);
				Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()))->UIAnimationDelegate.Broadcast(FName("NextWave"));
				GetWorld()->GetTimerManager().SetTimer(WaveIntervalTimerHandle, WaveIntervalTimerDelegate, 6.f, false);

			}
			UpdateWaveUI(Target);
		}
	}
	else if (Target->GetStageTypeInfo().WaveType == EWaveCategoryInfo::E_TimeLimitWave)
	{
		ManageWaveMonsterCount(Target, Enemy->CharacterID, false);
	}
	else if (Target->GetStageCategoryType() == EStageCategoryInfo::E_Boss)
	{
		float currHP = 0.0f;
		if (IsValid(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
		{
			ACharacterBase* playerRef = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			currHP = IsValid(playerRef) ? playerRef->GetCharacterState().CharacterCurrHP : 0.0f;
		}
		
		if (Target->GetMonsterList().IsEmpty() && currHP > 0.0f)
		{
			Target->DoStageClearTask();
			Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()))->UIAnimationDelegate.Broadcast(FName("ChapterClear"));
			UpdateWaveUI(Target);
		}
	}
		
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
	UpdateWaveUI(Target);
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
	UpdateWaveUI(Target);
	SetDefenseWaveClearTimer(Target);
	SetDefenseWaveSpawnTimer(Target);

}

void AWaveManager::SetDefenseWaveClearTimer(class AStageData* Target)
{
	// Get Wave Clear Time
	float totalTime = Target->GetStageTypeInfo().ClearTimeForEachWave.FindRef(Target->GetCurrentWaveLevel() + 1);
	GetWorldTimerManager().SetTimer(Target->GetDefenseWaveClearTimeTimerHandle(), this, &AWaveManager::ClearDefenseWave, totalTime, false);
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
		stage->DoStageClearTask();
		UpdateWaveUI(stage);
	}
	else
	{
		if (WaveIntervalTimerDelegate.IsBound())
			WaveIntervalTimerDelegate.Unbind();

		WaveIntervalTimerDelegate.BindUFunction(this, FName("SpawnDefenseWave"), stage);
		Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()))->UIAnimationDelegate.Broadcast(FName("NextWave"));
		GetWorld()->GetTimerManager().SetTimer(WaveIntervalTimerHandle, WaveIntervalTimerDelegate, 6.f, false);
	}
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

	int32 currWaveLevel = stage->GetCurrentWaveLevel();

	int32 dataTableID = (stage->GetStageTypeInfo().WaveComposition.IsValidIndex(currWaveLevel) ?
							stage->GetStageTypeInfo().WaveComposition[currWaveLevel] : 1);
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

void AWaveManager::UpdateWaveUI(class AStageData* Target)
{   
	if(!IsValid(Target)) return;
	int32 currWave = Target->GetCurrentWaveLevel();
	int32 endWave = Target->GetStageTypeInfo().MaxWave;

	if(Target->GetStageCategoryType()==EStageCategoryInfo::E_Lobby)
		WaveUpdateDelegate.Broadcast(-1,-1);
	else if (Target->GetStageCategoryType() == EStageCategoryInfo::E_Boss)
	{
		if(Target->GetIsClear())
			WaveUpdateDelegate.Broadcast(1, 1);
		else
			WaveUpdateDelegate.Broadcast(0, 1);
	}
	else
		WaveUpdateDelegate.Broadcast(currWave, endWave);
}