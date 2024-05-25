// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../../Item/Weapon/WeaponInventoryBase.h"
#include "CraftingManagerBase.generated.h"

UCLASS(BlueprintType)
class BACKSTREET_API UCraftingManagerBase : public UObject
{
	GENERATED_BODY()

// ----- Global, Component ------------------
public:
	UCraftingManagerBase();

	UFUNCTION()
		void InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef);

	UFUNCTION(BlueprintCallable)
		EChapterLevel GetCurrentChapterLevel();

// ------ Default Logic -----------------------------
public:
	UFUNCTION(BlueprintCallable)
		void UpdateCurrentWeaponInventoryRef();

	UFUNCTION(BlueprintCallable)
		TArray<FCraftingRecipeStruct> MakeDisplayingRecipeList(EWeaponType SelectedType);

private:
	UFUNCTION(BlueprintCallable)
		ECraftingSlotVisual SetRecipeVisual(FCraftingRecipeStruct Recipe);

	UFUNCTION(BlueprintCallable)
		bool IsIngredientWeaponValid(int32 IngredientWeaponID);


//-------- ETC. (Ref)-------------------------------
public:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* CraftingRecipeTable;

	UPROPERTY(BlueprintReadWrite)
		AWeaponInventoryBase* WeaponInventoryRef;

	UPROPERTY(BlueprintReadWrite)
		AWeaponInventoryBase* SubWeaponInventoryRef;

};
