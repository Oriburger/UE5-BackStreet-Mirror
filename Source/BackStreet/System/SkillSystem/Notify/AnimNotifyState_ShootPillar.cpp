// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ShootPillar.h"
#include "../../../Character/CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../../Character/Component/SkillManagerComponentBase.h"

void UAnimNotifyState_ShootPillar::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!IsValid(MeshComp)|| !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;
	
	OwnerCharacterRef = Cast<ACharacterBase>(MeshComp->GetOwner());
	ShootablePillarList = OwnerCharacterRef.Get()->SkillManagerComponent->TraceResultCache;
	TotalDurationTime = TotalDuration;
	CalculateShootDelayTime();
	
	if (ShootablePillarList.Num() >= 1) 
	{
		Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	}
}

void UAnimNotifyState_ShootPillar::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!OwnerCharacterRef.IsValid()) return;
	if (ShootIntervalTime == 0) return;

	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	Shootable = false;
	CumulativeFrameTime += FrameDeltaTime;

	if (FMath::IsNearlyEqual(CumulativeFrameTime, ShootIntervalTime, 0.03f) && PillarIndex <= ShootablePillarList.Num()-1)
	{
		PillarIndex++;
		CumulativeFrameTime = 0.0f;
		Shootable = true;
	}
}

void UAnimNotifyState_ShootPillar::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!OwnerCharacterRef.IsValid()) return;
	if (CumulativeFrameTime > 0.0f)	CumulativeFrameTime = 0.0f;

	PillarIndex = 0;
}

void UAnimNotifyState_ShootPillar::CalculateShootDelayTime()
{
	if (!OwnerCharacterRef.IsValid()) return;
	TArray<AActor*> resultList = OwnerCharacterRef.Get()->SkillManagerComponent->TraceResultCache;
	int32 length = resultList.Num();

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