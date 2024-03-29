// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/CraftingManagerBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
#include "../public/BackStreetGameModeBase.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
UCraftingManagerBase::UCraftingManagerBase()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> craftingRecipeTableFinder(TEXT("/Game/System/CraftingManager/Data/D_CraftingRecipe.D_CraftingRecipe"));
	checkf(craftingRecipeTableFinder.Succeeded(), TEXT("CraftingRecipeTable class discovery failed."));
	CraftingRecipeTable = craftingRecipeTableFinder.Object;
}

void UCraftingManagerBase::InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
}

void UCraftingManagerBase::UpdateCurrentInventoryRef()
{
	ACharacterBase* mainCharacter = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (IsValid(mainCharacter->GetInventoryRef()))
	{
		InventoryRef = mainCharacter->GetInventoryRef();
	}
	if (IsValid(mainCharacter->GetSubInventoryRef()))
	{
		SubInventoryRef = mainCharacter->GetSubInventoryRef();
	}
}

EChapterLevel UCraftingManagerBase::GetCurrentChapterLevel()
{
	return GamemodeRef.Get()->GetChapterManagerRef()->GetChapterLevel();
}

TArray<FCraftingRecipeStruct> UCraftingManagerBase::MakeDisplayingRecipeList(EWeaponType SelectedType)
{
	uint8 chapterLevel = (uint8)GetCurrentChapterLevel();
	TArray<FCraftingRecipeStruct*> craftingRecipeList;
	TArray<FCraftingRecipeStruct> displayingRecipeList;
	TArray<FName> craftingKeyList = CraftingRecipeTable->GetRowNames();
	for (FName key : craftingKeyList)
	{
		craftingRecipeList.Add(CraftingRecipeTable->FindRow<FCraftingRecipeStruct>(key, key.ToString()));
	}
	for (int i = 0; i < craftingRecipeList.Num(); ++i)
	{
		//check Chapter Level by WeaponID
		if (!craftingRecipeList[i]->AvailableChapterList.Contains(chapterLevel)
			|| craftingRecipeList[i]->CraftingType == EWeaponType::E_None)
		{
			continue;
		}
		//If user selected 'All' tab, SelectedType is E_None 
		if (SelectedType == EWeaponType::E_None)
		{
			displayingRecipeList.Add(*craftingRecipeList[i]);
			UE_LOG(LogTemp, Log, TEXT("DisplayingRecipe ID : %d is added To All"), craftingRecipeList[i]->ResultWeaponID);
		}
		//displayingRecipeList must consist of applicable weaponType
		else
		{
			//CheckWeaponType from WeaponID
			if (SelectedType == craftingRecipeList[i]->CraftingType)
			{
				displayingRecipeList.Add(*craftingRecipeList[i]);
				UE_LOG(LogTemp, Log, TEXT("DisplayingRecipe ID : %d is added"), craftingRecipeList[i]->ResultWeaponID);
			}
		}
	}
	return displayingRecipeList;
}

