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

// ------ �⺻ Info ---------------------------
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay")
		FItemInfoDataStruct ItemInfo;

// ------ ������ �⺻ ����-------------------------------------
public:	
	// �ܺο��� Init�ϱ����� Call
	UFUNCTION(BlueprintCallable)
		void InitItem(int32 NewItemID, FItemInfoDataStruct InfoOverride = FItemInfoDataStruct());

	//������ �ʱ� ȿ���� ����ϰ� Ȱ��ȭ ��Ų��. (Ÿ�Ӷ��� Ȱ��)
	UFUNCTION(BlueprintImplementableEvent)
		void ActivateItem();

	UFUNCTION()
		void OnOverlapBegins(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
			, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	//ĳ���Ͱ� Pick�̺�Ʈ�� ȣ���ߴٸ�
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnItemPicked();

protected:
	UFUNCTION()
		void InitializeItemMesh();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetItemInfoWithID(const int32 ItemID);

// ------ Projectile ���� ------------------------------------
public:
	UFUNCTION()
		void SetLaunchDirection(FVector NewDirection);

	UFUNCTION(BlueprintCallable)
		void ActivateProjectileMovement();

// ------ Asset ----------------------------------------------
protected:
	//������ ������ ���̺� (���� ���� ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		UDataTable* ItemDataInfoTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		UNiagaraSystem* ItemPickEffect;

// ------ ���� ������Ƽ ---------------------------------------------
protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AMainCharacterBase* GetPlayerRef() { return PlayerRef.Get(); }

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
	
	TWeakObjectPtr<class AMainCharacterBase> PlayerRef;

	TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;
};
