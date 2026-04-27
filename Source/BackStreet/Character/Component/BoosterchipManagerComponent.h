// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../../System/AbilitySystem/AbilityManagerBase.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "BoosterchipManagerComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class BACKSTREET_API UBoosterchipManagerComponent : public UAbilityManagerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	UBoosterchipManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//--------- Function ----------------------------------------------------
public:
	UFUNCTION(BlueprintCallable)
	void InitBoosterchipManager(ACharacterBase* NewCharacter, FBoosterchipManagerInfo InfoOverride = FBoosterchipManagerInfo());

	//특정 어빌리티를 추가 (실패 시 false)
	UFUNCTION(BlueprintCallable)
		virtual bool TryAddNewAbility(int32 AbilityID) override;

	//소유하고 있는 어빌리티를 제거 (실패 시 false)
	UFUNCTION(BlueprintCallable)
		virtual bool TryRemoveAbility(int32 AbilityID) override;

	UFUNCTION(BlueprintCallable)
		virtual void ClearAllAbility() override;

	//Cost를 고려하지 않고 어빌리티를 업그레이드 시도 (실패 시 false)
	UFUNCTION(BlueprintCallable)
		bool TryUpgradeAbility(int32 AbilityID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<EAbilityTierType, int32> GetAbilityUpgradeCostInfoMap() { return UpgradeCostInfoMap; };

	UFUNCTION()
		void UpdateProbilityValue(EAbilityTierType TierType, float NewProbability);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetAbilityUpgradeCost(EAbilityTierType TargetTier);

protected:
	UFUNCTION()
		int32 GetRandomAbilityIdxUsingGroupIdx(int32 SelectedGroupIdx);

	UFUNCTION()
		virtual bool InitAbilityInfoListFromTable() override;

	//이전 단계의 어빌리티가 뽑혔는지를 체크한다. 
	bool CanPickAbility(int32 AbilityID);

//--------- Property ----------------------------------------------------
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		FBoosterchipManagerInfo BoosterchipManagerInfo;	//내부의 AbilityManagerInfo는 Super의 AbilityManagerInfo를 초기화 하는 용도

	//티어에 따른 등장 확률, 미지정 시 모두 같은 확률 적용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TMap<EAbilityTierType, float> ProbabilityInfoMap;

	//어빌리티 티어에 따른 업그레이드 비용 정보	 (재화는 기어 고정 / 별도의 수정 불가)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TMap<EAbilityTierType, int32> UpgradeCostInfoMap;

private:
	//어빌리티 종류별 토탈 티어 합산 (3: 레전더리, 2: 레어, 1: 일반)
	TMap<EAbilityType, int32> AbilityTotalTierThreshold;

	//확률 계산을 위한 누적합 배열
	TArray<float> CumulativeProbabilityList;
	void UpdateCumulativeProbabilityList();
	//ProbabilityInfoMap의 float 값을 모두 더한 값
	float TotalProbabilityValue = 0.0f;

//--------- Getter ----------------------------------------------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FBoosterchipManagerInfo GetBoosterchipManagerInfo();

	//아직 뽑힌적 없는 무작위의 어빌리티를 반환한다. (개수와 타입 지정 가능)
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FAbilityInfoStruct> GetRandomAbilityInfoList(int32 Count, TArray<EAbilityType> TypeList);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetAbilityTotalTier(EAbilityType AbilityType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetAbilityTotalTierThreshold(EAbilityType AbilityType);
};
