// Fill out your copyright notice in the Description page of Project Settings.


#include "DebuffManagerComponent.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"

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

bool UDebuffManagerComponent::SetDebuffTimer(FDebuffInfoStruct DebuffInfo, AActor* Causer)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (OwnerCharacterRef.Get()->GetIsActionActive(ECharacterActionType::E_Die)) return false;

	FTimerDelegate timerDelegate, dotDamageDelegate;
	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();

	FTimerHandle& timerHandle = GetResetTimerHandle(DebuffInfo.Type);
	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffInfo.Type);

	if (GetDebuffIsActive((ECharacterDebuffType)DebuffInfo.Type))
	{
		FDebuffInfoStruct resetInfo = GetDebuffResetValue(DebuffInfo.Type);
		
		float remainTime = GetWorld()->GetTimerManager().GetTimerRemaining(timerHandle);
		float resetValue = resetInfo.Variable;

		ResetStatDebuffState(DebuffInfo.Type, resetInfo);
		if (!ResetValueInfoMap.Contains(DebuffInfo.Type))
			ResetValueInfoMap.Add(DebuffInfo.Type, DebuffInfo);
		else
			ResetValueInfoMap[DebuffInfo.Type] = { DebuffInfo.Type, 0.0f, 0.0f, false };

		return SetDebuffTimer({ DebuffInfo.Type, FMath::Min(DebuffInfo.TotalTime + remainTime, MAX_DEBUFF_TIME)
								, DebuffInfo.Variable, DebuffInfo.bIsPercentage }, Causer);
	}

	/*---- ����� Ÿ�̸� ���� ----------------------------*/
	DebuffInfo.Variable = FMath::Min(1.0f, FMath::Abs(DebuffInfo.Variable)); //�� ����
	characterState.CharacterDebuffState |= (1 << (int)DebuffInfo.Type);

	switch (DebuffInfo.Type)
	{
		//----������ �����-------------------
	case ECharacterDebuffType::E_Burn:
	case ECharacterDebuffType::E_Poison:
		dotDamageDelegate.BindUFunction(this, FName("ApplyDotDamage")
										, DebuffInfo.Type
										,	(float)(DebuffInfo.bIsPercentage ? characterState.TotalHP * DebuffInfo.Variable : DebuffInfo.Variable)
										,	OwnerCharacterRef.Get());
		GetWorld()->GetTimerManager().SetTimer(dotDamageHandle, dotDamageDelegate, 1.0f, true);
		break;
		//----���� ���� �����-------------------
	case ECharacterDebuffType::E_Stun:
		OwnerCharacterRef.Get()->StopAttack();
		characterState.CharacterActionState = ECharacterActionType::E_Stun;
		OwnerCharacterRef.Get()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		break;
	case ECharacterDebuffType::E_Slow:
		if (DebuffInfo.bIsPercentage)
		{
			characterState.DebuffMoveSpeed.PercentValue += DebuffInfo.Variable;
			characterState.DebuffAttackSpeed.PercentValue += DebuffInfo.Variable;
		}
		else
		{
			characterState.DebuffMoveSpeed.FixedValue += DebuffInfo.Variable;
			characterState.DebuffAttackSpeed.FixedValue += DebuffInfo.Variable;
		}
		break;
	case ECharacterDebuffType::E_AttackDown:
		if (DebuffInfo.bIsPercentage)
		{
			characterState.DebuffAttack.PercentValue += DebuffInfo.Variable;
		}
		else
		{
			characterState.DebuffAttack.FixedValue += DebuffInfo.Variable;
		}
		break;
	case ECharacterDebuffType::E_DefenseDown:
		if (DebuffInfo.bIsPercentage)
		{
			characterState.DebuffDefense.PercentValue += DebuffInfo.Variable;
		}
		else
		{
			characterState.DebuffDefense.FixedValue += DebuffInfo.Variable;
		}
		break;
	}
	if(!ResetValueInfoMap.Contains(DebuffInfo.Type))
		ResetValueInfoMap.Add(DebuffInfo.Type, DebuffInfo);
	ResetValueInfoMap[DebuffInfo.Type] = DebuffInfo;

	OwnerCharacterRef.Get()->UpdateCharacterStatAndState(characterStat, characterState);

	GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, 1.0f, false, DebuffInfo.TotalTime);

	OnDebuffAdded.Broadcast(DebuffInfo.Type);

	timerDelegate.BindUFunction(this, FName("ResetStatDebuffState"), DebuffInfo.Type, DebuffInfo);
	GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, 0.1f, false, DebuffInfo.TotalTime);

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
	dotDamageHandle.Invalidate();
	resetHandle.Invalidate();

	OnDebuffRemoved.Broadcast(DebuffType);

	if(ResetValueInfoMap.Contains(DebuffType))
		ResetValueInfoMap.Remove(DebuffType);
}


