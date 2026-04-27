// Fill out your copyright notice in the Description page of Project Settings.


#include "./BattleApplicationManager.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/ActionTrackingComponent.h"
#include "../../System/AbilitySystem/AbilityExtraActionBase.h"

// Sets default values for this component's properties
UBattleApplicationManager::UBattleApplicationManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UBattleApplicationManager::BeginPlay()
{
	UE_LOG(LogAbility, Log, TEXT("UBattleApplicationManager::BeginPlay"));
	Super::BeginPlay();

	//Owner 캐릭터 캐스팅
	OwnerCharacterRef = Cast<AMainCharacterBase>(GetOwner());

	UE_LOG(LogAbility, Log, TEXT("UBattleApplicationManager::BeginPlay - Initializing BattleApplicationManager for Character: %s"), *OwnerCharacterRef->GetName());

	//Initialize ComboEquipInfoList with empty entries for each EComboType
	BattleApplicationManagerInfo.EquipIDListPerComboType.Empty();
	BattleApplicationManagerInfo.SlotStateListPerComboType.Empty();
	for (uint8 start = (uint8)EComboType::E_None; start <= (uint8)EComboType::E_Dash; ++start)
	{
		float length = ComboTypeToLengthMap.Contains((EComboType)start) ? ComboTypeToLengthMap[(EComboType)start] : FBattleAppIDList::MaxLength;
		BattleApplicationManagerInfo.EquipIDListPerComboType.Add(FBattleAppIDList(length));
		BattleApplicationManagerInfo.SlotStateListPerComboType.Add(FBattleAppSlotStateList(length));
	}
	
	BattleApplicationManagerInfo.SlotStateListPerComboType = InitialSlotStateListPerComboType;
	UpdateAllComboLengthStat();

	//For test
	//SlotUnlockedInfoQueue.Add({ EComboType::E_Normal, 4, EBattleAppSlotState::E_Disabled, EBattleAppSlotState::E_Equipped });
	//SlotUnlockedInfoQueue.Add({ EComboType::E_Normal, 5, EBattleAppSlotState::E_Disabled, EBattleAppSlotState::E_Equipped });
	//SlotUnlockedInfoQueue.Add({ EComboType::E_Normal, 6, EBattleAppSlotState::E_Disabled, EBattleAppSlotState::E_Equipped });
	//SlotUnlockedInfoQueue.Add({ EComboType::E_Jump, 4, EBattleAppSlotState::E_Disabled, EBattleAppSlotState::E_Equipped });
	//SlotUnlockedInfoQueue.Add({ EComboType::E_Jump, 5, EBattleAppSlotState::E_Disabled, EBattleAppSlotState::E_Equipped });

	InitBattleApplicationManager(OwnerCharacterRef.Get(), FBattleApplicationManagerInfo());
}

void UBattleApplicationManager::InitBattleApplicationManager(ACharacterBase* NewCharacter, FBattleApplicationManagerInfo InfoOverride)
{
	Super::InitAbilityManager(OwnerCharacterRef.Get(), InfoOverride.AbilityManagerInfo);

	UE_LOG(LogAbility, Log, TEXT("UBattleApplicationManager::InitBattleApplicationManager - Initializing BattleApplicationManager for Character: %s"), *NewCharacter->GetName());
	
	if(InfoOverride.AbilityManagerInfo.AbilityInfoList.Num() > 0 && InfoOverride.SlotStateListPerComboType.Num() > 0)
	{
		BattleApplicationManagerInfo = InfoOverride;
	}
}

