#pragma once

#include "Engine/DataTable.h"
#include "../../Character/public/EnemyStatInfoStruct.h"
#include "StageInfoStruct.generated.h"

UENUM(BlueprintType)
enum class EStageCategoryInfo : uint8
{
	E_None				  	UMETA(DisplayName = "None"),
	E_Normal				UMETA(DisplayName = "Normal"),
	E_Mission			  	UMETA(DisplayName = "Mission"),
	E_Boss				    UMETA(DisplayName = "Boss"),
	E_Lobby					UMETA(DisplayName = "Lobby"),

};

USTRUCT(BlueprintType)
struct FStageEnemyRankStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint8 StageLevel;

	UPROPERTY(EditAnywhere)
		FName StageType;

	UPROPERTY(EditAnywhere)
		FEnemyStatStruct StatData;

};

USTRUCT(BlueprintType)
struct FStageEnemyTypeStruct : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
		FName StageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<int32> IDList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxSpawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MinSpawn;


};

USTRUCT(BlueprintType)
struct FStageDataStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
		int32 XPos;

	UPROPERTY()
		int32 YPos;

	UPROPERTY()
		FVector StageLocation;

	UPROPERTY()
		TArray<bool> GateInfo;

	UPROPERTY()
		EStageCategoryInfo Type;

	UPROPERTY()
		FName LevelToLoad;

	UPROPERTY()
		bool bIsVisited;

	UPROPERTY()
		bool bIsClear;

	UPROPERTY()
		bool bIsValid;

	UPROPERTY()
		TArray<FVector> MonsterSpawnPoints;

	UPROPERTY()
		TArray<FVector> ItemSpawnPoints;

	UPROPERTY()
		TArray<FVector> CharacterSpawnPoint;

	UPROPERTY()
		TArray<FVector> RewardBoxSpawnPoint;

	UPROPERTY()
		TArray<class AEnemyCharacterBase*> MonsterList;

	UPROPERTY()
		TArray<class AItemBase*> ItemList;

	UPROPERTY()
		TArray<class AItemBoxBase*> ItemBoxList;

	UPROPERTY()
		TArray<class AGateBase*> GateList;

	UPROPERTY()
		class ARewardBoxBase* RewardBoxRef;

	UPROPERTY()
		ULevelStreaming* LevelRef;

	UPROPERTY()
		FTimerHandle StageTimerHandle;

	UPROPERTY()
		int32 StageTime;


};