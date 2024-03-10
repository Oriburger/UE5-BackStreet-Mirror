// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/MeleeWeaponBase.h"
#include "../public/WeaponBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#define MAX_LINETRACE_POS_COUNT 6
#define DEFAULT_MELEE_ATK_RANGE 150.0f

AMeleeWeaponBase::AMeleeWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	this->Tags.Add("Melee");

	MeleeTrailParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ITEM_NIAGARA_COMPONENT"));
	MeleeTrailParticle->SetupAttachment(WeaponMesh);
	MeleeTrailParticle->bAutoActivate = false;
}

void AMeleeWeaponBase::Attack()
{
	if (WeaponID == 0) return;
	Super::Attack();

	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, SoundAssetInfo, "Wield");
	}

	GetWorldTimerManager().SetTimer(MeleeAtkTimerHandle, this, &AMeleeWeaponBase::MeleeAttack, 0.01f, true);
	MeleeTrailParticle->Activate(true);
	
	if (MeleeLineTraceQueryParams.GetIgnoredActors().Num() == 0)
	{
		MeleeLineTraceQueryParams.AddIgnoredActor(OwnerCharacterRef.Get());
	}
}

void AMeleeWeaponBase::StopAttack()
{
	Super::StopAttack();

	GetWorldTimerManager().ClearTimer(MeleeAtkTimerHandle);
	MeleePrevTracePointList.Empty();
	MeleeLineTraceQueryParams.ClearIgnoredActors();
	IgnoreActorList.Empty();
	MeleeTrailParticle->Deactivate();
}

float AMeleeWeaponBase::GetAttackRange()
{
	if (!OwnerCharacterRef.IsValid()) return DEFAULT_MELEE_ATK_RANGE + 50.0f;
	return DEFAULT_MELEE_ATK_RANGE;
}

void AMeleeWeaponBase::ClearAllTimerHandle()
{
	Super::ClearAllTimerHandle();    
	MeleeTrailParticle->Deactivate();
	GetWorldTimerManager().ClearTimer(MeleeAtkTimerHandle);
	MeleePrevTracePointList.Empty(); 
	MeleeLineTraceQueryParams.ClearIgnoredActors();
}

void AMeleeWeaponBase::UpdateWeaponStat(FWeaponStatStruct NewStat)
{
	Super::UpdateWeaponStat(NewStat);
}

void AMeleeWeaponBase::InitWeaponAsset()
{
	Super::InitWeaponAsset();

	FMeleeWeaponAssetInfoStruct& MeleeWeaponAssetInfo = WeaponAssetInfo.MeleeWeaponAssetInfo;

	if (MeleeWeaponAssetInfo.MeleeTrailParticle.IsValid())
	{	
		MeleeTrailParticle->SetAsset(MeleeWeaponAssetInfo.MeleeTrailParticle.Get());
		MeleeTrailParticleColor = MeleeWeaponAssetInfo.MeleeTrailParticleColor;
		MeleeTrailParticle->SetColorParameter(FName("Color"), MeleeTrailParticleColor);

		FVector startLocation = WeaponMesh->GetSocketLocation("Mid");
		FVector endLocation = WeaponMesh->GetSocketLocation("End");
		FVector socketLocalLocation = UKismetMathLibrary::MakeRelativeTransform(WeaponMesh->GetSocketTransform(FName("End"))
																				, WeaponMesh->GetComponentTransform()).GetLocation();

		MeleeTrailParticle->SetVectorParameter(FName("Start"), startLocation);
		MeleeTrailParticle->SetVectorParameter(FName("End"), endLocation);
		MeleeTrailParticle->SetRelativeLocation(socketLocalLocation);
	}

	if (MeleeWeaponAssetInfo.HitEffectParticle.IsValid())
		HitEffectParticle = MeleeWeaponAssetInfo.HitEffectParticle.Get();
	
}

