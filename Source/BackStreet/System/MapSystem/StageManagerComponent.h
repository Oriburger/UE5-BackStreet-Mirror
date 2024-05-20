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

	//Spawn enemy character
	UFUNCTION()
		void SpawnEnemy();

	//Spawn portals
	UFUNCTION()
		void SpawnPortal(int32 GateCount = 1);

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
	//Spawned actor list to manage life cycle
	//Gate, Enemy Character, Craftbox
	TArray<AActor*> SpawnedActorList;

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

	//Gate class to spawn on world
	TSubclassOf<class AGateBase> GateClass;
};
