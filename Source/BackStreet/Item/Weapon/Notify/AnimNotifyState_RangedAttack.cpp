// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_RangedAttack.h"
#include "../Throw/ProjectileBase.h"
#include "../../../System/AssetSystem/AssetManagerBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Character/Component/ItemInventoryComponent.h"
#include "../../../Character/MainCharacter/MainCharacterBase.h"
#include "../../../Character/Component/WeaponComponentBase.h"
#include "Kismet/KismetMathLibrary.h"

void UAnimNotifyState_RangedAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!IsValid(MeshComp) || !IsValid(MeshComp->GetOwner()) || !MeshComp->GetOwner()->ActorHasTag("Character")) return;

	//Initialize
	OwnerCharacterRef = Cast<ACharacterBase>(MeshComp->GetOwner());
	WeaponComponentRef = OwnerCharacterRef.IsValid() ? OwnerCharacterRef.Get()->WeaponComponent : nullptr;
	AssetManagerRef = GamemodeRef.IsValid() ? GamemodeRef.Get()->GetGlobalAssetManagerBaseRef() : nullptr;
	GamemodeRef = OwnerCharacterRef.IsValid() ? Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(OwnerCharacterRef.Get()->GetWorld())) : nullptr;

	if (GamemodeRef.IsValid() && WeaponComponentRef.IsValid())
	{
		bool result = TryFireProjectile();
	}
}

void UAnimNotifyState_RangedAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{

}

void UAnimNotifyState_RangedAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{

}

bool UAnimNotifyState_RangedAttack::TryFireProjectile(FRotator FireRotationOverride, FVector FireLocationOverride)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (!WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo
		&& !OwnerCharacterRef->GetCharacterGameplayInfo().bInfinite && WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount == 0)
	{
		GamemodeRef.Get()->PrintSystemMessage(ESystemMessageType::E_NoSubWeapon, true);
		return false;
	}

	//check is ownerch is main char using casting
	//and override FireRotationOverride to AMainCharacterBase::GetAimingRotation()
	if(AMainCharacterBase* mainCharacter = Cast<AMainCharacterBase>(OwnerCharacterRef.Get()))
	{
		FireRotationOverride = mainCharacter->GetAimingRotation(WeaponComponentRef.Get()->GetComponentLocation());
	}
	if(bUseMuzzleLocation)
	{
		FireLocationOverride = WeaponComponentRef.Get()->GetSocketLocation(FName("Muzzle"));
	}

	//Ammo consumption
	if (!WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo
		&& !OwnerCharacterRef.Get()->GetCharacterGameplayInfo().bInfinite)
	{
		//한번 발사때 n개의 탄환을 소비하는지?
		WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount -= 1;
	}

	//Get fire count
	int32 fireCount = FireCountOverride == -1 ? WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.ProjectileCountPerFire : FireCountOverride;
	TArray<FRotator> rotationList = GetFireRotationList(fireCount);
	for (int32 idx = 0; idx < fireCount; idx++)
	{
		FRotator fireRotation = FireRotationOverride;
		if (rotationList.IsValidIndex(idx) && FireRotationOverride.IsNearlyZero(0.1f))
		{
			fireRotation = rotationList[idx];
		}

		//----- 커스텀 발사체 미사용시 -------------------
		if (!bUseCustomProjectile)
		{
			AProjectileBase* newProjectile = CreateProjectile(fireRotation, FireLocationOverride);

			//스폰한 발사체가 Valid 하다면 발사
			if (IsValid(newProjectile))
			{
				newProjectile->CheckInitialDamageCondition(15.0f, 15.0f, false);
				newProjectile->ActivateProjectileMovement();
				SpawnShootNiagaraEffect(fireRotation, FireLocationOverride); //발사와 동시에 이미터를 출력한다.

				if (GamemodeRef.IsValid() && OwnerCharacterRef.Get()->ActorHasTag("Player"))
				{
					GamemodeRef.Get()->PlayCameraShakeEffect(ECameraShakeType::E_Hit, OwnerCharacterRef.Get()->GetActorLocation(), 100.0f);
				}
			}
			else
			{
				if (AssetManagerRef.IsValid())
				{
					AssetManagerRef.Get()->PlaySingleSound(OwnerCharacterRef.Get(), ESoundAssetType::E_Weapon
						, WeaponComponentRef.Get()->WeaponID, "ShootFail");
				}
			}
		}
		//----- 커스텀 발사체 사용시 -------------------
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UAnimNotifyState_RangedAttack::TryFireProjectile - Custom Projectile is not supported in AnimNotifyState."));
			

			/*
			
			구현 필요

			*/
		
		}
	}

	//broadcast delegate
	const int32 weaponID = WeaponComponentRef.Get()->WeaponID;
	const FWeaponStatStruct weaponStat = WeaponComponentRef.Get()->GetWeaponStat();
	const FWeaponStateStruct weaponState = WeaponComponentRef.Get()->GetWeaponState();
	WeaponComponentRef.Get()->OnWeaponStateUpdated.Broadcast(weaponID, weaponStat, weaponState);

	return true;
}


