// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/DebuffManagerComponent.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Character/public/CharacterBase.h"

#define MAX_DEBUFF_IDX 10
#define DEBUFF_DAMAGE_TIMER_IDX 20
#define MAX_DEBUFF_TIME 150.0f

// Sets default values for this component's properties
UDebuffManagerComponent::UDebuffManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UDebuffManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Init debuff manager
	InitDebuffManager();
}

bool UDebuffManagerComponent::SetDebuffTimer(ECharacterDebuffType DebuffType, AActor* Causer, float TotalTime, float Variable)
{
	if (!OwnerCharacterRef.IsValid()) return false;

	UE_LOG(LogTemp, Warning, TEXT("Set Debuff Timer #1"));
	FTimerDelegate timerDelegate, dotDamageDelegate;
	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();

	FTimerHandle& timerHandle = GetResetTimerHandle(DebuffType);
	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffType);

	if (GetDebuffIsActive((ECharacterDebuffType)DebuffType))
	{
		float resetValue = GetDebuffResetValue(DebuffType);
		float remainTime = GetWorld()->GetTimerManager().GetTimerRemaining(timerHandle);

		ResetStatDebuffState(DebuffType, resetValue);
		ResetValueInfoMap[DebuffType] = 0.0f;

		return SetDebuffTimer(DebuffType, Causer, FMath::Min(TotalTime + remainTime, MAX_DEBUFF_TIME), Variable);
	}

	/*---- 디버프 타이머 세팅 ----------------------------*/
	Variable = FMath::Min(1.0f, FMath::Abs(Variable)); //값 정제
	characterState.CharacterDebuffState |= (1 << (int)DebuffType);

	UE_LOG(LogTemp, Warning, TEXT("SetDebuffState : %.2lf"), Variable);

	switch ((ECharacterDebuffType)DebuffType)
	{
	//----데미지 디버프-------------------
	case ECharacterDebuffType::E_Burn:
	case ECharacterDebuffType::E_Poison:
		dotDamageDelegate.BindUFunction(OwnerCharacterRef.Get(), FName("TakeDebuffDamage"), Variable, DebuffType, Causer); 
		GetWorld()->GetTimerManager().SetTimer(dotDamageHandle, dotDamageDelegate, 1.0f, true);
		break;
		//----스탯 조정 디버프-------------------
	case ECharacterDebuffType::E_Stun:
		OwnerCharacterRef.Get()->StopAttack();
		characterState.CharacterActionState = ECharacterActionType::E_Stun;
		OwnerCharacterRef.Get()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
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
	if(!ResetValueInfoMap.Contains(DebuffType))
		ResetValueInfoMap.Add(DebuffType, 0.0f);
	ResetValueInfoMap[DebuffType] = Variable;

	OwnerCharacterRef.Get()->UpdateCharacterStat(characterStat);
	OwnerCharacterRef.Get()->UpdateCharacterState(characterState);

	timerDelegate.BindUFunction(this, FName("ResetStatDebuffState"), DebuffType, Variable);
	GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, 0.1f, false, TotalTime);

	return true;
}

void UDebuffManagerComponent::ClearDebuffManager()
{
	const uint16 startIdx = 0;
	const uint16 endIdx = MAX_DEBUFF_IDX;

	for (int idx = startIdx; idx <= endIdx; idx++)
	{
		ClearDebuffTimer((ECharacterDebuffType)idx);
	}
	ResetTimerMap.Empty();
	DotDamageTimerMap.Empty();
	ResetValueInfoMap.Empty();
}

void UDebuffManagerComponent::ClearDebuffTimer(ECharacterDebuffType DebuffType)
{
	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffType);
	FTimerHandle& resetHandle = GetResetTimerHandle(DebuffType);

	GetWorld()->GetTimerManager().ClearTimer(dotDamageHandle);
	GetWorld()->GetTimerManager().ClearTimer(resetHandle);

	if(ResetValueInfoMap.Contains(DebuffType))
		ResetValueInfoMap.Remove(DebuffType);
}

bool UDebuffManagerComponent::GetDebuffIsActive(ECharacterDebuffType DebuffType)
{
	if (!ResetTimerMap.Contains(DebuffType)) return false;
	return GetWorld()->GetTimerManager().IsTimerActive(ResetTimerMap[DebuffType]);
}

void UDebuffManagerComponent::InitDebuffManager()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
}

FTimerHandle& UDebuffManagerComponent::GetResetTimerHandle(ECharacterDebuffType DebuffType)
{
	if (!ResetTimerMap.Contains(DebuffType)) ResetTimerMap.Add(DebuffType, FTimerHandle());
	return ResetTimerMap[DebuffType];
}

FTimerHandle& UDebuffManagerComponent::GetDotDamageTimerHandle(ECharacterDebuffType DebuffType)
{
	if (!DotDamageTimerMap.Contains(DebuffType)) DotDamageTimerMap.Add(DebuffType, FTimerHandle());
	return DotDamageTimerMap[DebuffType];
}

float UDebuffManagerComponent::GetDebuffResetValue(ECharacterDebuffType DebuffType)
{
	if (!ResetValueInfoMap.Contains(DebuffType)) return 0.0f;
	return ResetValueInfoMap[DebuffType];
}

void UDebuffManagerComponent::ResetStatDebuffState(ECharacterDebuffType DebuffType, float ResetVal)
{
	if (!OwnerCharacterRef.IsValid()) return;

	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();

	FTimerHandle& timerHandle = GetResetTimerHandle(DebuffType);
	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffType);

	UE_LOG(LogTemp, Warning, TEXT("ResetStatDebuffState : %.2lf"), ResetVal);

	ResetVal = FMath::Max(0.001f, ResetVal);
	characterState.CharacterDebuffState &= ~(1 << (int)DebuffType);

	switch ((ECharacterDebuffType)DebuffType)
	{
	case ECharacterDebuffType::E_Burn:
	case ECharacterDebuffType::E_Poison:
		break;
	case ECharacterDebuffType::E_Slow:
		characterStat.CharacterMoveSpeed /= ResetVal;
		characterStat.CharacterAtkSpeed /= ResetVal;
		break;
	case ECharacterDebuffType::E_Stun:
		characterState.CharacterActionState = ECharacterActionType::E_Idle;
		OwnerCharacterRef.Get()->ResetActionState(true);
		OwnerCharacterRef.Get()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		break;
	case ECharacterDebuffType::E_AttackDown:
		characterStat.CharacterAtkMultiplier /= ResetVal;
		break;
	case ECharacterDebuffType::E_DefenseDown:
		characterStat.CharacterDefense /= ResetVal;
		break;
	}
	OwnerCharacterRef.Get()->UpdateCharacterStat(characterStat);
	OwnerCharacterRef.Get()->UpdateCharacterState(characterState);
	ClearDebuffTimer(DebuffType);
}

