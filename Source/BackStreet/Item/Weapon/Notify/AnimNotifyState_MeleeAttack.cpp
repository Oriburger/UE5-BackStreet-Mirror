// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_MeleeAttack.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Character/Component/WeaponComponentBase.h"
#include "../../../Character/Component/ActionTrackingComponent.h"
#include "../../../System/AssetSystem/AssetManagerBase.h"
#include "../../../Character/Component/SkillManagerComponentBase.h"
#include "Kismet/KismetMathLibrary.h"
#define MAX_LINETRACE_POS_COUNT 6
#define DEFAULT_MELEE_ATK_RANGE 150.0f

void UAnimNotifyState_MeleeAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!IsValid(MeshComp)|| !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;
	
	//Initialize
	OwnerCharacterRef = Cast<ACharacterBase>(MeshComp->GetOwner());
	WeaponComponentRef = OwnerCharacterRef.IsValid() ? OwnerCharacterRef.Get()->WeaponComponent : nullptr;
	AssetManagerRef = GamemodeRef.IsValid() ? GamemodeRef.Get()->GetGlobalAssetManagerBaseRef() : nullptr;
	GamemodeRef = OwnerCharacterRef.IsValid() ? Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(OwnerCharacterRef.Get()->GetWorld())) : nullptr;

	if (GamemodeRef.IsValid())
	{
		if (MeleeLineTraceQueryParams.GetIgnoredActors().Num() == 0)
		{
			MeleeLineTraceQueryParams.AddIgnoredActor(OwnerCharacterRef.Get());
		}
	}
}

void UAnimNotifyState_MeleeAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (!OwnerCharacterRef.IsValid()) return;

	MeleeAttack();
}

void UAnimNotifyState_MeleeAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!OwnerCharacterRef.IsValid()) return;
	
	MeleePrevTracePointList.Empty();
	MeleeLineTraceQueryParams.ClearIgnoredActors();
	IgnoreActorList.Empty();
}

void UAnimNotifyState_MeleeAttack::MeleeAttack()
{
	if (!WeaponComponentRef.IsValid()) return;
	FHitResult hitResult;
	bool bIsMeleeTraceSucceed = false;
	bool bIsJumpAttacking = OwnerCharacterRef.Get()->ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_JumpAttack)
		|| OwnerCharacterRef.Get()->ActionTrackingComponent->GetIsActionRecentlyFinished(ECharacterActionType::E_JumpAttack);
	bool bIsDashAttacking = OwnerCharacterRef.Get()->ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_DashAttack)
		|| OwnerCharacterRef.Get()->ActionTrackingComponent->GetIsActionRecentlyFinished(ECharacterActionType::E_DashAttack);

	FCharacterGameplayInfo ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfo();

	TArray<FVector> currTracePositionList = GetCurrentMeleePointList();
	TArray<AActor*> targetList = CheckMeleeAttackTargetWithSphereTrace();
	MeleePrevTracePointList = currTracePositionList;

	for (auto& target : targetList)
	{
		//if target is valid, apply damage
		if (IsValid(target))
		{
			//캐릭터가 아닌 대상에 데미지를 주는 경우
			if (!target->ActorHasTag("Character"))
			{
				UGameplayStatics::ApplyDamage(target, 10.0f, OwnerCharacterRef.Get()->GetController(), OwnerCharacterRef.Get(), nullptr);
				MeleeLineTraceQueryParams.AddIgnoredActor(target);
				continue;
			}

			FCharacterGameplayInfo targetInfo = Cast<ACharacterBase>(target)->GetCharacterGameplayInfo();
			bool bIsFatalAttack = false;
			float totalDamage = WeaponComponentRef.Get()->CalculateTotalDamage(targetInfo, bIsFatalAttack);

			//Apply Debuff
			const TArray<ECharacterDebuffType> targetDebuffTypeList = { ECharacterDebuffType::E_Burn, ECharacterDebuffType::E_Poison
																		, ECharacterDebuffType::E_Stun, ECharacterDebuffType::E_Slow };
			for (auto& debuffType : targetDebuffTypeList)
			{
				ECharacterStatType targetStatType = FCharacterGameplayInfo::GetDebuffStatType(bIsJumpAttacking, bIsDashAttacking, debuffType);
				if (targetStatType == ECharacterStatType::E_None) continue;

				bool result = WeaponComponentRef.Get()->ApplyWeaponDebuff(Cast<ACharacterBase>(target), debuffType, targetStatType);
				if (result)
				{
					UE_LOG(LogWeapon, Warning, TEXT("UAnimNotifyState_MeleeAttack() - bIsJumpAttack : %d,  bIsDashAttack : %d,  Debuff Stat Type : %d"), (int32)bIsJumpAttacking, (int32)bIsDashAttacking, (int32)targetStatType);
				}
			}

			//Activate Melee Hit Effect
			ActivateMeleeHitEffect(target->GetActorLocation(), target, bIsFatalAttack);

			//Apply Knockback
			if (!target->ActorHasTag("Boss") && !ownerInfo.bIsAirAttacking)
			{
				OwnerCharacterRef.Get()->ApplyKnockBack(target, WeaponComponentRef.Get()->WeaponStat.WeaponKnockBackStrength);
			}

			//Apply Damage
			UGameplayStatics::ApplyDamage(target, totalDamage, OwnerCharacterRef.Get()->GetController(), OwnerCharacterRef.Get(), nullptr);
			MeleeLineTraceQueryParams.AddIgnoredActor(target);
		}
	}
}

