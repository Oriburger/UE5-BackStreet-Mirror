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

	static ConstructorHelpers::FObjectFinder<UDataTable> itemTableFinder(TEXT("/Game/System/CraftingManager/Data/D_CraftingItemData.D_CraftingItemData"));
	checkf(itemTableFinder.Succeeded(), TEXT("ItemTable class discovery failed."));
	ItemTable = itemTableFinder.Object;
}


// Called when the game starts
void UItemInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
}

void UItemInventoryComponent::InitItemInventory()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	ItemMap = Cast<AMainCharacterBase>(OwnerCharacterRef)->SavedData.PlayerSaveGameData.ItemMap;
}

void UItemInventoryComponent::InitNewItemInventory()
{
	if (!ItemTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("DataTable is null!"));
		return;
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

void UItemInventoryComponent::AddItem(int32 ItemID, uint8 ItemCnt)
{
	if (!ItemMap.Contains(ItemID)) return;
	ItemMap[ItemID].ItemAmount += ItemCnt;
}

void UItemInventoryComponent::RemoveItem(int32 ItemID, uint8 ItemCnt)
{
	if (!ItemMap.Contains(ItemID)) return;
	ItemMap[ItemID].ItemAmount -= ItemCnt;
}

void UItemInventoryComponent::GetItemData(int32 ItemID, FItemDataStruct& ItemData)
{
	if (ItemMap.Contains(ItemID))
	{
		ItemData = ItemMap[ItemID];
	}
}


