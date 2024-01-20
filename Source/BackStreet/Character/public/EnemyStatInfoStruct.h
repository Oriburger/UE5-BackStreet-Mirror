// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine/DataTable.h"
#include "../../Item/public/ItemInfoStruct.h"
#include "CharacterInfoStruct.h"
#include "../../SkillSystem/public/SkillInfoStruct.h"
#include "EnemyStatInfoStruct.generated.h"


USTRUCT(BlueprintType)
struct FEnemyDropInfoStruct
{
	GENERATED_BODY()

public:

	//�ִ� ������ �������� ����. �̼� �������� ����.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem", meta = (UIMin = 0, UIMax = 2))
		int32 MaxSpawnItemCount;

	//���� �װ� ������ �������� Type ����Ʈ (�� �������� Idx�� ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<EItemCategoryInfo> SpawnItemTypeList;

	//���� �ױ� ������ ������ ID ����Ʈ (�� �������� Idx�� ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<uint8> SpawnItemIDList;

	//���� �װ� ������ �������� ���� Ȯ�� ����Ʈ  (0.0f ~ 1.0f),  (�� �������� Idx�� ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<float> ItemSpawnProbabilityList;
};

USTRUCT(BlueprintType)
struct FEnemyStatStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		int32 EnemyID;

	UPROPERTY(EditAnywhere)
		FName EnemyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FCharacterStatStruct CharacterStat;

	//���� ���ʷ� �����ϴ� ������ ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 DefaultWeaponID = 11100;

	//Info data of dropping item when enemy dies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FEnemyDropInfoStruct DropInfo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.2f, UIMax = 1.0f))
		float CharacterAtkSpeed;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<int32> EnemySkillList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<float> EnemySkillIntervalList;

	UPROPERTY(EditDefaultsOnly, BlueprintreadOnly)
		FSkillSetInfo SkillSetInfo;

};
