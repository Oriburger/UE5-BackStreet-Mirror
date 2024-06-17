#pragma once

#include "WeaponInfoStruct.h"
#include "../../Character/Struct/CharacterInfoEnum.h"
#include "Engine/DataTable.h"
#include "ItemInfoStruct.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	E_None					UMETA(DisplayName = "None"),
	E_Permanant			UMETA(DisplayName = "Permanant"),
	E_Temporary			UMETA(DisplayName = "Temporary"),
};

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
		int32 ItemID = 0;

	//아이템명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		EItemCategoryInfo ItemType = EItemCategoryInfo::E_None;

	//스폰할 아이템 스태틱 메시 정보 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		TSoftObjectPtr<UStaticMesh> ItemMesh;

	//메시의 초기 위치 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialLocation = FVector::ZeroVector; 

	//메시의 초기 회전 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FRotator InitialRotation = FRotator::ZeroRotator;

	//메시의 초기 크기 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialScale = FVector::ZeroVector;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TSoftObjectPtr<UMaterialInstance> OutlineMaterial;

	//특정 아이템을 위한 변수 (총알, 미션 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		float Variable = 0.0f;

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
		int32 StageID = 0;

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
		int32 WeaponID = 0;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FWeaponStatStruct WeaponStat; 

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FWeaponStateStruct WeaponState;
};

//for "Not Actor Item"
USTRUCT(BlueprintType)
struct FItemDataStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//Item ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 ItemID = 0;

	//Item name
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemName;

	//Item type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		EItemType ItemType = EItemType::E_None;

	//Attainable Chapter
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		uint8 AttainableChapter = 0;

	//Item Description
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemDescription;

	//Item Icon Image
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		UTexture2D* ItemImage;

	//Item Amount which can not edit in datatable
	UPROPERTY(BlueprintReadOnly)
		uint8 ItemAmount = 0;

};