TArray<AActor*> AMeleeWeaponBase::CheckMeleeAttackTargetWithSphereTrace()
{
	if (!OwnerCharacterRef.IsValid()) return TArray<AActor*>(); 

	FVector traceStartPos = (WeaponMesh->GetSocketLocation("GrabPoint")
							+ WeaponMesh->GetSocketLocation("End")) / 2;
	float traceRadius = UKismetMathLibrary::Vector_Distance(WeaponMesh->GetSocketLocation("GrabPoint")
						, WeaponMesh->GetSocketLocation("End")) + 10.0f;
	
	TEnumAsByte<EObjectTypeQuery> pawnTypeQuery = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn);
	TArray<AActor*> overlapResultList, meleeDamageTargetList;
	
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), traceStartPos, traceRadius, { pawnTypeQuery }
											 , ACharacterBase::StaticClass(), IgnoreActorList, overlapResultList);
	
	//Find target to apply damage in overlap result array
	//Character - Check if there's any obstacle between causer and target.
	FCollisionQueryParams collisionQueryParam;
	collisionQueryParam.AddIgnoredActor(OwnerCharacterRef.Get());
	collisionQueryParam.AddIgnoredActor(this);
	for (auto& target : overlapResultList)
	{
		if (!IsValid(target)) continue;
		if (!target->ActorHasTag("Character")) continue;
		if (target->ActorHasTag(OwnerCharacterRef.Get()->Tags[1])) continue; 
		if (Cast<ACharacterBase>(target)->GetIsActionActive(ECharacterActionType::E_Die)) continue;

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

void AMeleeWeaponBase::MeleeAttack()
{
	FHitResult hitResult;
	bool bIsMeleeTraceSucceed = false;

	TArray<FVector> currTracePositionList = GetCurrentMeleePointList();
	TArray<AActor*> targetList = CheckMeleeAttackTargetWithSphereTrace(); 
	MeleePrevTracePointList = currTracePositionList;

	for (auto& target : targetList)
	{
		//if target is valid, apply damage
		if (IsValid(target))
		{
			//Activate Melee Hit Effect
			ActivateMeleeHitEffect(target->GetActorLocation());
			
			//Apply Knockback
			OwnerCharacterRef.Get()->ApplyKnockBack(target, GetWeaponStat().WeaponKnockBackEnergy);
			//Apply Damage
			UGameplayStatics::ApplyDamage(target, WeaponStat.MeleeWeaponStat.WeaponMeleeDamage * WeaponStat.WeaponDamageRate * OwnerCharacterRef.Get()->GetCharacterStat().CharacterAtkMultiplier
				, OwnerCharacterRef.Get()->GetController(), OwnerCharacterRef.Get(), nullptr);
			MeleeLineTraceQueryParams.AddIgnoredActor(target); 

			//Apply Debuff 
			Cast<ACharacterBase>(target)->TryAddNewDebuff(WeaponStat.MeleeWeaponStat.DebuffType, OwnerCharacterRef.Get()
												, WeaponStat.MeleeWeaponStat.DebuffTotalTime, WeaponStat.MeleeWeaponStat.DebuffVariable);

			//Update Durability
			UpdateDurabilityState();
		}
	}
}

bool AMeleeWeaponBase::CheckMeleeAttackTarget(FHitResult& hitResult, const TArray<FVector>& TracePositionList)
{
	if (MeleePrevTracePointList.Num() == MAX_LINETRACE_POS_COUNT)
	{
		for (uint8 tracePointIdx = 0; tracePointIdx < MAX_LINETRACE_POS_COUNT; tracePointIdx++)
		{
			const FVector& beginPoint = MeleePrevTracePointList[tracePointIdx];
			const FVector& endPoint = TracePositionList[tracePointIdx];
			const float traceHalfHeight = UKismetMathLibrary::Vector_Distance(beginPoint, endPoint);
			const float traceRadius = 30.0f;

			// Use CapsuleTraceByChannel instead of LineTraceSingleByChannel
			bool bHit = GetWorld()->SweepSingleByChannel(hitResult, beginPoint, endPoint, FQuat::Identity, ECollisionChannel::ECC_Camera
														, FCollisionShape::MakeCapsule(traceRadius, traceHalfHeight), MeleeLineTraceQueryParams);

			if (bHit && IsValid(hitResult.GetActor()) //hitResult와 hitActor의 Validity 체크
				&& OwnerCharacterRef.Get()->Tags.IsValidIndex(1) //hitActor의 Tags 체크(1)
				&& hitResult.GetActor()->ActorHasTag("Character") //hitActor의 Type 체크
				&& !hitResult.GetActor()->ActorHasTag(OwnerCharacterRef.Get()->Tags[1])) //공격자와 피격자의 타입이 같은지 체크
			{
				return true;
			}
		}
	}
	return false;
}

void AMeleeWeaponBase::ActivateMeleeHitEffect(const FVector& Location)
{
	//근접 공격에서의 슬로우 효과를 시도
	ActivateSlowHitEffect();

	//효과 이미터 출력
	if (IsValid(HitEffectParticle))
	{
		FTransform emitterSpawnTransform(FQuat(0.0f), Location, FVector(1.5f));
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffectParticle, emitterSpawnTransform, true, EPSCPoolMethod::None, true);

	}
	//카메라 Shake 효과
	GamemodeRef.Get()->PlayCameraShakeEffect(OwnerCharacterRef.Get()->ActorHasTag("Player") ? ECameraShakeType::E_Attack : ECameraShakeType::E_Hit, Location);

	// 사운드
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, SoundAssetInfo, "HitImpact");
	}
}

void AMeleeWeaponBase::ActivateSlowHitEffect()
{
	if (OwnerCharacterRef.Get()->ActorHasTag("Player"))
	{
		FTimerHandle attackSlowEffectTimerHandle;
		const float dilationValue = WeaponState.ComboCount % 5 == 0 ? 0.08 : 0.15;

		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), dilationValue);
		GetWorldTimerManager().SetTimer(attackSlowEffectTimerHandle, this, &AMeleeWeaponBase::DeactivateSlowHitEffect, 0.015f, false);
	}
}

void AMeleeWeaponBase::DeactivateSlowHitEffect()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

TArray<FVector> AMeleeWeaponBase::GetCurrentMeleePointList()
{
	TArray<FVector> resultPosList;

	resultPosList.Add(WeaponMesh->GetSocketLocation("GrabPoint"));
	resultPosList.Add(WeaponMesh->GetSocketLocation("End"));
	for (uint8 positionIdx = 1; positionIdx < MAX_LINETRACE_POS_COUNT - 1; positionIdx++)
	{
		FVector direction = resultPosList[1] - resultPosList[0];

		resultPosList.Add(resultPosList[0] + direction / MAX_LINETRACE_POS_COUNT * positionIdx);
	}
	return resultPosList;
}