bool UBattleApplicationManager::TryAddNewAbility(int32 AbilityID)
{
	if (!Super::TryAddNewAbility(AbilityID))
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::TryAddNewAbility / Super::TryAddNewAbility failed for AbilityID %d"), AbilityID);
		return false;
	}

	//Unequipped에 넣기
	FAbilityInfoStruct newAbilityInfo = GetAbilityInfo(AbilityID, false);
	EComboType comboType = newAbilityInfo.TriggerActionType == ECharacterActionType::E_Attack ? EComboType::E_Normal
							: (newAbilityInfo.TriggerActionType == ECharacterActionType::E_JumpAttack ? EComboType::E_Jump
							: (newAbilityInfo.TriggerActionType == ECharacterActionType::E_DashAttack ? EComboType::E_Dash
							: EComboType::E_None));

	//============= 자동 장착 로직 ==========================================================================
	bool bIsMaxLengthReached = BattleApplicationManagerInfo.EquipIDListPerComboType[static_cast<int32>(comboType)].AbilityIDList.Num() >= ComboTypeToLengthMap[comboType];
	int32 slotIdxToEquip = -1;
	int32 unlockTargetDisabledIdx = -1;
	int32 unlockTargetLockedIdx = -1;

	//들어갈 자리를 찾고, 기타 프로퍼티들을 업데이트한다
	for (int32 idx = 0; idx < BattleApplicationManagerInfo.SlotStateListPerComboType[static_cast<int32>(comboType)].SlotStateList.Num(); ++idx)
	{
		EBattleAppSlotState slotState = BattleApplicationManagerInfo.SlotStateListPerComboType[static_cast<int32>(comboType)].SlotStateList[idx];

		if (slotState == EBattleAppSlotState::E_Empty)
		{
			slotIdxToEquip = idx;
			break;
		}
		else if (slotState == EBattleAppSlotState::E_Equipped)
		{
			bIsMaxLengthReached = (idx == BattleApplicationManagerInfo.SlotStateListPerComboType[static_cast<int32>(comboType)].SlotStateList.Num() - 1);
		}
		else if (slotState == EBattleAppSlotState::E_Disabled)
		{
			unlockTargetDisabledIdx = unlockTargetDisabledIdx == -1 ? idx : unlockTargetDisabledIdx;
		}
		else if (slotState == EBattleAppSlotState::E_Locked)
		{
			unlockTargetLockedIdx = unlockTargetLockedIdx == -1 ? idx : unlockTargetLockedIdx;
		}
	}
	
	//slotIdxToEquip가 -1이라는 것은 빈 슬롯이 없다는 뜻이다. 
	//이 경우, 빈 슬롯이 없는 상황에서 자동 장착 로직을 어떻게 처리할지 결정해야 한다. 
	//최대 장착 개수에 도달한 경우에는 자동 장착 로직을 건너뛰고
	//그렇지 않은 경우에는 잠긴 슬롯이나 비활성화된 슬롯을 해제하여 장착할 수 있도록 한다.
	if(slotIdxToEquip == -1)
	{
		if (bIsMaxLengthReached || (unlockTargetLockedIdx == -1 && unlockTargetDisabledIdx == -1))
		{
			UE_LOG(LogAbility, Warning, TEXT("UBattleApplicationManager::TryAddNewAbility - Cannot auto-equip AbilityID %d for ComboType %d because max length is reached and no slots can be unlocked"), AbilityID, static_cast<int32>(comboType));
			BattleAppEquipInfoMap.Add(AbilityID, FBattleAppEquipInfoStruct(false)); //장착 정보 업데이트
			return true;
		}
		slotIdxToEquip = unlockTargetDisabledIdx != -1 ? unlockTargetDisabledIdx : unlockTargetLockedIdx;
	}

	//Unlock 처리인지 체크하고, UI 연출을 위해 Queue에 넣는다
	if (BattleApplicationManagerInfo.SlotStateListPerComboType[static_cast<int32>(comboType)].SlotStateList[slotIdxToEquip] != EBattleAppSlotState::E_Empty)
	{
		EBattleAppSlotState prevState = BattleApplicationManagerInfo.SlotStateListPerComboType[static_cast<int32>(comboType)].SlotStateList[slotIdxToEquip];
		SlotUnlockedInfoQueue.Add(FBattleAppSlotUnlockInfo({ comboType, slotIdxToEquip, prevState, EBattleAppSlotState::E_Equipped }));
		BattleApplicationManagerInfo.SlotStateListPerComboType[static_cast<int32>(comboType)].SlotStateList[slotIdxToEquip] = EBattleAppSlotState::E_Empty; //슬롯 상태 업데이트
	}
	//============= 델리게이트 연결 (Extra Action) ============================================================
	if(ExtraActionInstancePool.Contains(AbilityID))
	{
		AAbilityExtraActionBase* extraActionInstance = ExtraActionInstancePool[AbilityID].IsValid() ? ExtraActionInstancePool[AbilityID].Get() : nullptr;
		if (IsValid(extraActionInstance))
		{
			extraActionInstance->OnExtraActionTriggered.AddDynamic(this, &UBattleApplicationManager::OnBattleAppExtraActionTriggered);
			extraActionInstance->OnExtraActionCompleted.AddDynamic(this, &UBattleApplicationManager::OnBattleAppExtraActionCompleted);
		}
	}

	//============= 최적	슬롯 정보 및 인덱스 저장 =============================================================
	if (comboType != EComboType::E_None)
	{
		BestFitComboSlotIdxMap.Add(AbilityID, slotIdxToEquip);
		BestFitComboTypeMap.Add(AbilityID, comboType);
	}

	//실제로 장착한다.
	BattleAppEquipInfoMap.Add(AbilityID, FBattleAppEquipInfoStruct(true, comboType == EComboType::E_None, comboType, slotIdxToEquip)); //Map에 추가
	TryEquipAbility(AbilityID, comboType, slotIdxToEquip);

	//캐릭터의 콤보 길이 스탯을 업데이트한다.
	UpdateAllComboLengthStat();

	return true;
}

