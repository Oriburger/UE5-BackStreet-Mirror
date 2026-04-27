// Fill out your copyright notice in the Description page of Project Settings.


#include "DebuffManagerComponent.h"
#include "../CharacterBase.h"
#include "../EnemyCharacter/EnemyCharacterBase.h"
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

void UDebuffManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearDebuffManager();
	Super::EndPlay(EndPlayReason);
}

void UDebuffManagerComponent::SetDebuffSound(FDebuffInfoStruct DebuffInfo, bool bIsStrongDebuff)
{
	if (!OwnerCharacterRef.IsValid() || DebuffInfo.Type == ECharacterDebuffType::E_None) return;
	if (OwnerCharacterRef.Get()->GetIsActionActive(ECharacterActionType::E_Die)) return;

	USoundCue* startSound;
	USoundCue* loopSound;
	GetDebuffSound(DebuffInfo.Type, startSound, loopSound);

	// 시작 사운드 재생
	if (IsValid(startSound))
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), startSound, GetOwner()->GetActorLocation(), 0.3f);
	}

	switch (DebuffInfo.Type)
	{
		//---- 추후에 특수효과 디버프 효과 추가-------------------
	case ECharacterDebuffType::E_Burn:
		OwnerCharacterRef.Get()->ActivateDebuffNiagara((uint8)DebuffInfo.Type);
		break;
	case ECharacterDebuffType::E_Poison:
		OwnerCharacterRef.Get()->ActivateDebuffNiagara((uint8)DebuffInfo.Type);
		break;
	case ECharacterDebuffType::E_FrameDrop:
		if (bIsStrongDebuff)
		{
			OwnerCharacterRef.Get()->ActivateDebuffNiagara((uint8)ECharacterDebuffType::E_Stun);
		}
		else
		{
			OwnerCharacterRef.Get()->ActivateDebuffNiagara((uint8)ECharacterDebuffType::E_Slow);
		}
		break;
	default:
		break;
	}

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
}

