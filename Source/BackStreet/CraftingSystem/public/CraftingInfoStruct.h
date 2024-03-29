#pragma once
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "CraftingInfoStruct.generated.h"

UENUM(BlueprintType)
enum class ECraftingType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_All				UMETA(DisplayName = "All"),
	E_Melee			UMETA(DisplayName = "Melee"),
	E_Shoot				UMETA(DisplayName = "Shoot"),
	E_Throw		UMETA(DisplayName = "Throw"),
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
		TArray<uint8> AvailableChapterList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		EWeaponType CraftingType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		uint8 CraftingLevel;
};
