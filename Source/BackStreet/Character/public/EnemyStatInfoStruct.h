// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine/DataTable.h"
#include "EnemyStatInfoStruct.generated.h"

USTRUCT(BlueprintType)
struct FEnemyStatStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		int32 EnemyID;

	UPROPERTY(EditAnywhere)
		FName EnemyName;

	// 이것도 재사용 가능 할 듯 싶음 Character뭐시기 struct로 바꾸는게 좋을 것 같음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.5f, UIMax = 10.0f))
		float CharacterMaxHP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = -1.0f, UIMax = 1.0f))
		float CharacterDefense;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.1f, UIMax = 10.0f))
		float CharacterAtkMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 100.0f, UIMax = 1000.0f))
		float CharacterMoveSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.2f, UIMax = 1.0f))
		float CharacterAtkSpeed;	
};