TArray<AActor*> UAnimNotifyState_MeleeAttack::CheckMeleeAttackTargetWithSphereTrace()
{
	if (!OwnerCharacterRef.IsValid()) return TArray<AActor*>();

	FVector traceStartPos = (WeaponComponentRef.Get()->GetSocketLocation("GrabPoint")
		+ WeaponComponentRef.Get()->GetSocketLocation("End")) / 2;
	float traceRadius = UKismetMathLibrary::Vector_Distance(WeaponComponentRef.Get()->GetSocketLocation("GrabPoint")
		, WeaponComponentRef.Get()->GetSocketLocation("End")) + 35.0f;

	TEnumAsByte<EObjectTypeQuery> pawnTypeQuery = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn);
	TEnumAsByte<EObjectTypeQuery> interactiveTypeQuery = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	TArray<AActor*> overlapResultList, meleeDamageTargetList;

	UKismetSystemLibrary::SphereOverlapActors(OwnerCharacterRef.Get(), traceStartPos, traceRadius, { pawnTypeQuery, interactiveTypeQuery }
		, AActor::StaticClass(), IgnoreActorList, overlapResultList);

	if (bIsDebugMode)
	{
		//print pos 
		UE_LOG(LogWeapon, Warning, TEXT("UAnimNotifyState_MeleeAttack::CheckMeleeAttackTargetWithSphereTrace(), Trace Start Pos : %s, Radius : %f, overlap count : %d")
			, *traceStartPos.ToString(), traceRadius, overlapResultList.Num());
		UKismetSystemLibrary::DrawDebugSphere(OwnerCharacterRef.Get(), traceStartPos, traceRadius, 25, FLinearColor::Red, 10.0f, 1.0f);
	}

	//Find target to apply damage in overlap result array
	//Character - Check if there's any obstacle between causer and target.
	FCollisionQueryParams collisionQueryParam;
	collisionQueryParam.AddIgnoredActor(OwnerCharacterRef.Get());
	for (auto& target : overlapResultList)
	{
		if (!IsValid(target)) continue;
		//if (!target->ActorHasTag("Character")) continue;
		if (target->ActorHasTag(OwnerCharacterRef.Get()->Tags[1])) continue;

		FHitResult hitResult;
		OwnerCharacterRef.Get()->GetWorld()->LineTraceSingleByChannel(hitResult, OwnerCharacterRef->GetActorLocation()
			, target->GetActorLocation(), ECollisionChannel::ECC_Camera, collisionQueryParam);

		if (hitResult.bBlockingHit && hitResult.GetActor() == target)
		{
			IgnoreActorList.Add(target);
			meleeDamageTargetList.Add(target);

			if(bIsDebugMode)
			{
				UKismetSystemLibrary::DrawDebugLine(GetWorld(), OwnerCharacterRef->GetActorLocation()
					, target->GetActorLocation(), FLinearColor::Green, 2.0f, 2.0f);
			}
		}
	}
	return meleeDamageTargetList;
}

