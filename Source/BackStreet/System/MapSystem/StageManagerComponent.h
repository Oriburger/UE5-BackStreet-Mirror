// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NewChapterManagerBase.h"
#include "Components/ActorComponent.h"
#include "StageManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateStageEnd, FStageInfo, StageInfo);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UStageManagerComponent : public UActorComponent
{
	GENERATED_BODY()

//======== Delegate ============
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStageEnd OnStageFinished;	

//======== Basic ===============
public:	
	// Sets default values for this component's properties
	UStageManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======== Resource Function ===============
public:
	//Initialize stage
	UFUNCTION()
		void InitStage(FStageInfo NewStageInfo);

protected:
	//Add loading screen
	UFUNCTION()
		void AddLoadingScreen();

	UFUNCTION()
		void RemoveLoadingScreen();

	//Spawn new level instance usina level name
	UFUNCTION()
		void CreateLevelInstance(TSoftObjectPtr<UWorld> MainLevel, TSoftObjectPtr<UWorld> OuterLevel);

	//Clear previous stage's level instance
	UFUNCTION()
		void ClearPreviousLevelData();

	//Clear previous stage's level instance
	UFUNCTION()
		void ClearPreviousActors();

	//Update spawn point member of stage info struct after map load 
	//There are several points to spawn enemy, player, craftbox etc
	UFUNCTION()
		void UpdateSpawnPointProperty();

	//Spawn enemy character
	UFUNCTION()
		void SpawnEnemy();

	//Spawn craftbox
	UFUNCTION()
		void SpawnCraftbox(); 

	//Spawn portals
	UFUNCTION()
		void SpawnPortal(int32 GateCount = 1);

	UFUNCTION()
		void CheckLoadStatusAndStartGame();

	UFUNCTION()
		bool GetLoadIsDone() { return LoadStatus == 0; }

private:
	UFUNCTION()
		void UpdateLoadStatusCount() { LoadStatus -= 1; }

private:
	//Spawned actor list to manage life cycle
	//Gate, Enemy Character, Craftbox
	TArray< TWeakObjectPtr<AActor> > SpawnedActorList;

	//Current stage's information 
	//Level instance ref and gameplay info struct
	FStageInfo CurrentStageInfo;
	ULevelStreamingDynamic* OuterAreaRef;
	ULevelStreamingDynamic* MainAreaRef;

	//The status of level instance load
	//0 : done.   Over 0 : loading 
	int32 LoadStatus = 0;

	//Property for level instance load
	float LoadTimeOut = 30.0f;
	FTimerHandle LoadCheckTimerHandle;

	//For instant loading widget
	TSubclassOf<UUserWidget> LoadingWidgetClass;
	UUserWidget* LoadingWidgetRef;

//======== Gameplay Function ===============
public:
	//Start current stage 
	//It will be called automatically after loading of level instance
	UFUNCTION()
		void StartStage();

	//not working yet
	UFUNCTION(BlueprintCallable)
		void FinishStage(bool bStageClear);

	//Get remaining time on time attack stage
	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetRemainingTime() { return GetOwner()->GetWorldTimerManager().GetTimerRemaining(TimeAttackTimerHandle); }

	//Get current stage info 
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FStageInfo GetCurrentStageInfo() { return CurrentStageInfo; }

private:
	//Bind to enemy character death delegate
	UFUNCTION()
		void UpdateEnemyCountAndCheckClear();

	//Combat type stages only!
	UFUNCTION()
		bool CheckStageClearStatus();

	//Decrease enemy count
	UFUNCTION()
		void UpdateRemainingEnemyCount() { RemainingEnemyCount -= 1; }

	//Finish current stage and chapter
	UFUNCTION()
		void SetGameIsOver(); 

private:
	//Combat stage only!
	//Count of enemy to defeat
	int32 RemainingEnemyCount = 0; 

	//Time-attack timer
	FTimerHandle TimeAttackTimerHandle;

	//BP Class, initialize with ConstructorFinder
	//Gate class to spawn on world
	TSubclassOf<class AGateBase> GateClass;
	TSubclassOf<class AEnemyCharacterBase> EnemyCharacterClass;
};
