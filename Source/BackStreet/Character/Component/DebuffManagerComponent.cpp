// Fill out your copyright notice in the Description page of Project Settings.


#include "DebuffManagerComponent.h"
#include "../CharacterBase.h"
#include "../../System/AssetSystem/AssetManagerBase.h"
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
	if (!OwnerCharacterRef.IsValid() || DebuffInfo.Type == ECharacterDebuffType::E_None) return false;
	if (OwnerCharacterRef.Get()->GetIsActionActive(ECharacterActionType::E_Die)) return false;

	FTimerDelegate timerDelegate, dotDamageDelegate;
	FCharacterGameplayInfo& characterInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();

	FTimerHandle& timerHandle = GetResetTimerHandle(DebuffInfo.Type);
	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffInfo.Type);
	float resistValue = -1.0f;
	float totalDamage = 0.0f;

	USoundCue* startSound;
	USoundCue* loopSound;
	GetDebuffSound(DebuffInfo.Type, startSound, loopSound);

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

		return SetDebuffTimer({ DebuffInfo.Type, FMath::Min(DebuffInfo.TotalTime, MAX_DEBUFF_TIME)
								, DebuffInfo.Variable, DebuffInfo.bIsPercentage }, Causer);
	}

	// 시작 사운드 재생
	if (IsValid(startSound))
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), startSound, GetOwner()->GetActorLocation(), 0.3f);
	}
	// Niagara 디버프 이펙트 활성화
	OwnerCharacterRef.Get()->ActivateDebuffNiagara((uint8)DebuffInfo.Type);

	// 루프 사운드 추가 및 실행
	if (!DebuffLoopAudioMap.Contains(DebuffInfo.Type) && IsValid(loopSound))
	{
		UAudioComponent* loopAudioComp = UGameplayStatics::SpawnSoundAttached(
			loopSound,
			GetOwner()->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			true, // 루프 활성화
			0.5f
		);
		if (loopAudioComp)
		{
			DebuffLoopAudioMap.Add(DebuffInfo.Type, loopAudioComp);
		}
	}

	/*---- 디버프 타이머 세팅 ----------------------------*/
	DebuffInfo.Variable = FMath::Min(1.0f, FMath::Abs(DebuffInfo.Variable)); //값 정제
	characterInfo.CharacterDebuffState |= (1 << (int)DebuffInfo.Type);

	switch (DebuffInfo.Type)
	{
		//----데미지 디버프-------------------
	case ECharacterDebuffType::E_Burn:
		resistValue = characterInfo.GetTotalValue(ECharacterStatType::E_BurnResist);
	case ECharacterDebuffType::E_Poison:
		resistValue = resistValue != -1.0f ? resistValue : characterInfo.GetTotalValue(ECharacterStatType::E_PoisonResist);
		totalDamage = (float)(DebuffInfo.bIsPercentage ? characterInfo.GetTotalValue(ECharacterStatType::E_MaxHealth) * DebuffInfo.Variable : DebuffInfo.Variable);
		totalDamage -= totalDamage * resistValue;
		dotDamageDelegate.BindUFunction(this, FName("ApplyDotDamage"), DebuffInfo.Type, totalDamage, OwnerCharacterRef.Get());
		GetWorld()->GetTimerManager().SetTimer(dotDamageHandle, dotDamageDelegate, 1.0f, true);
		break;
	//----스탯 조정 디버프-------------------
	case ECharacterDebuffType::E_Stun:
		OwnerCharacterRef.Get()->ResetActionState(true);
		characterInfo.CharacterActionState = ECharacterActionType::E_Stun;
		if (OwnerCharacterRef->ActorHasTag("Enemy"))
		{
			OwnerCharacterRef.Get()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		}
		break;
	case ECharacterDebuffType::E_Slow:
		resistValue = characterInfo.GetTotalValue(ECharacterStatType::E_SlowResist);
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_MoveSpeed, DebuffInfo.Variable - resistValue);
		break;
	case ECharacterDebuffType::E_AttackDown:
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_NormalPower, DebuffInfo.Variable);
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_DashAttackPower, DebuffInfo.Variable);
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_JumpAttackPower, DebuffInfo.Variable);
		break;
	case ECharacterDebuffType::E_DefenseDown:
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_Defense, DebuffInfo.Variable);
		break;
	}
	if(!ResetValueInfoMap.Contains(DebuffInfo.Type))
		ResetValueInfoMap.Add(DebuffInfo.Type, DebuffInfo);
	ResetValueInfoMap[DebuffInfo.Type] = DebuffInfo;

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

	//루프 사운드 제거
	if (DebuffLoopAudioMap.Contains(DebuffType))
	{
		UAudioComponent* loopAudioComp = DebuffLoopAudioMap[DebuffType];
		if (IsValid(loopAudioComp))
		{
			loopAudioComp->Stop();
			loopAudioComp->DestroyComponent();
		}
		// 맵에서 제거
		DebuffLoopAudioMap.Remove(DebuffType);
	}

	GetWorld()->GetTimerManager().ClearTimer(dotDamageHandle);
	GetWorld()->GetTimerManager().ClearTimer(resetHandle);
	dotDamageHandle.Invalidate();
	resetHandle.Invalidate();

	OwnerCharacterRef.Get()->DeactivateBuffEffect();
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
	//Initialize the owner character ref and gamemode ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
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
	if (!OwnerCharacterRef.IsValid() || DebuffType == ECharacterDebuffType::E_None) return;
	if (OwnerCharacterRef.Get()->GetIsActionActive(ECharacterActionType::E_Die)) return;

	FCharacterGameplayInfo& characterInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();

	FTimerHandle& timerHandle = GetResetTimerHandle(DebuffType);
	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffType);

	float ResetVal = FMath::Max(0.001f, DebuffInfo.Variable);
	characterInfo.CharacterDebuffState &= ~(1 << (int)DebuffType);

	UE_LOG(LogTemp, Warning, TEXT("UDebuffManagerComponent::ResetStatDebuffState - type : %d"), (int32)DebuffType);

	switch ((ECharacterDebuffType)DebuffType)
	{
	case ECharacterDebuffType::E_Burn:
	case ECharacterDebuffType::E_Poison:
		break;
	case ECharacterDebuffType::E_Slow:
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_MoveSpeed, 0);
		break;
	case ECharacterDebuffType::E_AttackDown:
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_NormalPower, 0);
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_DashAttackPower, 0);
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_JumpAttackPower, 0);
		break;
	case ECharacterDebuffType::E_DefenseDown:
		characterInfo.SetDebuffStatInfo(ECharacterStatType::E_Defense, 0);
		break;
	case ECharacterDebuffType::E_Stun:
		characterInfo.CharacterActionState = ECharacterActionType::E_Idle;
		OwnerCharacterRef.Get()->ResetActionState(true);
		//if(OwnerCharacterRef.Get()->ActorHasTag("Player"))
		//	OwnerCharacterRef.Get()->EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		//else if (OwnerCharacterRef.Get()->ActorHasTag("Enemy"))
		OwnerCharacterRef.Get()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		break;
	}
	ClearDebuffTimer(DebuffType);
}

void UDebuffManagerComponent::GetDebuffSound(ECharacterDebuffType DebuffType, USoundCue*& StartSound, USoundCue*& LoopSound)
{
	if (!GamemodeRef.IsValid()) return;

	//초기화
	StartSound = LoopSound = nullptr;

	AssetManagerBaseRef = GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();
	if (AssetManagerBaseRef.IsValid())
	{
		FString debuffString = UEnum::GetValueAsString<ECharacterDebuffType>(TEnumAsByte<ECharacterDebuffType>(DebuffType));
		debuffString.RemoveFromStart("ECharacterDebuffType::E_");

		StartSound = AssetManagerBaseRef.Get()->GetSingleSound(ESoundAssetType::E_System, 3000, FName(debuffString + "_Start"));
		LoopSound = AssetManagerBaseRef.Get()->GetSingleSound(ESoundAssetType::E_System, 3000, FName(debuffString + "_Loop"));
	}
}