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
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UDebuffManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Init debuff manager
	InitDebuffManager();
}


// Called every frame
void UDebuffManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UDebuffManagerComponent::SetDebuffTimer(ECharacterDebuffType DebuffType, AActor* Causer, float TotalTime, float Variable)
{
	if (!GamemodeRef.IsValid()) return false;

	FTimerDelegate timerDelegate dotDamageDelegate;
	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();

	FTimerHandle timerHandle = GetResetTimerHandle(DebuffType);
	if (GetDebuffIsActive((ECharacterDebuffType)DebuffType))
	{
		float resetValue = GetDebuffResetValue(DebuffType);
		float remainTime = GamemodeRef.Get()->GetWorldTimerManager().GetTimerRemaining(timerHandle);

		ResetStatDebuffState(DebuffType, resetValue);
		resetValue = 0.0f;

		return SetDebuffTimer(DebuffType, Causer, FMath::Min(TotalTime + remainTime, MAX_DEBUFF_TIME), Variable);
	}

	//Add timer map
	ResetTimerMap.Add(DebuffType, timerHandle);

	/*---- 디버프 타이머 세팅 ----------------------------*/
	Variable = FMath::Min(1.0f, FMath::Abs(Variable)); //값 정제
	characterState.CharacterDebuffState |= (1 << (int)DebuffType);

	switch ((ECharacterDebuffType)DebuffType)
	{
	//----데미지 디버프-------------------
	case ECharacterDebuffType::E_Burn:
	case ECharacterDebuffType::E_Poison:
		dotDamageDelegate.BindUFunction(OwnerCharacterRef.Get(), FName("TakeDebuffDamage"), Variable, DebuffType, Causer); 
		
		if (!DotDamageTimerMap.Contains(DebuffType))
			DotDamageTimerMap.Add(FTimerHandle());

		GetWorld()->GetTimerManager().SetTimer(GetDotDamageTimerHandle(), [DEBUFF_DAMAGE_TIMER_IDX], dotDamageDelegate, 1.0f, true);
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
	ResetValueInfoMap.Add(DebuffType, Variable);

	OwnerCharacterRef.Get()->UpdateCharacterStat(characterStat);
	OwnerCharacterRef.Get()->UpdateCharacterState(characterState);

	timerDelegate.BindUFunction(this, FName("ResetStatDebuffState"), DebuffType, OwnerCharacterRef.Get(), Variable);
	GamemodeRef.Get()->GetWorldTimerManager().SetTimer(timerHandle, timerDelegate, 0.1f, false, TotalTime);

	return true;
}

void UDebuffManagerComponent::ClearDebuffManagerTimer()
{

}

void UDebuffManagerComponent::ClearDebuffTimer(ECharacterDebuffType DebuffType)
{

}

bool UDebuffManagerComponent::GetDebuffIsActive(ECharacterDebuffType DebuffType)
{
	return ResetTimerMap.Contains(DebuffType);
}

void UDebuffManagerComponent::InitDebuffManager()
{
	//Initialize the gamemode ref
	GamemodeRef = GetWorld()->GetAuthGameMode<ABackStreetGameModeBase>();

	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
}

FTimerHandle UDebuffManagerComponent::GetResetTimerHandle(ECharacterDebuffType DebuffType)
{
	if (!ResetTimerMap.Contains(DebuffType)) return FTimerHandle();
	return ResetTimerMap[DebuffType];
}

FTimerHandle UDebuffManagerComponent::GetDotDamageTimerHandle(ECharacterDebuffType DebuffType)
{
	if (!DotDamageTimerMap.Contains(DebuffType)) return FTimerHandle();
	return DotDamageTimerMap[DebuffType];
}

float UDebuffManagerComponent::GetDebuffResetValue(ECharacterDebuffType DebuffType)
{
	if (!ResetValueInfoMap.Contains(DebuffType)) return 0.0f;
	return ResetValueInfoMap[DebuffType];
}

void UDebuffManagerComponent::ResetStatDebuffState(ECharacterDebuffType DebuffType, float ResetVal)
{

}

