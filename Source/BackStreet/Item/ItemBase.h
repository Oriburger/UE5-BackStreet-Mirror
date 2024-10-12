// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

#define MAX_WEAPON_TYPE 6
#define MAX_PROJECTILE_TYPE 2

DECLARE_DELEGATE_OneParam(FDelePickItem, AActor*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateItemInitialized, FItemInfoDataStruct, ItemInfo);

UCLASS()
class BACKSTREET_API AItemBase : public AActor
{
	GENERATED_BODY()

// ------ DELEGATE, Static Member ---------------------------------------
public:
	FDelePickItem OnPlayerBeginPickUp;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateItemInitialized OnItemInitialized;

// ------ Global, Component ---------------------------------------------
public:
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY()
		class USceneComponent* DefaultSceneRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UStaticMeshComponent* OutlineMeshComponent;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), BlueprintReadWrite)
		class UInteractiveCollisionComponent* ItemTriggerVolume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UWidgetComponent* IconWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UNiagaraComponent* ParticleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UProjectileMovementComponent* ProjectileMovement;

// ------ 기본 Info ---------------------------
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay")
		FItemInfoDataStruct ItemInfo;

// ------ 아이템 기본 로직-------------------------------------
public:	
	// 외부에서 Init하기위해 Call
	UFUNCTION(BlueprintCallable)
		void InitItem(int32 NewItemID, FItemInfoDataStruct InfoOverride = FItemInfoDataStruct());

	//아이템 초기 효과를 출력하고 활성화 시킨다. (타임라인 활용)
	UFUNCTION(BlueprintImplementableEvent)
		void ActivateItem();

	UFUNCTION()
		void OnOverlapBegins(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
			, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	//캐릭터가 Pick이벤트를 호출했다면
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnItemPicked();

protected:
	UFUNCTION()
		void InitializeItemMesh();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetItemInfoWithID(const int32 ItemID);

// ------ Projectile 로직 ------------------------------------
public:
	UFUNCTION()
		void SetLaunchDirection(FVector NewDirection);

	UFUNCTION(BlueprintCallable)
		void ActivateProjectileMovement();

// ------ Asset ----------------------------------------------
protected:
	//아이템 데이터 테이블 (에셋 정보 포함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		UDataTable* ItemDataInfoTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		UNiagaraSystem* ItemPickEffect;

// ------ 참조 프로퍼티 ---------------------------------------------
protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AMainCharacterBase* GetPlayerRef() { return PlayerRef.Get(); }

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
	
	TWeakObjectPtr<class AMainCharacterBase> PlayerRef;

	TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;
};
