// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "AnimNotifyState_GetDuration.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SetCameraLag.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UAnimNotifyState_SetCameraLag : public UAnimNotifyState_GetDuration
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		bool bUseSmoothReset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (EditCondition = "bUseSmoothReset"))
		float SmoothResetStartRatio = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		float FOV_Value = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		float FOV_InterpSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		float FOV_ResetSpeed = 0.001f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		float CameraLagSpeed = 5.0f;

private:
	//=== Restore ¿ë =========
	float PrevFOV_Value = 90.0f;
	
	float MainCamBoomLagSpeed = 0.0f;
	float LockCamBoomLagSpeed = 0.0f;
	float SubCamBoomLagSpeed = 0.0f;

	//=== Reset Interp ¿ë =========
	float ResetElapsed = 0.0f;
	
	TWeakObjectPtr<class AMainCharacterBase> OwnerCharacterRef;
};
