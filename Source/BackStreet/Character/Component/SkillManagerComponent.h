// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "SkillManagerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API USkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkillManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void InitSkillManager();

	UFUNCTION()
	void InitEquipSkillMap();

//======= User Basic Function =======================
public:
	UFUNCTION(BlueprintCallable)
		bool TrySkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool AddSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool RemoveSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool UpgradeSkill(int32 SkillID, uint8 NewLevel);

	UFUNCTION(BlueprintCallable)
		void UpdateObtainableSkillMap();

	UFUNCTION(BlueprintCallable)
		bool EquipSkill(int32 NewSkillID);

//======== Getter ============================
public:
	UFUNCTION(BlueprintCallable)
		FSkillStatStruct GetSkillInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		ASkillBase* GetOwnSkillBase(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool IsSkillValid(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct  GetOwnSkillStat(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStateStruct  GetOwnSkillState(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ESkillType, FSkillListContainer> GetObtainableSkillMap() { return ObtainableSkillMap; }

//======= DataTable ==========================
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Gamplay|Data")
		UDataTable* SkillStatTable;
//====== Property ===========================
private:
	//현재 가지고 있는 스킬을 SkillID에 따라 분류한 Map (SkillBase접근용)
	TMap<int32, TWeakObjectPtr<ASkillBase>> SkillMap;

	//현재 가지고 있는 스킬을 SkillType에 따라 분류한 Map (조합스테이지용)
	TMap<ESkillType, FSkillListContainer > SkillInventoryMap;

	//플레어어가 획득할 수 있는 스킬을 SkillType에 따라 분류한 Map(조합 스테이지용)
	TMap<ESkillType, FSkillListContainer>ObtainableSkillMap;

	//현재 플레이어가 장착한 스킬 Map
	TMap<ESkillType, int32> EquipedSkillMap;
public:
	//플레어어가 획득하기 위하여 찜해둔 스킬 리스트(조합 스테이지용)
	TArray<int32> KeepSkillList;

	//스킬제작UI에서 이미 제시되었던 스킬 리스트(조합 스테이지용)
	TArray<int32> DisplayedSkillList;

private:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};
