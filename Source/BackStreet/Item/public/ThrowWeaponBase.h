// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "../public/RangedWeaponBase.h"
#include "ThrowWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API AThrowWeaponBase : public ARangedWeaponBase
{
	GENERATED_BODY()

public:
	AThrowWeaponBase();
	//공격 처리
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//공격 마무리 처리
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual float GetAttackRange() override;

	UFUNCTION(BlueprintCallable)
		virtual bool TryFireProjectile() override;

	//장전을 시도. 현재 상태에 따른 성공 여부를 반환
	UFUNCTION(BlueprintCallable)
		virtual bool TryReload() override;

	//탄환의 개수를 더함 (ExtraAmmoCount까지)
	UFUNCTION(BlueprintCallable)
		virtual void AddAmmo(int32 Count) override;

//------ 투척 관련 ----------------------------------
public:
	//ThrowDirection으로 발사
	UFUNCTION()
		void Throw();
	
	//ThrowDirection, ThrowEndLocation 을 활용하여 발사 예측 지점들을 구함
	UFUNCTION()
		FPredictProjectilePathResult GetProjectilePathPredictResult();

	//SetThrowEndLocation 호출 이후에 호출 할 것
	//ThrowEndLocation 까지의 발사각을 구하여 ThrowDirection 프로퍼티를 업데이트
	UFUNCTION()
		void CalculateThrowDirection(FVector NewDestination);

	UFUNCTION()
		FVector GetThrowDirection() { return ThrowDirection; }

private:
	//매번 업데이트 할 변수 / 발사체가 도착할 위치
		FVector ThrowDestination;

	//매번 업데이트 할 변수 / 발사체가 발사될 방향
		FVector ThrowDirection;

	//마지막으로 던진 발사체의 스탯 정보
		FProjectileStatStruct ProjectileStat;
};
