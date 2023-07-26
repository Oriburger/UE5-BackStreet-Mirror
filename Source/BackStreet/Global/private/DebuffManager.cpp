// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/DebuffManager.h"
#include "../public/BackStreetGameModeBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/WeaponInventoryBase.h"

#define MAX_DEBUFF_IDX 10
#define DEBUFF_DAMAGE_TIMER_IDX 20
#define MAX_DEBUFF_TIME 150.0f

UDebuffManager::UDebuffManager() {}

bool UDebuffManager::SetDebuffTimer(ECharacterDebuffType DebuffType, ACharacterBase* Target, AActor* Causer, float TotalTime, float Variable)
{
	if (!GamemodeRef.IsValid() || !IsValid(Target) || Target->IsActorBeingDestroyed() ) return false;

	FTimerDelegate timerDelegate, debuffDamageDelegate;
	FTimerHandle timerHandle = GetDebuffTimerHandleRef(DebuffType, Target);

	FCharacterStateStruct characterState = Target->GetCharacterState();
	FCharacterStatStruct characterStat = Target->GetCharacterStat();

	if (GetDebuffIsActive((ECharacterDebuffType)DebuffType, Target))
	{
		float resetValue = GetDebuffResetValue(DebuffType, Target);
		float remainTime = GamemodeRef.Get()->GetWorldTimerManager().GetTimerRemaining(timerHandle);

		ResetStatDebuffState(DebuffType, Target, resetValue);
		resetValue = 0.0f;

		return SetDebuffTimer( DebuffType, Target, Causer, FMath::Min(TotalTime + remainTime, MAX_DEBUFF_TIME), Variable);
	}

	/*---- 디버프 타이머 세팅 ----------------------------*/
	Variable = FMath::Min(1.0f, FMath::Abs(Variable)); //값 정제
	characterState.CharacterDebuffState |= (1 << (int)DebuffType);

	switch ((ECharacterDebuffType)DebuffType)
	{
		//----스탯 조정 디버프-------------------
		case ECharacterDebuffType::E_Stun:
			Target->StopAttack();
			characterState.CharacterActionState = ECharacterActionType::E_Stun;
			Target->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
			break;
		case ECharacterDebuffType::E_Slow:
			characterStat.CharacterMoveSpeed *= Variable;
			characterStat.CharacterAtkSpeed *= Variable;
			break;
		case ECharacterDebuffType::E_AttackDown:
			characterStat.CharacterAtkMultiplier *= Variable;
			break;
		case ECharacterDebuffType::E_DefenseDown:
			characterStat.CharacterDefense *= Variable;
			break;
		//----데미지 디버프-------------------
		case ECharacterDebuffType::E_Burn:
		case ECharacterDebuffType::E_Poison:
			GamemodeRef.Get()->GetWorldTimerManager().SetTimer(GetDebuffTimerHandleRef(DebuffType, Target, true),
				[Target, Variable, DebuffType, Causer]() {
					if (IsValid(Target) && !Target->IsActorBeingDestroyed() && Target->ActorHasTag("Character"))
						Cast<ACharacterBase>(Target)->TakeDebuffDamage(Variable, DebuffType, Causer);
				}, 1.0f, true);
			break;
	}
	TMap<ECharacterDebuffType, float>& resetValueMapRef = GetResetValueMapRef(Target);
	if(resetValueMapRef.Contains(DebuffType))
		resetValueMapRef[DebuffType] = Variable;

	Target->UpdateCharacterStat(characterStat);
	Target->UpdateCharacterState(characterState);

	timerDelegate.BindUFunction(this, FName("ResetStatDebuffState"), DebuffType, Target, Variable);
	GamemodeRef.Get()->GetWorldTimerManager().SetTimer(timerHandle, timerDelegate, 0.1f, false, TotalTime);

	return true;
}

void UDebuffManager::ResetStatDebuffState(ECharacterDebuffType DebuffType, ACharacterBase* Target, float ResetVal)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return;
	FCharacterStateStruct characterState = Target->GetCharacterState();
	FCharacterStatStruct characterStat = Target->GetCharacterStat();

	ResetVal = FMath::Max(0.0f, ResetVal);
	characterState.CharacterDebuffState &= ~(1 << (int)DebuffType);

	switch ((ECharacterDebuffType)DebuffType)
	{
	case ECharacterDebuffType::E_Burn:
	case ECharacterDebuffType::E_Poison:
		GamemodeRef.Get()->GetWorldTimerManager().ClearTimer(GetDebuffTimerHandleRef(DebuffType, Target, true));
		break;
	case ECharacterDebuffType::E_Slow:
		characterStat.CharacterMoveSpeed /= ResetVal;
		characterStat.CharacterAtkSpeed /= ResetVal;
		break;
	case ECharacterDebuffType::E_Stun:	
		characterState.CharacterActionState = ECharacterActionType::E_Idle;
		Target->ResetActionState(true);
		Target->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		break;
	case ECharacterDebuffType::E_AttackDown:
		characterStat.CharacterAtkMultiplier /= ResetVal;
		break;
	case ECharacterDebuffType::E_DefenseDown:
		characterStat.CharacterDefense /= ResetVal;
		break;
	}
	Target->UpdateCharacterStat(characterStat);
	Target->UpdateCharacterState(characterState);
	ClearDebuffTimer(DebuffType, Target);
}