bool UBattleApplicationManager::TryRemoveAbility(int32 AbilityID)
{
	if(!Super::TryRemoveAbility(AbilityID))
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::TryRemoveAbility / Super::TryRemoveAbility failed for AbilityID %d"), AbilityID);
		return false;
	}

	/* BattleApp은 제거할 수 없기 때문에 일단 미구현
	 
	//EquipInfoMap 갱신
	FAbilityInfoStruct removedAbilityInfo = GetAbilityInfo(AbilityID, false);

	*/

	return true;
}

void UBattleApplicationManager::ClearAllAbility()
{
	Super::ClearAllAbility();
}

bool UBattleApplicationManager::InitAbilityInfoListFromTable()
{
	return Super::InitAbilityInfoListFromTable();
}

void UBattleApplicationManager::OnBattleAppExtraActionTriggered(int32 AbilityID, AAbilityExtraActionBase* ExtraActionInstance)
{
	if (!BestFitEquippedAbilityIDSet.Contains(AbilityID)
		|| !BestFitComboTypeMap.Contains(AbilityID) || !BestFitComboSlotIdxMap.Contains(AbilityID)) return;

	//====== 콤보 카운트 / 타입 세팅 ===================================================
	FCharacterGameplayInfo& ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();
	const EComboType comboType = BestFitComboTypeMap[AbilityID];
	const ECharacterStatType comboLengthStatType = comboType == EComboType::E_Normal ? ECharacterStatType::E_MaxNormalComboIdx
												: (comboType == EComboType::E_Jump ? ECharacterStatType::E_MaxAerialComboIdx
												: (comboType == EComboType::E_Dash ? ECharacterStatType::E_MaxDashComboIdx
												: ECharacterStatType::E_None));

	const float maxComboLength = ownerInfo.GetTotalValue(comboLengthStatType);
	const int32 comboCount = OwnerCharacterRef.Get()->ActionTrackingComponent->CurrentComboCount % (int32)maxComboLength;
	const int32 abilityID = AbilityID;

	//====== 스탯 업데이트 ============================================================
	ECharacterStatType targetStatType = comboType == EComboType::E_Normal ? ECharacterStatType::E_NormalFatality
		: (comboType == EComboType::E_Jump ? ECharacterStatType::E_JumpAttackFatality
			: (comboType == EComboType::E_Dash ? ECharacterStatType::E_DashAttackFatality
				: ECharacterStatType::E_None));

	if (!ownerInfo.StatGroupList.IsValidIndex((int32)targetStatType))
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::OnBattleAppExtraActionTriggered newStat for %d is not valid"), (int32)targetStatType);
	}
	ownerInfo.SetAbilityStatInfo(targetStatType, ownerInfo.GetAbilityStatInfo(targetStatType) + 1.2f);
	if (ownerInfo.GetIsContainProbability(targetStatType))
	{
		ownerInfo.SetProbabilityStatInfo(targetStatType, ownerInfo.GetProbabilityStatInfo(targetStatType) + 0.5);
	}
	OnStatUpdated.Broadcast(targetStatType, ownerInfo.GetStatGroup(targetStatType));

	UE_LOG(LogAbility, Log, TEXT("UBattleApplicationManager::OnBattleAppExtraActionTriggered - Triggered Extra Action for AbilityID %d, updated stat %d to %f"), AbilityID, (int32)targetStatType, ownerInfo.GetTotalValue(targetStatType));
}

