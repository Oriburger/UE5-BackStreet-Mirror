// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInventoryComponent.h"
#include "../CharacterBase.h"
#include "../MainCharacter/MainCharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"

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
	TArray<FItemDataStruct*> allRows;
	ItemTable->GetAllRows(ContextString, allRows);

	for (FItemDataStruct* row : allRows)
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
	ItemMap = playerCharacterRef->SavedData.PlayerSaveGameData.ItemMap;
}

void UItemInventoryComponent::AddItem(int32 ItemID, uint8 ItemCnt)
{
	if (!ItemMap.Contains(ItemID)) return;
	ItemMap[ItemID].ItemAmount += ItemCnt;
	OnUpdateItem.Broadcast();
}

void UItemInventoryComponent::RemoveItem(int32 ItemID, uint8 ItemCnt)
{
	if (!ItemMap.Contains(ItemID)) return;
	ItemMap[ItemID].ItemAmount -= ItemCnt;
	OnUpdateItem.Broadcast();
}

void UItemInventoryComponent::GetItemData(int32 ItemID, FItemDataStruct& ItemData)
{
	if (ItemMap.Contains(ItemID))
	{
		ItemData = ItemMap[ItemID];
	}
}

TArray<uint8> UItemInventoryComponent::GetCraftingItemAmount()
{
	TArray<uint8> currItemAmountList;
	//하드코딩 BIC이후 변경예정
	for (int32 itemID = 1; itemID <= 3; itemID++)
	{
		if (ItemMap.Contains(itemID))
		{
			uint8 itemAmount = ItemMap[itemID].ItemAmount;
			currItemAmountList.Add(itemAmount);
		}
		else
		{
			currItemAmountList.Add(0);
		}
	}
	return currItemAmountList;
}


