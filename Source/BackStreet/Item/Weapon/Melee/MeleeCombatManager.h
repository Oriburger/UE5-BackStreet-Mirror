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

	//-------- Melee 관련 ---------------------------
public:
	UFUNCTION(BlueprintCallable)
		TArray<AActor*> CheckMeleeAttackTargetWithSphereTrace();

	//근접 공격을 수행
	UFUNCTION()
		void MeleeAttack();

private:
	//검로 Trace, 근접 무기의 각 지점에서 이전 월드 좌표 -> 현재 월드 좌표로 LineTrace를 진행 
	UFUNCTION()
		bool CheckMeleeAttackTarget(FHitResult& hitResult, const TArray<FVector>& TracePositionList);

	//사운드, 파티클, 카메라 등의 근접 공격 효과를 출력한다.
	UFUNCTION()
		void ActivateMeleeHitEffect(const FVector& Location, bool bImpactEffect = false);

private:
	TArray<FVector> GetCurrentMeleePointList();
	TArray<FVector> MeleePrevTracePointList;

	//근접 공격의 오버랩 체크 시, 무시할 액터의 리스트이다.
	TArray<AActor*> IgnoreActorList;
	FCollisionQueryParams MeleeLineTraceQueryParams;
	FTimerHandle MeleeAtkTimerHandle;
};
