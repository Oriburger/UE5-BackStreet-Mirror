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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		EItemCategoryInfo ItemType = EItemCategoryInfo::E_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		FItemInfoStruct ItemInfo;

// ------ 아이템 기본 로직-------------------------------------
public:	
	// 외부에서 Init하기위해 Call
	UFUNCTION(BlueprintCallable)
		void InitItem(FItemInfoStruct NewItemInfo);

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

protected:
	UFUNCTION()
		void InitializeItemMesh();

// ------ Projectile 로직 ------------------------------------
public:
	UFUNCTION()
		void SetLaunchDirection(FVector NewDirection);

	UFUNCTION(BlueprintCallable)
		void ActivateProjectileMovement();

// ------ Asset ----------------------------------------------
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		class USoundCue* PickSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		TSoftObjectPtr<UStaticMesh> MainMeshAsset;

	//아이템 데이터 테이블 (에셋 정보 포함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Asset|Data")
		UDataTable* ItemDataInfoTable;

// ------ 참조 프로퍼티 ---------------------------------------------
private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};