void UBattleApplicationManager::OnBattleAppExtraActionCompleted(int32 AbilityID, AAbilityExtraActionBase* ExtraActionInstance)
{
	if (!BestFitEquippedAbilityIDSet.Contains(AbilityID)
		|| !BestFitComboTypeMap.Contains(AbilityID) || !BestFitComboSlotIdxMap.Contains(AbilityID)) return;

	//====== 콤보 카운트 / 타입 세팅 ===================================================
	FCharacterGameplayInfo& ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();
	const EComboType comboType = BestFitComboTypeMap[AbilityID];
	const ECharacterStatType comboLengthStatType = comboType == EComboType::E_Normal ? ECharacterStatType::E_MaxNormalComboIdx
		: (comboType == EComboType::E_Jump ? ECharacterStatType::E_MaxAerialComboIdx
			: (comboType == EComboType::E_Dash ? ECharacterStatType::E_MaxDashComboIdx
				: ECharacterStatType::E_None));

	const float maxComboLength = ownerInfo.GetTotalValue(comboLengthStatType);
	const int32 comboCount = OwnerCharacterRef.Get()->ActionTrackingComponent->CurrentComboCount % (int32)maxComboLength;
	const int32 abilityID = AbilityID;

	//====== 스탯 업데이트 ============================================================
	ECharacterStatType targetStatType = comboType == EComboType::E_Normal ? ECharacterStatType::E_NormalFatality
		: (comboType == EComboType::E_Jump ? ECharacterStatType::E_JumpAttackFatality
			: (comboType == EComboType::E_Dash ? ECharacterStatType::E_DashAttackFatality
				: ECharacterStatType::E_None));

	if (!ownerInfo.StatGroupList.IsValidIndex((int32)targetStatType))
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::OnBattleAppExtraActionCompleted newStat for %d is not valid"), (int32)targetStatType);
	}
	ownerInfo.SetAbilityStatInfo(targetStatType, ownerInfo.GetAbilityStatInfo(targetStatType) - 1.2f);
	if (ownerInfo.GetIsContainProbability(targetStatType))
	{
		ownerInfo.SetProbabilityStatInfo(targetStatType, ownerInfo.GetProbabilityStatInfo(targetStatType) - 0.5);
	}
	OnStatUpdated.Broadcast(targetStatType, ownerInfo.GetStatGroup(targetStatType));

	UE_LOG(LogAbility, Log, TEXT("UBattleApplicationManager::OnBattleAppExtraActionCompleted - Completed Extra Action for AbilityID %d, updated stat %d to %f"), AbilityID, (int32)targetStatType, ownerInfo.GetTotalValue(targetStatType));
}

FBattleApplicationManagerInfo UBattleApplicationManager::GetBattleApplicationManagerInfo()
{
	BattleApplicationManagerInfo.AbilityManagerInfo = AbilityManagerInfo;
	return BattleApplicationManagerInfo;
}

void UBattleApplicationManager::GetAbilityEquipInfoList(EComboType TargetComboType, FBattleAppIDList& OutEquipIDList, FBattleAppIDList& OutUnequippedIDList)
{
	if (!BattleApplicationManagerInfo.EquipIDListPerComboType.IsValidIndex(static_cast<int32>(TargetComboType)))
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::GetAbilityEquipInfoList - Invalid TargetComboType %d"), static_cast<int32>(TargetComboType));
		return;
	}
	OutEquipIDList = BattleApplicationManagerInfo.EquipIDListPerComboType[static_cast<int32>(TargetComboType)];
	OutUnequippedIDList = TargetComboType == EComboType::E_None ? BattleApplicationManagerInfo.UnequippedPassiveAppIDList : BattleApplicationManagerInfo.UnequippedComboAppIDList;
}

