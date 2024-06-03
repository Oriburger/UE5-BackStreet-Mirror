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
		void TrySkill(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* SkillInfo);

		ASkillBase* SpawnSkillBase(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* SkillInfo);

		ASkillBase* GetSkillFromSkillBaseMap(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* SkillInfo);

	UFUNCTION()
		void RemoveSkillInSkillBaseMap(ACharacterBase* NewCauser);


//--------- DataTable ----------------------
private:
	//Get SkillInfo for Inner Class
	FSkillInfoStruct* GetSkillInfoStructBySkillID(FOwnerSkillInfoStruct* OwnerSkillInfo);

public:
	//Get SkillInfo for Outer Class
	UFUNCTION(BlueprintCallable)
		FSkillInfoStruct GetSkillInfoStructByOwnerSkillInfo(FOwnerSkillInfoStruct OwnerSkillInfo);

protected:
	//Skill Info Table
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* SkillInfoTable;

//-------- ETC. (Ref)-------------------------------
private:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};