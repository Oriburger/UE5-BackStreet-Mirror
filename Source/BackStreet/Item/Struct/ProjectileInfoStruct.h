#pragma once

#include "../../Global/BackStreet.h"
#include "../../Character/Struct/CharacterInfoEnum.h"
#include "Engine/DataTable.h"
#include "ProjectileInfoStruct.generated.h"

USTRUCT(BlueprintType)
struct FProjectileStateStruct
{
public:
	GENERATED_USTRUCT_BODY()
	
	bool bCanAttackCauser = false;
};

USTRUCT(BlueprintType)
struct FProjectileStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 ProjectileID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName ProjectileName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName Description;

	//발사체 이름 (현지화 지원)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FText ProjectileNameText;

	//발사체 설명 (현지화 지원)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FText DescriptionText;

	//폭발 타입의 발사체인지? (RadialDamage)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsExplosive = false;

	//총알형 발사체인지? (bSimulatePhysics 유무)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsBullet = false; 

	//발사체의 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float ProjectileSpeed = 2000.0f;

	//Damage of projectile, inherited by parent (ranged weapon)
	UPROPERTY(BlueprintReadOnly)
		float ProjectileDamage = 0.2f;

	//중력 스케일
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GravityScale = 1.0f;
	
	//발사체 수명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float LifeTime = 10.0f;

	//유도가 되는지?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsHoming = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsHoming"))
		float HomingAcceleration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsHoming"))
		float TargetingCollisionScale = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool DebuffInfoOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "DebuffInfoOverride"))
		FDebuffInfoStruct DebuffInfo;
};

USTRUCT(BlueprintType)
struct FProjectileAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//아이템 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 ProjectileID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		FName ProjectileName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FName ProjectileNameText;

	//스폰할 아이템 스태틱 메시 정보 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		TSoftObjectPtr<UStaticMesh> ProjectileMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		float CollisionRadius = 18.0f;

	//메시의 초기 위치 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialLocation = FVector::ZeroVector;

	//메시의 초기 회전 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FRotator InitialRotation = FRotator::ZeroRotator;

	//메시의 초기 크기 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialScale = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> HitEffectParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> TrailParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> ExplodeParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UParticleSystem> HitEffectParticleLegacy;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> HitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> ExplosionSound;
};