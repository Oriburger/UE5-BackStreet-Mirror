#pragma once

#include "Engine/DataTable.h"
#include "../../Character/public/CharacterInfoEnum.h"
#include "../../SkillSystem/public/SkillInfoStruct.h"
#include "WeaponInfoStruct.generated.h"

UENUM(BlueprintType)
enum class ECameraShakeType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Hit				UMETA(DisplayName = "Hit"),
	E_Attack			UMETA(DisplayName = "Attack"),
	E_Explosion			UMETA(DisplayName = "Explosion"),
	E_Boss				UMETA(DisplayName = "Boss")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Melee				UMETA(DisplayName = "Melee"),
	E_Throw				UMETA(DisplayName = "Throw"),
	E_Shoot				UMETA(DisplayName = "Shoot")

};

USTRUCT(BlueprintType)
struct FDebuffInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		ECharacterDebuffType Type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float TotalTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float Variable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsPercentage;
};

USTRUCT(BlueprintType)
struct FRangedWeaponStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	//----- �߻�ü ���� Property ------------------
	//���� ź������?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsInfiniteAmmo;

	//���� źâ����?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bInfiniteMagazine;

	//�� źâ�� �ִ� �߻�ü ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxAmmoPerMagazine;

	//������ �� �ִ� �ִ� �߻�ü ���� 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxTotalAmmo = 200;

	//���� �ð�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float LoadingDelayTime;
};

USTRUCT(BlueprintType)
struct FWeaponStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 WeaponID;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FName WeaponName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FName Description;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UTexture2D* WeaponImage;

//----- ���� Stat -------
	//Weapon Attack Speed Rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float WeaponAtkSpeedRate = 1.0f;

	//Default Weapon Damage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 500.0f))
		float WeaponDamage = 10.0f;

	//Critical Damage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bCriticalApply = false; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 2.0f))
		float CriticalDamageRate = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bFixDamageApply = false; 

	//Fixed Damage Amount
	//This value is able to be used in skillsystem with editing dynamically
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 100.0f))
		float FixedDamageAmount = 0.0f;

	// ������ PROPERTY �߰�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bInfinite = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxDurability = 10;

	//�κ��丮�� �����ϴ� ĭ ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		uint8 WeaponWeight = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float WeaponKnockBackEnergy = 500.0f;

	//Weapon Skill ������ ��뷮 ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FSkillGaugeInfo SkillGaugeInfo;

	//Skill ID List
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FSkillSetInfo SkillSetInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FDebuffInfoStruct DebuffInfo;

//----- ���Ÿ� Stat ------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FRangedWeaponStatStruct RangedWeaponStat;
};

USTRUCT(BlueprintType)
struct FRangedWeaponStateStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//���� źâ�� �ִ� �߻�ü ��
	UPROPERTY(BlueprintReadOnly)
		int32 CurrentAmmoCount = 0;

	//���� źâ�� �ִ� źȯ���� ����
	UPROPERTY(BlueprintReadOnly)
		int32 ExtraAmmoCount = 0;
};

USTRUCT(BlueprintType)
struct FWeaponStateStruct
{
public:						
	GENERATED_USTRUCT_BODY()

	//���� ������
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 CurrentDurability = 10;

	//�޺� ��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 ComboCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FRangedWeaponStateStruct RangedWeaponState;
};

USTRUCT(BlueprintType)
struct FMeleeWeaponAssetInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UParticleSystem> HitEffectParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> MeleeTrailParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		FColor MeleeTrailParticleColor;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> HitImpactSound;
};

USTRUCT(BlueprintType)
struct FRangedWeaponAssetInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//������ �߻�ü�� ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 ProjectileID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> ShootEffectParticle;
};

USTRUCT(BlueprintType)
struct FWeaponAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//������ ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 WeaponID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		FName WeaponName;

	//������ ������ ����ƽ �޽� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		TSoftObjectPtr<UStaticMesh> WeaponMesh;

	//�޽��� �ʱ� ��ġ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialLocation;

	//�޽��� �ʱ� ȸ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FRotator InitialRotation;

	//�޽��� �ʱ� ũ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UParticleSystem> DestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> AttackSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> AttackFailSound;

	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
		FRangedWeaponAssetInfoStruct RangedWeaponAssetInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
		FMeleeWeaponAssetInfoStruct MeleeWeaponAssetInfo;
};