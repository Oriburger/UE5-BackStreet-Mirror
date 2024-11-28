// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "SkillManagerComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateEquiped, int32, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateSkillUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSkillActivated, int32, SkillID);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKSTREET_API USkillManagerComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USkillManagerComponentBase();

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillUpdated OnSkillUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillActivated OnSkillActivated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//========== Basic ==============================
public:
	UFUNCTION(BlueprintCallable)
		bool TryAddSkill(int32 NewSkillID);

	UFUNCTION(BlueprintCallable)
		bool TryRemoveSkill(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable)
		bool TryActivateSkill(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable)
		void DestroyCurrentSkill();

//========== Getter ==============================
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillInfo GetSkillInfoData(int32 TargetSkillID, bool& bIsInInventory);

	//It must be flushed after using.
	UPROPERTY(BlueprintReadWrite)
		TArray<AActor*> TraceResultCache;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|")
		TMap<int32, FSkillInfo> SkillInventory;

	UPROPERTY(EditDefaultsOnly, Category = "Gamplay|Data")
		UDataTable* SkillInfoTable;

private:
	TMap<int32, FSkillInfo> SkillInfoCacheMap;

	UPROPERTY()
		TWeakObjectPtr<ASkillBase> CurrentSkill;


//===============================================
//===============================================
//=======  LEGACY  ==============================

 
//======= Basic function ========================
protected:
	virtual void InitSkillManager();

	virtual void InitSkillMap();

public:
	virtual bool TrySkill(int32 SkillID);

	virtual bool AddSkill(int32 SkillID);

	virtual bool RemoveSkill(int32 SkillID);

	virtual bool UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel);

	virtual void ClearAllSkill();

//======== Getter ============================
public:
	virtual bool IsSkillValid(int32 SkillID);

	virtual bool GetIsSkillUpgradable(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel);

	virtual ASkillBase* GetOwnSkillBase(int32 SkillID);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct GetSkillInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		ESkillType GetSkillTypeInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct GetOwnSkillStat(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStateStruct GetOwnSkillState(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillUpgradeLevelInfo GetCurrentSkillLevelInfo(int32 SkillID, ESkillUpgradeType Target);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillUpgradeLevelInfo GetSkillUpgradeLevelInfo(int32 SkillID, ESkillUpgradeType Target, int32 TargetLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsUpgradeTargetValid(int32 SkillID, ESkillUpgradeType UpgradeTarget);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsUpgradeLevelValid(int32 SkillID, ESkillUpgradeType UpgradeTarget, int32 TargetLevel);

//======= DataTable =============================================================================
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gamplay|Data")
		UDataTable* SkillStatTable;

//======= Ref =========
private:
	TMap<int32, FSkillStatStruct> SkillInfoCache;

protected:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};