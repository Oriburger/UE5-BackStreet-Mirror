// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_GetDuration.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UAnimNotifyState_GetDuration : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetElapsedTime() const { return ElapsedTime; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetRemainingPercentage() const { return RemainingTime > 0.0f ? (RemainingTime / TotalNotifyDuration) * 100.0f : 0.0f; }

private:
	float TotalNotifyDuration = 0.0f;
	float RemainingTime = 0.0f;
	float ElapsedTime = 0.0f;
};
