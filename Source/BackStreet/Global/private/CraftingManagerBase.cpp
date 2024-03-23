// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/CraftingManagerBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../public/BackStreetGameModeBase.h"

// Sets default values
UCraftingManagerBase::UCraftingManagerBase()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> craftingRecipeTableFinder(TEXT("/Game/System/CraftingManager/Data/D_CraftingRecipe.D_CraftingRecipe"));
	checkf(craftingRecipeTableFinder.Succeeded(), TEXT("CraftingRecipeTable class discovery failed."));
	CraftingRecipeTable = craftingRecipeTableFinder.Object;
}

void UCraftingManagerBase::InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef)
{
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
