// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "CraftingManagerBase.generated.h"

#define MAX_STAT_LEVEL 5

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateFailedToGetCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateFailedToGetWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateFailedToGetItemInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateSkillLevelUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateStatLevelUpdated);

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

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStatLevelUpdated OnStatLevelUpdated;

// ------ Default SkillUpgrade Logic -----------------------------
public:
	UFUNCTION(BlueprintCallable)
		void AddSkill(int32 NewSkillID);

	//Return true when successfully upgraded
	UFUNCTION(BlueprintCallable)		
		bool UpgradeSkill(int32 NewSkillID, uint8 TempSkillLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsSkillUpgradeAvailable(int32 NewSkillID, uint8 TempSkillLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsValidLevelForSkillUpgrade(FSkillInfoStruct NewSkillInfo, uint8 TempSkillLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		uint8  GetSkillMaxLevel(int32 NewSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsOwnMaterialEnoughForSkillUpgrade(FSkillInfoStruct NewSkillInfo, uint8 TempSkillLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetRequiredMaterialAmount(FSkillInfoStruct NewSkillInfo, uint8 TempSkillLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		ESkillType GetSkillTypeByID(int32 NewSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FName> GetSkillVariableKeyList(int32 NewSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FVariableByLevelStruct> GetSkillVariableValueList(int32 NewSkillID);

private:
	//For convenience
	FSkillUpgradeInfoStruct GetSkillUpgradeInfoStructBySkillID(int32 SkillID);

// ------ Default StatUpgrade Logic -----------------------------
public:
	UFUNCTION(BlueprintCallable)
		bool UpgradeStat(TArray<uint8> TempUpgradedStatList);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsStatUpgradeAvailable(TArray<uint8> TempUpgradedStatList);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsValidLevelForStatUpgrade(TArray<uint8> TempUpgradedStatList);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsOwnMaterialEnoughForStatUpgrade(TArray<uint8> TempUpgradedStatList);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<uint8> GetRequiredMaterialAmountForStat(TArray<uint8> TempUpgradedStatList);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetCurrentStatLevel();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetStatMaxLevel();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetStatMaxLevelByWeaponID(int32 WeaponID);

//-------- ETC. (Ref)-------------------------------
public:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	UPROPERTY()
		TWeakObjectPtr<class USkillManagerBase> SkillManagerRef;

	UPROPERTY()
		TWeakObjectPtr<class AMainCharacterBase> MainCharacterRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* PlayerActiveSkillTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UDataTable* StatUpgradeInfoTable;
};
