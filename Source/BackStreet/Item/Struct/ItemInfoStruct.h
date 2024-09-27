#pragma once

#include "WeaponInfoStruct.h"
#include "../../Character/Struct/CharacterInfoEnum.h"
#include "Engine/DataTable.h"
#include "ItemInfoStruct.generated.h"

UENUM(BlueprintType)
enum class EItemLifeType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Permanant			UMETA(DisplayName = "Permanant"),
	E_Temporary			UMETA(DisplayName = "Temporary"),
};

UENUM(BlueprintType)
enum class EItemCategoryInfo : uint8
{
	E_None					UMETA(DisplayName = "None"),
	E_Weapon				UMETA(DisplayName = "Weapon"),
	E_Ability				UMETA(DisplayName = "Ability"),
	E_Craft					UMETA(DisplayName = "Craft"),
	E_Research				UMETA(DisplayName = "Research"),
	E_GatchaTicket			UMETA(DisplayName = "GatchaTicket"),
};

UENUM(BlueprintType)
enum class ECraftingItemType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Screw				UMETA(DisplayName = "Screw"),
	E_Spring				UMETA(DisplayName = "Spring"),
	E_Gear				UMETA(DisplayName = "Gear"),
	E_Wrench			UMETA(DisplayName = "Wrench"),
};

//������ �ڽ� ���� ���� ������ ���� ����ü
USTRUCT(BlueprintType)
struct FItemInfoDataStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//������ ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 ItemID = 0;

	//�����۸�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemName;

	//Item life type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		EItemLifeType ItemLifeType = EItemLifeType::E_None;

	//Item Type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		EItemCategoryInfo ItemType = EItemCategoryInfo::E_None;

	//Is item has actual appearance with actor? 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actor")
		bool bIsActorItem = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actor")
		TSubclassOf<class AItemBase> ItemClass;

//=============Asset Info =================
// 
	//������ ������ ����ƽ �޽� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		TSoftObjectPtr<UStaticMesh> ItemMesh;

	//�޽��� �ʱ� ��ġ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialLocation = FVector::ZeroVector; 

	//�޽��� �ʱ� ȸ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FRotator InitialRotation = FRotator::ZeroRotator;

	//�޽��� �ʱ� ũ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialScale = FVector::ZeroVector;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TSoftObjectPtr<UMaterialInstance> OutlineMaterial;

	//Item Icon Image
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		UTexture2D* ItemImage;

	//Item Icon Image
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		UTexture2D* DeactiveItemImage;

//=============State =================
	
	//Item Amount which can not edit in datatable
	UPROPERTY(BlueprintReadOnly)
		uint8 ItemAmount = 0;
};

//������ �ڽ� ���� ���� ������ ���� ����ü
USTRUCT(BlueprintType)
struct FItemBoxSpawnInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//�������� ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 StageID = 0;

	//������ ������ Ŭ���� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<int32> SpawnItemBoxIDList;

	//0~1 ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<float> SpawnProbabilityList;
};

//�κ��丮�� �����ϴ� ��ҵ��� ������ ���� ����ü
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