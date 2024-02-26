#pragma once
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../../Global/public/BackStreet.h"
#include "WaveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateStage, class AStageData*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateWaveUpdate, int32, CurrWave, int32, EndWave);

UCLASS()
class BACKSTREET_API AWaveManager : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStage StageClearDelegate;

	// Gate ��Ȱ��ȭ �� ���� ����
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStage StartDefenseWaveDelegate;

	// Gate Ȱ��ȭ �� ���� ���� ����
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStage ClearDefenseWaveDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateWaveUpdate WaveUpdateDelegate;

public:
	AWaveManager();

	// ��������Ʈ ���ε�
	UFUNCTION()
		void InitWaveManager(class AChapterManagerBase* Target);

	// ���� ��� ���� �ʱ�ȭ
	UFUNCTION()
		void InitReference(class AChapterManagerBase* TargetA, class AResourceManager* TargetB);

	// �� ���������� ��� ���̺� Ŭ���� ���� üũ
	UFUNCTION()
		bool CheckAllWaveClear(class AStageData* Target);

	// �Լ��� �����.., ���Ͱ� ������ ȣ��Ǿ� ���̺� Ŭ���� ���� üũ �� ���� �� ����
	UFUNCTION()
		void CheckWaveCategoryByType(class AStageData* Target, AEnemyCharacterBase* Enemy);

	// �������� Ÿ�Ժ� ���̺� ����
	UFUNCTION()
		void SetWave(class AStageData* Target);

	// �ϵ��� ���̺� ����
	UFUNCTION()
		void SpawnHadesWave(class AStageData* Target);

	// ���潺 ���̺� ����
	UFUNCTION()
		void SpawnDefenseWave(class AStageData* Target);

	// Set Timer related to Clear Time
	UFUNCTION()
		void SetDefenseWaveClearTimer(class AStageData* Target);

	//  Set Timer related to Spawn Time
	UFUNCTION()
		void SetDefenseWaveSpawnTimer(class AStageData* Target);

	UFUNCTION()
		void ClearDefenseWaveTimer(class AStageData* Target);

	UFUNCTION()
		void SpawnDefenseWaveMonster();

	// ��� ���� ����, ���� ���̺� Ȥ�� �������� Ŭ����
	UFUNCTION()
		void ClearDefenseWave();

	// ���潺 ���̺��� ���������� �����ϴ� ���� �� ����
	UFUNCTION()
		void ManageWaveMonsterCount(class AStageData* Target, int32 EnemyID, bool IsSpawn);

	// Select EnemyID To Spawn
	UFUNCTION()
		int32 SelectEnemyID();

	// Get the total number of enemy spawned during a wave
	UFUNCTION()
		int32 GetTotalNumberOfEnemySpawn(class AStageData* Target);

	// Call WaveUpdateDelegate
	UFUNCTION()
		void UpdateWaveUI(class AStageData* Target);


public:
	TWeakObjectPtr<class AChapterManagerBase> ChapterManager;

	TWeakObjectPtr<class AResourceManager> ResourceManager;

public:

	UPROPERTY()
		FTimerHandle WaveIntervalTimerHandle;

		FTimerDelegate  WaveIntervalTimerDelegate;

	UDataTable* WaveEnemyDataTable;


};
