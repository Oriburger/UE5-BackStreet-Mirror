// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "GameFramework/Actor.h"
#include "WeaponInventoryBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleInventoryUpdate, const TArray<FInventoryItemInfoStruct>&, newInventoryInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDeleInventoryInfoUpdate, int32, inventoryIdx, bool, bIsCurrentWeapon, const FInventoryItemInfoStruct&, newInventoryInfo);

UCLASS()
class BACKSTREET_API AWeaponInventoryBase : public AActor
{
	GENERATED_BODY()
//------ Delegate ---------------------------------
public:
	//인벤토리가 업데이트 되었을 때 (추가, 정렬, 제거 등)
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDeleInventoryUpdate OnInventoryIsUpdated;

	//인벤토리 내 원소가 업데이트 되었을 때
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDeleInventoryInfoUpdate OnInventoryItemIsUpdated;

//------ Global ------------------------------------
public:
	AWeaponInventoryBase();

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//----- 인벤토리 핵심 로직-------------------------------
public: 
	//인벤토리를 초기화
	UFUNCTION()
		void InitInventory();

	//무기를 장착
	UFUNCTION()
		void EquipWeapon(int32 InventoryIdx, bool bIsNewWeapon = false);

	//무기 추가를 시도. 불가능하면 false를 반환
	UFUNCTION()
		bool AddWeapon(int32 NewWeaponID);

	//무기를 인벤토리로부터 제거
	UFUNCTION(BlueprintCallable)
		void RemoveWeapon(int32 WeaponID);

	//현재 장착하고 있는 Weapon Actor 정보를 제거한다.
	UFUNCTION()
		void RemoveCurrentWeapon();

	//다음 무기로 전환
	UFUNCTION()
		bool SwitchToNextWeapon();

	//현재 무기의 정보와 인벤토리 내 정보를 동기화
	UFUNCTION(BlueprintCallable)
		void SyncCurrentWeaponInfo(bool bIsLoadInfo);

	//해당 Weapon이 인벤토리에 포함이 되어있는지 반환
	UFUNCTION(BlueprintCallable)
		bool GetWeaponIsContained(int32 WeaponID);
	
	UFUNCTION(BlueprintCallable)
		bool TryAddAmmoToWeapon(int32 WeaponID, int32 AmmoCount);

protected:
	UFUNCTION()
		class AWeaponBase* SpawnWeaponActor(int32 WeaponID);

	//현재 무기를 인벤토리로 보낸다.
	UFUNCTION()
		void RestoreCurrentWeapon();

	//인벤토리를 비움
	UFUNCTION()
		void ClearInventory();

	//인벤토리를 Weight를 기준으로 오름차순 정렬
	UFUNCTION()
		void SortInventory();

	//해당 Weapon이 인벤토리에 포함이 되어있는지 반환
	UFUNCTION()
		int32 GetWeaponInventoryIdx(int32 WeaponID);

	//인벤토리에 TargetWeaponID라는 ID를 가진 Weapon이 있는지 체크.
	//중복되는게 있다면 해당 인벤토리의 Idx를 반환
	UFUNCTION()
		int32 CheckWeaponDuplicate(int32 TargetWeaponID);

//------ 프로퍼티 관련 ----------------------------------
public:
	//현재 선택된 인벤토리 Idx를 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentIdx() {  return CurrentIdx;  }

	//현재 무기 개수를 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentWeaponCount() {  return CurrentWeaponCount = InventoryArray.Num(); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentCapacity() { return CurrentCapacity; }

	UFUNCTION()
		class AWeaponBase* GetCurrentWeaponRef();

	UFUNCTION()
		FInventoryItemInfoStruct GetCurrentWeaponInfo();

private:
	UFUNCTION()
		int32 GetNextInventoryIdx();

	UFUNCTION()
		void SetCurrentWeaponRef(class AWeaponBase* NewWeapon);

	UFUNCTION()
		void SetCurrentIdx(int32 newValue) { CurrentIdx = newValue; };

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", meta = (UIMin = 1, UIMax = 10))
		int32 MaxCapacity = 6;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", meta = (UIMin = 1, UIMax = 10))
		int32 MaxWeaponCount = 6;

	//Weapon ID 배열, BP에서 지정하고 Idx로 구분
	UPROPERTY(EditAnywhere, Category = "Gameplay|Class")
		TArray<int32> WeaponIDList;

	//Weapon Class 배열, BP에서 지정하고 Idx로 구분
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Class")
		TArray<TSubclassOf<class AWeaponBase> > WeaponClassList;


private:
	UPROPERTY()
		TArray<FInventoryItemInfoStruct> InventoryArray;

	UPROPERTY()
		int32 CurrentIdx = 0; 

	UPROPERTY()
		int32 CurrentCapacity = 0;

	UPROPERTY()
		int32 CurrentWeaponCount = 0;

	UPROPERTY()
		TMap<int32, UClass*> WeaponClassInfoMap;


private: 
	//게임모드 Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//인벤토리 소유자 플레이어 
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//현재 장비하고 있는 WeaponRef
	TWeakObjectPtr<class AWeaponBase> CurrentWeaponRef;
};
