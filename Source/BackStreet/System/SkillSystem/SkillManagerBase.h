// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "SkillManagerBase.generated.h"

USTRUCT(BlueprintType)
struct FSkillBaseListContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<ASkillBase*> SkillBaseList;
};

UCLASS(BlueprintType)
class BACKSTREET_API USkillManagerBase : public UObject
{
	GENERATED_BODY()
	
//------ Global, Component -------------------
public:
	USkillManagerBase();
	
	//Initiate Skill Asset Data and Skill
	UFUNCTION()
		void InitSkillManagerBase(ABackStreetGameModeBase* NewGamemodeRef);

public:
	UPROPERTY(VisibleInstanceOnly)
		TMap<ACharacterBase*, FSkillBaseListContainer> SkillBaseMap;
		

//------- Default Property, Action -------------------
public:
		void TrySkill(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* SkillInfo);

		ASkillBase* ActivateSkillBase(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* SkillInfo);

		ASkillBase* UpdateSkillInfo(ASkillBase* SkillBase, FOwnerSkillInfoStruct* OwnerSkillInfo);

		ASkillBase* GetSkillFromSkillBaseMap(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* SkillInfo);

		ASkillBase* MakeSkillBase(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* OwnerSkillInfo);

	UFUNCTION()
		void RemoveSkillInSkillBaseMap(ACharacterBase* NewCauser);

//--------- Getter --------------------------
	//Checks if given struct's SkillID != 0 returns true
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsSkillInfoStructValid(const FSkillInfoStruct& SkillInfo);
	
	//Get current player's weapon SkillInfo by type
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillInfoStruct GetCurrentSkillInfoByType(ESkillType SkillType);

	//Get current player's weapon SkillID by type
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetSkillIDByType(ESkillType SkillType);

	//Get current player's weapon SkillImage by type
	UFUNCTION(BlueprintCallable, BlueprintPure)
		UTexture2D* GetSkillImageByType(ESkillType SkillType);

	//Get current player's weapon SkillLevel by type
	UFUNCTION(BlueprintCallable, BlueprintPure)
		uint8 GetSkillLevelByType(ESkillType SkillType);

	//Get current player's weapon SkillCoolTime by type
	UFUNCTION(BlueprintCallable, BlueprintPure)
			float GetSkillCoolTimeByType(ESkillType SkillType);


//--------- DataTable, Data ----------------------
private:
	//Get SkillInfo for Activate Skill
	FSkillInfoStruct* GetSkillInfoStructByOwnerSkillInfo(FOwnerSkillInfoStruct* OwnerSkillInfo);

public:
	//Get SkillInfo for Outer Class
	UFUNCTION(BlueprintCallable)
		FSkillInfoStruct GetSkillInfoStructBySkillID(int32 SkillID);

protected:
	//Skill Info Table
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* SkillInfoTable;

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* SkillUpgradeInfoTable;


//-------- ETC. (Ref)-------------------------------
private:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};