/* =====================================================================================
* 디버프 스택 추가 함수
* @ 황유호 
* 캐릭터에 디버프 종류에 맞는 스택을 추가하는 함수. 디버프 스택이 최대치에 도달하면 강력한 디버프를 적용하고 스택을 초기화.
======================================================================================== */
void UDebuffManagerComponent::ApplyDebuffStack(FDebuffInfoStruct DebuffInfo, AActor* Causer)
{
	if (!OwnerCharacterRef.IsValid() || DebuffInfo.Type == ECharacterDebuffType::E_None) return;
	if (DebuffInfo.StackAmount == 0.0f) return;
	
	//get character info and default stat ref
	FCharacterGameplayInfo& characterInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();

	FTimerDelegate decayTimerDelegate;			//timer delegate setup for stack decay

	float defaultMaxDebuffStack = 0.0f;
	float totalCurrentStack = 0.0f;

	if (OwnerCharacterRef.Get()->GetWorldTimerManager().IsTimerActive(DebuffStackLockTimerHandle))	return;

	if(!ResetTimerMap.Contains(DebuffInfo.Type)) ResetTimerMap.Add(DebuffInfo.Type, FTimerHandle());			//타이머 핸들 contains 체크 후 추가

	if (DecayTimerMap.Contains(DebuffInfo.Type) && GetWorld()->GetTimerManager().IsTimerActive(DecayTimerMap[DebuffInfo.Type]))
		GetWorld()->GetTimerManager().ClearTimer(DecayTimerMap[DebuffInfo.Type]);								//스택 감소 타이머 비활성화

	if (GetWorld()->GetTimerManager().IsTimerActive(ResetTimerMap[DebuffInfo.Type]))
	{
		GetWorld()->GetTimerManager().ClearTimer(ResetTimerMap[DebuffInfo.Type]);								
	}
	decayTimerDelegate.BindUFunction(this, FName("StartStackDecay"), DebuffInfo.Type);							//디버프 스택 감소 시작 함수 바인딩
	GetWorld()->GetTimerManager().SetTimer(ResetTimerMap[DebuffInfo.Type], decayTimerDelegate, 3.0f, false);	//3초 후 디버프 스택 감소 시작

	//적 캐릭터 디버프 스택 시스템 적용
	if(OwnerCharacterRef.Get()->ActorHasTag("Enemy"))
	{
		switch (DebuffInfo.Type)
		{
		case ECharacterDebuffType::E_Burn:
			defaultMaxDebuffStack = characterInfo.GetTotalValue(ECharacterStatType::E_MaxFlameStack);
			totalCurrentStack = FMath::Clamp(characterInfo.CurrentFlameStack + DebuffInfo.StackAmount, 0.0f, defaultMaxDebuffStack);
			characterInfo.CurrentFlameStack = totalCurrentStack;
			
			if (characterInfo.CurrentFlameStack >= defaultMaxDebuffStack)
			{
				ActivateDebuff(DebuffInfo, true);
				totalCurrentStack = 0.0f;
				characterInfo.CurrentFlameStack = totalCurrentStack;
			}
			else if (characterInfo.CurrentFlameStack >= defaultMaxDebuffStack * 0.5f)
			{
				FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffInfo.Type);
				
				//apply weak burn debuff
				if (!GetWorld()->GetTimerManager().IsTimerActive(dotDamageHandle))
				{
					ActivateDebuff(DebuffInfo, false);
				}
			}
			break; 
			 
		case ECharacterDebuffType::E_Poison:
			defaultMaxDebuffStack = characterInfo.GetTotalValue(ECharacterStatType::E_MaxPoisonStack);
			totalCurrentStack = FMath::Clamp(characterInfo.CurrentPoisonStack + DebuffInfo.StackAmount, 0.0f, defaultMaxDebuffStack);
			characterInfo.CurrentPoisonStack = totalCurrentStack;

			if (characterInfo.CurrentPoisonStack >= defaultMaxDebuffStack)
			{
				ActivateDebuff(DebuffInfo, true);
				totalCurrentStack = 0.0f;
				characterInfo.CurrentPoisonStack = totalCurrentStack;
			}
			else if (characterInfo.CurrentPoisonStack >= defaultMaxDebuffStack * 0.5f)
			{
				//if(약한 디버프가 적용안되어 있다면)
				ActivateDebuff(DebuffInfo, false);
			}
			break;
			          
		case ECharacterDebuffType::E_FrameDrop:
			defaultMaxDebuffStack = characterInfo.GetTotalValue(ECharacterStatType::E_MaxFrameDropStack);
			totalCurrentStack = FMath::Clamp(characterInfo.CurrentframeDropStack + DebuffInfo.StackAmount, 0.0f, defaultMaxDebuffStack);
			characterInfo.CurrentframeDropStack = totalCurrentStack;

			if (characterInfo.CurrentframeDropStack >= defaultMaxDebuffStack)
			{
				ActivateDebuff(DebuffInfo, true);
				totalCurrentStack = 0.0f;
				characterInfo.CurrentframeDropStack = totalCurrentStack;
			}
			else if (characterInfo.CurrentframeDropStack >= defaultMaxDebuffStack * 0.5f)
			{
				ActivateDebuff(DebuffInfo, false);
			}
			break;

		case ECharacterDebuffType::E_AttackDown:
			break;

		case ECharacterDebuffType::E_Slow:
			break;

		case ECharacterDebuffType::E_Stun:
			break;
		}
	}
	//플레이어 캐릭터 디버프 스택 시스템 적용
	else if (OwnerCharacterRef.Get()->ActorHasTag("Player"))
	{
		switch (DebuffInfo.Type)
		{
		case ECharacterDebuffType::E_Burn:
			defaultMaxDebuffStack = characterInfo.GetTotalValue(ECharacterStatType::E_MaxFlameStack);
			totalCurrentStack = FMath::Clamp(characterInfo.CurrentFlameStack + DebuffInfo.StackAmount, 0.0f, defaultMaxDebuffStack);
			characterInfo.CurrentFlameStack = totalCurrentStack;

			if (characterInfo.CurrentFlameStack >= defaultMaxDebuffStack)
			{
				FTimerHandle& timerHandle = GetResetTimerHandle(DebuffInfo.Type);
				FTimerDelegate timerDelegate;
				totalCurrentStack = 0.0f;
				characterInfo.CurrentFlameStack = totalCurrentStack;

				ActivateDebuff(DebuffInfo, false);													//최대 스택 도달 시 화염 지속효과(과열) 적용 후 스택 초기화
				timerDelegate.BindUFunction(this, FName("ClearDebuffTimer"), DebuffInfo.Type);		//디버프 타이머 초기화 함수 바인딩
				GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, 10.0f, false);	//10초 후 디버프 타이머 초기화
			}
			break;

		case ECharacterDebuffType::E_Poison:
			defaultMaxDebuffStack = characterInfo.GetTotalValue(ECharacterStatType::E_MaxPoisonStack);
			totalCurrentStack = FMath::Clamp(characterInfo.CurrentPoisonStack + DebuffInfo.StackAmount, 0.0f, defaultMaxDebuffStack);
			characterInfo.CurrentPoisonStack = totalCurrentStack;

			if (characterInfo.CurrentPoisonStack >= defaultMaxDebuffStack)
			{
				ActivateDebuff(DebuffInfo, false);
				totalCurrentStack = 0.0f;
				characterInfo.CurrentPoisonStack = totalCurrentStack;
			}
			break;

		case ECharacterDebuffType::E_FrameDrop:
			defaultMaxDebuffStack = characterInfo.GetTotalValue(ECharacterStatType::E_MaxFrameDropStack);
			totalCurrentStack = FMath::Clamp(characterInfo.CurrentframeDropStack + DebuffInfo.StackAmount, 0.0f, defaultMaxDebuffStack);
			characterInfo.CurrentframeDropStack = totalCurrentStack;

			if (characterInfo.CurrentframeDropStack >= defaultMaxDebuffStack)
			{
				ActivateDebuff(DebuffInfo, true);
				totalCurrentStack = 0.0f;
				characterInfo.CurrentframeDropStack = totalCurrentStack;
			}
			break;

		}

	}

	float totalDebuffRatio = CalculateDebuffStackPercentage(characterInfo, totalCurrentStack);
	UE_LOG(LogTemp, Warning, TEXT("ApplyDebuffStack() totalCurrentStack : %.2f"), totalCurrentStack);

	if (totalDebuffRatio < 0.0f) return;
	OnDebuffStackChanged.Broadcast(DebuffInfo.Type, totalDebuffRatio);

	if (!ResetValueInfoMap.Contains(DebuffInfo.Type))
		ResetValueInfoMap.Add(DebuffInfo.Type, DebuffInfo);
	ResetValueInfoMap[DebuffInfo.Type] = DebuffInfo;
	
}

