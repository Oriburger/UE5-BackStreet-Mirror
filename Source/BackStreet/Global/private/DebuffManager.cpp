// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/DebuffManager.h"
#include "../public/BackStreetGameModeBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/WeaponInventoryBase.h"

#define MAX_DEBUFF_IDX 10
#define DEBUFF_DAMAGE_TIMER_IDX 20
#define MAX_DEBUFF_TIME 150.0f

//새로운 타이머 핸들 리스트를 생성할 때 복사할 리스트
TArray<FTimerHandle> TempTimerHandleList;
TArray<float> TempResetValueList;
float TempResetValue = 0.0f;

UDebuffManager::UDebuffManager()
{
	//추후 초기화를 위한 Temp 배열을 초기화
	for (int idx = 0; idx <= DEBUFF_DAMAGE_TIMER_IDX; idx++)
	{
		TempTimerHandleList.Add(FTimerHandle());
		TempResetValueList.Add(0.0f);
	}
}

bool UDebuffManager::SetDebuffTimer(ECharacterDebuffType DebuffType, ACharacterBase* Target, AActor* Causer, float TotalTime, float Variable)
{
	if (!GamemodeRef.IsValid() || !IsValid(Target) || Target->IsActorBeingDestroyed() ) return false;

	UE_LOG(LogTemp, Warning, TEXT("TRY #2"));

	FTimerDelegate timerDelegate, healTimerDelegate, dotDamageDelegate;
	FTimerHandle& timerHandle = GetDebuffTimerHandleRef(DebuffType, Target);

	FCharacterStateStruct characterState = Target->GetCharacterState();
	FCharacterStatStruct characterStat = Target->GetCharacterStat();

	if (GetDebuffIsActive((ECharacterDebuffType)DebuffType, Target))
	{
		float& resetValue = GetDebuffResetValueRef(DebuffType, Target);
		float remainTime = GamemodeRef.Get()->GetWorldTimerManager().GetTimerRemaining(timerHandle);

		ResetStatDebuffState(DebuffType, Target, resetValue);
		resetValue = 0.0f;

		return SetDebuffTimer( DebuffType, Target, Causer, FMath::Min(TotalTime + remainTime, MAX_DEBUFF_TIME), Variable);
	}

	UE_LOG(LogTemp, Warning, TEXT("TRY #3"));

	/*---- 디버프 타이머 세팅 ----------------------------*/
	Variable = FMath::Min(1.0f, FMath::Abs(Variable)); //값 정제
	characterState.CharacterDebuffState |= (1 << (int)DebuffType);

	switch ((ECharacterDebuffType)DebuffType)
	{
		//----데미지 디버프-------------------
		case ECharacterDebuffType::E_Flame:
		case ECharacterDebuffType::E_Poison:
			dotDamageDelegate.BindUFunction(Target, FName("TakeDebuffDamage"), Variable, DebuffType, Causer);\

			UE_LOG(LogTemp, Warning, TEXT("%lf"), Variable);

			if (!(GetTimerHandleListRef(Target)).IsValidIndex(DEBUFF_DAMAGE_TIMER_IDX))
			{
				UE_LOG(LogTemp, Warning, TEXT("Timer Handle List is Invalid!!! - - - - - "));
			}
			else 
				GamemodeRef.Get()->GetWorldTimerManager().SetTimer((GetTimerHandleListRef(Target))[DEBUFF_DAMAGE_TIMER_IDX], dotDamageDelegate, 1.0f, true);
			break;
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
	}
	TArray<float>& resetValueListRef = GetResetValueListRef(Target);
	if(resetValueListRef.IsValidIndex(GetDebuffInfoListIndex(DebuffType)))
		resetValueListRef[GetDebuffInfoListIndex(DebuffType)] = Variable;

	Target->UpdateCharacterStat(characterStat);
	Target->UpdateCharacterState(characterState);

	timerDelegate.BindUFunction(this, FName("ResetStatDebuffState"),  DebuffType, Target, Variable);
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
	case ECharacterDebuffType::E_Flame:
	case ECharacterDebuffType::E_Poison:
		GamemodeRef.Get()->GetWorldTimerManager().ClearTimer((GetTimerHandleListRef(Target))[DEBUFF_DAMAGE_TIMER_IDX]);
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
	
	for (auto timerListIterator = TimerInfoMap.CreateIterator(); timerListIterator; ++timerListIterator)
	{
		TArray<FTimerHandle>& timerListRef = timerListIterator->Value.TimerHandleList;
		for (auto& targetTimer : timerListRef)
		{
			GamemodeRef.Get()->GetWorldTimerManager().ClearTimer(targetTimer);
		}
		timerListIterator->Value.ResetValueHandleList.Empty();
		timerListIterator->Value.TimerHandleList.Empty();
	}
}

void UDebuffManager::InitDebuffManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;

	for (int newTimerIdx = 0; newTimerIdx <= MAX_DEBUFF_IDX; newTimerIdx += 1)
	{
		TempTimerHandleList.Add(FTimerHandle());
		TempResetValueList.Add(0.0f);
	}
	GamemodeRef = NewGamemodeRef;
}

bool UDebuffManager::GetDebuffIsActive(ECharacterDebuffType DebuffType, ACharacterBase* Target)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return false;
	if (Target->GetCharacterState().CharacterDebuffState & (1 << (int)DebuffType)) return true;
	return false;
}

FTimerHandle& UDebuffManager::GetDebuffTimerHandleRef(ECharacterDebuffType DebuffType, ACharacterBase* Target)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return	TempTimerHandleList[0];
	TArray<FTimerHandle>& timerHandleListRef = GetTimerHandleListRef(Target);
	int16 targetListIdx = GetDebuffInfoListIndex(DebuffType);
	return timerHandleListRef.IsValidIndex(targetListIdx) ? timerHandleListRef[targetListIdx] : TempTimerHandleList[0];
}

uint16 UDebuffManager::GetDebuffInfoListIndex(ECharacterDebuffType DebuffType)
{
	return (uint16)DebuffType;
}

float& UDebuffManager::GetDebuffResetValueRef(ECharacterDebuffType DebuffType, ACharacterBase* Target)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return TempResetValue;
	TArray<float>& resetValueListRef = GetResetValueListRef(Target);
	const uint16 debuffTypeIndex = GetDebuffInfoListIndex(DebuffType);
	if (resetValueListRef.IsValidIndex(debuffTypeIndex) && IsValid(Target))
	{
		return resetValueListRef[debuffTypeIndex];
	}
	return TempResetValue;
}


TArray<FTimerHandle>& UDebuffManager::GetTimerHandleListRef(ACharacterBase* Target)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return TempTimerHandleList;
	if (!TimerInfoMap.Contains(Target->GetUniqueID()))
	{
		AddNewTimerList(Target);
	}
	return TimerInfoMap[Target->GetUniqueID()].TimerHandleList;
}

TArray<float>& UDebuffManager::GetResetValueListRef(ACharacterBase* Target)
{
	if (!TimerInfoMap.Contains(Target->GetUniqueID()))
	{
		AddNewTimerList(Target);
	}
	return TimerInfoMap[Target->GetUniqueID()].ResetValueHandleList;
}

void UDebuffManager::AddNewTimerList(ACharacterBase* Target)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return;
	FDebuffTimerInfoStruct newTimerInfo = { TempTimerHandleList, TempResetValueList };
	TimerInfoMap.Add({ Target->GetUniqueID(), newTimerInfo });
}