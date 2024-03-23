#pragma once
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "CraftingInfoStruct.generated.h"

USTRUCT(BlueprintType)
struct FCraftingRecipeStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 ResualtWeaponID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<int32> IngredientWeaponID;

};
