// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "WeaponInventoryBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateTutorialWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleInventoryUpdate, const TArray<FInventoryItemInfoStruct>&, newInventoryInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDeleInventoryInfoUpdate, int32, inventoryIdx, bool, bIsCurrentWeapon, const FInventoryItemInfoStruct&, newInventoryInfo);

UCLASS()
class BACKSTREET_API AWeaponInventoryBase : public AActor
{
	GENERATED_BODY()

//------ Delegate ---------------------------------
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTutorialWeapon OnAddWeapon;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTutorialWeapon OnSwapWeapon;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTutorialWeapon OnDropWeapon;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTutorialWeapon OnWeaponIsRemoved;

	//인벤토리가 업데이트 되었을 때 (추가, 정렬, 제거 등)
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDeleInventoryUpdate OnInventoryIsUpdated;

	//인벤토리 내 원소가 업데이트 되었을 때
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDeleInventoryInfoUpdate OnInventoryItemIsUpdated;

//------ Global ------------------------------------
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void PrintDebugMessage(); 

	AWeaponInventoryBase();

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//----- 필수 프로퍼티과 관련 함수 (반드시 BP에서 지정할 것)------
protected:
	//EWeaponType과 인덱스를 맞춰줄 것 / 0은 비워두기
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Basic")
		TArray<TSubclassOf<AWeaponBase>> WeaponClassList;

	//무기 스탯 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* WeaponStatInfoTable;

private:
	UFUNCTION()
		FWeaponStatStruct GetWeaponStatInfoWithID(int32 TargetWeaponID);

public: 
	UFUNCTION(BlueprintCallable, BlueprintPure)
		EWeaponType GetWeaponType(int32 TargetWeaponID);

//----- 인벤토리 핵심 로직-------------------------------
public: 
	//인벤토리를 초기화
	UFUNCTION()
		void InitInventory(int32 NewMaxCapacity = 1);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FInventoryItemInfoStruct> GetInventoryArray() { return InventoryArray; }

	//무기를 장착
	UFUNCTION()
		void EquipWeapon(int32 NewWeaponID);

	//새로운 인덱스로 전환
	UFUNCTION(BlueprintCallable)
		bool EquipWeaponByIdx(int32 NewIdx);

	//무기 추가를 시도. 불가능하면 false를 반환
	UFUNCTION(BlueprintCallable)
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

	//현재 무기의 정보와 인벤토리 내 정보를 동기화 (bIsLoadInfo : 액터 정보를 블러올 것인지?)
	UFUNCTION(BlueprintCallable)
		void SyncCurrentWeaponInfo(bool bIsLoadActorInfo = false);

	//해당 Weapon이 인벤토리에 포함이 되어있는지 반환
	UFUNCTION(BlueprintCallable)
		bool GetWeaponIsContained(int32 WeaponID);
	
	//현재 인벤토리가 선택이 되어있는지?
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsFocused();

	UFUNCTION(BlueprintCallable)
		bool TryAddAmmoToWeapon(int32 WeaponID, int32 AmmoCount);

protected:
	//인벤토리를 비움
	UFUNCTION()
		void ClearInventory();

	//인벤토리를 Weight를 기준으로 오름차순 정렬
	UFUNCTION()
		void SortInventory();

	//해당 Weapon이 인벤토리에 포함이 되어있는지 반환
	UFUNCTION(BlueprintCallable)
		int32 GetWeaponInventoryIdx(int32 WeaponID);

	//인벤토리에 TargetWeaponID라는 ID를 가진 Weapon이 있는지 체크.
	//중복되는게 있다면 해당 인벤토리의 Idx를 반환
	UFUNCTION()
		int32 CheckWeaponDuplicate(int32 TargetWeaponID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsEqualWeaponType(EWeaponType TypeA, EWeaponType TypeB);

//------ 프로퍼티 관련 ----------------------------------
public:
	UFUNCTION(BlueprintCallable)
		bool SetCurrentIdx(int32 NewIdx);

	//현재 선택된 인벤토리 Idx를 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentIdx() {  return CurrentIdx;  }

	//현재 무기 개수를 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentWeaponCount() {  return InventoryArray.Num(); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentCapacity() { return TotalWeight; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsInventoryEmpty() { return TotalWeight == 0; }

	UFUNCTION()
		class AWeaponBase* GetCurrentWeaponRef();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FInventoryItemInfoStruct GetCurrentWeaponInfo();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FInventoryItemInfoStruct GetWeaponInfoByID(int32 WeaponID);

private:
	UFUNCTION()
		int32 GetNextInventoryIdx();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", meta = (UIMin = 1, UIMax = 10))
		int32 MaxCapacity = 6;
	
private:
	UPROPERTY()
		TArray<FInventoryItemInfoStruct> InventoryArray;

	UPROPERTY()
		int32 CurrentIdx = 0; 

	UPROPERTY()
		int32 TotalWeight = 0;

//---- 그 외 Ref Ptr------------------------------
private: 
	//게임모드 Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//인벤토리 소유자 플레이어 
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};
