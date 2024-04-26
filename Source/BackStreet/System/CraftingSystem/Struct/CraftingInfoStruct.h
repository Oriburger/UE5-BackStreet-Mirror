#pragma once

#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "CraftingInfoStruct.generated.h"

UENUM(BlueprintType)
enum class ECraftingSlotVisual : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Craftable			UMETA(DisplayName = "Craftable"),
	E_Uncraftable		UMETA(DisplayName = "Uncraftable"),
	E_Unidentified		UMETA(DisplayName = "UnIdentified"),
	E_Hide				UMETA(DisplayName = "Hide"),
};

USTRUCT(BlueprintType)
struct FCraftingRecipeStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 ResultWeaponID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<int32> IngredientWeaponID;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<uint8> CraftableChapterList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		EWeaponType ResultWeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		uint8 CraftableLevel;

	//Craftable State for widget visual
	//UnIdentified and Uncraftable: E_UnIdentified
	//Identified but Uncraftable : E_Uncraftable
	//Identified and Craftable : E_Craftable
	UPROPERTY(BlueprintReadWrite)
		ECraftingSlotVisual CraftingSlotVisual = ECraftingSlotVisual::E_Craftable;
};
