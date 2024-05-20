// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Actor.h"
#include "NewChapterManagerBase.generated.h"


//이동 예정 LJH
USTRUCT(BlueprintType)
struct FStageInfo
{
	GENERATED_BODY()

public:
	//Stage type (entry, combat, time attack, boss, miniGame, gatcha etc..)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EStageCategoryInfo StageType;

	//Map location in 2nd array (현재 선형으로 1차원 X값만 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector2D TilePos;

	//Map name of this stage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FName LevelAssetName;

	//Outer map name of this stage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FName OuterLevelAssetName;
	
	//Dyanmically update after level load
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector> EnemySpawnLocationList; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector PlayerStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector> GateLocationList;
};

USTRUCT(BlueprintType)
struct FChapterInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	//Table Key
	UPROPERTY(EditDefaultsOnly)
		int32 ChapterID;

	//Entry stage map name list
	UPROPERTY(EditDefaultsOnly)
		TArray<FName> EntryStageMapNameList;

	//Combat & elite combat stage map name list
	UPROPERTY(EditDefaultsOnly)
		TArray<FName> CombatStageMapNameList;
	
	//Time attack & elite time attack stage map name list
	UPROPERTY(EditDefaultsOnly)
		TArray<FName> TimeAttackStageMapNameList;

	UPROPERTY(EditDefaultsOnly)
		TArray<FName> CraftStageMapNameList;

	//Boss stage map name list
	UPROPERTY(EditDefaultsOnly)
		TArray<FName> BossStageMapNameList;

	//Outer Area Map Name
	UPROPERTY(EditDefaultsOnly)
		TArray<FName> OuterAreaMapNameList;
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
	UPROPERTY(EditDefaultsOnly)
		class UStageManagerComponent* StageManagerComponent;

	UPROPERTY(EditDefaultsOnly)
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
	UFUNCTION()
		void MoveStage(FVector2D direction);

protected:
	UFUNCTION()
		void InitChapter(int32 NewChapterID);

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