void UAnimNotifyState_MeleeAttack::ActivateMeleeHitEffect(const FVector& Location, AActor* AttachTarget, bool bImpactEffect)
{
	//Activate Slow Effect (Hit stop)
	if (OwnerCharacterRef.Get()->ActorHasTag("Player"))
	{
		const float dilationValue = bImpactEffect ? 0.2 : 0.2f;
		const float effectLength = bImpactEffect ? 0.15f : 0.0633f;
		GamemodeRef.Get()->ActivateSlowHitEffect(dilationValue, effectLength);
	}

	//Spawn emitter
	FTransform emitterSpawnTransform(FQuat(0.0f), Location, FVector(0.5f));
	FRotator randomRotator = FRotator::ZeroRotator;
	randomRotator.Pitch += UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f);
	randomRotator.Yaw += UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f);
	randomRotator.Roll += UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f);

	if (!bUseWeaponAsset)
	{
		if (IsValid(HitEffectNiagara) && IsValid(LargeHitEffectNiagara))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(OwnerCharacterRef.Get(), bImpactEffect ? LargeHitEffectNiagara : HitEffectNiagara, emitterSpawnTransform.GetLocation()
				, WeaponComponentRef.Get()->WeaponState.SlashRotation + randomRotator, emitterSpawnTransform.GetScale3D());
		}
		if (IsValid(HitEffectSound) && IsValid(LargeHitEffectSound))
		{
			UGameplayStatics::PlaySoundAtLocation(OwnerCharacterRef.Get(), bImpactEffect ? LargeHitEffectSound : HitEffectSound, Location);
		}
	}
	else
	{
		if (!bImpactEffect)
		{
			if (IsValid(WeaponComponentRef.Get()->HitEffectParticle))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(OwnerCharacterRef.Get(), WeaponComponentRef.Get()->HitEffectParticle, emitterSpawnTransform.GetLocation()
					, WeaponComponentRef.Get()->WeaponState.SlashRotation + randomRotator, emitterSpawnTransform.GetScale3D());
			}
			if (IsValid(WeaponComponentRef.Get()->HitEffectSound))
			{
				UGameplayStatics::PlaySoundAtLocation(OwnerCharacterRef.Get(), WeaponComponentRef.Get()->HitEffectSound, Location);
			}
		}
		else
		{
			if (IsValid(WeaponComponentRef.Get()->HitEffectParticleLarge))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(OwnerCharacterRef.Get(), WeaponComponentRef.Get()->HitEffectParticleLarge, emitterSpawnTransform.GetLocation()
					, WeaponComponentRef.Get()->WeaponState.SlashRotation + randomRotator, emitterSpawnTransform.GetScale3D());
			}
			if (IsValid(WeaponComponentRef.Get()->HitEffectSoundLarge))
			{
				UGameplayStatics::PlaySoundAtLocation(OwnerCharacterRef.Get(), WeaponComponentRef.Get()->HitEffectSoundLarge, Location);
			}
		}

	}

	//카메라 Shake 효과
	if (!OwnerCharacterRef.Get()->GetCharacterGameplayInfo().bIsInvincibility)
	{
		GamemodeRef.Get()->PlayCameraShakeEffect(OwnerCharacterRef.Get()->ActorHasTag("Player") ? ECameraShakeType::E_Attack : ECameraShakeType::E_Hit, Location);
	}
}

TArray<FVector> UAnimNotifyState_MeleeAttack::GetCurrentMeleePointList()
{
	if (WeaponComponentRef.IsValid() == false) return TArray<FVector>();
	TArray<FVector> resultPosList;

	resultPosList.Add(WeaponComponentRef.Get()->GetSocketLocation("GrabPoint"));
	resultPosList.Add(WeaponComponentRef.Get()->GetSocketLocation("End"));
	for (uint8 positionIdx = 1; positionIdx < MAX_LINETRACE_POS_COUNT - 1; positionIdx++)
	{
		FVector direction = resultPosList[1] - resultPosList[0];

		resultPosList.Add(resultPosList[0] + direction / MAX_LINETRACE_POS_COUNT * positionIdx);
	}
	return resultPosList;
}
