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
		void InitInventory(FItemInventoryInfoStruct InitialData = FItemInventoryInfoStruct());

	//OnItemUpdated�� �����ٶ�@@ - 240905 @ljh
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
	UFUNCTION(BlueprintCallable)
		void AddItem(int32 ItemID, uint8 ItemCnt);

	UFUNCTION(BlueprintCallable)
		void RemoveItem(int32 ItemID, uint8 ItemCnt);

private:
	//���⸦ �߰��ϰ� �����Ѵ�. WeaponComponent���� ���� ������ �����Ѵ�
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
		int32 ConvertCraftingItemTypeToItemID(ECraftingItemType CraftingItemType) { return static_cast<int32>(CraftingItemType); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsItemEnough(int32 ItemID, uint8 NeedItemAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetMainWeaponInfoData();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetSubWeaponInfoData();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInventoryInfoStruct GetInventoryInfoData() { return InventoryInfoData; }
	
protected:
	UFUNCTION(BlueprintCallable)
		bool TryUpdateWeaponState(int32 WeaponID, FWeaponStateStruct NewState);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Property")
		FItemInventoryInfoStruct InventoryInfoData;

public:
	//Delegate �̺�Ʈ
	UFUNCTION()
		void OnWeaponStateUpdated(int32 WeaponID, FWeaponStatStruct WeaponStat, FWeaponStateStruct NewState);

private:
	//���� �κ��丮�� ��ȿ�� ���� ID���� ����Ʈ�� �ҷ��´�
	TArray<int32> GetValidWeaponIDList(EWeaponType WeaponType);	

	//WeaponComponent�κ��� �ҷ��´�
	FWeaponStateStruct& GetWeaponState(int32 WeaponID);

//===== Property ==========================
private:
	UDataTable* ItemTable;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	TWeakObjectPtr<class UBackStreetGameInstance> GameInstanceRef;

	TWeakObjectPtr<class UWeaponComponentBase> WeaponRef;

};