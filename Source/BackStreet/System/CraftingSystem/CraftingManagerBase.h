// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../../Item/Weapon/WeaponInventoryBase.h"
#include "CraftingManagerBase.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateFailedToGetCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateFailedToGetWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateFailedToGetItemInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateSkillLevelUpdated);

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
		FDelegateFailedToGetCharacter OnFailedToGetCharacter;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateFailedToGetWeapon OnFailedToGetWeapon;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateFailedToGetItemInventory OnFailedToGetItemInventory;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillLevelUpdated OnSkillLevelUpdated;


// ------ Default SkillUpgrade Logic -----------------------------
public:
	UFUNCTION(BlueprintCallable)
		bool AddSkill(int32 SkillID);

	//Return true when successfully upgraded
	UFUNCTION(BlueprintCallable)		
		bool UpgradeSkill(ESkillType SkillType, uint8 Adder);

	UFUNCTION()
		bool IsSkillUpgradeAvailable(ESkillType SkillType, uint8 Adder);

	UFUNCTION()
		bool IsValidLevelForSkillUpgrade(FSkillInfoStruct SkillInfo, uint8 Adder);

	UFUNCTION()
		uint8  GetSkillMaxLevel(int32 SkillID);

	UFUNCTION()
		bool IsOwnMaterialEnoughForSkillUpgrade(FSkillInfoStruct SkillInfo, uint8 Adder);

	UFUNCTION()
		TArray<uint8> GetRequiredMaterialAmount(FSkillInfoStruct SkillInfo, uint8 Adder);

//-------- ETC. (Ref)-------------------------------
public:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	UPROPERTY()
		TWeakObjectPtr<class USkillManagerBase> SkillManagerRef;

	UPROPERTY()
		TWeakObjectPtr<class AMainCharacterBase> MainCharacterRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* SkillUpgradeInfoTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* PlayerActiveSkillTable;
};
