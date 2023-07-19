// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "../public/WeaponBase.h"
#include "MeleeWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API AMeleeWeaponBase : public AWeaponBase
{
	GENERATED_BODY()
public:
	//공격 처리
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//공격 마무리 처리
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual float GetAttackRange() override;


//------ 스탯/상태 관련 ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		virtual void UpdateWeaponStat(FWeaponStatStruct NewStat) override;

//-------- Melee 관련 ---------------------------
public:
	//근접 공격을 수행
	UFUNCTION()
		void MeleeAttack();
	
private:
	//검로 Trace, 근접 무기의 각 지점에서 이전 월드 좌표 -> 현재 월드 좌표로 LineTrace를 진행 
	UFUNCTION()
		bool CheckMeleeAttackTarget(FHitResult& hitResult, const TArray<FVector>& TracePositionList);

	//사운드, 파티클, 카메라 등의 근접 공격 효과를 출력한다.
	UFUNCTION()
		void ActivateMeleeHitEffect(const FVector& Location);

	//마지막 콤보에서의 슬로우 타격 효과 활성화를 시도한다.
	UFUNCTION()
		bool TryActivateSlowHitEffect();

private:
	UFUNCTION()
		TArray<FVector> GetCurrentMeleePointList();

	UPROPERTY()
		TArray<FVector> MeleePrevTracePointList;

	//UPROPERTY()
	FCollisionQueryParams MeleeLineTraceQueryParams;


protected:
	virtual void ClearAllTimerHandle() override;

	UPROPERTY()
		FTimerHandle MeleeAtkTimerHandle;
};