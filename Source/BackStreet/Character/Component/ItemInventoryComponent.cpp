// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInventoryComponent.h"
#include "WeaponComponentBase.h"
#include "../CharacterBase.h"
#include "../MainCharacter/MainCharacterBase.h"
#include "../../System/MapSystem/NewChapterManagerBase.h"
#include "../../Global/BackStreetGameInstance.h"
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

	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	if (OwnerCharacterRef.IsValid())
	{
		WeaponRef = Cast<UWeaponComponentBase>(OwnerCharacterRef.Get()->WeaponComponent);
	}
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameInstanceRef = GetWorld()->GetGameInstance<UBackStreetGameInstance>();
}

void UItemInventoryComponent::InitInventory()
{
	if (GamemodeRef.IsValid())
	{
		ItemTable = GamemodeRef.Get()->ItemInfoTable;
		if (!ItemTable)
		{
			UE_LOG(LogItem, Error, TEXT("UItemInventoryComponent::InitInventory() - DataTable is null!"));
			return;
		}
		//øµ±∏¿Á»≠ Save ∞¸∑√ ∑Œ¡˜
		if (IsValid(GamemodeRef.Get()->GetChapterManagerRef()))
		{
			UE_LOG(LogItem, Log, TEXT("UItemInventoryComponent::InitInventory() - on chapter cleared event bind"));
			//GamemodeRef.Get()->GetChapterManagerRef()->OnChapterCleared.AddDynamic(this, &UItemInventoryComponent::OnChapterCleared);
		}
		else
		{
			UE_LOG(LogItem, Error, TEXT("UItemInventoryComponent::InitInventory() - ChapterManager is not valid"));
		}
	}
	if (WeaponRef.IsValid())
	{
		WeaponRef.Get()->OnWeaponStateUpdated.AddDynamic(this, &UItemInventoryComponent::OnWeaponStateUpdated);
	}

	static const FString ContextString(TEXT("GENERAL"));
	TArray<FItemInfoDataStruct*> allRows;
	ItemTable->GetAllRows(ContextString, allRows);

	ItemMap.Empty();
	for (FItemInfoDataStruct* row : allRows)
	{
		if (row)
		{
			ItemMap.Add(row->ItemID, *row);
			ItemMap[row->ItemID].ItemAmount = 0;

			if (GameInstanceRef.IsValid())
			{
				if (GenesiumID == row->ItemID)
				{
					ItemMap[row->ItemID].ItemAmount = GameInstanceRef.Get()->GetGenesiumCount();
				}
				else if (FusionCellID == row->ItemID)
				{
					ItemMap[row->ItemID].ItemAmount = GameInstanceRef.Get()->GetFusionCellCount();
				}
			}
		}
	}
	
	
}

void UItemInventoryComponent::SetItemInventoryFromSaveData()
{
	//ItemMap = playerCharacterRef->SavedData.PlayerSaveGameData.ItemMap;
}

void UItemInventoryComponent::AddItem(int32 ItemID, uint8 ItemCnt)
{
	if (!ItemMap.Contains(ItemID)) return;
	if (ItemMap[ItemID].ItemType == EItemCategoryInfo::E_SubWeapon
		|| ItemMap[ItemID].ItemType == EItemCategoryInfo::E_Weapon)
	{
		TryAddWeapon(ItemID, ItemMap[ItemID].ItemType, ItemCnt);
	}
	else
	{
		OnItemAdded.Broadcast(ItemMap[ItemID], ItemCnt);
		ItemMap[ItemID].ItemAmount = FMath::Min(MAX_ITEM_COUNT_THRESHOLD, ItemMap[ItemID].ItemAmount + ItemCnt);
	}
	OnUpdateItem.Broadcast();
}

void UItemInventoryComponent::RemoveItem(int32 ItemID, uint8 ItemCnt)
{
	if (!ItemMap.Contains(ItemID)) return;

	ItemMap[ItemID].ItemAmount = FMath::Max(0, ItemMap[ItemID].ItemAmount - ItemCnt);

	if (ItemMap[ItemID].ItemType == EItemCategoryInfo::E_SubWeapon
		|| ItemMap[ItemID].ItemType == EItemCategoryInfo::E_Weapon)
	{
		TryRemoveWeapon(ItemID, ItemMap[ItemID].ItemType, ItemCnt);
	}
	
	OnUpdateItem.Broadcast();
	OnItemRemoved.Broadcast(ItemMap[ItemID], ItemMap[ItemID].ItemAmount);
}

