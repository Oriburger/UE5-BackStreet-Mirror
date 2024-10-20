// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "SkillManagerComponentBase.h"
#include "EnemySkillManagerComponent.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UEnemySkillManagerComponent : public USkillManagerComponentBase
{
	GENERATED_BODY()

public:
	UEnemySkillManagerComponent();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======= Basic function ========================
protected:
	UFUNCTION(BlueprintCallable)
		virtual void InitSkillManager() override;

	UFUNCTION(BlueprintCallable)
		virtual void InitSkillMap() override;

public:
	UFUNCTION(BlueprintCallable)
		virtual bool TrySkill(int32 SkillID) override;

	UFUNCTION(BlueprintCallable)
		virtual bool AddSkill(int32 SkillID) override;

	UFUNCTION(BlueprintCallable)
		virtual bool RemoveSkill(int32 SkillID) override;

	UFUNCTION(BlueprintCallable)
		virtual bool UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel) override;

	UFUNCTION(BlueprintCallable)
		virtual void ClearAllSkill() override;

//======= Property =============================
private:
	TMap<int32, class ASkillBase*> OwnedSkillInfoMap;

//======= Getter ================================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		virtual bool IsSkillValid(int32 SkillID) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		virtual ASkillBase* GetOwnSkillBase(int32 SkillID) override;
};
