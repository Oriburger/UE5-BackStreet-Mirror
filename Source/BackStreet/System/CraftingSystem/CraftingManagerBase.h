// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../../Item/Weapon/WeaponInventoryBase.h"
#include "CraftingManagerBase.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateGetSkill, FOwnerSkillInfoStruct, SkillInfo);

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

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateGetSkill SkillUpdateDelegate;

// ------ Default Logic -----------------------------
public:
	UFUNCTION(BlueprintCallable)
		void UpdateCurrentWeaponInventoryRef();

	UFUNCTION(BlueprintCallable)
		TArray<FCraftingRecipeStruct> MakeDisplayingRecipeList(EWeaponType SelectedType);

	UFUNCTION(BlueprintCallable)
		void AddSkill(int32 SkillID);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* CraftingSkillTable;

	UPROPERTY(BlueprintReadWrite)
		AWeaponInventoryBase* WeaponInventoryRef;

	UPROPERTY(BlueprintReadWrite)
		AWeaponInventoryBase* SubWeaponInventoryRef;

};
