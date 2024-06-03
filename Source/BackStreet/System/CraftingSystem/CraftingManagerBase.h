// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../../Item/Weapon/WeaponInventoryBase.h"
#include "CraftingManagerBase.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateGetSkill, FSkillInfoStruct, SkillInfo);

UCLASS(BlueprintType)
class BACKSTREET_API UCraftingManagerBase : public UObject
{
	GENERATED_BODY()

// ----- Global, Component ------------------
public:
	UCraftingManagerBase();

	UFUNCTION()
		void InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef);

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateGetSkill SkillUpdateDelegate;

// ------ Default Logic -----------------------------
public:
	UFUNCTION(BlueprintCallable)
		void AddSkill(int32 SkillID);



//-------- ETC. (Ref)-------------------------------
public:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	UPROPERTY()
		TWeakObjectPtr<class AMainCharacterBase> MainCharacterRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* CraftingRecipeTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* CraftingSkillTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* CraftingItemData;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<FOwnerSkillInfoStruct> DisplaySkillInfoList; 
};
