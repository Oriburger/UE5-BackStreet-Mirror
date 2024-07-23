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

//======= User Basic Function =======================
public:
	UFUNCTION(BlueprintCallable)
		bool TrySkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool AddSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool RemoveSkill(int32 SkillID);

//======== Getter ============================
public:
	UFUNCTION(BlueprintCallable)
		TArray<int32> GetOwnSkillID();

	UFUNCTION(BlueprintCallable)
		ASkillBase* GetSkillBase(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool IsSkillValid(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct  GetSkillStat(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStateStruct  GetSkillState(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel);

	UFUNCTION()
		TArray<FSkillInventoryContainer> GetObtainableSkillList(int32 WeaponID);

//======= DataTable ==========================
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Gamplay|Data")
		UDataTable* SkillStatTable;
//====== Property ===========================
private:
	//가진 SkillBase에 대한 정보를 모두 담은 SkillMap
	TMap<int32, TWeakObjectPtr<ASkillBase>> SkillMap;

	//스킬의 타입에 따라 분류된 SkillID정보를 담은 인벤토리 맵
	TMap<ESkillType, FSkillInventoryContainer > SkillInventoryMap;

private:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};
