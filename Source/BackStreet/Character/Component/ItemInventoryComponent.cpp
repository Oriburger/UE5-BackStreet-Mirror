// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInventoryComponent.h"
#include "../CharacterBase.h"
#include "../MainCharacter/MainCharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#define MAX_ITEM_COUNT_THRESHOLD 99

// Sets default values for this component's properties
UItemInventoryComponent::UItemInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UItemInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UItemInventoryComponent::InitInventory()
{
	UE_LOG(LogTemp, Warning, TEXT(""))
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GamemodeRef.IsValid())
	{
		ItemTable = GamemodeRef.Get()->ItemInfoTable;
		if (!ItemTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("DataTable is null!"));
			return;
		}
	}

	static const FString ContextString(TEXT("GENERAL"));
	TArray<FItemInfoDataStruct*> allRows;
	ItemTable->GetAllRows(ContextString, allRows);

	for (FItemInfoDataStruct* row : allRows)
	{
		if (row)
		{
			ItemMap.Add(row->ItemID, *row);
		}
	}
}

void UItemInventoryComponent::SetItemInventoryFromSaveData()
{
	AMainCharacterBase* playerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	//ItemMap = playerCharacterRef->SavedData.PlayerSaveGameData.ItemMap;
}

void UItemInventoryComponent::AddItem(int32 ItemID, uint8 ItemCnt)
{
	if (!ItemMap.Contains(ItemID)) return;
	if (ItemMap[ItemID].ItemType == EItemCategoryInfo::E_SubWeapon)
	{
		if (CurrSubWeaponCount >= MaxSubWeaponCount) return;
		CurrSubWeaponCount += 1;
	}
	else if (ItemMap[ItemID].ItemType == EItemCategoryInfo::E_Weapon)
	{
		if (CurrMainWeaponCount >= MaxMainWeaponCount) return;
		CurrMainWeaponCount += 1;
	}
	ItemMap[ItemID].ItemAmount = FMath::Min(MAX_ITEM_COUNT_THRESHOLD, ItemMap[ItemID].ItemAmount + ItemCnt);
	OnUpdateItem.Broadcast();
	OnItemAdded.Broadcast(ItemMap[ItemID], ItemCnt);
}

void UItemInventoryComponent::RemoveItem(int32 ItemID, uint8 ItemCnt)
{
	if (!ItemMap.Contains(ItemID)) return;
	if (ItemMap[ItemID].ItemType == EItemCategoryInfo::E_SubWeapon)
	{
		CurrSubWeaponCount = FMath::Max(0, CurrSubWeaponCount - 1);
	}
	else if (ItemMap[ItemID].ItemType == EItemCategoryInfo::E_Weapon)
	{
		CurrMainWeaponCount = FMath::Max(0, CurrMainWeaponCount - 1);
	}
	ItemMap[ItemID].ItemAmount = FMath::Max(0, ItemMap[ItemID].ItemAmount - ItemCnt);
	OnUpdateItem.Broadcast();
	OnItemRemoved.Broadcast(ItemMap[ItemID], ItemMap[ItemID].ItemAmount);
}

void UItemInventoryComponent::GetItemData(int32 ItemID, FItemInfoDataStruct& ItemData)
{
	if (ItemMap.Contains(ItemID))
	{
		ItemData = ItemMap[ItemID];
	}
}

TMap<ECraftingItemType, uint8> UItemInventoryComponent::GetCraftingItemAmount()
{
	TMap<ECraftingItemType, uint8> currItemAmountMap;
	TArray< ECraftingItemType> craftingItemTypeList = {ECraftingItemType::E_Screw, ECraftingItemType::E_Spring , ECraftingItemType::E_Gear , ECraftingItemType::E_Wrench };

	for (ECraftingItemType type : craftingItemTypeList)
	{
		int32 itemID = ConvertCraftingItemTypeToItemID(type);
		if (ItemMap.Contains(itemID))
		{
			uint8 itemAmount = ItemMap[itemID].ItemAmount;
			currItemAmountMap.Add(type, itemAmount);
		}
		else
		{
			currItemAmountMap.Add(type, 0);
		}
	}
	return currItemAmountMap;
}

bool UItemInventoryComponent::GetIsItemEnough(int32 ItemID, uint8 NeedItemAmount)
{
	if (!ItemMap.Contains(ItemID)) return false;

	if(ItemMap[ItemID].ItemAmount<NeedItemAmount) return false;
	return true;
}

FItemInfoDataStruct UItemInventoryComponent::GetMainWeaponInfoData()
{
	TArray<int32> keyList;
	ItemMap.GenerateKeyArray(keyList);

	for (int32& key : keyList)
	{
		if (ItemMap[key].ItemType == EItemCategoryInfo::E_Weapon && ItemMap[key].ItemAmount > 0)
			return ItemMap[key];
	}

	return FItemInfoDataStruct();
}

FItemInfoDataStruct UItemInventoryComponent::GetSubWeaponInfoData()
{
	TArray<int32> keyList;
	ItemMap.GenerateKeyArray(keyList);

	for (int32& key : keyList)
	{
		if (ItemMap[key].ItemType == EItemCategoryInfo::E_SubWeapon && ItemMap[key].ItemAmount > 0)
			return ItemMap[key];
	}

	return FItemInfoDataStruct();
}


