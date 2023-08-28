// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/MeleeWeaponBase.h"
#include "../public/WeaponBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Global/public/DebuffManager.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#define MAX_LINETRACE_POS_COUNT 6
#define DEFAULT_MELEE_ATK_RANGE 150.0f

AMeleeWeaponBase::AMeleeWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MeleeTrailParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ITEM_NIAGARA_COMPONENT"));
	MeleeTrailParticle->SetupAttachment(WeaponMesh);
	MeleeTrailParticle->bAutoActivate = false;
}

void AMeleeWeaponBase::Attack()
{
	Super::Attack();

	this->Tags.Add("Melee");
	
	PlayEffectSound(AttackSound);
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
	// State �ʱ�ȭ
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
		
	if (MeleeWeaponAssetInfo.HitImpactSound.IsValid())
		HitImpactSound = MeleeWeaponAssetInfo.HitImpactSound.Get();
}

void AMeleeWeaponBase::MeleeAttack()
{
	FHitResult hitResult;
	bool bIsMeleeTraceSucceed = false;

	TArray<FVector> currTracePositionList = GetCurrentMeleePointList();
	bIsMeleeTraceSucceed = CheckMeleeAttackTarget(hitResult, currTracePositionList);
	MeleePrevTracePointList = currTracePositionList;

	//hitResult�� Valid�ϴٸ� �Ʒ� ���ǹ����� �������� ����
	if (bIsMeleeTraceSucceed)
	{
		//ȿ���� ���
		ActivateMeleeHitEffect(hitResult.Location);

		//�������� �ְ�, �ߺ� üũ�� ���ش�.
		UGameplayStatics::ApplyDamage(hitResult.GetActor(), WeaponStat.MeleeWeaponStat.WeaponMeleeDamage * WeaponStat.WeaponDamageRate
			, OwnerCharacterRef.Get()->GetController(), OwnerCharacterRef.Get(), nullptr);
		MeleeLineTraceQueryParams.AddIgnoredActor(hitResult.GetActor());

		//������� �ο�
		if (IsValid(GamemodeRef.Get()->GetGlobalDebuffManagerRef()))
		{
			GamemodeRef.Get()->GetGlobalDebuffManagerRef()->SetDebuffTimer(WeaponStat.MeleeWeaponStat.DebuffType, Cast<ACharacterBase>(hitResult.GetActor())
				, OwnerCharacterRef.Get(), WeaponStat.MeleeWeaponStat.DebuffTotalTime, WeaponStat.MeleeWeaponStat.DebuffVariable);
		}
		//Ŀ���� �̺�Ʈ 
		OnMeleeAttackSuccess(hitResult, WeaponStat.MeleeWeaponStat.WeaponMeleeDamage);

		//�������� ������Ʈ
		UpdateDurabilityState();
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

			//DrawDebugCapsule(GetWorld(), (beginPoint + endPoint) / 2, traceHalfHeight, traceRadius
			//				, UKismetMathLibrary::FindLookAtRotation(beginPoint, endPoint).Quaternion()
			//				, FColor::Yellow, false, 2.0f, 0U, 1.0f);

			if (bHit && IsValid(hitResult.GetActor()) //hitResult�� hitActor�� Validity üũ
				&& OwnerCharacterRef.Get()->Tags.IsValidIndex(1) //hitActor�� Tags üũ(1)
				&& hitResult.GetActor()->ActorHasTag("Character") //hitActor�� Type üũ
				&& !hitResult.GetActor()->ActorHasTag(OwnerCharacterRef.Get()->Tags[1])) //�����ڿ� �ǰ����� Ÿ���� ������ üũ
			{
				return true;
			}
		}
	}
	return false;
}

void AMeleeWeaponBase::ActivateMeleeHitEffect(const FVector& Location)
{
	//���� ���ݿ����� ���ο� ȿ���� �õ�
	TryActivateSlowHitEffect();

	//ȿ�� �̹��� ���
	if (IsValid(HitEffectParticle))
	{
		FTransform emitterSpawnTransform(FQuat(0.0f), Location, FVector(1.5f));
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffectParticle, emitterSpawnTransform, true, EPSCPoolMethod::None, true);

	}
	//ī�޶� Shake ȿ��
	GamemodeRef.Get()->PlayCameraShakeEffect(OwnerCharacterRef.Get()->ActorHasTag("Player") ? ECameraShakeType::E_Attack : ECameraShakeType::E_Hit, Location);

	// ����
	if (HitImpactSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitImpactSound, GetActorLocation());
	}
}

bool AMeleeWeaponBase::TryActivateSlowHitEffect()
{
	//������ �޺� ���ο� ȿ��, ���� MaxComboCnt ������Ƽ �߰� ����
		//�ִϸ��̼� �迭 ������ ĳ����. OwnerCharacter->GetAnimArray(WeaponType).Num() ) 
		// ��> �̷������� �ϸ� �� �� ������...
	if (OwnerCharacterRef.Get()->ActorHasTag("Player"))
	{
		FTimerHandle attackSlowEffectTimerHandle;
		const float dilationValue = WeaponState.ComboCount % 5 == 0 ? 0.08 : 0.15; 

		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), dilationValue);
		GetWorldTimerManager().SetTimer(attackSlowEffectTimerHandle, FTimerDelegate::CreateLambda([&]() {
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
		}), 0.015f, false);
		return true;
	}
	return false;
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