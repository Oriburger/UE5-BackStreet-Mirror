// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

#define MAX_WEAPON_TYPE 6
#define MAX_PROJECTILE_TYPE 2

DECLARE_DELEGATE_OneParam(FDeleSpawnMissionItem, class AItemBase*);
DECLARE_DELEGATE_OneParam(FDelePickItem, AActor*);

UCLASS()
class BACKSTREET_API AItemBase : public AActor
{
	GENERATED_BODY()

// ------ DELEGATE, Static Member ---------------------------------------
public:
	FDelePickItem OnPlayerBeginPickUp;

	FDeleSpawnMissionItem Dele_MissionItemSpawned;

	UFUNCTION(BlueprintCallable)
		static int32 GetTargetItemKey(int32 Type, uint8 ItemID) { return Type * 1000 + ItemID; }

// ------ Global, Component ---------------------------------------------
public:
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		class USphereComponent* RootCollisionVolume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), BlueprintReadWrite)
		class USphereComponent* ItemTriggerVolume;

	UPROPERTY(EditDefaultsOnly)
		class UWidgetComponent* InfoWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UNiagaraComponent* ParticleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UProjectileMovementComponent* ProjectileMovement;

// ------ 기본 Info ---------------------------
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EItemCategoryInfo ItemType = EItemCategoryInfo::E_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		uint8 ItemID;
// ------ 아이템 기본 로직-------------------------------------
public:
	// 외부에서 Init하기위해 Call
	UFUNCTION()
		void InitItem(EItemCategoryInfo SetType, uint8 NewItemID = 0);

	//아이템 초기 효과를 출력하고 활성화 시킨다.
	UFUNCTION(BlueprintImplementableEvent)
		void ActivateItem();

	UFUNCTION()
		void OnOverlapBegins(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
			, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	//캐릭터가 Pick이벤트를 호출했다면
	UFUNCTION(BlueprintImplementableEvent)
		void OnItemPicked(AActor* Causer);

// ------ Projectile 로직 ------------------------------------
public:
	UFUNCTION()
		void SetLaunchDirection(FVector NewDirection);

	UFUNCTION(BlueprintCallable)
		void ActivateProjectileMovement();

// ------ Asset ----------------------------------------------
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Sound")
		class USoundCue* PickSound;

// ------ 참조 프로퍼티 ---------------------------------------------
private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};
