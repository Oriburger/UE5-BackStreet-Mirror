// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_RangedAttack.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UAnimNotifyState_RangedAttack : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	//false라면 WeaponComponent의 ProjectileClass를 사용함 (현재 지원안함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		bool bUseCustomProjectile = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Class", meta = (EditCondition = "bUseCustomProjectile"))
		bool bUseCustomClass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Class", meta = (EditCondition = "bUseCustomProjectile && bUseCustomClass"))
		TSubclassOf<class AActor> CustomProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Info", meta = (EditCondition = "bUseCustomProjectile && !bUseCustomClass"))
		bool bUseProjectileID = false;

	//(현재 지원안함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Info", meta = (EditCondition = "bUseCustomProjectile && !bUseCustomClass && bUseProjectileID"))
		int32 ProjectileID = -1;

	//asset, stat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Info", meta = (EditCondition = "bUseCustomProjectile && !bUseCustomClass && !bUseProjectileID"))
		FProjectileAssetInfoStruct ProjectileAssetInfo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Info", meta = (EditCondition = "bUseCustomProjectile && !bUseCustomClass && !bUseProjectileID"))
		FProjectileStatStruct ProjectileStatInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		int32 FireCountOverride = -1;

	//true라면 WeaponComponent의 MuzzleSocketLocation을 사용함
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		bool bUseMuzzleLocation = false;

	//false라면 WeaponComponent의 ProjectileClass를 사용함
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX")
		bool bUseCustomAsset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX", meta = (EditCondition = "bUseCustomAsset"))
		class UNiagaraSystem* ShootNiagaraEmitter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX/SFX", meta = (EditCondition = "bUseCustomAsset"))
		class USoundCue* ShootEffectSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool bIsDebugMode = false;

//------ Ranged 오버라이더블 ----------------------------
public:
	UFUNCTION(BlueprintCallable)
		bool TryFireProjectile(FRotator FireRotationOverride = FRotator::ZeroRotator, FVector FireLocationOverride = FVector::ZeroVector);

//------ Basic ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		class AProjectileBase* CreateProjectile(FRotator FireRotationOverride = FRotator::ZeroRotator, FVector FireLocationOverride = FVector::ZeroVector);

//------- Getter / Setter ---------------------------
protected:
	//Get Rotation List for FireProjectile
	UFUNCTION()
		TArray<FRotator> GetFireRotationList(int32 FireCount);

//------ Asset----------------------------------
protected:
	//투사체가 발사되는 이펙트를 출력한다
	UFUNCTION()
		void SpawnShootNiagaraEffect(FRotator FireRotationOverride, FVector FireLocationOverride);

//--------타이머 관련--------------------
private:
	//타이머 이벤트 전달을 위해서만 사용
	//델리게이트 바인딩 이후 파라미터를 직접 지정하면
	//타이머 이벤트 수행 도중에 파라미터가 메모리에서 소멸되기에 널 예외 발생
	FRotator FireRotationForTimer;

protected:
	//gamemode ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//inventory owner character (equal to getowner())
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class UAssetManagerBase> AssetManagerRef;

	//Weapon component 
	TWeakObjectPtr<class UWeaponComponentBase> WeaponComponentRef;
};
