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

void UItemInventoryComponent::InitInventory(FItemInventoryInfoStruct InitialData)
{
	if (GamemodeRef.IsValid())
	{
		ItemTable = GamemodeRef.Get()->ItemInfoTable;
		if (!ItemTable)
		{
			UE_LOG(LogItem, Error, TEXT("UItemInventoryComponent::InitInventory() - DataTable is null!"));
			return;
		}
	}
	if (!WeaponRef.IsExplicitlyNull() && WeaponRef.IsValid())
	{
		WeaponRef.Get()->OnWeaponStateUpdated.AddDynamic(this, &UItemInventoryComponent::OnWeaponStateUpdated);
	}

	if (InitialData.ItemMap.Num() > 0)
	{
		InventoryInfoData = InitialData;
	}
	else
	{
		static const FString ContextString(TEXT("GENERAL"));
		TArray<FItemInfoDataStruct*> allRows;
		ItemTable->GetAllRows(ContextString, allRows);

		InventoryInfoData.ItemMap.Empty();

		for (FItemInfoDataStruct* row : allRows)
		{
			if (row)
			{
				InventoryInfoData.ItemMap.Add(row->ItemID, *row);
				InventoryInfoData.ItemMap[row->ItemID].ItemAmount = 0;
			}
		}
	}
}

void UItemInventoryComponent::AddItem(int32 ItemID, uint8 ItemCnt)
{
	if (!InventoryInfoData.ItemMap.Contains(ItemID)) return;
	if (InventoryInfoData.ItemMap[ItemID].ItemType == EItemCategoryInfo::E_SubWeapon
		|| InventoryInfoData.ItemMap[ItemID].ItemType == EItemCategoryInfo::E_Weapon)
	{
		TryAddWeapon(ItemID, InventoryInfoData.ItemMap[ItemID].ItemType, ItemCnt);
	}
	else
	{
		OnItemAdded.Broadcast(InventoryInfoData.ItemMap[ItemID], ItemCnt);
		InventoryInfoData.ItemMap[ItemID].ItemAmount = FMath::Min(MAX_ITEM_COUNT_THRESHOLD, InventoryInfoData.ItemMap[ItemID].ItemAmount + ItemCnt);
	}

	OnItemUpdated.Broadcast();
}

void UItemInventoryComponent::RemoveItem(int32 ItemID, uint8 ItemCnt)
{
	if (!InventoryInfoData.ItemMap.Contains(ItemID)) return;

	InventoryInfoData.ItemMap[ItemID].ItemAmount = FMath::Max(0, InventoryInfoData.ItemMap[ItemID].ItemAmount - ItemCnt);

	if (InventoryInfoData.ItemMap[ItemID].ItemType == EItemCategoryInfo::E_SubWeapon
		|| InventoryInfoData.ItemMap[ItemID].ItemType == EItemCategoryInfo::E_Weapon)
	{
		TryRemoveWeapon(ItemID, InventoryInfoData.ItemMap[ItemID].ItemType, ItemCnt);
	}

	OnItemUpdated.Broadcast();
	OnItemRemoved.Broadcast(InventoryInfoData.ItemMap[ItemID], InventoryInfoData.ItemMap[ItemID].ItemAmount);
}

