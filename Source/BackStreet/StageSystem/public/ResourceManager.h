// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../../Global/public/BackStreet.h"
#include "ResourceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateWave, class AStageData*, Target);

UCLASS()
class BACKSTREET_API AResourceManager : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateWave SetWaveDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateWave CheckWaveClearDelegate;

public:
	AResourceManager();

protected:
	virtual void BeginPlay() override;


public:
	UFUNCTION()
		void InitReference(class AWaveManager* Target);

	UFUNCTION()
		void SpawnStageActor(class AStageData* Target);

	UFUNCTION()
		void SpawnBossMonster(class AStageData* Target);

	UFUNCTION()
		void SpawnItem(class AStageData* Target);

	UFUNCTION()
		void SpawnRewardBox(class AStageData* Target);

	UFUNCTION()
		void SpawnCraftingBox(class AStageData* Target);

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

public:
	UFUNCTION()
		AEnemyCharacterBase* SpawnMonster(class AStageData* Target, int32 EnemyID, FVector Location);

	UFUNCTION()
		void BindMonsterDelegate(class AStageData* Target, class AEnemyCharacterBase* Monster);

	UFUNCTION()
		FVector GetSpawnLocation(class AStageData* Target);

	UFUNCTION()
		int32 GetSpawnPointIdx(class AStageData* Target);

	UFUNCTION()
		int32 SelectSpawnMonsterID(class AStageData* Target);

	UFUNCTION()
		int32 SelectSpawnMonsterAmount(class AStageData* Target);

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

	TWeakObjectPtr<class AWaveManager> WaveManager;


};