bool UDebuffManagerComponent::GetDebuffIsActive(ECharacterDebuffType DebuffType)
{
	if (!ResetTimerMap.Contains(DebuffType)) return false;
	return GetWorld()->GetTimerManager().IsTimerActive(ResetTimerMap[DebuffType]);
}

void UDebuffManagerComponent::PrintAllDebuff()
{
	const uint16 startIdx = 0;
	const uint16 endIdx = MAX_DEBUFF_IDX;

	for (int idx = startIdx; idx <= endIdx; idx++)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debuff %d : %d"), idx, (int32)GetDebuffIsActive((ECharacterDebuffType)idx));
	}
}

void UDebuffManagerComponent::InitDebuffManager()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
}

void UDebuffManagerComponent::ApplyDotDamage(ECharacterDebuffType DebuffType, float DamageAmount, AActor* Causer)
{
	if (!OwnerCharacterRef.IsValid()) return; 
	OwnerCharacterRef.Get()->TakeDebuffDamage(DebuffType, DamageAmount, Causer);
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

FDebuffInfoStruct UDebuffManagerComponent::GetDebuffResetValue(ECharacterDebuffType DebuffType)
{
	if (!ResetValueInfoMap.Contains(DebuffType)) return {};
	return ResetValueInfoMap[DebuffType];
}

void UDebuffManagerComponent::ResetStatDebuffState(ECharacterDebuffType DebuffType, FDebuffInfoStruct DebuffInfo)
{
	if (!OwnerCharacterRef.IsValid()) return;
	if (OwnerCharacterRef.Get()->GetIsActionActive(ECharacterActionType::E_Die)) return;

	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();

	FTimerHandle& timerHandle = GetResetTimerHandle(DebuffType);
	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffType);

	float ResetVal = FMath::Max(0.001f, DebuffInfo.Variable);
	characterState.CharacterDebuffState &= ~(1 << (int)DebuffType);

	switch ((ECharacterDebuffType)DebuffType)
	{
	case ECharacterDebuffType::E_Burn:
	case ECharacterDebuffType::E_Poison:
		break;
	case ECharacterDebuffType::E_Slow:
		if (DebuffInfo.bIsPercentage)
		{
			characterState.DebuffMoveSpeed.PercentValue -= ResetVal;
			characterState.DebuffAttackSpeed.PercentValue -= ResetVal;
		}
		else
		{
			characterState.DebuffMoveSpeed.FixedValue -= ResetVal;
			characterState.DebuffAttackSpeed.FixedValue -= ResetVal;
		}
		break;
	case ECharacterDebuffType::E_Stun:
		characterState.CharacterActionState = ECharacterActionType::E_Idle;
		OwnerCharacterRef.Get()->ResetActionState(true);
		OwnerCharacterRef.Get()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		break;
	case ECharacterDebuffType::E_AttackDown:
		if (DebuffInfo.bIsPercentage)
		{
			characterState.DebuffAttack.PercentValue -= ResetVal;
		}
		else
		{
			characterState.DebuffAttack.FixedValue -= ResetVal;
		}
		break;
	case ECharacterDebuffType::E_DefenseDown:
		if (DebuffInfo.bIsPercentage)
		{
			characterState.DebuffDefense.PercentValue -= ResetVal;
		}
		else
		{
			characterState.DebuffDefense.FixedValue -= ResetVal;
		}
		
		break;
	}
	OwnerCharacterRef.Get()->UpdateCharacterStatAndState(characterStat, characterState);
	ClearDebuffTimer(DebuffType);
}
