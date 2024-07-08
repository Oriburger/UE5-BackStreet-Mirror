// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "../CombatManager.h"
#include "MeleeCombatManager.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UMeleeCombatManager : public UCombatManager
{
	GENERATED_BODY()
public:
	UMeleeCombatManager();

	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		FColor MeleeTrailParticleColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraSystem* HitEffectParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraSystem* HitEffectParticleLarge;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* WieldSound;

	//-------- Melee ���� ---------------------------
public:
	UFUNCTION(BlueprintCallable)
		TArray<AActor*> CheckMeleeAttackTargetWithSphereTrace();

	//���� ������ ����
	UFUNCTION()
		void MeleeAttack();

private:
	//�˷� Trace, ���� ������ �� �������� ���� ���� ��ǥ -> ���� ���� ��ǥ�� LineTrace�� ���� 
	UFUNCTION()
		bool CheckMeleeAttackTarget(FHitResult& hitResult, const TArray<FVector>& TracePositionList);

	//����, ��ƼŬ, ī�޶� ���� ���� ���� ȿ���� ����Ѵ�.
	UFUNCTION()
		void ActivateMeleeHitEffect(const FVector& Location, bool bImpactEffect = false);

private:
	TArray<FVector> GetCurrentMeleePointList();
	TArray<FVector> MeleePrevTracePointList;

	//���� ������ ������ üũ ��, ������ ������ ����Ʈ�̴�.
	TArray<AActor*> IgnoreActorList;
	FCollisionQueryParams MeleeLineTraceQueryParams;
	FTimerHandle MeleeAtkTimerHandle;
};
