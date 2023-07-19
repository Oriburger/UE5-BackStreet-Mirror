// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "GameFramework/Actor.h"
#include "ItemBoxBase.generated.h"
#define MAX_TRY_SPAWN_COUNT 25

DECLARE_DELEGATE_OneParam(FDeleAddMissionItem, class AItemBase*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleOpenItemBox, AActor*, Owner);

UCLASS()
class BACKSTREET_API AItemBoxBase : public AActor
{
	GENERATED_BODY()

// ----- Delegate ---------------------------
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDeleOpenItemBox OnPlayerOpenBegin;

	FDeleAddMissionItem OnMissionItemSpawned;

// ----- Global, Component ------------------
public:	
	AItemBoxBase();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditDefaultsOnly)
		class USphereComponent* OverlapVolume;

	UPROPERTY(EditDefaultsOnly)
		class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly)
		class UNiagaraComponent* ParticleComponent;

// ------ 기본 로직 -----------------------------
public:
	// 외부에서 Init하기위해 Call
	UFUNCTION()
		void InitItemBox(bool _bIncludeMissionItem);

	UFUNCTION()
		void OnItemBoxOpened(AActor* Causer);

	UFUNCTION()
		void OnOverlapBegins(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
			, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	//최대 MaxSpawnCount 만큼의 아이템을 확률에 기반하여 스폰하여 List 형태로 반환
	UFUNCTION()
		TArray<class AItemBase*> SpawnItems(int32 TargetSpawnCount);

	UFUNCTION()
		void LaunchItem(class AItemBase* TargetItem);

// ------ 기본 Property ---------------------------
private:
	UPROPERTY()
		bool bIncludeMissionItem = false;

protected:
	//최대로 스폰할 아이템 개수
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings", meta = (UIMin = 1, UIMax = 10))
		int32 MaxSpawnItemCount;

	//최소로 스폰할 아이템 개수
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings", meta = (UIMin = 1, UIMax = 10))
		int32 MinSpawnItemCount;

	//현재 아이템 박스가 스폰 가능한 아이템의 Category Type 리스트 : idx별로 아이템을 구분  / 미션 아이템은 0번에 필수로 포함!!
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings")
		TArray<EItemCategoryInfo> SpawnItemTypeList;

	//현재 아이템 박스가 스폰 가능한 아이템의 ID 리스트 : idx별로 아이템을 구분 / 미션 아이템은 0번에 필수로 포함!!
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings")
		TArray<int32> SpawnItemIDList;

	//각 아이템의 등장 확률를 담은 리스트 (0 ~ 100)  / 미션 아이템은 필수로 포함!!
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Settings")
		TArray<float> ItemSpawnProbabilityList;

	UFUNCTION()
		bool GetMissionItemIsIncluded() { return bIncludeMissionItem; }

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

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* OpenEffectSound;

// ----- Ref -----------------------------------
private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};
