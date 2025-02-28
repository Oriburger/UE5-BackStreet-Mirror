// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "ItemInventoryComponent.generated.h"
#define MAX_CRAFTING_ITEM_IDX 3

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateUpdateItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateAddItem, const FItemInfoDataStruct&, NewItemInfo, const int32, AddCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateRemoveItem, const FItemInfoDataStruct&, NewItemInfo, const int32, LeftCount);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UItemInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UItemInventoryComponent();

	UFUNCTION()
		void InitInventory();

	//OnItemUpdated로 수정바람@@ - 240905 @ljh
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateUpdateItem OnUpdateItem;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAddItem OnItemAdded; 

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateRemoveItem OnItemRemoved;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======= Logic ============================
public:
	//When SavedData already has Inventory data
	UFUNCTION()
		void SetItemInventoryFromSaveData();

public:
	UFUNCTION(BlueprintCallable)
		void AddItem(int32 ItemID, uint8 ItemCnt);

	UFUNCTION(BlueprintCallable)
		void RemoveItem(int32 ItemID, uint8 ItemCnt);

private:
	//무기를 추가하고 제거한다. WeaponComponent와의 정보 연동을 진행한다
	bool TryAddWeapon(int32 ItemID, EItemCategoryInfo ItemCategory, int32 ItemCount);
	bool TryRemoveWeapon(int32 ItemID, EItemCategoryInfo ItemCategory, int32 RemoveCount);

//======= Getter =======================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetItemData(int32 ItemID, FItemInfoDataStruct& ItemData);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ECraftingItemType, uint8> GetAllCraftingItemAmount();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetItemAmount(int32 ItemID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		ECraftingItemType ConvertItemIDToCraftingItemType(int32 ItemID) { return static_cast<ECraftingItemType>(ItemID); }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 ConvertCraftingItemTypeToItemID(ECraftingItemType CraftingItemType){ return static_cast<int32>(CraftingItemType); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsItemEnough(int32 ItemID, uint8 NeedItemAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetMainWeaponInfoData();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetSubWeaponInfoData();
	
//====== Weapon ===========================
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxItemCount = 100; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxSubWeaponCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxMainWeaponCount = 1;

protected:
	UFUNCTION(BlueprintCallable)
		bool TryUpdateWeaponState(int32 WeaponID, FWeaponStateStruct NewState);

public:
	//Delegate 이벤트
	UFUNCTION()
		void OnWeaponStateUpdated(int32 WeaponID, FWeaponStatStruct WeaponStat, FWeaponStateStruct NewState);

private:
	//현재 인벤토리의 유효한 무기 ID들을 리스트로 불러온다
	TArray<int32> GetValidWeaponIDList(EWeaponType WeaponType);	

	//WeaponComponent로부터 불러온다
	FWeaponStateStruct& GetWeaponState(int32 WeaponID);

	TMap<int32, FWeaponStateStruct> WeaponStateMap;

	int32 CurrSubWeaponCount = 0;

	int32 CurrMainWeaponCount = 0;

//===== Property ==========================
private:
	UDataTable* ItemTable;

public:
	// Item List
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<int32, FItemInfoDataStruct> ItemMap;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	TWeakObjectPtr<class UWeaponComponentBase> WeaponRef;

};
