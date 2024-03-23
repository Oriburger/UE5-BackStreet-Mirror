// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BackStreet.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "CraftingManagerBase.generated.h"

UCLASS()
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
	UFUNCTION()
		void UpdateCurrentInventoryRef();


public:
	UPROPERTY()
		UDataTable* CraftingRecipeTable;

	UPROPERTY()
		AWeaponInventoryBase* InventoryRef;

	UPROPERTY()
		AWeaponInventoryBase* SubInventoryRef;

};
