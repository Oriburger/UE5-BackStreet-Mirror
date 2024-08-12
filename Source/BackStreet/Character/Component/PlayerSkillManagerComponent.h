// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "SkillManagerComponentBase.h"
#include "PlayerSkillManagerComponent.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UPlayerSkillManagerComponent : public USkillManagerComponentBase
{
	GENERATED_BODY()
public:
	UPlayerSkillManagerComponent();

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
		virtual bool UpgradeSkill(int32 SkillID, uint8 NewLevel) override;

//======= User Basic Function =======================
public:
	UFUNCTION(BlueprintCallable)
		void UpdateObtainableSkillMap();

	UFUNCTION(BlueprintCallable)
		bool EquipSkill(int32 NewSkillID);

	UFUNCTION(BlueprintCallable)
		void ClearAllSkill();

//====== Getter ================================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		virtual bool IsSkillValid(int32 SkillID) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		virtual bool IsSkilUpgradable(int32 SkillID, uint8 NewLevel) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		virtual ASkillBase* GetOwnSkillBase(int32 SkillID) override;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ESkillType, FSkillListContainer> GetSkillInventoryMap() { return SkillInventoryMap; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ESkillType, FObtainableSkillListContainer> GetObtainableSkillMap() { return ObtainableSkillMap; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ESkillType, int32> GetEquipedSkillMap() { return EquipedSkillMap; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel);

//====== Property ===============================================================================
private:
	//현재 가지고 있는 스킬을 SkillType에 따라 분류한 Map
	TMap<ESkillType, FSkillListContainer> SkillInventoryMap;

	//플레어어가 획득할 수 있는 스킬을 SkillType에 따라 분류한 Map(조합 스테이지용)
	TMap<ESkillType, FObtainableSkillListContainer> ObtainableSkillMap;

	//현재 플레이어가 장착한 스킬 Map
	TMap<ESkillType, int32> EquipedSkillMap;

public: //====== @@@@@ 추후 private로 전환 바랍니다@@@@@@@@@@@ ====================
	//플레어어가 획득하기 위하여 찜해둔 스킬 리스트(조합 스테이지용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<int32> KeepSkillList;

	//스킬제작UI에서 이미 제시되었던 스킬 리스트(조합 스테이지용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<int32> DisplayedSkillList;

};