void UBattleApplicationManager::GetAbilitySlotStateList(TArray<FBattleAppSlotStateList>& OutSlotStateListPerComboType)
{
	OutSlotStateListPerComboType = BattleApplicationManagerInfo.SlotStateListPerComboType;
}

bool UBattleApplicationManager::GetIsAbilityEquipped(int32 AbilityID)
{
	for(uint8 type = 0; type < BattleApplicationManagerInfo.EquipIDListPerComboType.Num(); ++type)
	{
		if (BattleApplicationManagerInfo.EquipIDListPerComboType[type].AbilityIDList.Contains(AbilityID))
		{
			return true;
		}
	}
	return false;
}

bool UBattleApplicationManager::TryEquipAbility(int32 AbilityID, EComboType TargetComboType, int32 TargetSlotIndex)
{
	FBattleAppIDList& targetEquipIDList = BattleApplicationManagerInfo.EquipIDListPerComboType[static_cast<int32>(TargetComboType)];
	FBattleAppIDList& targetUnequippedIDList = TargetComboType == EComboType::E_None ? BattleApplicationManagerInfo.UnequippedPassiveAppIDList : BattleApplicationManagerInfo.UnequippedComboAppIDList;
	FBattleAppSlotStateList& targetSlotStateList = BattleApplicationManagerInfo.SlotStateListPerComboType[static_cast<int32>(TargetComboType)];

	// ===== 슬롯 유효성(이미 있는 슬롯은 아닌지, 잠겼거나 비활성화된 슬롯은 아닌지), 어빌리티 ID 유효성, 체크 =========================
	if(AbilityID <= 0 || !AbilityManagerInfo.AbilityInfoMap.Contains(AbilityID))
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::TryEquipAbility - Invalid AbilityID %d"), AbilityID);
		return false;
	}
	else if(!targetSlotStateList.SlotStateList.IsValidIndex(TargetSlotIndex))
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::TryEquipAbility - Invalid TargetSlotIndex %d for ComboType %d"), TargetSlotIndex, static_cast<int32>(TargetComboType));
		return false;
	}
	else if(targetSlotStateList.SlotStateList[TargetSlotIndex] == EBattleAppSlotState::E_Disabled || targetSlotStateList.SlotStateList[TargetSlotIndex] == EBattleAppSlotState::E_Locked)
	{
		UE_LOG(LogAbility, Warning, TEXT("UBattleApplicationManager::TryEquipAbility - Cannot equip AbilityID %d to SlotIndex %d for ComboType %d because the slot is disabled or locked"), AbilityID, TargetSlotIndex, static_cast<int32>(TargetComboType));
		return false;
	}
	else if(targetSlotStateList.SlotStateList[TargetSlotIndex] == EBattleAppSlotState::E_Equipped)
	{
		int32 oldID = targetEquipIDList.AbilityIDList.IsValidIndex(TargetSlotIndex) ? targetEquipIDList.AbilityIDList[TargetSlotIndex] : -1;
		if(oldID == -1)
		{ 
			UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::TryEquipAbility - Inconsistent state: SlotIndex %d for ComboType %d is marked as Equipped but has no AbilityID"), TargetSlotIndex, static_cast<int32>(TargetComboType));
			return false;
		}
		BattleAppEquipInfoMap[oldID].Reset();
		targetUnequippedIDList.AbilityIDList.AddUnique(oldID);
	}

	//===== 실제 장착 로직 ===========================================================================================================
	BattleAppEquipInfoMap[AbilityID].Update(true, TargetComboType == EComboType::E_None, TargetComboType, TargetSlotIndex); //장착 정보 업데이트
	targetUnequippedIDList.AbilityIDList.Remove(AbilityID);
	targetEquipIDList.AbilityIDList[TargetSlotIndex] = AbilityID;
	targetSlotStateList.SlotStateList[TargetSlotIndex] = EBattleAppSlotState::E_Equipped; //장착 상태로 업데이트

	//===== 최적 슬롯 로직 ===========================================================================================================
	if (BestFitComboSlotIdxMap.Contains(AbilityID) && BestFitComboTypeMap.Contains(AbilityID))
	{
		if (BestFitComboSlotIdxMap[AbilityID] == TargetSlotIndex && BestFitComboTypeMap[AbilityID] == TargetComboType)
		{
			BestFitEquippedAbilityIDSet.Add(AbilityID); //최적 슬롯에 장착한 경우, 최적 슬롯에 장착된 것으로 간주
		}
		else
		{
			BestFitEquippedAbilityIDSet.Remove(AbilityID); //최적 슬롯이 아닌 곳에 장착한 경우, 최적 슬롯에 장착된 것으로 간주하지 않음
		}
	}

	//===== AbilityInfo 갱신 (TriggerActionType과 TriggerComboIdxList) ===============================================================
	bool result = false;
	FAbilityInfoStruct& abilityInfo = GetAbilityInfoRef(AbilityID, true, result);

	if (!result) //Error Handling
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::TryEquipAbility - Failed to get AbilityInfo for AbilityID %d after equipping"), AbilityID);
		return false;
	}

	if (TargetComboType != EComboType::E_None)
	{
		abilityInfo.TriggerActionType = TargetComboType == EComboType::E_Normal ? ECharacterActionType::E_Attack
			: (TargetComboType == EComboType::E_Jump ? ECharacterActionType::E_JumpAttack
				: ECharacterActionType::E_DashAttack);
	}
	abilityInfo.TriggerComboIdxList = { TargetSlotIndex };

	//debug (log idx and type
	UE_LOG(LogAbility, Log, TEXT("UBattleApplicationManager::TryEquipAbility - Updated AbilityInfo for AbilityID %d: TriggerActionType %d, TriggerComboIdxList [%s]"), AbilityID, abilityInfo.TriggerActionType, *FString::JoinBy(abilityInfo.TriggerComboIdxList, TEXT(", "), [](int32 idx) { return FString::FromInt(idx); }));

	OnAbilityInfoUpdated.Broadcast(AbilityID, abilityInfo);

	//===== Delegate Broadcast ================================================================================================
	OnEquipInfoUpdated.Broadcast(TargetComboType, targetEquipIDList, targetUnequippedIDList);
	OnSlotStateListUpdated.Broadcast(TargetComboType, targetSlotStateList);
	OnSingleSlotStateUpdated.Broadcast(TargetComboType, TargetSlotIndex, EBattleAppSlotState::E_Empty, EBattleAppSlotState::E_Equipped);

	return true;
}

