// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Global/BackStreet.h"
#include "ItemBase.h"
#include "GameFramework/Actor.h"
#include "ItemBoxBase.generated.h"
#define MAX_TRY_SPAWN_COUNT 25

UCLASS()
class BACKSTREET_API AItemBoxBase : public AItemBase
{
	GENERATED_BODY()

// ----- Global, Component ------------------
public:	
	AItemBoxBase();

protected:
	virtual void BeginPlay() override;

// ------ 기본 로직 -----------------------------
private:
	//최대 MaxSpawnCount 만큼의 아이템을 확률에 기반하여 스폰하여 List 형태로 반환
	UFUNCTION()
		TArray<class AItemBase*> SpawnItems(int32 TargetSpawnCount);

	UFUNCTION()
		void LaunchItem(class AItemBase* TargetItem);

// ------ 기본 Property ---------------------------
protected:
	//최대로 스폰할 아이템 개수
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings", meta = (UIMin = 1, UIMax = 10))
		int32 MaxSpawnItemCount;

	//최소로 스폰할 아이템 개수
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings", meta = (UIMin = 1, UIMax = 10))
		int32 MinSpawnItemCount;

	//현재 아이템 박스가 스폰 가능한 아이템의 ID 리스트 : idx별로 아이템을 구분 / 미션 아이템은 0번에 필수로 포함!!
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings")
		TArray<int32> SpawnItemIDList;

	//각 아이템의 등장 확률를 담은 리스트 (0 ~ 100)  / 미션 아이템은 필수로 포함!!
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings")
		TArray<float> ItemSpawnProbabilityList;

// ----- Item Launch Option --------------------------
protected:
	UPROPERTY(EditAnywhere, Category = "Gameplay|Settings|Launch")
		float CornHalfRadius = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Gameplay|Settings|Launch")
		float LaunchSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Gameplay|Settings|Launch")
		float SpawnLocationInterval = 15.0f;

// ----- Asset -----------------------------------
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|VFX")
		class UParticleSystem* OpenEffectParticle;
};
