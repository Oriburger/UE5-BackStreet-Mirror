// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "ItemInventoryComponent.generated.h"
#define MAX_CRAFTING_ITEM_IDX 3

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateUpdateItem);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UItemInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UItemInventoryComponent();

	UFUNCTION()
		void InitInventory();

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateUpdateItem OnUpdateItem;

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetItemData(int32 ItemID, FItemInfoDataStruct& ItemData);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ECraftingItemType, uint8> GetCraftingItemAmount();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		ECraftingItemType ConvertItemIDToCraftingItemType(int32 ItemID) { return static_cast<ECraftingItemType>(ItemID); }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 ConvertCraftingItemTypeToItemID(ECraftingItemType CraftingItemType){ return static_cast<int32>(CraftingItemType); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsItemEnough(int32 ItemID, uint8 NeedItemAmount);

//====== Property ===========================
private:
	UPROPERTY()
		UDataTable* ItemTable;

public:
	// Item List
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<int32, FItemInfoDataStruct> ItemMap;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

};
