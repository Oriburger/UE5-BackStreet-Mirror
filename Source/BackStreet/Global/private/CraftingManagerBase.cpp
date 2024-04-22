// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/CraftingManagerBase.h"
#include "../../Character/public/MainCharacterBase.h"
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

	TArray<FCraftingRecipeStruct> craftableRecipeList;
	TArray<FCraftingRecipeStruct> uncraftableRecipeList;
	TArray<FCraftingRecipeStruct> unidentifiedRecipeList;
	TArray<FCraftingRecipeStruct> displayingRecipeList;

	TArray<FName> craftingKeyList = CraftingRecipeTable->GetRowNames();
	for (FName key : craftingKeyList)
	{
		FCraftingRecipeStruct* craftingRecipe = CraftingRecipeTable->FindRow<FCraftingRecipeStruct>(key, key.ToString());
		check(craftingRecipe!=nullptr);

		craftingRecipeList.Add(craftingRecipe);
	}

	for (int i = 0; i < craftingRecipeList.Num(); ++i)
	{
		//check Chapter Level by WeaponID
		if (!craftingRecipeList[i]->CraftableChapterList.Contains(chapterLevel)
			|| craftingRecipeList[i]->ResultWeaponType == EWeaponType::E_None)
		{
			continue;
		}
		//If user selected 'All' tab, SelectedType is E_None 
		if (SelectedType == EWeaponType::E_None)
		{
			craftingRecipeList[i]->CraftingSlotVisual = SetRecipeVisual(*craftingRecipeList[i]);
			if(craftingRecipeList[i]->CraftingSlotVisual == ECraftingSlotVisual::E_Craftable) craftableRecipeList.Add(*craftingRecipeList[i]);
			else if (craftingRecipeList[i]->CraftingSlotVisual == ECraftingSlotVisual::E_Uncraftable) uncraftableRecipeList.Add(*craftingRecipeList[i]);
			else if (craftingRecipeList[i]->CraftingSlotVisual == ECraftingSlotVisual::E_Unidentified) unidentifiedRecipeList.Add(*craftingRecipeList[i]);
		}
		//displayingRecipeList must consist of applicable weaponType
		else
		{
			//CheckWeaponType from WeaponID
			if (SelectedType == craftingRecipeList[i]->ResultWeaponType)
			{
				craftingRecipeList[i]->CraftingSlotVisual = SetRecipeVisual(*craftingRecipeList[i]);
				if (craftingRecipeList[i]->CraftingSlotVisual == ECraftingSlotVisual::E_Craftable) craftableRecipeList.Add(*craftingRecipeList[i]);
				else if (craftingRecipeList[i]->CraftingSlotVisual == ECraftingSlotVisual::E_Uncraftable) uncraftableRecipeList.Add(*craftingRecipeList[i]);
				else if (craftingRecipeList[i]->CraftingSlotVisual == ECraftingSlotVisual::E_Unidentified) unidentifiedRecipeList.Add(*craftingRecipeList[i]);
			}
		}
	}
	displayingRecipeList.Append(craftableRecipeList);
	displayingRecipeList.Append(uncraftableRecipeList);
	displayingRecipeList.Append(unidentifiedRecipeList);

	return displayingRecipeList;
}

ECraftingSlotVisual UCraftingManagerBase::SetRecipeVisual(FCraftingRecipeStruct Recipe)
{	
	AMainCharacterBase* mainCharacter = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	uint8 playerCraftingLevel = 0;//*mainCharacter->GetCharacterInstance()->CraftingLevelMap.Find(GetCurrentChapterLevel());
	uint8 IngredientIdx = Recipe.IngredientWeaponID.Num();

	//Check recipe adapt for current chapter
	if(!Recipe.CraftableChapterList.Contains((uint8)GetCurrentChapterLevel())) return ECraftingSlotVisual::E_Hide;
	
	//Check recipe adapt for playerCraftingLevel
	if(Recipe.CraftableLevel > playerCraftingLevel) return ECraftingSlotVisual::E_Unidentified;
	
	//Check player has ingredient weapon
	for (int32 RecipeWeaponID : Recipe.IngredientWeaponID)
	{
		if(IsIngredientWeaponValid(RecipeWeaponID)) IngredientIdx--;
	}
	if(IngredientIdx == 0) return ECraftingSlotVisual::E_Craftable;
	else return ECraftingSlotVisual::E_Uncraftable;
}



bool UCraftingManagerBase::IsIngredientWeaponValid(int32 IngredientID)
{
	if (IsValid(InventoryRef))
	{
		for (FInventoryItemInfoStruct InventoryWeapon : InventoryRef->GetInventoryArray())
		{
			if (InventoryWeapon.WeaponID == IngredientID)
			{
				return 1;
			}
		}
	}
	if (IsValid(SubInventoryRef))
	{
		for (FInventoryItemInfoStruct SubInventoryWeapon : SubInventoryRef->GetInventoryArray())
		{
			if (SubInventoryWeapon.WeaponID == IngredientID)
			{
				return 1;
			}
		}
	}
	return 0;
}





