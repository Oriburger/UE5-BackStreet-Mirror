// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SlashMultiEnemy.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UAnimNotifyState_SlashMultiEnemy : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX")
		class UNiagaraSystem* SlashEffectNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX")
		class USoundCue* SlashEffectSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float SlashInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float SlashSpeedTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float DamageMultipiler = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float InterpSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		bool bContainExtraSlash = false;

protected:
	UFUNCTION()
		void ApplyDamageWithInterval(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetZigZagIdxByDistance(int32 TargetIdx, int32 Length);

private:
	bool bIsInterping = false;
	float EndPoint = 0.0f;
	float ElapsedTime = 0.0f;
	int32 SlashIdx = 0;
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
	TArray<FVector> MoveErrorValueList = { FVector(50.0f, 50.0f, 10.0f)
										,  FVector(-50.0f, 50.0f, 10.0f)
										,  FVector(50.0f, -50.0f, 10.0f)
										,  FVector(-50.0f, -50.0f, 10.0f) };
	FTimerHandle damageIntervalHandle;
};