/* =====================================================================================
* 디버프 활성화 함수
* @ 황유호
* 캐릭터에 디버프 종류에 맞는 디버프를 활성화하는 함수. 강력한 디버프 여부에 따라 다른 효과 적용.
======================================================================================== */
void UDebuffManagerComponent::ActivateDebuff(FDebuffInfoStruct DebuffInfo, bool bIsStrongDebuff)
{
	if (!OwnerCharacterRef.IsValid() || DebuffInfo.Type == ECharacterDebuffType::E_None) return;
	
	FCharacterGameplayInfo& characterInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();

	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffInfo.Type);
	FTimerHandle& timerHandle = GetResetTimerHandle(DebuffInfo.Type);
	FTimerDelegate timerDelegate, dotDamageDelegate;

	TArray<AActor*> overlappedActors;
	float totalDamage = 0.0f;

	//강력한 디버프가 아닌 경우 일반 디버프 적용
	if (!bIsStrongDebuff)
	{
		switch (DebuffInfo.Type)
		{
		case ECharacterDebuffType::E_Burn:
			totalDamage = (float)characterInfo.GetTotalValue(ECharacterStatType::E_MaxHealth) * 0.01f;	//화상 디버프는 최대 체력의 1% 고정 도트 데미지
			dotDamageDelegate.BindUFunction(this, FName("ApplyDotDamage"), DebuffInfo.Type, totalDamage, OwnerCharacterRef.Get());
			GetWorld()->GetTimerManager().SetTimer(dotDamageHandle, dotDamageDelegate, 1.0f, true);
			break;

		case ECharacterDebuffType::E_Poison:
			totalDamage = characterInfo.GetTotalValue(ECharacterStatType::E_MaxHealth) * 0.25f;					//산화 디버프는 최대 체력의 25% 고정 데미지
			OwnerCharacterRef.Get()->TakeDebuffDamage(DebuffInfo.Type, totalDamage, OwnerCharacterRef.Get());
			break;

		case ECharacterDebuffType::E_FrameDrop:
			characterInfo.SetDebuffStatInfo(ECharacterStatType::E_MoveSpeed, 0.5f);
			break;

		default:
			break;
		}
	}

	else {
		
		if (DebuffInfo.Type == ECharacterDebuffType::E_Burn || DebuffInfo.Type == ECharacterDebuffType::E_Poison)
		{
			overlappedActors = CheckSphereOverlap(500.0f);	// 500 유닛 반경 내의 액터 체크
		}

		switch (DebuffInfo.Type)
		{
		case ECharacterDebuffType::E_Burn:
		{
			for (AActor* actor : overlappedActors)
			{
				if (actor != OwnerCharacterRef.Get() && actor->ActorHasTag("Enemy"))
				{
					ACharacterBase* targetCharacter = Cast<ACharacterBase>(actor);
					if (IsValid(targetCharacter))
					{
						targetCharacter->ApplyDebuffStack(FDebuffInfoStruct(DebuffInfo.Type, (float)characterInfo.GetTotalValue(ECharacterStatType::E_MaxFlameStack) * 0.25f), OwnerCharacterRef.Get());	//최대 발화 스택의 25% 스택 추가
					}

				}
			}
			break;
		}

		case ECharacterDebuffType::E_Poison:

			totalDamage = (float)characterInfo.GetTotalValue(ECharacterStatType::E_MaxHealth) * 0.5f;	//산화 특수효과 디버프는 최대 체력의 50% 고정 데미지

			for (AActor* actor : overlappedActors)
			{
				if (actor != OwnerCharacterRef.Get() && actor->ActorHasTag("Enemy"))
				{
					AEnemyCharacterBase* targetCharacter = Cast<AEnemyCharacterBase>(actor);
					if (IsValid(targetCharacter))
					{
						targetCharacter->TakeDebuffDamage(DebuffInfo.Type, totalDamage, OwnerCharacterRef.Get());
					}
				}
				AEnemyCharacterBase* ownerEnemyCharacter = Cast<AEnemyCharacterBase>(OwnerCharacterRef.Get());
				if (IsValid(ownerEnemyCharacter))
				{
					ownerEnemyCharacter->TakeDebuffDamage(DebuffInfo.Type, totalDamage, OwnerCharacterRef.Get());
				}
			}

			break;

		case ECharacterDebuffType::E_FrameDrop:
			OwnerCharacterRef.Get()->ResetActionState(true);
			characterInfo.CharacterActionState = ECharacterActionType::E_Stun;
			if (OwnerCharacterRef->ActorHasTag("Enemy"))
			{
				OwnerCharacterRef.Get()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
			}
			timerDelegate.BindUFunction(this, FName("ResetStatDebuffState"), DebuffInfo.Type, DebuffInfo);
			GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, 5.0f, false);
			break;

		case ECharacterDebuffType::E_AttackDown:
			characterInfo.SetDebuffStatInfo(ECharacterStatType::E_NormalPower, 0.25f);
			characterInfo.SetDebuffStatInfo(ECharacterStatType::E_DashAttackPower, 0.25f);
			characterInfo.SetDebuffStatInfo(ECharacterStatType::E_JumpAttackPower, 0.25f);
			break;

		case ECharacterDebuffType::E_DefenseDown:
			characterInfo.SetDebuffStatInfo(ECharacterStatType::E_Defense, 0.25f);
			break;

		case ECharacterDebuffType::E_Slow:
			characterInfo.SetDebuffStatInfo(ECharacterStatType::E_MoveSpeed, 0.25f);
			break;

		case ECharacterDebuffType::E_Stun:
			OwnerCharacterRef.Get()->ResetActionState(true);
			characterInfo.CharacterActionState = ECharacterActionType::E_Stun;
			if (OwnerCharacterRef->ActorHasTag("Enemy"))
			{
				OwnerCharacterRef.Get()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
			}
			timerDelegate.BindUFunction(this, FName("ResetStatDebuffState"), DebuffInfo.Type, DebuffInfo);
			GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, 5.0f, false);
			break;

		default:
			break;
		}

		UE_LOG(LogTemp, Warning, TEXT("DebuffStackLockTimerHandle Start"));
		OwnerCharacterRef.Get()->GetWorldTimerManager().SetTimer(
			DebuffStackLockTimerHandle,
			this,
			&UDebuffManagerComponent::OnStackLockTimerFinished,
			3.0f,
			false
		);
		OnDebuffStackLock.Broadcast(DebuffInfo.Type, 3.0f);
	}

	SetDebuffSound(DebuffInfo, bIsStrongDebuff);
}

