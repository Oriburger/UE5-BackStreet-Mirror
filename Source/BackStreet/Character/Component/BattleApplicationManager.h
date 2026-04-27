// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../../System/AbilitySystem/AbilityManagerBase.h"
#include "Components/ActorComponent.h"
#include "BattleApplicationManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDelegateEquipInfoUpdated, EComboType, TargetComboType, FBattleAppIDList, NewEquipIDList, FBattleAppIDList, UnequippedAppIDList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateSlotStateUpdated, EComboType, TargetComboType, FBattleAppSlotStateList, NewSlotStateList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FDelegateSingleSlotStateUpdated, EComboType, TargetComboType, int32, SlotIdx, EBattleAppSlotState, OldState, EBattleAppSlotState, NewState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class BACKSTREET_API UBattleApplicationManager : public UAbilityManagerComponent
{
	GENERATED_BODY()

//========= Delegate ================================
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateEquipInfoUpdated OnEquipInfoUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSlotStateUpdated OnSlotStateListUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSingleSlotStateUpdated OnSingleSlotStateUpdated;

//========= Basic ===================================
public:	
	// Sets default values for this component's properties
	UBattleApplicationManager();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======== Basic ===============================
public:
	UFUNCTION(BlueprintCallable)
		void InitBattleApplicationManager(ACharacterBase* NewCharacter, FBattleApplicationManagerInfo InfoOverride);

	//특정 어빌리티를 추가 (실패 시 false)
	UFUNCTION(BlueprintCallable)
		virtual bool TryAddNewAbility(int32 AbilityID) override;

	//소유하고 있는 어빌리티를 제거 (실패 시 false)
	UFUNCTION(BlueprintCallable)
		virtual bool TryRemoveAbility(int32 AbilityID) override;

	UFUNCTION(BlueprintCallable)
		virtual void ClearAllAbility() override;

protected:
	UFUNCTION(BlueprintCallable)
		virtual bool InitAbilityInfoListFromTable() override;

	UFUNCTION()
		void OnBattleAppExtraActionTriggered(int32 AbilityID, AAbilityExtraActionBase* ExtraActionInstance);

	UFUNCTION()
		void OnBattleAppExtraActionCompleted(int32 AbilityID, AAbilityExtraActionBase* ExtraActionInstance);

//======== Property ================================
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TMap<EComboType, int32> ComboTypeToLengthMap = { {EComboType::E_None, 6}, {EComboType::E_Normal, 7}, {EComboType::E_Jump, 7}, {EComboType::E_Dash, 7} };

	//초기화 시 콤보 타입 별 슬롯 상태 리스트, 0번째는 Passive BattleApp용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TArray<FBattleAppSlotStateList> InitialSlotStateListPerComboType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		TMap<int32, FBattleAppEquipInfoStruct> BattleAppEquipInfoMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		TMap<int32, int32> BestFitComboSlotIdxMap; //최적의 콤보 인덱스, 초기 스탯을 저장한다. 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		TMap<int32, EComboType> BestFitComboTypeMap; //최적의 콤보 타입, 초기 스탯을 저장한다.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		TSet<int32> BestFitEquippedAbilityIDSet; //최적의 콤보에 장착된 어빌리티 ID 집합

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		FBattleApplicationManagerInfo BattleApplicationManagerInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State")
		TArray<FBattleAppSlotUnlockInfo> SlotUnlockedInfoQueue;
//======== Function ============================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FBattleApplicationManagerInfo GetBattleApplicationManagerInfo();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetAbilityEquipInfoList(EComboType TargetComboType, FBattleAppIDList& OutEquipIDList, FBattleAppIDList& OutUnequippedIDList);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetAbilitySlotStateList(TArray<FBattleAppSlotStateList>& OutSlotStateListPerComboType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsAbilityEquipped(int32 AbilityID);	

	UFUNCTION(BlueprintCallable)
		bool TryEquipAbility(int32 AbilityID, EComboType TargetComboType, int32 TargetSlotIndex);

	UFUNCTION(BlueprintCallable)
		bool TryUnequipAbility(EComboType TargetComboType, int32 TargetSlotIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetBestFitComboInfo(int32 AbilityID, int32& OutBestFitSlotIdx, EComboType& OutBestFitComboType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FBattleAppSlotUnlockInfo> GetSlotUnlockInfoQueueAndFlush();

protected:
	UFUNCTION()
		void UpdateComboLengthStat(EComboType TargetComboType);

	UFUNCTION()
		void UpdateAllComboLengthStat();
};
