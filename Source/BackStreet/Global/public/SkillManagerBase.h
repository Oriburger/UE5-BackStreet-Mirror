// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "SkillManagerBase.generated.h"

USTRUCT(BlueprintType)
struct FSkillBaseListContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<ASkillBase*> SkillBaseList;
};

UCLASS()
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
	UFUNCTION()
		void TrySkill(ACharacterBase* NewCauser, int32 NewSkillID);

	UFUNCTION()
		ASkillBase* SpawnSkillBase(ACharacterBase* NewCauser, int32 NewSkillID);

	UFUNCTION()
		ASkillBase* GetSkillFromSkillBaseMap(ACharacterBase* NewCauser, int32 NewSkillID);

	UFUNCTION()
		void RemoveSkillInSkillBaseMap(ACharacterBase* NewCauser);


//--------- DataTable ----------------------
public:
	FSkillInfoStruct* GetSkillInfoStructBySkillID(int32 NewSkillID);

protected:
	//Skill Info Table
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* SkillInfoTable;

//-------- ETC. (Ref)-------------------------------
private:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};