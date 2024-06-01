// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Item/Weapon/WeaponBase.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
UCraftingManagerBase::UCraftingManagerBase()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> craftingRecipeTableFinder(TEXT("/Game/System/CraftingManager/Data/D_CraftingRecipe.D_CraftingRecipe"));
	static ConstructorHelpers::FObjectFinder<UDataTable> craftingSkillTableFinder(TEXT("/Game/System/CraftingManager/Data/D_CraftingSkillInfo.D_CraftingSkillInfo"));
	checkf(craftingRecipeTableFinder.Succeeded(), TEXT("CraftingRecipeTable class discovery failed."));
	checkf(craftingSkillTableFinder.Succeeded(), TEXT("CraftingSkillTable class discovery failed."));
	CraftingRecipeTable = craftingRecipeTableFinder.Object;
	CraftingSkillTable = craftingSkillTableFinder.Object;
}

void UCraftingManagerBase::InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
}

void UCraftingManagerBase::UpdateCurrentWeaponInventoryRef()
{
	ACharacterBase* mainCharacter = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (IsValid(mainCharacter->GetWeaponInventoryRef()))
	{
		WeaponInventoryRef = mainCharacter->GetWeaponInventoryRef();
	}
	if (IsValid(mainCharacter->GetSubWeaponInventoryRef()))
	{
		SubWeaponInventoryRef = mainCharacter->GetSubWeaponInventoryRef();
	}
}

EChapterLevel UCraftingManagerBase::GetCurrentChapterLevel()
{
	return EChapterLevel::E_Chapter1;
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

void UCraftingManagerBase::AddSkill(int32 SkillID)
{
	AMainCharacterBase* mainCharacter = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!IsValid(mainCharacter->GetCurrentWeaponRef())) return;
	if (mainCharacter->GetCurrentWeaponRef()->GetWeaponStat().WeaponID == 0) return;

	FWeaponStateStruct weaponState = mainCharacter->GetCurrentWeaponRef()->GetWeaponState();

	if (!IsValid(CraftingSkillTable)) { UE_LOG(LogTemp, Log, TEXT("There's no CraftingSkillInfoTable")); return; }

	FString rowName = FString::FromInt(SkillID);
	FOwnerSkillInfoStruct* skillInfo = CraftingSkillTable->FindRow<FOwnerSkillInfoStruct>(FName(rowName), rowName);
	if (skillInfo == nullptr) return;

	switch (skillInfo->SkillType)
	{
	case ESkillType::E_Weapon0:
	{
		weaponState.ActiveWeaponSkillInfo0 = *skillInfo;
	}
	case ESkillType::E_Weapon1:
	{
		weaponState.ActiveWeaponSkillInfo1 = *skillInfo;
	}
	case ESkillType::E_Weapon2:
	{
		weaponState.ActiveWeaponSkillInfo2 = *skillInfo;
	}
	default:
		break;
	}

	mainCharacter->GetCurrentWeaponRef()->SetWeaponState(weaponState);
	SkillUpdateDelegate.Broadcast(*skillInfo);
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
	if (IsValid(WeaponInventoryRef))
	{
		for (FInventoryItemInfoStruct InventoryWeapon : WeaponInventoryRef->GetInventoryArray())
		{
			if (InventoryWeapon.WeaponID == IngredientID)
			{
				return 1;
			}
		}
	}
	if (IsValid(SubWeaponInventoryRef))
	{
		for (FInventoryItemInfoStruct SubInventoryWeapon : SubWeaponInventoryRef->GetInventoryArray())
		{
			if (SubInventoryWeapon.WeaponID == IngredientID)
			{
				return 1;
			}
		}
	}
	return 0;
}





