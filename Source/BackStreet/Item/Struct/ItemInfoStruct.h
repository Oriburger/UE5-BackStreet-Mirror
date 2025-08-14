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
	E_SubWeapon				UMETA(DisplayName = "SubWeapon"),
	E_Ability				UMETA(DisplayName = "Ability"),
	E_Craft					UMETA(DisplayName = "Craft"),
	E_Research				UMETA(DisplayName = "Research"),
	E_Skill					UMETA(DisplayName = "Skill"),
};

//������ �ڽ� ���� ���� ������ ���� ����ü
USTRUCT(BlueprintType)
struct FItemInfoDataStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//������ ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
		int32 ItemID = 0;

	//�����۸�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemDescription;

	//�����۸� (����ȭ ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FText ItemNameText;

	//������ ���� (����ȭ ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FText ItemDescriptionText;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		bool bShowIcon = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		bool bSupportIconTurn = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		bool bShowMesh = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		bool bSupportMeshTurn = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		bool bShowInfoWidget = false;

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
	UPROPERTY(BlueprintReadWrite)
		uint8 ItemAmount = 0;
};

USTRUCT(BlueprintType)
struct FItemInventoryInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxItemCount = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxSubWeaponCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxMainWeaponCount = 1;

	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
		TMap<int32, FWeaponStateStruct> WeaponStateMap;

	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
		int32 CurrSubWeaponCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
		int32 CurrMainWeaponCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
		UDataTable* ItemTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TMap<int32, FItemInfoDataStruct> ItemMap;
};