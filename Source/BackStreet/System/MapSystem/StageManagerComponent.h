// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NewChapterManagerBase.h"
#include "Components/ActorComponent.h"
#include "StageManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UStageManagerComponent : public UActorComponent
{
	GENERATED_BODY()

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
	//Spawn new level instance usina level name
	UFUNCTION()
		void CreateLevelInstance(FName LevelName, FName OuterLevelName);

	//Clear previous stage's level instance
	UFUNCTION()
		void ClearPreviousLevelInstance();

	//Update spawn point member of stage info struct after map load 
	//There are several points to spawn enemy, player, craftbox etc
	UFUNCTION()
		void UpdateSpawnPointProperty();

	//Spawn monster 
	UFUNCTION()
		void SpawnEnemy();

	UFUNCTION()
		void CheckLoadStatusAndStartGame();

//======== Gameplay Function ===============
public:
	//Start current stage 
	//It will be called automatically after loading of level instance
	UFUNCTION()
		void StartStage();

	//not working yet
	UFUNCTION()
		void FinishStage(bool bIsGameOver);

//======== Property ===============
public:
	UFUNCTION()
		bool GetLoadIsDone() { return LoadStatus == 0;  }

private:
	UFUNCTION()
		void UpdateLoadStatusCount() { LoadStatus -= 1; }

private:
	ULevelStreamingDynamic* OuterAreaRef;

	ULevelStreamingDynamic* MainAreaRef;

	int32 LoadStatus = 0;

	float LoadTimeOut = 30.0f;

	FTimerHandle LoadCheckTimerHandle;

	FStageInfo CurrentStageInfo;
};