bool UBattleApplicationManager::TryUnequipAbility(EComboType TargetComboType, int32 TargetSlotIndex)
{
	FBattleAppIDList& targetEquipIDList = BattleApplicationManagerInfo.EquipIDListPerComboType[static_cast<int32>(TargetComboType)];
	FBattleAppIDList& targetUnequippedIDList = TargetComboType == EComboType::E_None ? BattleApplicationManagerInfo.UnequippedPassiveAppIDList : BattleApplicationManagerInfo.UnequippedComboAppIDList;
	FBattleAppSlotStateList& targetSlotStateList = BattleApplicationManagerInfo.SlotStateListPerComboType[static_cast<int32>(TargetComboType)];

	//====== 슬롯 유효성(이미 있는 슬롯은 아닌지, 잠겼거나 비활성화된 슬롯은 아닌지), 어빌리티 ID 유효성, 체크======================================
	if (!targetSlotStateList.SlotStateList.IsValidIndex(TargetSlotIndex) 
		|| targetSlotStateList.SlotStateList[TargetSlotIndex] != EBattleAppSlotState::E_Equipped)
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::TryUnequipAbility - Failed Reason  %d %d"), targetSlotStateList.SlotStateList.IsValidIndex(TargetSlotIndex),
			targetSlotStateList.SlotStateList[TargetSlotIndex] != EBattleAppSlotState::E_Equipped);
		return false;
	}
	int32 oldID = targetEquipIDList.AbilityIDList.IsValidIndex(TargetSlotIndex) ? targetEquipIDList.AbilityIDList[TargetSlotIndex] : -1;
	if (oldID == -1)
	{
		UE_LOG(LogAbility, Warning, TEXT("UBattleApplicationManager::TryUnequipAbility - Inconsistent state: SlotIndex %d for ComboType %d is marked as Equipped but has no AbilityID"), TargetSlotIndex, static_cast<int32>(TargetComboType));
		return false;
	}

	//===== AbilityInfo 갱신 (TriggerActionType과 TriggerComboIdxList) ===============================================================
	bool result = false;
	FAbilityInfoStruct& abilityInfo = GetAbilityInfoRef(oldID, true, result);

	abilityInfo.TriggerComboIdxList.Empty();
	OnAbilityInfoUpdated.Broadcast(oldID, abilityInfo);

	//====== Unequip 처리 ==================================================================================================================
	targetEquipIDList.AbilityIDList[TargetSlotIndex] = -1;
	BattleAppEquipInfoMap[oldID].Reset();
	targetUnequippedIDList.AbilityIDList.AddUnique(oldID);
	targetSlotStateList.SlotStateList[TargetSlotIndex] = EBattleAppSlotState::E_Empty; //장착 상태로 업데이트

	//====== Delegate Broadcast ===========================================================================================================
	OnEquipInfoUpdated.Broadcast(TargetComboType, targetEquipIDList, targetUnequippedIDList);
	OnSlotStateListUpdated.Broadcast(TargetComboType, targetSlotStateList);
	OnSingleSlotStateUpdated.Broadcast(TargetComboType, TargetSlotIndex, EBattleAppSlotState::E_Equipped, EBattleAppSlotState::E_Empty);
	return true;
}

