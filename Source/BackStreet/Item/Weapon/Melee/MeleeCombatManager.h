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

//-------- Melee  ---------------------------
public:
	UFUNCTION(BlueprintCallable)
		TArray<AActor*> CheckMeleeAttackTargetWithSphereTrace();

	//Simulate melee attack
	UFUNCTION()
		void MeleeAttack();

private:
	//sword trail trace, 근접 무기의 각 지점에서 이전 월드 좌표 -> 현재 월드 좌표로 LineTrace를 진행 
	UFUNCTION()
		bool CheckMeleeAttackTarget(FHitResult& hitResult, const TArray<FVector>& TracePositionList);

	//spawn various effect (sfx, vfx, camera effect)
	UFUNCTION()
		void ActivateMeleeHitEffect(const FVector& Location, bool bImpactEffect = false);

private:
	TArray<FVector> GetCurrentMeleePointList();
	TArray<FVector> MeleePrevTracePointList;

	//when player check melee attack's overlapping, ignore these actors
	TArray<AActor*> IgnoreActorList;
	FCollisionQueryParams MeleeLineTraceQueryParams;
	FTimerHandle MeleeAtkTimerHandle;
};
