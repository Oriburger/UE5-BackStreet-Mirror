// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "SkillManagerBase.generated.h"

/*
 */
UCLASS()
class BACKSTREET_API USkillManagerBase : public UObject
{
	GENERATED_BODY()
	
//------ Global, Component -------------------
public:
	USkillManagerBase();
	
	//Initiate Skill Asset Data and Skill
	UFUNCTION()
		void InitSkillManagerBase(class ABackStreetGameModeBase* NewGamemodeRef);

//------- Default Property, Action -------------------
public:
	//SkillManagerBase processes information for the skill
	UFUNCTION(BlueprintCallable)
		void ActivateSkill(AActor* NewCauser, TArray<ACharacterBase*> NewTargetList);

protected:
	//Set MainCharacter's Skill Grade by Compare SkillGauge
	UFUNCTION()
		void SetMainCharacterSkillGrade(AActor* NewCauser);

	//Set EnemyCharacter's Skill Grade by Compare Defficulty
	UFUNCTION()
		void SetEnemyCharacterSkillGrade(AActor* NewCauser);

	//Return is Not E_None
	UFUNCTION()
		bool IsValidGrade(AActor* NewCauser);

	//Add skill at SkillMap if there is no skill object that matches the ID
	UFUNCTION()
		ASkillBase* ComposeSkillMap(AActor* NewCauser, int32 NewSkillID);

	//Create New skill ref
	UFUNCTION()
		class ASkillBase* MakeSkillBase(AActor* NewCauser, int32 NewSkillID);


//--------- DataTable ----------------------
public:
	UFUNCTION(BlueprintPure, BlueprintCallable)
		UDataTable* GetSkillInfoTable() { return SkillInfoTable; }

protected:
	//Skill Info Table
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* SkillInfoTable;

//-------- ETC. (Ref)-------------------------------
private:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};