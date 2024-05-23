// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Actor.h"
#include "NewChapterManagerBase.generated.h"

USTRUCT(BlueprintType)
struct FEnemyGroupInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TMap<int32, int32> EnemySet; // EnemyID,스폰 수
};

USTRUCT(BlueprintType)
struct FEnemyCompositionInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<FEnemyGroupInfo> CompositionList;
};

//이동 예정 LJH
USTRUCT(BlueprintType)
struct FStageInfo
{
	GENERATED_BODY()

public:
	//==== Static Proptery ====================
	
	//Stage type (entry, combat, time attack, boss, miniGame, gatcha etc..)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EStageCategoryInfo StageType;

	//Map location in 2nd array (현재 선형으로 1차원 X값만 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector2D TilePos;

	//Main level of this stage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TSoftObjectPtr<UWorld> MainLevelAsset;

	//Outer level of this stage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TSoftObjectPtr<UWorld> OuterLevelAsset;

	//Battle stages only use this value!
	//List of enemy's composition to spawn 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FEnemyCompositionInfo EnemyCompositionInfo;

	//==== Dynamic Proptery ====================
	//Dyanmically update after level load

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector> EnemySpawnLocationList; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector PlayerStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector> PortalLocationList;

	//Timed-stage only use this value!
	UPROPERTY()
		float TimeLimitValue = 0.0f;

	UPROPERTY()
		bool bIsClear = false;

	UPROPERTY()
		bool bIsVisited = false;
};

USTRUCT(BlueprintType)
struct FChapterInfo : public FTableRowBase
{
	GENERATED_BODY()

//======= Gameplay ====================
public:
	//Table Key
	UPROPERTY(EditDefaultsOnly)
		int32 ChapterID;

	//Time attack's time limit value
	UPROPERTY(EditDefaultsOnly)
		float EliteTimeAtkStageTimeOut = 60.0f;
	UPROPERTY(EditDefaultsOnly)
		float NormalTimeAtkStageTimeOut = 50.0f;

	//Enemy composition information of stage type
	UPROPERTY(EditDefaultsOnly)
		TMap<EStageCategoryInfo, FEnemyCompositionInfo> EnemyCompositionInfoMap;

//======= Asset =======================
public:
	//Entry stage level list
	UPROPERTY(EditDefaultsOnly)
		TArray<TSoftObjectPtr<UWorld>> EntryStageLevelList;

	//Combat stage level list
	UPROPERTY(EditDefaultsOnly)
		TArray<TSoftObjectPtr<UWorld>> CombatStageLevelList;

	//Time attack stage level list
	UPROPERTY(EditDefaultsOnly)
		TArray<TSoftObjectPtr<UWorld>> TimeAttackStageLevelList;

	//Craft stage level list
	UPROPERTY(EditDefaultsOnly)
		TArray<TSoftObjectPtr<UWorld>> CraftStageLevelList;

	//Boss stage level list
	UPROPERTY(EditDefaultsOnly)
		TArray<TSoftObjectPtr<UWorld>> BossStageLevelList;

	//Outer area level asset
	UPROPERTY(EditDefaultsOnly)
		TArray<TSoftObjectPtr<UWorld>> OuterStageLevelList;
};





UCLASS()
class BACKSTREET_API ANewChapterManagerBase : public AActor
{
	GENERATED_BODY()

//======== Basic ===============
public:
	// Sets default values for this actor's properties
	ANewChapterManagerBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UStageManagerComponent* StageManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UStageGeneratorComponent* StageGeneratorComponent;

//======== Main Function ===============
public:
	//Start new chapter using id (data table)
	UFUNCTION(BlueprintCallable)
		void StartChapter(int32 NewChapterID);

	//Finish current chapter
	UFUNCTION()
		void FinishChapter(bool bIsGameOver);

	//?
	UFUNCTION()
		void ResetChapter();

	//Move next stage
	UFUNCTION(BlueprintCallable)
		void MoveStage(FVector2D direction);

protected:
	//Init chapter using data table 
	//It must be called before "StartChapter"
	UFUNCTION()
		void InitChapter(int32 NewChapterID);

	//Delegate function called by UStageManageComponent
	UFUNCTION()
		void OnStageFinished(FStageInfo StageInfo);

//======== Property ===============
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FChapterInfo GetCurrentChapterInfo() { return CurrentChapterInfo; }

private:
	int32 ChapterID;

	FChapterInfo CurrentChapterInfo;

	UDataTable* ChapterInfoTable;

	FVector2D CurrentStageLocation;

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	TWeakObjectPtr<class AMainCharacterBase> PlayerRef;

	TArray<FStageInfo> StageInfoList;
};
