// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "../CombatManager.h"
#include "RangedCombatManager.generated.h"


UCLASS()
class BACKSTREET_API URangedCombatManager : public UCombatManager
{
	GENERATED_BODY()
public:
	URangedCombatManager();
	


public:
	//공격 처리		
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//공격 마무리 처리
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

//------ Ranged 오버라이더블 ----------------------------
public:
	UFUNCTION(BlueprintCallable)
		bool TryFireProjectile(FRotator FireRotationOverride = FRotator::ZeroRotator);
	
	//Add ammo count
	UFUNCTION(BlueprintCallable)
		void AddAmmo(int32 Count);

//------ Basic ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		class AProjectileBase* CreateProjectile(FRotator FireRotationOverride = FRotator::ZeroRotator);

//------- Getter / Setter ---------------------------
public:
	UFUNCTION()
		void SetInfiniteAmmoMode(bool NewMode);

	UFUNCTION(BlueprintCallable)
		int32 GetLeftAmmoCount();

protected:
	//Get Rotation List to FireProjectile
	UFUNCTION()
		TArray<FRotator> GetFireRotationList(int32 FireCount);

//------ Asset----------------------------------
protected:
	//발사 순간에 출력될 이미터 (임시, 추후 데이터 테이블로 관리 예정)
	UPROPERTY(VisibleInstanceOnly)
		class UNiagaraSystem* ShootNiagaraEmitter;

	//투사체가 발사되는 이펙트를 출력한다
	UFUNCTION()
		void SpawnShootNiagaraEffect();

//--------타이머 관련--------------------
private:
	//타이머 이벤트 전달을 위해서만 사용
	//델리게이트 바인딩 이후 파라미터를 직접 지정하면
	//타이머 이벤트 수행 도중에 파라미터가 메모리에서 소멸되기에 널 예외 발생
	FRotator FireRotationForTimer; 
};

