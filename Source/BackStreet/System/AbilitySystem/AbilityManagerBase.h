// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "AbilityManagerBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateAbilityListUpdate, const TArray<FAbilityInfoStruct>&, AbilityInfoList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateStatUpdate, ECharacterStatType, StatType, FStatValueGroup, StatValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateAbilityInfoUpdated, int32, AbilityID, FAbilityInfoStruct, NewAbilityInfo);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class BACKSTREET_API UAbilityManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	UAbilityManagerComponent();

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAbilityListUpdate OnAbilityListUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStatUpdate OnStatUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAbilityInfoUpdated OnAbilityAdded;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAbilityInfoUpdated OnAbilityInfoUpdated;

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
	virtual bool TryAddNewAbility(int32 AbilityID);

	//소유하고 있는 어빌리티를 제거 (실패 시 false)
	virtual bool TryRemoveAbility(int32 AbilityID);

	//모든 어빌리티 초기화
	virtual void ClearAllAbility();

protected:
	UFUNCTION()
		bool TryUpdateCharacterStat(const FAbilityInfoStruct TargetAbilityInfo, bool bIsReset = false);

	//배열로부터 AbilityInfo를 불러들임 (활성화 된 것 중에서 찾을지 지정 가능)
	UFUNCTION()
		FAbilityInfoStruct GetAbilityInfo(int32 AbilityID, bool bActiveAbilityOnly = false);

	//배열로부터 AbilityInfo를 불러들임 (활성화 된 것 중에서 찾을지 지정 가능)
	UFUNCTION()
		FAbilityInfoStruct& GetAbilityInfoRef(int32 AbilityID, bool bActiveAbilityOnly, bool& Result);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<int32> GetOwnedAbilityIDList();

	//배열로부터 AbilityInfoList의 idx를 불러들임 (활성화 된 것 중에서 찾을지 지정 가능)
	UFUNCTION()
		int32 GetAbilityListIdx(int32 AbilityID, bool bActiveAbilityOnly = false);

	virtual bool InitAbilityInfoListFromTable();

//--------- Extra Action ------------------------------------------------
protected: //delegate
	UFUNCTION()
		class AAbilityExtraActionBase* CreateAndInitializeExtraAction(FAbilityInfoStruct& NewApplicationInfo);

	UFUNCTION()
		void OnExtraActionLifeSpanEnded(int32 AbilityID, AAbilityExtraActionBase* ExtraActionInstance);

protected:
	//Extra Action Instance Pool
	UPROPERTY()
		TMap<int32, TWeakObjectPtr<class AAbilityExtraActionBase>> ExtraActionInstancePool;

//--------- Property ----------------------------------------------------
public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 GearItemID = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Wealth")
		int32 CoreItemID = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		UDataTable* AbilityDataTable;

	//최대 어빌리티 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxAbilityCount = 9999999;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		bool bPrintDebugLog = false;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		FAbilityManagerInfoStruct AbilityManagerInfo;

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FAbilityInfoStruct> GetAllAbilityInfoList() { return AbilityManagerInfo.AbilityInfoList; }

	UFUNCTION()
		FAbilityManagerInfoStruct GetAbilityManagerInfo() const { return AbilityManagerInfo; }
};