/* =====================================================================================
* 디버프 스택 감소 시작 함수
* @ 황유호
* 캐릭터에 디버프 종류에 맞는 스택을 감소시키기 위해 UpdateStackDecay() 함수를 주기적으로 호출하는 타이머를 시작.
======================================================================================== */
void UDebuffManagerComponent::StartStackDecay(ECharacterDebuffType DebuffType)
{
	if (DebuffType == ECharacterDebuffType::E_None) return;

	FTimerDelegate timerDelegate;
	timerDelegate.BindUFunction(this, FName("UpdateStackDecay"), DebuffType);

	if (!DecayTimerMap.Contains(DebuffType)) DecayTimerMap.Add(DebuffType, FTimerHandle());		//타이머 핸들 contains 체크 후 추가
	GetWorld()->GetTimerManager().SetTimer(DecayTimerMap[DebuffType], timerDelegate, 0.1f, true);
}

/* =====================================================================================
* 디버프 스택 업데이트 함수
* @ 황유호
* 캐릭터에 디버프 종류에 맞는 CurrentStack(현재 스택 수치)을 감소 및 0으로 초기화 시키는 함수.
======================================================================================== */
void UDebuffManagerComponent::UpdateStackDecay(ECharacterDebuffType DebuffType)
{
	if (!OwnerCharacterRef.IsValid() || DebuffType == ECharacterDebuffType::E_None) return;

	FCharacterGameplayInfo& characterInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();
	float totalCurrentStack = 0.0f;

	switch (DebuffType) 
	{
	case ECharacterDebuffType::E_Burn:
		if (characterInfo.CurrentFlameStack > 0.0f)
		{
			characterInfo.CurrentFlameStack -= 1.0f;											// 초당 10씩 감소 (0.1초당 1)
			totalCurrentStack = characterInfo.CurrentFlameStack;

			if (characterInfo.CurrentFlameStack <= 0.0f)
			{
				totalCurrentStack = 0.0f;
				characterInfo.CurrentFlameStack = totalCurrentStack;
				GetWorld()->GetTimerManager().ClearTimer(DecayTimerMap[DebuffType]);			//디버프 감소 타이머 클리어
				GetWorld()->GetTimerManager().ClearTimer(GetDotDamageTimerHandle(DebuffType));	//화상 도트 데미지 타이머도 클리어

				ClearDebuffTimer(DebuffType);

				OnDebuffDeactivated.Broadcast(DebuffType);										//디버프 제거 이벤트 브로드캐스트
			}
		}
		break;

	case ECharacterDebuffType::E_Poison:
		if (characterInfo.CurrentPoisonStack > 0.0f)
		{
			characterInfo.CurrentPoisonStack -= 1.0f;
			totalCurrentStack = characterInfo.CurrentPoisonStack;

			if (characterInfo.CurrentPoisonStack <= 0.0f)
			{
				totalCurrentStack = 0.0f;
				characterInfo.CurrentPoisonStack = totalCurrentStack;
				GetWorld()->GetTimerManager().ClearTimer(DecayTimerMap[DebuffType]);

				ClearDebuffTimer(DebuffType);

				OnDebuffDeactivated.Broadcast(DebuffType);
			}
		}
		break;

	case ECharacterDebuffType::E_FrameDrop:

		if (characterInfo.CurrentframeDropStack > 0.0f)
		{
			characterInfo.CurrentframeDropStack -= 1.0f;
			totalCurrentStack = characterInfo.CurrentframeDropStack;

			if (characterInfo.CurrentframeDropStack <= 0.0f)
			{
				float defaultMoveSpeed = OwnerCharacterRef.Get()->GetCharacterDefaultStatRef().DefaultMoveSpeed;	//이동 속도 원래대로 복구하기 위한 default move speed 참조

				totalCurrentStack = 0.0f;
				characterInfo.CurrentframeDropStack = totalCurrentStack;
				GetWorld()->GetTimerManager().ClearTimer(DecayTimerMap[DebuffType]);
				
				if (defaultMoveSpeed >= OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_MoveSpeed))
				{
					characterInfo.SetDebuffStatInfo(ECharacterStatType::E_MoveSpeed, 0.0f);							//프레임 드롭 디버프가 사라질 때 이동 속도 원래대로 복구
					OwnerCharacterRef.Get()->GetCharacterMovement()->MaxWalkSpeed = characterInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed);

					ClearDebuffTimer(DebuffType);

					OnDebuffDeactivated.Broadcast(DebuffType);
				}
			}
		}
		break;
	}

	float totalDebuffRatio = CalculateDebuffStackPercentage(characterInfo, totalCurrentStack);
	if (totalDebuffRatio < 0.0f) return;
	OnDebuffStackChanged.Broadcast(DebuffType, totalDebuffRatio);
}