AProjectileBase* UAnimNotifyState_RangedAttack::CreateProjectile(FRotator FireRotationOverride, FVector FireLocationOverride)
{
	FWeaponStatStruct weaponStat = WeaponComponentRef.Get()->WeaponStat;
	FWeaponStateStruct weaponState = WeaponComponentRef.Get()->WeaponState;

	FActorSpawnParameters spawmParams;
	spawmParams.Owner = OwnerCharacterRef.Get();
	spawmParams.Instigator = OwnerCharacterRef.Get()->GetInstigator();
	spawmParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

	FVector spawnLocation = FireLocationOverride.IsNearlyZero(0.01f) ?
		WeaponComponentRef.Get()->GetSocketLocation(FName("Muzzle"))
		: FireLocationOverride;
	FRotator spawnRotation = FireRotationOverride.IsNearlyZero(0.01f) ?
		OwnerCharacterRef.Get()->GetMesh()->GetComponentRotation()
		: FireRotationOverride;

	//캐릭터의 Forward 방향으로 맞춰준다.
	if (FireRotationOverride.IsNearlyZero(0.001f))
	{
		spawnRotation.Pitch = spawnRotation.Roll = 0.0f;
		spawnRotation.Yaw += 90.0f;
	}

	FTransform spawnTransform = { spawnRotation, spawnLocation, {1.0f, 1.0f, 1.0f} };
	AProjectileBase* newProjectile = Cast<AProjectileBase>(OwnerCharacterRef.Get()->GetWorld()->SpawnActor(AProjectileBase::StaticClass(), &spawnTransform, spawmParams));
	
	//bUseCustomProjectile이 아니면. WeaponComponentRef의 ProjectileAssetInfo, ProjectileStatInfo를 사용하여 초기화
	FProjectileAssetInfoStruct projectileAssetInfo = bUseCustomProjectile ? ProjectileAssetInfo : WeaponComponentRef.Get()->ProjectileAssetInfo;
	FProjectileStatStruct projectileStatInfo = bUseCustomProjectile ? ProjectileStatInfo : WeaponComponentRef.Get()->ProjectileStatInfo;

	if (IsValid(newProjectile) && WeaponComponentRef.Get()->ProjectileAssetInfo.ProjectileID != 0)
	{
		newProjectile->SetOwner(OwnerCharacterRef.Get());
		newProjectile->ProjectileStat.DebuffInfo = WeaponComponentRef.Get()->WeaponStat.DebuffInfo;
		newProjectile->InitProjectile(OwnerCharacterRef.Get(), projectileAssetInfo, projectileStatInfo);
		newProjectile->ProjectileStat.DamageOverride = WeaponComponentRef.Get()->WeaponStat.WeaponDamage;
		return newProjectile;
	}
	return nullptr;
}

TArray<FRotator> UAnimNotifyState_RangedAttack::GetFireRotationList(int32 FireCount)
{
	//RotationList for return
	TArray<FRotator> rotationList;
	//Get Character Rotation Yaw = Standard
	float standardRotation = OwnerCharacterRef.Get()->GetActorRotation().Yaw;
	//Get min and max Angle
	float maxFireAngle = WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.MultipleMaxFireAngle / (FireCount + 1.0f);
	float minFireAngle = -maxFireAngle;
	float step = (maxFireAngle - minFireAngle) / (FireCount - 1.0f);
	int32 halfReps = FireCount / 2;

	//Calculation Rotation Yaw and add to rotationList
	for (int32 idx = -halfReps; idx <= halfReps; idx++)
	{
		// idx == 0 add ZeroRotator
		if (idx == 0)
		{
			rotationList.Add(FRotator::ZeroRotator);
			continue;
		}

		//Calculation Yaw  
		float resultYaw = static_cast<float>(idx) * step / halfReps;
		rotationList.Add(FRotator(0.0f, standardRotation + resultYaw, 0.0f));
	}

	return rotationList;
}

void UAnimNotifyState_RangedAttack::SpawnShootNiagaraEffect(FRotator FireRotationOverride, FVector FireLocationOverride)
{
	if (!OwnerCharacterRef.IsValid() || !WeaponComponentRef.IsValid()) return;

	FVector spawnLocation = FireLocationOverride;
	FRotator spawnRotation = FireRotationOverride;

	if (bUseCustomAsset)
	{
		if (IsValid(ShootNiagaraEmitter))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(OwnerCharacterRef.Get()->GetWorld(), ShootNiagaraEmitter, spawnLocation, spawnRotation, FVector(0.1f));
		}
		if (IsValid(ShootEffectSound))
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShootEffectSound, spawnLocation);
		}
	}
	else
	{
		if (IsValid(WeaponComponentRef.Get()->ShootEffectParticle))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(OwnerCharacterRef.Get()->GetWorld(), WeaponComponentRef.Get()->ShootEffectParticle, spawnLocation, spawnRotation, FVector(0.1f));
		}
		if (AssetManagerRef.IsValid())
		{
			AssetManagerRef.Get()->PlaySingleSound(OwnerCharacterRef.Get(), ESoundAssetType::E_Weapon
				, WeaponComponentRef.Get()->WeaponID, "Shoot");
		}
	}
}
