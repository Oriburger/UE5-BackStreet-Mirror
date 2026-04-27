// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UAnimNotifyState_MeleeAttack : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX")
		bool bUseWeaponAsset = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX")
		class UNiagaraSystem* HitEffectNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX")
		class USoundCue* HitEffectSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX")
		class UNiagaraSystem* LargeHitEffectNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX")
		class USoundCue* LargeHitEffectSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool bIsDebugMode = false;

protected:
	UFUNCTION(BlueprintCallable)
		 TArray<AActor*> CheckMeleeAttackTargetWithSphereTrace();

	UFUNCTION()
		void MeleeAttack();

private:
	
	//spawn various effect (sfx, vfx, camera effect)
	UFUNCTION()
		void ActivateMeleeHitEffect(const FVector& Location, AActor* AttachTarget = nullptr, bool bImpactEffect = false);

protected:
	//gamemode ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//inventory owner character (equal to getowner())
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class UAssetManagerBase> AssetManagerRef;

	//Weapon component 
	TWeakObjectPtr<class UWeaponComponentBase> WeaponComponentRef;

private:
	TArray<FVector> GetCurrentMeleePointList();
	TArray<FVector> MeleePrevTracePointList;

	//when player check melee attack's overlapping, ignore these actors
	TArray<AActor*> IgnoreActorList;
	FCollisionQueryParams MeleeLineTraceQueryParams;
};
