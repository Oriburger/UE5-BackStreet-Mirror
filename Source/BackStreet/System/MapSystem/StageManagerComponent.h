// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NewChapterManagerBase.h"
#include "Components/ActorComponent.h"
#include "StageManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateStageEnd, FStageInfo, StageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateRewardGrant, const TArray<int32>&, RewardID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateStageClear);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateLoadBegin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateLoadDone);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateTimeAttackBegin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateTimeAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateTimeOver);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKSTREET_API UStageManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	//======== Delegate ============
public:
	UPROPERTY()
		FDelegateStageEnd OnStageFinished;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStageClear OnStageCleared;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateRewardGrant OnRewardGranted;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTimeOver OnTimeIsOver;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateLoadBegin OnStageLoadBegin;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateLoadDone OnStageLoadDone;
	
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTimeAttackBegin OnTimeAttackStageBegin;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTimeAttackEnd OnTimeAttackStageEnd;

//======== Basic ===============
public:	
	// Sets default values for this component's properties
	UStageManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION()
		void Initialize(FChapterInfo NewChapterInfo);

//======== Resource Function ===============
public:
	//Initialize stage
	UFUNCTION()
		void InitStage(FStageInfo NewStageInfo);

	UFUNCTION()
		void ClearResource();

	//Time attack stage only
	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetStageRemainingTime();

	UFUNCTION(BlueprintCallable)
		void RegisterActor(AActor* TargetActor);

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

	UFUNCTION()
		void ClearEnemyActors();

	//Update spawn point member of stage info struct after map load 
	//There are several points to spawn enemy, player, craftbox etc
	UFUNCTION()
		bool TryUpdateSpawnPointProperty();

	//Spawn enemy character
	UFUNCTION()
		void SpawnEnemy();
	
	//Spawn craftbox
	UFUNCTION()
		void SpawnCraftbox(); 

	UFUNCTION()
		void SpawnItemBox();

	//Spawn portals
	UFUNCTION()
		void SpawnPortal(int32 GateCount = 1);

	UFUNCTION()
		void CheckLoadStatusAndStartGame();

	UFUNCTION()
		bool GetLoadIsDone() { return LoadStatus == 0; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetVisitedCraftStageCount() { return VisitedCraftstageCount; }

private:
	UFUNCTION()
		void UpdateLoadStatusCount() { LoadStatus -= 1; }

private:
	//Spawned actor list to manage life cycle
	//Gate, Enemy Character, Craftbox
	TArray< TWeakObjectPtr<AActor> > SpawnedActorList;

	//Current stage's information 
	//Level instance ref and gameplay info struct
	TSoftObjectPtr<UWorld> PreviousLevel;
	FChapterInfo CurrentChapterInfo;
	FStageInfo CurrentStageInfo;
	ULevelStreamingDynamic* OuterAreaRef;
	ULevelStreamingDynamic* MainAreaRef;

	//The status of level instance load
	//0 : done.   Over 0 : loading 
	int32 LoadStatus = 0;

	//Property for level instance load
	float LoadTimeOut = 120.0f;
	FTimerHandle LoadCheckTimerHandle;

	//For instant loading widget
	UUserWidget* LoadingWidgetRef;

	//How many craft stages were visited?
	int32 VisitedCraftstageCount = 0;

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
	//Grant stage reward
	UFUNCTION()
		void GrantStageRewards();

	UFUNCTION()
		AActor* SpawnItemActor(FItemInfoDataStruct ItemInfo);

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

	TWeakObjectPtr<ANewChapterManagerBase> ChapterManagerRef;

	TWeakObjectPtr<class ACharacterBase> PlayerRef;

public:
	//BP Class, initialize with BP
	//Gate class to spawn on world
	//Boss character's class use the data table value

	UPROPERTY(EditDefaultsOnly, Category = "Class")
		TSubclassOf<class AGateBase> GateClass;

	UPROPERTY(EditDefaultsOnly, Category = "Class")
		TSubclassOf<class AEnemyCharacterBase> EnemyCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "Class")
		TSubclassOf<class AItemBoxBase> ItemBoxClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Class")
		TSubclassOf<UUserWidget> LoadingWidgetClass;
};
