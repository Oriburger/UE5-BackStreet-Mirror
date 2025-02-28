// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_ShootPillar.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UAnimNotifyState_ShootPillar : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected: 

	UPROPERTY(BlueprintReadOnly)
		bool bIsShootable = false;

	UPROPERTY(BlueprintReadOnly)
		int32 PillarIndex = 0;

private:

	UPROPERTY()
		int32 PillarMaxCount = 0;

	UPROPERTY()
		float TotalDurationTime = 0.0f;

	UPROPERTY()
		float ShootIntervalTime = 0.0f;

	UPROPERTY()
		float CumulativeFrameTime = 0.0f;

	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};
