// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../../Global/BackStreet.h"
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
	// Initialize member variable
	UFUNCTION()
		void InitReference(class AWaveManager* Target);

	// Called when the stage is first visited, it creates resources within the stage 
	UFUNCTION()
		void SpawnStageActor(class AStageData* Target);

	// Create a Boss Monster
	UFUNCTION()
		void SpawnBossMonster(class AStageData* Target);

	// Create an Item
	UFUNCTION()
		void SpawnItem(class AStageData* Target);

	// Create a RewardBox
	UFUNCTION()
		void SpawnRewardBox(class AStageData* Target);

	// Create a CraftingBox
	UFUNCTION()
		void SpawnCraftingBox(class AStageData* Target);

	// Remove a monster from the stage monster list.
	UFUNCTION()
		void RemoveMonsterFromList(AEnemyCharacterBase* Target);

	// Delete all resources in the chapter
	UFUNCTION()
		void CleanAllResource();

	// Delete all resources in the Target Stage
	UFUNCTION()
		void CleanStage(class AStageData* Target);

	// Delete all Monsters in the Target Stage.
	UFUNCTION()
		void CleanStageMonster(class AStageData* Target);

	// Delete all Items in the Target Stage.
	UFUNCTION()
		void CleanStageItem(class AStageData* Target);

public:
	// Create Monster And Bind Monster Delegate
	UFUNCTION()
		AEnemyCharacterBase* SpawnMonster(class AStageData* Target, int32 EnemyID);

	// Creates a monster on the target stage
	UFUNCTION()
		AEnemyCharacterBase* CreateMonster(class AStageData* Target, int32 EnemyID, FVector Location);

	// Bind delegates related to the stage system and monsters
	UFUNCTION()
		void BindMonsterDelegate(class AStageData* Target, class AEnemyCharacterBase* Monster);

	// Gets a location to spawn a monster
	UFUNCTION()
		FVector GetSpawnLocation(class AStageData* Target);

	// Gets the current spawn point and increases MonsterSpawnPointOrderIdx
	UFUNCTION()
		int32 GetSpawnPointIdx(class AStageData* Target);

	//// Select the ID of the monster to spawn
	//UFUNCTION()
	//	int32 SelectSpawnMonsterID(class AStageData* Target);

	//// Select the total amount of monsters to spawn
	//UFUNCTION()
	//	int32 SelectSpawnMonsterAmount(class AStageData* Target);

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