bool UItemInventoryComponent::TryAddWeapon(int32 ItemID, EItemCategoryInfo ItemCategory, int32 ItemCount)
{
	if (ItemCategory == EItemCategoryInfo::E_Weapon)
	{
		//임시 코드
		if (InventoryInfoData.CurrMainWeaponCount >= InventoryInfoData.MaxMainWeaponCount) return false;
		InventoryInfoData.CurrMainWeaponCount += 1;
		InventoryInfoData.ItemMap[ItemID].ItemAmount = FMath::Min(MAX_ITEM_COUNT_THRESHOLD, InventoryInfoData.ItemMap[ItemID].ItemAmount + ItemCount);
	}
	else if (ItemCategory == EItemCategoryInfo::E_SubWeapon)
	{
		//가능한 최대 소지수를 넘었다면? 
		if (InventoryInfoData.CurrSubWeaponCount >= InventoryInfoData.MaxSubWeaponCount)
		{
			TArray<int32> subWeaponIDList = GetValidWeaponIDList(EWeaponType::E_Shoot);
			if (!subWeaponIDList.Contains(ItemID) && subWeaponIDList.Num() > 0)
			{
				//만약 새로 줍는 무기라면 버리고 로직을 수행
				RemoveItem(subWeaponIDList[0], 99);
			}
		}
		//무기를 줍는다 (카운팅 / 정보 연동)
		InventoryInfoData.CurrSubWeaponCount += 1;
		FWeaponStateStruct& targetState = GetWeaponState(ItemID - 20000);
		int32 prevCount = targetState.RangedWeaponState.CurrentAmmoCount;
		targetState.RangedWeaponState.CurrentAmmoCount += ItemCount;

		//@@ TEMPORARY CODE @@@@@@@
		int32 maxAmmoCount = WeaponRef.Get()->GetWeaponStatInfoWithID(ItemID - 20000).RangedWeaponStat.MaxTotalAmmo;
		maxAmmoCount = maxAmmoCount * FMath::Max(1.0f, OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_SubWeaponCapacity));
		targetState.RangedWeaponState.UpdateAmmoValidation(maxAmmoCount);
		ItemCount = targetState.RangedWeaponState.CurrentAmmoCount - prevCount;


		if (ItemCount <= 0) return false;

		InventoryInfoData.ItemMap[ItemID].ItemAmount = FMath::Min(MAX_ITEM_COUNT_THRESHOLD, InventoryInfoData.ItemMap[ItemID].ItemAmount + ItemCount);
		TryUpdateWeaponState(ItemID - 20000, targetState);

		OnItemAdded.Broadcast(InventoryInfoData.ItemMap[ItemID], ItemCount);
	}
	return true;
}

bool UItemInventoryComponent::TryRemoveWeapon(int32 ItemID, EItemCategoryInfo ItemCategory, int32 RemoveCount)
{
	if (InventoryInfoData.ItemMap[ItemID].ItemType == EItemCategoryInfo::E_Weapon)
	{
		InventoryInfoData.CurrMainWeaponCount = FMath::Max(0, InventoryInfoData.CurrMainWeaponCount - 1);
	}
	else if (InventoryInfoData.ItemMap[ItemID].ItemType == EItemCategoryInfo::E_SubWeapon)
	{
		//탄환 수만큼 현재 클래스 내 정보에서 빼고, WeaponComponent 내 정보도 업데이트 
		FWeaponStateStruct& weaponStruct = GetWeaponState(ItemID - 20000);
		weaponStruct.RangedWeaponState.CurrentAmmoCount -= RemoveCount;
		weaponStruct.RangedWeaponState.CurrentAmmoCount = FMath::Max(0, weaponStruct.RangedWeaponState.CurrentAmmoCount);
		TryUpdateWeaponState(ItemID - 20000, weaponStruct);

		//탄환이 다 떨어졌다면 카운트 줄이기
		if (weaponStruct.RangedWeaponState.CurrentAmmoCount == 0)
		{
			InventoryInfoData.CurrSubWeaponCount = FMath::Max(0, InventoryInfoData.CurrSubWeaponCount - 1);
		}
	}
	return true;
}

void UItemInventoryComponent::GetItemData(int32 ItemID, FItemInfoDataStruct& ItemData)
{
	if (InventoryInfoData.ItemMap.Contains(ItemID))
	{
		ItemData = InventoryInfoData.ItemMap[ItemID];
	}
}

int32 UItemInventoryComponent::GetItemAmount(int32 ItemID)
{
	if (!InventoryInfoData.ItemMap.Contains(ItemID)) return -1;
	return InventoryInfoData.ItemMap[ItemID].ItemAmount;
}

bool UItemInventoryComponent::GetIsItemEnough(int32 ItemID, uint8 NeedItemAmount)
{
	if (!InventoryInfoData.ItemMap.Contains(ItemID)) return false;

	if (InventoryInfoData.ItemMap[ItemID].ItemAmount < NeedItemAmount) return false;
	return true;
}

