#pragma once

#include "Engine/DataTable.h"
#include "../../Character/public/CharacterInfoEnum.h"
#include "WeaponInfoStruct.h"
#include "ItemInfoStruct.generated.h"


UENUM(BlueprintType)
enum class EItemCategoryInfo : uint8
{
	E_None					UMETA(DisplayName = "None"),
	E_Weapon				UMETA(DisplayName = "Weapon"),
	E_Bullet				UMETA(DisplayName = "Bullet"),
	E_DeBuff				UMETA(DisplayName = "DeBuff"),
	E_StatUp				UMETA(DisplayName = "StatUp"),
	E_Mission				UMETA(DisplayName = "Mission"),
};


//인벤토리를 구성하는 요소들의 정보를 담은 구조체
USTRUCT(BlueprintType)
struct FInventoryItemInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		int32 WeaponID;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FWeaponStatStruct WeaponStat; 

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FWeaponStateStruct WeaponState;
};

