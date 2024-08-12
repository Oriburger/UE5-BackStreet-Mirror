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
struct FRequiredMaterialContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<uint8> RequiredMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float StatByLevel;
};

USTRUCT(BlueprintType)
struct FWeaponUpgradedStatInfo : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	//주어진 무기의 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		uint8 MaxLevel = 5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<FRequiredMaterialContainer> RequiredMaterialByLevel;
};

USTRUCT(BlueprintType)
struct FStatUpgradeInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 WeaponID = 0;

	//0 : Attack, 1 : AttackSpeed, 2 : FinalImpact
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<FWeaponUpgradedStatInfo> RequiredInfo;
};

// 레거시 코드 ==================

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
