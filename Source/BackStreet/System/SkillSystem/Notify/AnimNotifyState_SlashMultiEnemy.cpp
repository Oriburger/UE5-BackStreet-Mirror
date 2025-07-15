// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SlashMultiEnemy.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Character/Component/WeaponComponentBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../../Character/Component/SkillManagerComponentBase.h"

void UAnimNotifyState_SlashMultiEnemy::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!IsValid(MeshComp)|| !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;
	
	//Initialize
	OwnerCharacterRef = Cast<ACharacterBase>(MeshComp->GetOwner());
	bIsInterping = false;
	ElapsedTime = SlashIdx = 0;
	EndPoint = MeshComp->GetAnimInstance()->Montage_GetPosition(MeshComp->GetAnimInstance()->GetCurrentActiveMontage());
	EndPoint += TotalDuration;

	OriginalTransform = OwnerCharacterRef.Get()->GetActorTransform();
}

void UAnimNotifyState_SlashMultiEnemy::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (!OwnerCharacterRef.IsValid()) return;

	if(!bIsInterping) ElapsedTime += FrameDeltaTime;

	const TArray<AActor*> resultList = OwnerCharacterRef.Get()->SkillManagerComponent->TraceResultCache;
	if (SlashIdx >= resultList.Num())
	{
		OwnerCharacterRef.Get()->GetWorldTimerManager().ClearTimer(damageIntervalHandle);
		damageIntervalHandle.Invalidate();
		UAnimMontage* currentMontage = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
		MeshComp->GetAnimInstance()->Montage_JumpToSection("End", currentMontage);
		return;
	}
	
	if (ElapsedTime >= SlashInterval)
	{
		ElapsedTime = 0.0f;
		const int32 targetIdx = resultList.Num() - SlashIdx - 1; // resultList.Num() - GetZigZagIdxByDistance(SlashIdx, resultList.Num()) - 1;

		if (!resultList.IsValidIndex(targetIdx)) return;
		
		if (IsValid(resultList[targetIdx]))
		{
			FVector targetLocation = resultList[targetIdx]->GetActorLocation();
			targetLocation += MoveErrorValueList[UKismetMathLibrary::RandomInteger(MoveErrorValueList.Num())];

			bIsInterping = true;
			OwnerCharacterRef.Get()->SetLocationWithInterp(targetLocation, InterpSpeed, false, true, false, false);
			OwnerCharacterRef.Get()->GetWorldTimerManager().SetTimer(damageIntervalHandle, FTimerDelegate::CreateLambda([=]() {
				ApplyDamageWithInterval(resultList[targetIdx]);
				bIsInterping = false;
			}), SlashSpeedTime, false);
		}
	}
}

void UAnimNotifyState_SlashMultiEnemy::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!OwnerCharacterRef.IsValid()) return;
	OwnerCharacterRef.Get()->GetWorldTimerManager().ClearTimer(damageIntervalHandle);
	damageIntervalHandle.Invalidate();

	TArray<AActor*>& resultList = OwnerCharacterRef.Get()->SkillManagerComponent->TraceResultCache;
	if (bContainExtraSlash)
	{
		for (auto& target : resultList)
		{
			if (!IsValid(target)) continue;
			ApplyDamageWithInterval(target);
		}
	}
	if (bResetTransform)
	{
		OwnerCharacterRef.Get()->SetActorTransform(OriginalTransform);
	}
	resultList.Empty();
}

int32 UAnimNotifyState_SlashMultiEnemy::GetZigZagIdxByDistance(int32 TargetIdx, int32 Length)
{
	if (TargetIdx % 2 == 0) return TargetIdx / 2;
	return (Length - 1) / 2 - TargetIdx / 2;
}

void UAnimNotifyState_SlashMultiEnemy::ApplyDamageWithInterval(AActor* TargetActor)
{
	bIsInterping = true;
	if (!IsValid(TargetActor)) return;

	FCharacterGameplayInfo gameplayInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfo();
	bool bIsFatalAttack = false;
	float totalDamage = OwnerCharacterRef.Get()->WeaponComponent->GetWeaponStat().WeaponDamage;
	
	UGameplayStatics::ApplyDamage(TargetActor, totalDamage * DamageMultipiler, OwnerCharacterRef.Get()->GetController(), OwnerCharacterRef.Get(), nullptr);
	
	if (SlashEffectNiagara != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(OwnerCharacterRef.Get(), SlashEffectNiagara, TargetActor->GetActorLocation(), UKismetMathLibrary::RandomRotator());
	}
	if (SlashEffectSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(OwnerCharacterRef.Get(), SlashEffectSound, TargetActor->GetActorLocation());
	}
	SlashIdx += 1;
}