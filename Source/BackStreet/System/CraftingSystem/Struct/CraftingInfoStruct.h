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
struct FCraftingRecipeStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 SkillID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 WeaponID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		uint8 MaxLevel;

	//<MaterialID, Need Num of Material>
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<int32, FCraftingMaterialStruct> RequiredMaterialMap;
};
