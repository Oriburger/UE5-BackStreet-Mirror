// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeCombatManager.h"
#include "../CombatManager.h"
#include "../../../System/AssetSystem/AssetManagerBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Character/Component/ActionTrackingComponent.h"
#include "../../../Character/Component/WeaponComponentBase.h"
#include "../../../System/AssetSystem/AssetManagerBase.h"
#include "Kismet/KismetMathLibrary.h"
#define MAX_LINETRACE_POS_COUNT 6
#define DEFAULT_MELEE_ATK_RANGE 150.0f

UMeleeCombatManager::UMeleeCombatManager()
{
}

void UMeleeCombatManager::Attack()
{
	Super::Attack();

	if (AssetManagerRef.IsValid())
	{
		AssetManagerRef.Get()->PlaySingleSound(OwnerCharacterRef.Get(), ESoundAssetType::E_Weapon, WeaponComponentRef.Get()->WeaponID, "Wield");
	}

	GamemodeRef.Get()->GetWorldTimerManager().SetTimer(MeleeAtkTimerHandle, this, &UMeleeCombatManager::MeleeAttack, 0.01f, true);
	if (MeleeLineTraceQueryParams.GetIgnoredActors().Num() == 0)
	{
		MeleeLineTraceQueryParams.AddIgnoredActor(OwnerCharacterRef.Get());
	}
}

void UMeleeCombatManager::StopAttack()
{
	Super::StopAttack();

	GamemodeRef.Get()->GetWorldTimerManager().ClearTimer(MeleeAtkTimerHandle);
	MeleePrevTracePointList.Empty();
	MeleeLineTraceQueryParams.ClearIgnoredActors();
	IgnoreActorList.Empty();
}

TArray<AActor*> UMeleeCombatManager::CheckMeleeAttackTargetWithSphereTrace()
{
	if (!OwnerCharacterRef.IsValid()) return TArray<AActor*>();

	FVector traceStartPos = (WeaponComponentRef.Get()->GetSocketLocation("GrabPoint")
		+ WeaponComponentRef.Get()->GetSocketLocation("End")) / 2;
	float traceRadius = UKismetMathLibrary::Vector_Distance(WeaponComponentRef.Get()->GetSocketLocation("GrabPoint")
		, WeaponComponentRef.Get()->GetSocketLocation("End")) + 35.0f;

	TEnumAsByte<EObjectTypeQuery> pawnTypeQuery = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn);
	TArray<AActor*> overlapResultList, meleeDamageTargetList;

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), traceStartPos, traceRadius, { pawnTypeQuery }
	, ACharacterBase::StaticClass(), IgnoreActorList, overlapResultList);

	//Find target to apply damage in overlap result array
	//Character - Check if there's any obstacle between causer and target.
	FCollisionQueryParams collisionQueryParam;
	collisionQueryParam.AddIgnoredActor(OwnerCharacterRef.Get());
	for (auto& target : overlapResultList)
	{
		if (!IsValid(target)) continue;
		if (!target->ActorHasTag("Character")) continue;
		if (target->ActorHasTag(OwnerCharacterRef.Get()->Tags[1])) continue;

		FHitResult hitResult;
		GetWorld()->LineTraceSingleByChannel(hitResult, OwnerCharacterRef->GetActorLocation()
			, target->GetActorLocation(), ECollisionChannel::ECC_Camera, collisionQueryParam);

		if (hitResult.bBlockingHit && hitResult.GetActor() == target)
		{
			IgnoreActorList.Add(target);
			meleeDamageTargetList.Add(target);
		}
	}
	return meleeDamageTargetList;
}

