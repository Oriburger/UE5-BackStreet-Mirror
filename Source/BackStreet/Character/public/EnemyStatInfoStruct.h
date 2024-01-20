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

	//최대 스폰할 아이템의 개수. 미션 아이템은 무시.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem", meta = (UIMin = 0, UIMax = 2))
		int32 MaxSpawnItemCount;

	//적이 죽고 스폰할 아이템의 Type 리스트 (각 아이템은 Idx로 구별)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<EItemCategoryInfo> SpawnItemTypeList;

	//적이 죽구 스폰할 아이템 ID 리스트 (각 아이템은 Idx로 구별)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<uint8> SpawnItemIDList;

	//적이 죽고 스폰할 아이템의 스폰 확률 리스트  (0.0f ~ 1.0f),  (각 아이템은 Idx로 구별)
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

	//적이 최초로 소유하는 무기의 ID
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
