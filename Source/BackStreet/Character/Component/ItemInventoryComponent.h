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

	//OnItemUpdated∑Œ ºˆ¡§πŸ∂˜@@ - 240905 @ljh
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

protected:
	UFUNCTION()
		void OnChapterCleared();

private:
	//π´±‚∏¶ √ﬂ∞°«œ∞Ì ¡¶∞≈«—¥Ÿ. WeaponComponentøÕ¿« ¡§∫∏ ø¨µø¿ª ¡¯«‡«—¥Ÿ
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
<<<<<<< HEAD
=======
		ECraftingItemType ConvertItemIDToCraftingItemType(int32 ItemID) { return static_cast<ECraftingItemType>(ItemID); }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 ConvertCraftingItemTypeToItemID(ECraftingItemType CraftingItemType){ return static_cast<int32>(CraftingItemType); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
>>>>>>> 978addf43 ([fix] FORCEINLINE Ï†úÍ±∞ Î∞è ÎØ∏ÏÇ¨Ïö© Î°úÏßÅ Ï†úÍ±∞)
		bool GetIsItemEnough(int32 ItemID, uint8 NeedItemAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetMainWeaponInfoData();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetSubWeaponInfoData();
	
//====== PROPERTY ===========================
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxItemCount = 100; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxSubWeaponCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxMainWeaponCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Wealth")
		int32 GenesiumID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Wealth")
		int32 FusionCellID;

protected:
	UFUNCTION(BlueprintCallable)
		bool TryUpdateWeaponState(int32 WeaponID, FWeaponStateStruct NewState);

public:
	//Delegate ¿Ã∫•∆Æ
	UFUNCTION()
		void OnWeaponStateUpdated(int32 WeaponID, FWeaponStatStruct WeaponStat, FWeaponStateStruct NewState);

private:
	//«ˆ¿Á ¿Œ∫•≈‰∏Æ¿« ¿Ø»ø«— π´±‚ IDµÈ¿ª ∏ÆΩ∫∆Æ∑Œ ∫“∑Øø¬¥Ÿ
	TArray<int32> GetValidWeaponIDList(EWeaponType WeaponType);	

	//WeaponComponent∑Œ∫Œ≈Õ ∫“∑Øø¬¥Ÿ
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

	TWeakObjectPtr<class UBackStreetGameInstance> GameInstanceRef;

	TWeakObjectPtr<class UWeaponComponentBase> WeaponRef;

};
