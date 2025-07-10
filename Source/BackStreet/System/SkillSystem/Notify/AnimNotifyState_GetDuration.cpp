// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_GetDuration.h"

void UAnimNotifyState_GetDuration::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!IsValid(MeshComp)|| !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;
	
	//Initialize
	ElapsedTime = 0;
	TotalNotifyDuration = TotalDuration;
	RemainingTime = TotalDuration;
}

void UAnimNotifyState_GetDuration::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	ElapsedTime += FrameDeltaTime;
	RemainingTime -= FrameDeltaTime;
}

void UAnimNotifyState_GetDuration::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}