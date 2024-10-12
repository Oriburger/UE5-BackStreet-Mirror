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

// ------ �⺻ ���� -----------------------------
private:
	//�ִ� MaxSpawnCount ��ŭ�� �������� Ȯ���� ����Ͽ� �����Ͽ� List ���·� ��ȯ
	UFUNCTION()
		TArray<class AItemBase*> SpawnItems(int32 TargetSpawnCount);

	UFUNCTION()
		void LaunchItem(class AItemBase* TargetItem);

// ------ �⺻ Property ---------------------------
protected:
	//�ִ�� ������ ������ ����
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings", meta = (UIMin = 1, UIMax = 10))
		int32 MaxSpawnItemCount;

	//�ּҷ� ������ ������ ����
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings", meta = (UIMin = 1, UIMax = 10))
		int32 MinSpawnItemCount;

	//���� ������ �ڽ��� ���� ������ �������� ID ����Ʈ : idx���� �������� ���� / �̼� �������� 0���� �ʼ��� ����!!
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings")
		TArray<int32> SpawnItemIDList;

	//�� �������� ���� Ȯ���� ���� ����Ʈ (0 ~ 100)  / �̼� �������� �ʼ��� ����!!
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
