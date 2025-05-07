// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "AbilityManagerBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateAbilityUpdate, const TArray<FAbilityInfoStruct>&, AbilityInfoList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateStatUpdate, ECharacterStatType, StatType, FStatValueGroup, StatValue);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKSTREET_API UAbilityManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	UAbilityManagerComponent();

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAbilityUpdate OnAbilityUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStatUpdate OnStatUpdated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//--------- Function ----------------------------------------------------
public:
	// Active Ability getter 함수
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<ECharacterAbilityType> GetActiveAbilityList() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FAbilityInfoStruct> GetActiveAbilityInfoList() const;

public:
	//어빌리티 매니저 초기화, 부모 설정
	UFUNCTION()
		void InitAbilityManager(ACharacterBase* NewCharacter, FAbilityManagerInfoStruct AbilityManagerInfoData = FAbilityManagerInfoStruct());

	//특정 어빌리티를 추가 (실패 시 false)
	UFUNCTION()
		bool TryAddNewAbility(int32 AbilityID);

	//소유하고 있는 어빌리티를 제거 (실패 시 false)
	UFUNCTION()
		bool TryRemoveAbility(int32 AbilityID);

	//모든 어빌리티 초기화
	UFUNCTION(BlueprintCallable)
		void ClearAllAbility();

	UFUNCTION()
		void UpdateProbilityValue(EAbilityTierType TierType, float NewProbability);

protected:
	UFUNCTION()
		bool TryUpdateCharacterStat(const FAbilityInfoStruct TargetAbilityInfo, bool bIsReset = false);

	//배열로부터 AbilityInfo를 불러들임 (활성화 된 것 중에서 찾을지 지정 가능)
	UFUNCTION()
		FAbilityInfoStruct GetAbilityInfo(int32 AbilityID, bool bActiveAbilityOnly = false);

	UFUNCTION()
		int32 GetRandomAbilityIdxUsingGroupIdx(int32 SelectedGroupIdx);

	//배열로부터 AbilityInfoList의 idx를 불러들임 (활성화 된 것 중에서 찾을지 지정 가능)
	UFUNCTION()
		int32 GetAbilityListIdx(int32 AbilityID, bool bActiveAbilityOnly = false);

private:
	bool InitAbilityInfoListFromTable();
	//이전 단계의 어빌리티가 뽑혔는지를 체크한다. 
	bool CanPickAbility(int32 AbilityID);

//--------- Property ----------------------------------------------------
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		UDataTable* AbilityInfoTable;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		FAbilityManagerInfoStruct AbilityManagerInfo;

private:
	//어빌리티 종류별 토탈 티어 합산 (3: 레전더리, 2: 레어, 1: 일반)
	TMap<EAbilityType, int32> AbilityTotalTier;
	TMap<EAbilityType, int32> AbilityTotalTierThreshold;

	//확률 계산을 위한 누적합 배열
	TArray<float> CumulativeProbabilityList;
	void UpdateCumulativeProbabilityList();
	//ProbabilityInfoMap의 float 값을 모두 더한 값
	float TotalProbabilityValue = 0.0f;

	//소유자 캐릭터 약참조
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

//--------- Getter ----------------------------------------------------
public:
	//해당 Ability가 Active한지 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsAbilityActive(int32 AbilityID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetMaxAbilityCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FAbilityInfoStruct> GetAbilityInfoList() { return AbilityManagerInfo.ActiveAbilityInfoList; }

	//아직 뽑힌적 없는 무작위의 어빌리티를 반환한다. (개수와 타입 지정 가능)
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FAbilityInfoStruct> GetRandomAbilityInfoList(int32 Count, TArray<EAbilityType> TypeList);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetAbilityTotalTier(EAbilityType AbilityType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetAbilityTotalTierThreshold(EAbilityType AbilityType);

	UFUNCTION()
		FAbilityManagerInfoStruct GetAbilityManagerInfo() const { return AbilityManagerInfo; }
};