void UDebuffManager::ClearDebuffTimer(ECharacterDebuffType DebuffType, ACharacterBase* Target)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return;
	GamemodeRef.Get()->GetWorldTimerManager().ClearTimer(GetDebuffTimerHandleRef(DebuffType, Target));
}

void UDebuffManager::ClearAllDebuffTimer()
{
	const uint16 startIdx = 0;
	const uint16 endIdx = MAX_DEBUFF_IDX;
	
	for (auto timerMapIterator = TimerInfoMap.CreateIterator(); timerMapIterator; ++timerMapIterator)
	{
		TMap<ECharacterDebuffType, FTimerHandle>& timerMapRef = timerMapIterator->Value.TimerHandleMap;
		for (auto& targetTimer : timerMapRef)
		{
			GamemodeRef.Get()->GetWorldTimerManager().ClearTimer(targetTimer.Value);
		}
		timerMapIterator->Value.ResetValueHandleMap.Empty();
		timerMapIterator->Value.TimerHandleMap.Empty();
	}
}

void UDebuffManager::InitDebuffManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
}

bool UDebuffManager::GetDebuffIsActive(ECharacterDebuffType DebuffType, ACharacterBase* Target)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return false;
	if (Target->GetCharacterState().CharacterDebuffState & (1 << (int)DebuffType)) return true;
	return false;
}

FTimerHandle& UDebuffManager::GetDebuffTimerHandleRef(ECharacterDebuffType DebuffType, ACharacterBase* Target, bool bIsDebuffDamageTimerMap)
{
	check(IsValid(Target) || Target->IsActorBeingDestroyed());
	TMap<ECharacterDebuffType, FTimerHandle>& timerHandleListRef = GetTimerHandleMapRef(Target, bIsDebuffDamageTimerMap);
	if (!timerHandleListRef.Contains(DebuffType)) timerHandleListRef.Add(DebuffType, FTimerHandle());
	return timerHandleListRef[DebuffType];
}

float UDebuffManager::GetDebuffResetValue(ECharacterDebuffType DebuffType, ACharacterBase* Target)
{
	check(IsValid(Target) || Target->IsActorBeingDestroyed());
	TMap<ECharacterDebuffType, float>& resetValueListRef = GetResetValueMapRef(Target);
	if (!resetValueListRef.Contains(DebuffType)) resetValueListRef.Add(DebuffType, 0.0f);
	return resetValueListRef[DebuffType];
}

TMap<ECharacterDebuffType, FTimerHandle>& UDebuffManager::GetTimerHandleMapRef(ACharacterBase* Target, bool bIsDebuffDamageTimerMap)
{
	check(IsValid(Target) || Target->IsActorBeingDestroyed());
	if (!TimerInfoMap.Contains(Target->GetUniqueID()))
	{
		AddNewTimerList(Target);
	}
	if(bIsDebuffDamageTimerMap) return TimerInfoMap[Target->GetUniqueID()].DebuffDamageTimerHandleMap;
	return TimerInfoMap[Target->GetUniqueID()].TimerHandleMap;
}

TMap<ECharacterDebuffType, float>& UDebuffManager::GetResetValueMapRef(ACharacterBase* Target)
{
	check(IsValid(Target) || Target->IsActorBeingDestroyed());
	if (!TimerInfoMap.Contains(Target->GetUniqueID()))
	{
		AddNewTimerList(Target);
	}
	return TimerInfoMap[Target->GetUniqueID()].ResetValueHandleMap;
}

void UDebuffManager::AddNewTimerList(ACharacterBase* Target)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return;
	FDebuffTimerInfoStruct newTimerInfo = { TMap<ECharacterDebuffType, FTimerHandle>()
											, TMap<ECharacterDebuffType, FTimerHandle>()
											, TMap<ECharacterDebuffType, float>() };
	TimerInfoMap.Add({ Target->GetUniqueID(), newTimerInfo });
}