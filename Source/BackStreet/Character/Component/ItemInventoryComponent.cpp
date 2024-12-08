// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInventoryComponent.h"
#include "WeaponComponentBase.h"
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

	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	if (OwnerCharacterRef.IsValid())
	{
		WeaponRef = Cast<UWeaponComponentBase>(OwnerCharacterRef.Get()->WeaponComponent);
	}
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}

void UItemInventoryComponent::InitInventory()
{
	if (GamemodeRef.IsValid())
	{
		ItemTable = GamemodeRef.Get()->ItemInfoTable;
		if (!ItemTable)
		{
			UE_LOG(LogTemp, Error, TEXT("DataTable is null!"));
			return;
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

bool UItemInventoryComponent::TryAddWeapon(int32 ItemID, EItemCategoryInfo ItemCategory, int32 ItemCount)
{
	if (ItemCategory == EItemCategoryInfo::E_Weapon)
	{
		//임시 코드
		if (CurrMainWeaponCount >= MaxMainWeaponCount) return false;
		CurrMainWeaponCount += 1;
		ItemMap[ItemID].ItemAmount = FMath::Min(MAX_ITEM_COUNT_THRESHOLD, ItemMap[ItemID].ItemAmount + ItemCount);
	}
	else if (ItemCategory == EItemCategoryInfo::E_SubWeapon)
	{
		//가능한 최대 소지수를 넘었다면? 
		if (CurrSubWeaponCount >= MaxSubWeaponCount)
		{
			TArray<int32> subWeaponIDList = GetValidWeaponIDList(EWeaponType::E_Shoot);
			if (!subWeaponIDList.Contains(ItemID) && subWeaponIDList.Num() > 0)
			{
				//만약 새로 줍는 무기라면 버리고 로직을 수행
				RemoveItem(subWeaponIDList[0], 99);
			}
		}
		//무기를 줍는다 (카운팅 / 정보 연동)
		CurrSubWeaponCount += 1;
		FWeaponStateStruct& targetState = GetWeaponState(ItemID - 20000);
		int32 prevCount = targetState.RangedWeaponState.CurrentAmmoCount;
		targetState.RangedWeaponState.CurrentAmmoCount += ItemCount;
		
		//@@ TEMPORARY CODE @@@@@@@
		int32 maxAmmoCount = WeaponRef.Get()->GetWeaponStatInfoWithID(ItemID - 20000).RangedWeaponStat.MaxTotalAmmo;
		maxAmmoCount = maxAmmoCount * (1.0f + OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_SubWeaponCapacity));
		targetState.RangedWeaponState.UpdateAmmoValidation(maxAmmoCount);
		ItemCount = targetState.RangedWeaponState.CurrentAmmoCount - prevCount;

		UE_LOG(LogTemp, Warning, TEXT("UItemInventoryComponent::TryAddWeapon maxAmmo : %d,  added : %d, after : %d"), maxAmmoCount, ItemCount, targetState.RangedWeaponState.CurrentAmmoCount);

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
		//탄환 수만큼 현재 클래스 내 정보에서 빼고, WeaponComponent 내 정보도 업데이트 
		FWeaponStateStruct& weaponStruct = GetWeaponState(ItemID - 20000);
		weaponStruct.RangedWeaponState.CurrentAmmoCount -= RemoveCount;
		weaponStruct.RangedWeaponState.CurrentAmmoCount = FMath::Max(0, weaponStruct.RangedWeaponState.CurrentAmmoCount);
		TryUpdateWeaponState(ItemID - 20000, weaponStruct);

		//탄환이 다 떨어졌다면 카운트 줄이기
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
		int32 itemID = ConvertCraftingItemTypeToItemID(type);
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
	if (!ItemMap.Contains(ItemID)) return -1;
	return ItemMap[ItemID].ItemAmount;
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

void UItemInventoryComponent::OnWeaponStateUpdated(int32 WeaponID, EWeaponType WeaponType, FWeaponStateStruct NewState)
{
	if (!WeaponStateMap.Contains(WeaponID)) WeaponStateMap.Add(WeaponID, FWeaponStateStruct());
	if (!ItemMap.Contains(WeaponID + 20000))
	{
		UE_LOG(LogTemp, Error, TEXT("UItemInventoryComponent::OnWeaponStateUpdated %d is not found in itemmap"), WeaponID);
		return;
	}
	
	if (WeaponType == EWeaponType::E_Shoot)
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