FItemInfoDataStruct UItemInventoryComponent::GetMainWeaponInfoData()
{
	if (InventoryInfoData.CurrMainWeaponCount == 0) FItemInfoDataStruct();
	TArray<int32> keyList;
	InventoryInfoData.ItemMap.GenerateKeyArray(keyList);

	for (int32& key : keyList)
	{
		if (InventoryInfoData.ItemMap[key].ItemType == EItemCategoryInfo::E_Weapon && InventoryInfoData.ItemMap[key].ItemAmount > 0)
			return InventoryInfoData.ItemMap[key];
	}

	return FItemInfoDataStruct();
}

FItemInfoDataStruct UItemInventoryComponent::GetSubWeaponInfoData()
{
	if (InventoryInfoData.CurrSubWeaponCount == 0) FItemInfoDataStruct();
	TArray<int32> keyList;
	InventoryInfoData.ItemMap.GenerateKeyArray(keyList);

	for (int32& key : keyList)
	{
		if (InventoryInfoData.ItemMap[key].ItemType == EItemCategoryInfo::E_SubWeapon && InventoryInfoData.ItemMap[key].ItemAmount > 0)
		{
			return InventoryInfoData.ItemMap[key];
		}
	}
	return FItemInfoDataStruct();
}

void UItemInventoryComponent::OnWeaponStateUpdated(int32 WeaponID, FWeaponStatStruct WeaponStat, FWeaponStateStruct NewState)
{
	if (!InventoryInfoData.WeaponStateMap.Contains(WeaponID)) InventoryInfoData.WeaponStateMap.Add(WeaponID, FWeaponStateStruct());
	if (!InventoryInfoData.ItemMap.Contains(WeaponID + 20000))
	{
		UE_LOG(LogItem, Error, TEXT("UItemInventoryComponent::OnWeaponStateUpdated %d is not found in InventoryInfoData.ItemMap"), WeaponID);
		return;
	}

	if (WeaponStat.WeaponType == EWeaponType::E_Shoot)
	{
		const int32 ammoVariance = NewState.RangedWeaponState.CurrentAmmoCount - InventoryInfoData.ItemMap[WeaponID + 20000].ItemAmount;
		InventoryInfoData.ItemMap[WeaponID + 20000].ItemAmount = NewState.RangedWeaponState.CurrentAmmoCount;
		if (ammoVariance > 0)
		{
			OnItemAdded.Broadcast(InventoryInfoData.ItemMap[WeaponID + 20000], ammoVariance);
		}
		else if (ammoVariance <= 0)
		{
			OnItemRemoved.Broadcast(InventoryInfoData.ItemMap[WeaponID + 20000], InventoryInfoData.ItemMap[WeaponID + 20000].ItemAmount);
			if (InventoryInfoData.ItemMap[WeaponID + 20000].ItemAmount == 0)
			{
				InventoryInfoData.CurrSubWeaponCount = FMath::Max(0, InventoryInfoData.CurrSubWeaponCount - 1);
			}
		}
	}
	InventoryInfoData.WeaponStateMap[WeaponID] = NewState;
}

bool UItemInventoryComponent::TryUpdateWeaponState(int32 WeaponID, FWeaponStateStruct NewState)
{
	if (!WeaponRef.IsValid()) return false;
	return WeaponRef.Get()->UpdateWeaponStateCache(WeaponID, NewState);
}

TArray<int32> UItemInventoryComponent::GetValidWeaponIDList(EWeaponType WeaponType)
{
	TArray<int32> keyList, result;
	InventoryInfoData.ItemMap.GenerateKeyArray(keyList);
	for (int32& key : keyList)
	{
		if (InventoryInfoData.ItemMap[key].ItemType == EItemCategoryInfo::E_SubWeapon && InventoryInfoData.ItemMap[key].ItemAmount > 0)
		{
			result.Add(key);
		}
	}
	return result;
}

FWeaponStateStruct& UItemInventoryComponent::GetWeaponState(int32 WeaponID)
{
	if (!InventoryInfoData.WeaponStateMap.Contains(WeaponID))
	{
		InventoryInfoData.WeaponStateMap.Add(WeaponID, FWeaponStateStruct());
	}
	return InventoryInfoData.WeaponStateMap[WeaponID];
}