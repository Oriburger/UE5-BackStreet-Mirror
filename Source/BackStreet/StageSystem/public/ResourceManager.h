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

public:
	UFUNCTION()
		TSubclassOf<AEnemyCharacterBase> GetEnemyWithID(int32 EnemyID);

	UFUNCTION()
		FName GetRandomMap();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FName> MapNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AEnemyCharacterBase>> EnemyAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AEnemyCharacterBase>> BossAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AItemBoxBase>> ItemBoxAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class ARewardBoxBase>> RewardBoxAssets;

	
};
