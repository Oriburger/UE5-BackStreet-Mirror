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

//������ �ڽ� ���� ���� ������ ���� ����ü
USTRUCT(BlueprintType)
struct FItemInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//������ ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 ItemID = 0;

	//�����۸�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		EItemCategoryInfo ItemType = EItemCategoryInfo::E_None;

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

	//Ư�� �������� ���� ���� (�Ѿ�, �̼� ��)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		float Variable = 0.0f;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
//		FSound
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