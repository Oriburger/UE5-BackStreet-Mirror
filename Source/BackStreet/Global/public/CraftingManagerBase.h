// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../public/BackStreet.h"
#include "../../Item/public/WeaponInventoryBase.h"
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
		void UpdateCurrentInventoryRef();

	UFUNCTION(BlueprintCallable)
		TArray<FCraftingRecipeStruct> MakeDisplayingRecipeList(EWeaponType SelectedType);

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
		AWeaponInventoryBase* InventoryRef;

	UPROPERTY(BlueprintReadWrite)
		AWeaponInventoryBase* SubInventoryRef;

};
