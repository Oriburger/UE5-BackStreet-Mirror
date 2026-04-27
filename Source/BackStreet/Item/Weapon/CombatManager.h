// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "UObject/NoExportTypes.h"
#include "CombatManager.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UCombatManager : public UObject
{
	GENERATED_BODY()

//Basic function
public:
	UFUNCTION()
		void InitCombatManager(class ABackStreetGameModeBase * NewGamemodeRef, class ACharacterBase* NewOwnerCharacterRef
								, class UWeaponComponentBase * NewWeaponComponentRef);

	virtual void Attack();

	virtual void StopAttack();

//property 
protected:
	//gamemode ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//inventory owner character (equal to getowner())
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//Weapon component 
	TWeakObjectPtr<class UWeaponComponentBase> WeaponComponentRef;

	//Asset manager
	TWeakObjectPtr<class UAssetManagerBase> AssetManagerRef;
};
