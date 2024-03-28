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

	// ------ 기본 로직 -----------------------------
public:
	UFUNCTION(BlueprintCallable)
		void UpdateCurrentInventoryRef();


public:
	UPROPERTY()
		UDataTable* CraftingRecipeTable;

	UPROPERTY()
		AWeaponInventoryBase* InventoryRef;

	UPROPERTY()
		AWeaponInventoryBase* SubInventoryRef;

};