void UBattleApplicationManager::GetBestFitComboInfo(int32 AbilityID, int32& OutBestFitSlotIdx, EComboType& OutBestFitComboType)
{
	if (BestFitComboSlotIdxMap.Contains(AbilityID) && BestFitComboTypeMap.Contains(AbilityID))
	{
		OutBestFitSlotIdx = BestFitComboSlotIdxMap[AbilityID];
		OutBestFitComboType = BestFitComboTypeMap[AbilityID];
	}
	else
	{
		OutBestFitSlotIdx = -1;
		OutBestFitComboType = EComboType::E_None;
	}
}

TArray<FBattleAppSlotUnlockInfo> UBattleApplicationManager::GetSlotUnlockInfoQueueAndFlush()
{
	TArray<FBattleAppSlotUnlockInfo> result = SlotUnlockedInfoQueue;
	SlotUnlockedInfoQueue.Empty();
	return result;
}

void UBattleApplicationManager::UpdateComboLengthStat(EComboType TargetComboType)
{
	//CharacterStat 업데이트 (Combo Length), 
	if(!OwnerCharacterRef.IsValid())	
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::UpdateComboLengthStat - OwnerCharacterRef is not valid"));
		return;
	}

	FCharacterGameplayInfo& ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();

	ECharacterStatType statType = TargetComboType == EComboType::E_Normal ? ECharacterStatType::E_MaxNormalComboIdx
								: (TargetComboType == EComboType::E_Jump ? ECharacterStatType::E_MaxAerialComboIdx
								: ECharacterStatType::E_MaxDashComboIdx);
	
	//Reset first
	ownerInfo.SetAbilityStatInfo(statType, -1.0f);
	for (auto& type : BattleApplicationManagerInfo.SlotStateListPerComboType[(int32)TargetComboType].SlotStateList)
	{
		if (type == EBattleAppSlotState::E_None || type == EBattleAppSlotState::E_Disabled) continue;
		ownerInfo.SetAbilityStatInfo(statType, ownerInfo.GetAbilityStatInfo(statType) + 1.0f);	
	}
}

void UBattleApplicationManager::UpdateAllComboLengthStat()
{
	for (uint8 type = 0; type < BattleApplicationManagerInfo.EquipIDListPerComboType.Num(); ++type)
	{
		UpdateComboLengthStat((EComboType)type);
	}
}

// Called every frame
void UBattleApplicationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}