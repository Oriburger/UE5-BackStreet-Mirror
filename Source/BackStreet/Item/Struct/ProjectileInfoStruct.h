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

	//�߻�ü �̸� (����ȭ ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FText ProjectileNameText;

	//�߻�ü ���� (����ȭ ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FText DescriptionText;

	//���� Ÿ���� �߻�ü����? (RadialDamage)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsExplosive = false;

	//�Ѿ��� �߻�ü����? (bSimulatePhysics ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsBullet = false; 

	//�߻�ü�� �ӵ�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float ProjectileSpeed = 2000.0f;

	//Damage of projectile, inherited by parent (ranged weapon)
	UPROPERTY(BlueprintReadOnly)
		float ProjectileDamage = 0.2f;

	//�߷� ������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GravityScale = 1.0f;
	
	//�߻�ü ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float LifeTime = 10.0f;

	//������ �Ǵ���?
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

	//������ ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 ProjectileID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		FName ProjectileName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FName ProjectileNameText;

	//������ ������ ����ƽ �޽� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		TSoftObjectPtr<UStaticMesh> ProjectileMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		float CollisionRadius = 18.0f;

	//�޽��� �ʱ� ��ġ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialLocation = FVector::ZeroVector;

	//�޽��� �ʱ� ȸ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FRotator InitialRotation = FRotator::ZeroRotator;

	//�޽��� �ʱ� ũ�� ����
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