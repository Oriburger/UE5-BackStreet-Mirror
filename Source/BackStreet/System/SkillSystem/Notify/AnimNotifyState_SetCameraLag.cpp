// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SetCameraLag.h"
//maincharacter
#include "../../../Character/MainCharacter/MainCharacterBase.h"

void UAnimNotifyState_SetCameraLag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!IsValid(MeshComp)|| !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;
	
	OwnerCharacterRef = Cast<AMainCharacterBase>(MeshComp->GetOwner());
	if (!OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAnimNotifyState_SetCameraLag::NotifyBegin OwnerCharacterRef is Invalid"));
		return;
	}

	//Restore 프로퍼티 초기화
	MainCamBoomLagSpeed = OwnerCharacterRef->CameraBoom->CameraLagSpeed;
	LockCamBoomLagSpeed = OwnerCharacterRef->TargetingModeBoom->CameraLagSpeed;
	SubCamBoomLagSpeed = OwnerCharacterRef->SubCameraBoom->CameraLagSpeed;
	ResetElapsed = 0.0f;

	//Lag 활성화
	OwnerCharacterRef.Get()->CameraBoom->bEnableCameraLag = true;
	OwnerCharacterRef.Get()->TargetingModeBoom->bEnableCameraLag = true;
	OwnerCharacterRef.Get()->SubCameraBoom->bEnableCameraLag = true;

	OwnerCharacterRef.Get()->CameraBoom->CameraLagSpeed = CameraLagSpeed;
	OwnerCharacterRef.Get()->TargetingModeBoom->CameraLagSpeed = CameraLagSpeed;
	OwnerCharacterRef.Get()->SubCameraBoom->CameraLagSpeed = CameraLagSpeed;

	//FOV
	PrevFOV_Value = OwnerCharacterRef.Get()->FollowingCamera->FieldOfView;
	OwnerCharacterRef.Get()->SetFieldOfViewWithInterp(FOV_Value, FOV_InterpSpeed, false);
}

void UAnimNotifyState_SetCameraLag::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!IsValid(MeshComp) || !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;

	if (bUseSmoothReset)
	{
		ResetElapsed += FrameDeltaTime;
		const float alpha = FMath::Clamp(ResetElapsed / (TotalNotifyDuration), 0.0f, 1.0f);
		const float remainingRatio = RemainingTime / TotalNotifyDuration;
		float deltaLagSpeed = FMath::Lerp(CameraLagSpeed, 45.0f, alpha);

		if (remainingRatio <= SmoothResetStartRatio) //Update 단계
		{
			UE_LOG(LogTemp, Log, TEXT("UAnimNotifyState_SetCameraLag::NotifyTick : remainingRatio %.2lf,  deltaLagSpeed  %.2lf"), remainingRatio, deltaLagSpeed);
			OwnerCharacterRef.Get()->CameraBoom->CameraLagSpeed = deltaLagSpeed;
			OwnerCharacterRef.Get()->TargetingModeBoom->CameraLagSpeed = deltaLagSpeed;
			OwnerCharacterRef.Get()->SubCameraBoom->CameraLagSpeed = deltaLagSpeed;
		}
	}
}

void UAnimNotifyState_SetCameraLag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!IsValid(MeshComp) || !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;
	if (!OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAnimNotifyState_SetCameraLag::NotifyEnd OwnerCharacterRef is Invalid"));
		return;
	}
	//FOV
	OwnerCharacterRef.Get()->SetFieldOfViewWithInterp(PrevFOV_Value, FOV_ResetSpeed, false);

	//Restore
	OwnerCharacterRef.Get()->CameraBoom->bEnableCameraLag = false;
	OwnerCharacterRef.Get()->TargetingModeBoom->bEnableCameraLag = false;
	OwnerCharacterRef.Get()->SubCameraBoom->bEnableCameraLag = true;

	OwnerCharacterRef.Get()->CameraBoom->CameraLagSpeed = MainCamBoomLagSpeed;
	OwnerCharacterRef.Get()->TargetingModeBoom->CameraLagSpeed = LockCamBoomLagSpeed;
	OwnerCharacterRef.Get()->SubCameraBoom->CameraLagSpeed = 45.0f;
}