// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/StaticMeshComponent.h"
#include "WeaponComponentBase.generated.h"
#define MAX_WEAPON_UPGRADABLE_STAT_IDX 3
/**
 * 
 */
UCLASS()
class BACKSTREET_API UWeaponComponentBase : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	UWeaponComponentBase();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//------- VFX --------------------------------
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraComponent* WeaponTrailParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraSystem* HitEffectParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraSystem* HitEffectParticleLarge;

//------- Basic property, Action -------------------
public:
	//process attack 
	UFUNCTION()
		void Attack();

	//stop process attack
	UFUNCTION()
		void StopAttack();

	//essential property
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		int32 WeaponID;

	//init weapon with load asset
	UFUNCTION(BlueprintCallable)
		void InitWeapon(int32 NewWeaponID);

//--------- Data Table, Asset Load ----------------------
public:
	UFUNCTION()
		void InitWeaponAsset();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponAssetInfoStruct GetWeaponAssetInfoWithID(int32 TargetWeaponID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStatStruct GetWeaponStatInfoWithID(int32 TargetWeaponID);

protected:
	//data table for weapon asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* WeaponAssetInfoTable;

	//data table for weapon stat
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* WeaponStatInfoTable;

	//data table for projectile asset
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* ProjectileAssetInfoTable;

	//data table for projectile stat
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* ProjectileStatInfoTable;

public:
	//asset info for this weapon instance
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		FWeaponAssetInfoStruct WeaponAssetInfo;


//------ State, Stat ---------------------------------
public:
	//Weapon stat
	UPROPERTY(EditInstanceOnly, Category = "Gameplay|Stat")
		FWeaponStatStruct WeaponStat;

	//Weapon state
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Stat")
		FWeaponStateStruct WeaponState;

	//발사체의 에셋 정보를 담을 캐시 변수
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileAssetInfoStruct ProjectileAssetInfo;

	//발사체의 스탯을 담을 캐시 변수
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileStatStruct ProjectileStatInfo;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsWeaponRangeType() { return WeaponStat.WeaponType == EWeaponType::E_Shoot || WeaponStat.WeaponType == EWeaponType::E_Throw; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStatStruct GetWeaponStat() { return WeaponStat; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStateStruct GetWeaponState() { return WeaponState; }

	UFUNCTION()
		FProjectileStatStruct GetProjectileStatInfo(int32 TargetProjectileID);

	UFUNCTION()
		FProjectileAssetInfoStruct GetProjectileAssetInfo(int32 TargetProjectileID);

	UFUNCTION(BlueprintCallable)
		void SetWeaponStat(FWeaponStatStruct NewStat) { WeaponStat = NewStat; }

	UFUNCTION(BlueprintCallable)
		void SetWeaponState(FWeaponStateStruct NewState) { WeaponState = NewState; }

	UFUNCTION(BlueprintCallable)
		uint8 GetLimitedStatLevel(EWeaponStatType WeaponStatType);

	UFUNCTION(BlueprintCallable)
		uint8 GetMaxStatLevel(EWeaponStatType WeaponStatType);

	//Calculate total damage to target character
	UFUNCTION()
		float CalculateTotalDamage(FCharacterStateStruct TargetState);

// ======		Upgrade		================
public:	
	UFUNCTION(BlueprintCallable)
	bool UpgradeStat(TArray<uint8> NewLevelList);
//-------- Combo ------------------------------------
public:
	//increase combo count
	UFUNCTION(BlueprintCallable)
		void UpdateComboState();

	//set timer for resetting combo count
	//UFUNCTION(BlueprintCallable)
	//	virtual void SetResetComboTimer();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentComboCnt() { return WeaponState.ComboCount; }

	UFUNCTION(BlueprintCallable)
		void ResetComboCnt() { WeaponState.ComboCount = 0; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsFinalCombo();

//-------- Combat Manager -------------------------------
public:
	UPROPERTY()
		class UMeleeCombatManager* MeleeCombatManager;

	UPROPERTY()
		class URangedCombatManager* RangedCombatManager;

//-------- Ref, TimerHandle ------------------------------
private:
	//gamemode ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//inventory owner character (equal to getowner())
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//Combo timer handle 
	FTimerHandle ComboTimerHandle;
};
