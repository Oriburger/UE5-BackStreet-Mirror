#pragma once
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../../Global/public/BackStreet.h"
#include "WaveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateStage, class AStageData*, Target);

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

public:
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

	// Ŭ���� üũ �� ���� ���� Ÿ�̸� ����, �ð��Ǹ� Clear Wave ȣ�� 
	UFUNCTION()
		void SetDefenseWaveTimer(class AStageData* Target, float time);

	// ��� ���� ����, ���� ���̺� Ȥ�� �������� Ŭ����
	UFUNCTION()
		void ClearDefenseWave(class AStageData* Target);

	//  ���潺 ���̺��� �ð� ���
	UFUNCTION()
		void CalculateWaveTime();

	/*UFUNCTION()
		void SpawnMonsterWithID(class AStageData* Target, int32 EnemyID);*/

	// ���潺 ���̺��� ���������� �����ϴ� ���� �� ����
	UFUNCTION()
		void ManageDefenseWaveMonsterCount(class AStageData* Target, int32 EnemyID, bool IsSpawn);

public:
	TWeakObjectPtr<class AChapterManagerBase> ChapterManager;

	TWeakObjectPtr<class AResourceManager> ResourceManager;

public:
	UPROPERTY()
		FTimerHandle SpawnTimerHandle;

};
