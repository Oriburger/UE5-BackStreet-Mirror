// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ShootPillar.h"
#include "../../../Character/CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../../Character/Component/SkillManagerComponentBase.h"

void UAnimNotifyState_ShootPillar::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!IsValid(MeshComp)|| !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;
	
	OwnerCharacterRef = Cast<ACharacterBase>(MeshComp->GetOwner());
	TotalDurationTime = TotalDuration;

	CalculateShootDelayTime();
}

void UAnimNotifyState_ShootPillar::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (!OwnerCharacterRef.IsValid()) return;

	Shootable = false;

	CumulativeFrameTime += FrameDeltaTime;

	if (FMath::IsNearlyEqual(CumulativeFrameTime, ShootIntervalTime, 0.01f))
	{
		CumulativeFrameTime = 0.0f;
		Shootable = true;
	}
}

void UAnimNotifyState_ShootPillar::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!OwnerCharacterRef.IsValid()) return;
	if (CumulativeFrameTime > 0.0f)	CumulativeFrameTime = 0.0f;
}

void UAnimNotifyState_ShootPillar::CalculateShootDelayTime()
{
	if (!OwnerCharacterRef.IsValid()) return;
	TArray<AActor*> resultList = OwnerCharacterRef.Get()->SkillManagerComponent->TraceResultCache;
	int32 length = resultList.Num();

	ShootIntervalTime = TotalDurationTime / (float)length;
}