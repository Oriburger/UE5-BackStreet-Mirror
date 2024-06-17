#pragma once

#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "CraftingInfoStruct.generated.h"

USTRUCT(BlueprintType)
struct FCraftingMaterialStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<uint8> RequiredMaterialByLevel;
};

USTRUCT(BlueprintType)
struct FVariableByLevelStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<float> VariableByLevel;
};


USTRUCT(BlueprintType)
struct FSkillUpgradeInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 SkillID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 WeaponID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		uint8 MaxLevel = 0;

	//<MaterialID, Need Num of Material>
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<int32, FCraftingMaterialStruct> RequiredMaterialMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<float> CoolTimeByLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, FVariableByLevelStruct> SkillVariableMap;
};

USTRUCT(BlueprintType)
struct FStatUpgradeInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 WeaponID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<EWeaponStatType, uint8> MaxWeaponStatLevelMap;

	//<MaterialID, Need Num of Material>
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<int32, FCraftingMaterialStruct> StatAttackRequiredMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<int32, FCraftingMaterialStruct> StatAttackSpeedRequiredMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<int32, FCraftingMaterialStruct> StatFinalImpactRequiredMap;
};
