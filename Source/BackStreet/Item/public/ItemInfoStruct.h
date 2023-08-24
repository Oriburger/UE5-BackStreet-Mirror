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
		int32 ItemID;

	//�����۸�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		EItemCategoryInfo ItemType;

	//������ ������ ����ƽ �޽� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TSoftObjectPtr<UStaticMesh> ItemMesh;

	//�޽��� �ʱ� ��ġ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Transform")
		FVector InitialLocation; 

	//�޽��� �ʱ� ȸ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Transform")
		FRotator InitialRotation;

	//�޽��� �ʱ� ũ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Transform")
		FVector InitialScale;

	//������ �޽��� �ƿ����� �÷�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Outline")
		FColor OutlineColor = FColor::Red; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Outline")
		TSoftObjectPtr<UMaterialInstanceDynamic> OutlineMaterial;

	//Ư�� �������� ���� ���� (�Ѿ�, �̼� ��)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		float Variable;

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
		int32 StageID;

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
		int32 WeaponID;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FWeaponStatStruct WeaponStat; 

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FWeaponStateStruct WeaponState;
};

