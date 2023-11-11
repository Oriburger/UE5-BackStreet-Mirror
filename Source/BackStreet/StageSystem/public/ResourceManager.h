// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../../Global/public/BackStreet.h"
#include "ResourceManager.generated.h"


UCLASS()
class BACKSTREET_API AResourceManager : public AActor
{
	GENERATED_BODY()

public:
	AResourceManager();

protected:
	virtual void BeginPlay() override;


public:

	UFUNCTION()
		void SpawnStageActor(class AStageData* Target);

	UFUNCTION()
		void SpawnMonster(class AStageData* Target);

	UFUNCTION()
		void SpawnBossMonster(class AStageData* Target);

	UFUNCTION()
		void SpawnItem(class AStageData* Target);

	UFUNCTION()
		void SpawnRewardBox(class AStageData* Target);

	UFUNCTION()
		void SpawnCraftingBox(class AStageData* Target);

	UFUNCTION()
		void BindDelegate(class AStageData* Target);

	UFUNCTION()
		void DieMonster(AEnemyCharacterBase* Target);

	UFUNCTION()
		void CleanAllResource();

	UFUNCTION()
		void CleanStage(class AStageData* Target);

	UFUNCTION()
		void CleanStageMonster(class AStageData* Target);

	UFUNCTION()
		void CleanStageItem(class AStageData* Target);

	// Wave 관련
public:
	UFUNCTION()
		bool CheckAllWaveClear(class AStageData* Target);

	// 클리어 체크 및 스폰 관련 타이머 설정, 시간되면 Clear Wave 호출 
	UFUNCTION()
		void SetDefenseWaveTimer(class AStageData* Target,float time);

	// 모든 몬스터 삭제, 다음 웨이브 혹은 스테이지 클리어
	UFUNCTION()
		void ClearDefenseWave(class AStageData* Target);

	UFUNCTION()
		void CalculateWaveTime();

	UFUNCTION()
		void SpawnDefenseWave();

	// 웨이브별 초기 세팅, SpawnStageActor에서 호출할듯?
	UFUNCTION()
		void SetWave(class AStageData* Target);

	UFUNCTION()
		void SpawnMonsterWithID(class AStageData* Target, int32 EnemyID);

	UFUNCTION()
		int32 GetSpawnPointIdx(class AStageData* Target);

	UFUNCTION()
		void ManageDefenseWaveMonsterCount(class AStageData* Target, int32 EnemyID, bool IsSpawn);

public:
	UFUNCTION()
		TSubclassOf<AEnemyCharacterBase> GetEnemyWithID(int32 EnemyID);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AEnemyCharacterBase>> EnemyAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AEnemyCharacterBase>> BossAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AItemBoxBase>> ItemBoxAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class ARewardBoxBase>> RewardBoxAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class ACraftBoxBase>> CraftingBoxAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* WaveCompositionDataTable;

public:
	UPROPERTY()
		FTimerHandle SpawnTimerHandle;
	
};