void UItemInventoryComponent::OnChapterCleared()
{
	if (!GameInstanceRef.IsValid())
	{
		UE_LOG(LogItem, Error, TEXT("UItemInventoryComponent::OnChapterCleared() : GameInstanceRef is not valid"));
		return;
	}

	UE_LOG(LogItem, Log, TEXT("UItemInventoryComponent::OnChapterCleared()"));

	if (ItemMap.Contains(GenesiumID) && ItemMap[GenesiumID].ItemAmount > 0)
	{
		UE_LOG(LogItem, Log, TEXT("UItemInventoryComponent::OnChapterCleared() : Genesium saved %d"), ItemMap[GenesiumID].ItemAmount);
		GameInstanceRef.Get()->SetGenesiumCount(ItemMap[GenesiumID].ItemAmount);
	}
	if (ItemMap.Contains(FusionCellID) && ItemMap[FusionCellID].ItemAmount > 0)
	{
		UE_LOG(LogItem, Log, TEXT("UItemInventoryComponent::OnChapterCleared() : FusionCell saved %d"), ItemMap[FusionCellID].ItemAmount);
		GameInstanceRef.Get()->SetFusionCellCount(ItemMap[FusionCellID].ItemAmount);
	}
}

bool UItemInventoryComponent::TryAddWeapon(int32 ItemID, EItemCategoryInfo ItemCategory, int32 ItemCount)
{
	if (ItemCategory == EItemCategoryInfo::E_Weapon)
	{
		//¿”Ω√ ƒ⁄µÂ
		if (CurrMainWeaponCount >= MaxMainWeaponCount) return false;
		CurrMainWeaponCount += 1;
		ItemMap[ItemID].ItemAmount = FMath::Min(MAX_ITEM_COUNT_THRESHOLD, ItemMap[ItemID].ItemAmount + ItemCount);
	}
	else if (ItemCategory == EItemCategoryInfo::E_SubWeapon)
	{
		//∞°¥…«— √÷¥Î º“¡ˆºˆ∏¶ ≥—æ˙¥Ÿ∏È? 
		if (CurrSubWeaponCount >= MaxSubWeaponCount)
		{
			TArray<int32> subWeaponIDList = GetValidWeaponIDList(EWeaponType::E_Shoot);
			if (!subWeaponIDList.Contains(ItemID) && subWeaponIDList.Num() > 0)
			{
				//∏∏æ‡ ªı∑Œ ¡›¥¬ π´±‚∂Û∏È πˆ∏Æ∞Ì ∑Œ¡˜¿ª ºˆ«‡
				RemoveItem(subWeaponIDList[0], 99);
			}
		}
		//π´±‚∏¶ ¡›¥¬¥Ÿ (ƒ´øÓ∆√ / ¡§∫∏ ø¨µø)
		CurrSubWeaponCount += 1;
		FWeaponStateStruct& targetState = GetWeaponState(ItemID - 20000);
		int32 prevCount = targetState.RangedWeaponState.CurrentAmmoCount;
		targetState.RangedWeaponState.CurrentAmmoCount += ItemCount;
		
		//@@ TEMPORARY CODE @@@@@@@
		int32 maxAmmoCount = WeaponRef.Get()->GetWeaponStatInfoWithID(ItemID - 20000).RangedWeaponStat.MaxTotalAmmo;
		maxAmmoCount = maxAmmoCount * FMath::Max(1.0f, OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_SubWeaponCapacity));
		targetState.RangedWeaponState.UpdateAmmoValidation(maxAmmoCount);
		ItemCount = targetState.RangedWeaponState.CurrentAmmoCount - prevCount;


		if (ItemCount <= 0) return false;

		ItemMap[ItemID].ItemAmount = FMath::Min(MAX_ITEM_COUNT_THRESHOLD, ItemMap[ItemID].ItemAmount + ItemCount);
		TryUpdateWeaponState(ItemID - 20000, targetState);
		
		OnItemAdded.Broadcast(ItemMap[ItemID], ItemCount);
	}
	return true;
}

bool UItemInventoryComponent::TryRemoveWeapon(int32 ItemID, EItemCategoryInfo ItemCategory, int32 RemoveCount)
{
	if (ItemMap[ItemID].ItemType == EItemCategoryInfo::E_Weapon)
	{
		CurrMainWeaponCount = FMath::Max(0, CurrMainWeaponCount - 1);
	}
	else if (ItemMap[ItemID].ItemType == EItemCategoryInfo::E_SubWeapon)
	{
		//≈∫»Ø ºˆ∏∏≈≠ «ˆ¿Á ≈¨∑°Ω∫ ≥ª ¡§∫∏ø°º≠ ª©∞Ì, WeaponComponent ≥ª ¡§∫∏µµ æ˜µ•¿Ã∆Æ 
		FWeaponStateStruct& weaponStruct = GetWeaponState(ItemID - 20000);
		weaponStruct.RangedWeaponState.CurrentAmmoCount -= RemoveCount;
		weaponStruct.RangedWeaponState.CurrentAmmoCount = FMath::Max(0, weaponStruct.RangedWeaponState.CurrentAmmoCount);
		TryUpdateWeaponState(ItemID - 20000, weaponStruct);

		//≈∫»Ø¿Ã ¥Ÿ ∂≥æÓ¡≥¥Ÿ∏È ƒ´øÓ∆Æ ¡Ÿ¿Ã±‚
		if (weaponStruct.RangedWeaponState.CurrentAmmoCount == 0)
		{
			CurrSubWeaponCount = FMath::Max(0, CurrSubWeaponCount - 1);
		}
	}
	return true;
}