TArray<AActor*> UDebuffManagerComponent::CheckSphereOverlap(float SphereRadius)
{
	FVector sphereStartLocation;
	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	TArray<AActor*> actorsToIgnore;
	TArray<AActor*> sphereOverlapActors;
	TArray<AActor*> targetList;

	actorsToIgnore.Add(OwnerCharacterRef.Get());
	sphereStartLocation = OwnerCharacterRef.Get()->GetActorLocation();
	objectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	bool result = UKismetSystemLibrary::SphereOverlapActors(GetWorld(), sphereStartLocation, SphereRadius, objectTypes, nullptr, actorsToIgnore, sphereOverlapActors);
	if (result)
	{
		for (AActor* overlapActor : sphereOverlapActors)
		{
			if (!IsValid(overlapActor)) continue;
			targetList.Add(overlapActor);
		}
	}

	return targetList;
}


//==============================================================================================
//==============================================================================================



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
	DecayTimerMap.Empty();
}

void UDebuffManagerComponent::ClearDebuffTimer(ECharacterDebuffType DebuffType)
{
	FTimerHandle& dotDamageHandle = GetDotDamageTimerHandle(DebuffType);
	FTimerHandle& resetHandle = GetResetTimerHandle(DebuffType);
	FTimerHandle& decayHandle = GetDecayTimerHandle(DebuffType);

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
	GetWorld()->GetTimerManager().ClearTimer(decayHandle);
	dotDamageHandle.Invalidate();
	resetHandle.Invalidate();
	decayHandle.Invalidate();

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

/*================================ Get Timer Map ===================================*/

FTimerHandle& UDebuffManagerComponent::GetDecayTimerHandle(ECharacterDebuffType DebuffType)
{
	if (!DecayTimerMap.Contains(DebuffType)) DecayTimerMap.Add(DebuffType, FTimerHandle());
	return DecayTimerMap[DebuffType];
}

FTimerHandle& UDebuffManagerComponent::GetDeActivateDebuffTimerMap(ECharacterDebuffType DebuffType)
{
	if (!DeactivateDebuffTimerMap.Contains(DebuffType)) DeactivateDebuffTimerMap.Add(DebuffType, FTimerHandle());
	return DeactivateDebuffTimerMap[DebuffType];
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

void UDebuffManagerComponent::OnStackLockTimerFinished()
{
}

FDebuffInfoStruct UDebuffManagerComponent::GetDebuffResetValue(ECharacterDebuffType DebuffType)
{
	if (!ResetValueInfoMap.Contains(DebuffType)) return {};
	return ResetValueInfoMap[DebuffType];
}

float UDebuffManagerComponent::CalculateDebuffStackPercentage(FCharacterGameplayInfo& CharacterInfo, float StackAmount)
{
	if (!OwnerCharacterRef.IsValid() || StackAmount < 0.0f) return -1.0f;
	float debuffStackRatio = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, CharacterInfo.GetTotalValue(ECharacterStatType::E_MaxFlameStack)), FVector2D(0.0f, 1.0f), StackAmount);
	return debuffStackRatio;
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