// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatManager.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/AssetSystem/AssetManagerBase.h"
#include "../../Character/CharacterBase.h"
#include "../../Character/Component/WeaponComponentBase.h"

void UCombatManager::InitCombatManager(ABackStreetGameModeBase* NewGamemodeRef, ACharacterBase* NewOwnerCharacterRef, UWeaponComponentBase* NewWeaponComponentRef)
{
	GamemodeRef = NewGamemodeRef;
	OwnerCharacterRef = NewOwnerCharacterRef;
	WeaponComponentRef = NewWeaponComponentRef;
	AssetManagerRef = GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();
}

void UCombatManager::Attack()
{

}

void UCombatManager::StopAttack()
{

}
