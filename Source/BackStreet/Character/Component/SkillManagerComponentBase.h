// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "SkillManagerComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateEquiped, int32, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateSkillUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSkillActivated, FSkillInfo, SkillInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSkillDeactivated, FSkillInfo, SkillInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSkillListUpdated, const TArray<FSkillInfo>&, SkillInventory);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKSTREET_API USkillManagerComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USkillManagerComponentBase();

	UFUNCTION()
		void InitSkillManager(FSkillManagerInfoStruct SkillManagerInfoOverride = FSkillManagerInfoStruct());

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillUpdated OnSkillUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillActivated OnSkillActivated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillDeactivated OnSkillDeactivated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillListUpdated OnSkillInventoryUpdated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	void InitSkillInfoCache();

	void RestoreCoolTimeTimer();

//========== Basic ==============================
public:
	UFUNCTION(BlueprintCallable)
		bool TryAddSkill(int32 NewSkillID, bool& bIsInInventory);

	UFUNCTION(BlueprintCallable)
		bool TryRemoveSkill(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable)
		bool TryActivateSkill(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable)
		void DeactivateCurrentSkill();

	UFUNCTION(BlueprintCallable)
		bool UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel);

	UFUNCTION(BlueprintCallable)
		void ClearAllSkill();

	UFUNCTION(BlueprintCallable)
		void StopSkillAnimMontage();

private:
	//스킬의 사용 가능 여부를 업데이트 한다.
	UFUNCTION()
		void UpdateSkillValidity(int32 TargetSkillID, bool NewState);

//========== Getter ==============================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<int32> GetCraftableIDList();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<int32> GetOwnedSkillIDList();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillInfo GetSkillInfoData(int32 TargetSkillID, bool& bIsInInventory);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<int32> GetPlayerSkillIDList();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetTotalCoolTime(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetRemainingCoolTime(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetCoolTimeVariables(int32 TargetSkillID, float& RemainingTime, float& TotalTime);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillManagerInfoStruct GetSkillManagerInfo();

	//It must be flushed after using.
	UPROPERTY(BlueprintReadWrite)
		TArray<AActor*> TraceResultCache;

	UPROPERTY(BlueprintReadWrite)
		TArray<AActor*> SpawnedActorList;

//======= Ref =========
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* SkillInfoTable;

protected:
	//Skill Info Cache Map
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|Data")
		FSkillManagerInfoStruct SkillManagerInfo;

protected:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};