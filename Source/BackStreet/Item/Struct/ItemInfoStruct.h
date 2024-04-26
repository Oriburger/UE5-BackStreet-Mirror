#pragma once

#include "WeaponInfoStruct.h"
#include "../Character/Struct/CharacterInfoEnum.h"
#include "Engine/DataTable.h"
#include "ItemInfoStruct.generated.h"


UENUM(BlueprintType)
enum class EItemCategoryInfo : uint8
{
	E_None					UMETA(DisplayName = "None"),
	E_Weapon				UMETA(DisplayName = "Weapon"),
	E_Bullet				UMETA(DisplayName = "Bullet"),
	E_StatUp				UMETA(DisplayName = "Ability"),
	E_Mission				UMETA(DisplayName = "Mission"),
};

//아이템 박스 스폰 관련 정보를 담은 구조체
USTRUCT(BlueprintType)
struct FItemInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//아이템 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 ItemID;

	//아이템명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		EItemCategoryInfo ItemType;

	//스폰할 아이템 스태틱 메시 정보 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		TSoftObjectPtr<UStaticMesh> ItemMesh;

	//메시의 초기 위치 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialLocation; 

	//메시의 초기 회전 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FRotator InitialRotation;

	//메시의 초기 크기 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialScale;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TSoftObjectPtr<UMaterialInstance> OutlineMaterial;

	//특정 아이템을 위한 변수 (총알, 미션 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		float Variable;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
//		FSound
};

//아이템 박스 스폰 관련 정보를 담은 구조체
USTRUCT(BlueprintType)
struct FItemBoxSpawnInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//스테이지 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 StageID;

	//스폰할 아이템 클래스 정보 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<int32> SpawnItemBoxIDList;

	//0~1 값을 지님
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<float> SpawnProbabilityList;
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