void UItemInventoryComponent::GetItemData(int32 ItemID, FItemInfoDataStruct& ItemData)
{
	if (ItemMap.Contains(ItemID))
	{
		ItemData = ItemMap[ItemID];
	}
}

TMap<ECraftingItemType, uint8> UItemInventoryComponent::GetAllCraftingItemAmount()
{
	TMap<ECraftingItemType, uint8> currItemAmountMap;
	TArray< ECraftingItemType> craftingItemTypeList = {ECraftingItemType::E_Screw, ECraftingItemType::E_Spring , ECraftingItemType::E_Gear , ECraftingItemType::E_Wrench };

	for (ECraftingItemType type : craftingItemTypeList)
	{
		int32 itemID = int32(type);
		if (ItemMap.Contains(itemID))
		{
			currItemAmountMap.Add(type, ItemMap[itemID].ItemAmount);
		}
		else
		{
			currItemAmountMap.Add(type, 0);
		}
	}
	return currItemAmountMap;
}

int32 UItemInventoryComponent::GetItemAmount(int32 ItemID)
{
<<<<<<< HEAD
	if (!ItemMap.Contains(ItemID)) return -1;
	return ItemMap[ItemID].ItemAmount;
=======
	if (!InventoryInfoData.ItemMap.Contains(ItemID)) return -1;
	return InventoryInfoData.ItemMap[ItemID].ItemAmount;
>>>>>>> 978addf43 ([fix] FORCEINLINE Ï†úÍ±∞ Î∞è ÎØ∏ÏÇ¨Ïö© Î°úÏßÅ Ï†úÍ±∞)
}

bool UItemInventoryComponent::GetIsItemEnough(int32 ItemID, uint8 NeedItemAmount)
{
	if (!ItemMap.Contains(ItemID)) return false;

	if(ItemMap[ItemID].ItemAmount<NeedItemAmount) return false;
	return true;
}

FItemInfoDataStruct UItemInventoryComponent::GetMainWeaponInfoData()
{
	if (CurrMainWeaponCount == 0) FItemInfoDataStruct();
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
	if(CurrSubWeaponCount == 0) FItemInfoDataStruct();
	TArray<int32> keyList;
	ItemMap.GenerateKeyArray(keyList);

	for (int32& key : keyList)
	{
		if (ItemMap[key].ItemType == EItemCategoryInfo::E_SubWeapon && ItemMap[key].ItemAmount > 0)
		{
			return ItemMap[key];
		}
	}
	return FItemInfoDataStruct();
}

void UItemInventoryComponent::OnWeaponStateUpdated(int32 WeaponID, FWeaponStatStruct WeaponStat, FWeaponStateStruct NewState)
{
	if (!WeaponStateMap.Contains(WeaponID)) WeaponStateMap.Add(WeaponID, FWeaponStateStruct());
	if (!ItemMap.Contains(WeaponID + 20000))
	{
		UE_LOG(LogItem, Error, TEXT("UItemInventoryComponent::OnWeaponStateUpdated %d is not found in itemmap"), WeaponID);
		return;
	}
	
	if (WeaponStat.WeaponType == EWeaponType::E_Shoot)
	{
		const int32 ammoVariance = NewState.RangedWeaponState.CurrentAmmoCount - ItemMap[WeaponID + 20000].ItemAmount;
		ItemMap[WeaponID + 20000].ItemAmount = NewState.RangedWeaponState.CurrentAmmoCount;
		if (ammoVariance > 0)
		{
			OnItemAdded.Broadcast(ItemMap[WeaponID + 20000], ammoVariance);
		}
		else if (ammoVariance <= 0)
		{
			OnItemRemoved.Broadcast(ItemMap[WeaponID + 20000], ItemMap[WeaponID + 20000].ItemAmount);
			if (ItemMap[WeaponID + 20000].ItemAmount == 0)
			{
				CurrSubWeaponCount = FMath::Max(0, CurrSubWeaponCount - 1);
			}
		}
	}
	WeaponStateMap[WeaponID] = NewState;
}

bool UItemInventoryComponent::TryUpdateWeaponState(int32 WeaponID, FWeaponStateStruct NewState)
{
	if (!WeaponRef.IsValid()) return false;
	return WeaponRef.Get()->UpdateWeaponStateCache(WeaponID, NewState);
}

TArray<int32> UItemInventoryComponent::GetValidWeaponIDList(EWeaponType WeaponType)
{
	TArray<int32> keyList, result;
	ItemMap.GenerateKeyArray(keyList);
	for (int32& key : keyList)
	{
		if (ItemMap[key].ItemType == EItemCategoryInfo::E_SubWeapon && ItemMap[key].ItemAmount > 0)
		{
			result.Add(key);
		}
	}
	return result;
}

FWeaponStateStruct& UItemInventoryComponent::GetWeaponState(int32 WeaponID)
{
	if (!WeaponStateMap.Contains(WeaponID))
	{
		WeaponStateMap.Add(WeaponID, FWeaponStateStruct());
	}
	return WeaponStateMap[WeaponID];
}