void UMeleeCombatManager::MeleeAttack()
{
	if (!WeaponComponentRef.IsValid()) return;
	FHitResult hitResult;
	bool bIsFinalCombo = WeaponComponentRef.Get()->GetIsFinalCombo();
	bool bIsMeleeTraceSucceed = false;
	bool bIsJumpAttacking = OwnerCharacterRef.Get()->ActionTrackingComponent->GetIsActionInProgress(FName("JumpAttack"))
							|| OwnerCharacterRef.Get()->ActionTrackingComponent->GetIsActionRecentlyFinished(FName("JumpAttack"));
	bool bIsDashAttacking = OwnerCharacterRef.Get()->ActionTrackingComponent->GetIsActionInProgress(FName("DashAttack"));

	FCharacterGameplayInfo ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfo();

	TArray<FVector> currTracePositionList = GetCurrentMeleePointList();
	TArray<AActor*> targetList = CheckMeleeAttackTargetWithSphereTrace();
	MeleePrevTracePointList = currTracePositionList;

	for (auto& target : targetList)
	{
		//if target is valid, apply damage
		if (IsValid(target))
		{
			FCharacterGameplayInfo targetInfo = Cast<ACharacterBase>(target)->GetCharacterGameplayInfo();
			bool bIsFatalAttack = false;
			float totalDamage = WeaponComponentRef.Get()->CalculateTotalDamage(targetInfo, bIsFatalAttack);
			UE_LOG(LogTemp, Warning, TEXT("UMeleeCombatManager::MeleeAttack()---- TotalDmg : %.2lf / bIsFatalAtk : %d"), totalDamage, (int32)bIsFatalAttack);

			//Apply Debuff
			const TArray<ECharacterDebuffType> targetDebuffTypeList = { ECharacterDebuffType::E_Burn, ECharacterDebuffType::E_Poison
																		, ECharacterDebuffType::E_Stun, ECharacterDebuffType::E_Slow };
			for (auto& debuffType : targetDebuffTypeList)
			{
				ECharacterStatType targetStatType = FCharacterGameplayInfo::GetDebuffStatType(bIsJumpAttacking, bIsDashAttacking, debuffType);
				if (targetStatType == ECharacterStatType::E_None) continue;
				WeaponComponentRef.Get()->ApplyWeaponDebuff(Cast<ACharacterBase>(target), debuffType, targetStatType);
			}
			
			//Activate Melee Hit Effect
			ActivateMeleeHitEffect(target->GetActorLocation(), target, bIsFatalAttack);

			//Apply Knockback
			if (!target->ActorHasTag("Boss")
				&& !ownerInfo.bIsAirAttacking)
			{
				OwnerCharacterRef.Get()->ApplyKnockBack(target, WeaponComponentRef.Get()->WeaponStat.WeaponKnockBackStrength);
			}

			//Apply Damage
			UGameplayStatics::ApplyDamage(target, totalDamage, OwnerCharacterRef.Get()->GetController(), OwnerCharacterRef.Get(), nullptr);
			MeleeLineTraceQueryParams.AddIgnoredActor(target);

			//Apply Debuff 
			Cast<ACharacterBase>(target)->TryAddNewDebuff(WeaponComponentRef.Get()->WeaponStat.DebuffInfo, OwnerCharacterRef.Get());
		}
	}
}

bool UMeleeCombatManager::CheckMeleeAttackTarget(FHitResult& hitResult, const TArray<FVector>& TracePositionList)
{
	return false;
}

void UMeleeCombatManager::ActivateMeleeHitEffect(const FVector& Location, AActor* AttachTarget, bool bImpactEffect)
{
	//Activate Slow Effect (Hit stop)
	if (OwnerCharacterRef.Get()->ActorHasTag("Player"))
	{
		const float dilationValue = WeaponComponentRef.Get()->GetIsFinalCombo() ? 0.08 : 0.75;
		GamemodeRef.Get()->ActivateSlowHitEffect(dilationValue);
	}

	//Spawn emitter
	FTransform emitterSpawnTransform(FQuat(0.0f), Location, FVector(0.5f));
	FRotator randomRotator = FRotator::ZeroRotator;
	randomRotator.Pitch += UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f);
	randomRotator.Yaw += UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f);
	randomRotator.Roll += UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f);

	if (!bImpactEffect)
	{
		if (IsValid(WeaponComponentRef.Get()->HitEffectParticle))
		{
			if (IsValid(AttachTarget))
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(WeaponComponentRef.Get()->HitEffectParticle, AttachTarget->GetRootComponent()
					, FName(""), FVector(), WeaponComponentRef.Get()->WeaponState.SlashRotation, EAttachLocation::KeepRelativeOffset, true);
			}
			else
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponComponentRef.Get()->HitEffectParticle, emitterSpawnTransform.GetLocation()
					, WeaponComponentRef.Get()->WeaponState.SlashRotation + randomRotator, emitterSpawnTransform.GetScale3D());
			}
		}
		if (AssetManagerRef.IsValid())
		{
			AssetManagerRef.Get()->PlaySingleSound(OwnerCharacterRef.Get(), ESoundAssetType::E_Weapon, WeaponComponentRef.Get()->WeaponID, "HitImpact");
		}
	}
	else
	{
		if (IsValid(WeaponComponentRef.Get()->HitEffectParticleLarge))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponComponentRef.Get()->HitEffectParticleLarge, emitterSpawnTransform.GetLocation()
				, WeaponComponentRef.Get()->WeaponState.SlashRotation + randomRotator, emitterSpawnTransform.GetScale3D());
		}
		if (AssetManagerRef.IsValid())
		{
			AssetManagerRef.Get()->PlaySingleSound(OwnerCharacterRef.Get(), ESoundAssetType::E_Weapon, WeaponComponentRef.Get()->WeaponID, "HitImpactLarge");
		}
	}

	//카메라 Shake 효과
	if (OwnerCharacterRef.Get()->GetCharacterGameplayInfo().bIsInvincibility) return;

	GamemodeRef.Get()->PlayCameraShakeEffect(OwnerCharacterRef.Get()->ActorHasTag("Player") ? ECameraShakeType::E_Attack : ECameraShakeType::E_Hit, Location);
}

TArray<FVector> UMeleeCombatManager::GetCurrentMeleePointList()
{
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
