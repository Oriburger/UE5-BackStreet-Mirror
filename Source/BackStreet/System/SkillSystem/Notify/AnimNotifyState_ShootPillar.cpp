// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ShootPillar.h"
#include "../../../Character/CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../../Character/Component/SkillManagerComponentBase.h"

void UAnimNotifyState_ShootPillar::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!IsValid(MeshComp)|| !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;
	
	OwnerCharacterRef = Cast<ACharacterBase>(MeshComp->GetOwner());
	if (!OwnerCharacterRef.IsValid() && !IsValid(OwnerCharacterRef.Get()->SkillManagerComponent)) return;
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	PillarMaxCount = OwnerCharacterRef.Get()->SkillManagerComponent->TraceResultCache.Num();
	TotalDurationTime = TotalDuration;
	CalculateShootDelayTime();
}

void UAnimNotifyState_ShootPillar::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!OwnerCharacterRef.IsValid()) return;
	if (ShootIntervalTime == 0) return;

	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	bIsShootable = false;
	CumulativeFrameTime += FrameDeltaTime;

	if (FMath::IsNearlyEqual(CumulativeFrameTime, ShootIntervalTime, 0.03f) && PillarIndex <= PillarMaxCount - 1)
	{
		CumulativeFrameTime = 0.0f;
		PillarIndex++;
		bIsShootable = true;
	}
}

void UAnimNotifyState_ShootPillar::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!OwnerCharacterRef.IsValid()) return;
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (CumulativeFrameTime > 0.0f)	CumulativeFrameTime = 0.0f;
	PillarIndex = 0;
}

void UAnimNotifyState_ShootPillar::CalculateShootDelayTime()
{
	if (!OwnerCharacterRef.IsValid()) return;
	int32 length = PillarMaxCount;

	if (length <= 1)
	{
		ShootIntervalTime = 0;
	}
	else if (length == 2) 
	{
		ShootIntervalTime = TotalDurationTime;
	}
	else 
	{
		ShootIntervalTime = TotalDurationTime / ((float)length-1);
	}
}