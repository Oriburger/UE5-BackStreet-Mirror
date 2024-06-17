#pragma once

#include "Engine/DataTable.h"
#include "StageInfoStruct.generated.h"

UENUM(BlueprintType)
enum class EStageCategoryInfo : uint8
{
	E_None				  	UMETA(DisplayName = "None"),
	E_Lobby					UMETA(DisplayName = "Lobby"),

	E_Entry					UMETA(DisplayName = "Entry"),
	E_Combat				UMETA(DisplayName = "Combat"),
	E_TimeAttack			UMETA(DisplayName = "TimeAttack"),
	E_EliteCombat			UMETA(DisplayName = "EliteCombat"),
	E_EliteTimeAttack		UMETA(DisplayName = "EliteTimeAttack"),
	E_Boss					UMETA(DisplayName = "Boss"),

	E_Craft					UMETA(DisplayName = "Craft"),
	E_MiniGame				UMETA(DisplayName = "MiniGame"),
	E_Gatcha				UMETA(DisplayName = "Gatcha"),
};

UENUM(BlueprintType)
enum class EWaveCategoryInfo : uint8
{
	E_None				  		UMETA(DisplayName = "None"),
	E_NomalWave					UMETA(DisplayName = "E_NomalWave"),
	E_TimeLimitWave			  	UMETA(DisplayName = "E_TimeLimitWave"),
};

UENUM(BlueprintType)
enum class EChapterLevel : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Chapter1			UMETA(DisplayName = "Chapter1"),
	E_Chapter2			UMETA(DisplayName = "Chapter2"),
	E_Chapter3			UMETA(DisplayName = "Chapter3"),
	E_Chapter4			UMETA(DisplayName = "Chapter4")
};

USTRUCT(BlueprintType)
struct FEnemyGroupInfo
{
	GENERATED_BODY()

public:
	//DO NOT ASSIGN THE BOSS ID 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TMap<int32, int32> EnemySet; // EnemyID, spawn count
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
		EStageCategoryInfo StageType = EStageCategoryInfo::E_None;

	//Map location in 2nd array (현재 선형으로 1차원 X값만 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector2D TilePos = FVector2D::ZeroVector;
	
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

	//Boss Stage only
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector BossSpawnLocation = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FRotator BossSpawnRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector PlayerStartLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FRotator PlayerStartRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector> PortalLocationList;

	//Timed-stage only use this value!
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TimeLimitValue = 0.0f;

	//Is stage visited?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsVisited = false;

	//Is stage finished?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsFinished = false;

	//Is stage cleared
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsClear = false;

	//Is game over
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsGameOver = false;
};

USTRUCT(BlueprintType)
struct FChapterInfo : public FTableRowBase
{
	GENERATED_BODY()

	//======= Gameplay ====================
public:
	//Table Key
	UPROPERTY(EditDefaultsOnly)
		int32 ChapterID = 0;

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

	//Boss character class per chapter
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class AEnemyCharacterBase> BossCharacterClass;
};
