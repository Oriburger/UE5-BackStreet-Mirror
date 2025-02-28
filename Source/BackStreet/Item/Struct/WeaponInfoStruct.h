#pragma once

#include "../../System/SkillSystem/Struct/SkillInfoStruct.h"
#include "../../Character/Struct/CharacterInfoEnum.h"
#include "Engine/DataTable.h"
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
	E_Shoot				UMETA(DisplayName = "Shoot")
};

UENUM(BlueprintType)
enum class EWeaponStatType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Attack				UMETA(DisplayName = "Attack"),
	E_AttackSpeed		UMETA(DisplayName = "AttackSpeed"),
	E_FinalAttack		UMETA(DisplayName = "FinalAttack")
};

USTRUCT(BlueprintType)
struct FUpgradableStatInfoContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	//업그레이드 스탯

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<uint8> RequiredMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float StatAdder = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bCanUpgradeLevel = false;
};

USTRUCT(BlueprintType)
struct FUpgradableStatInfoByLevelContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	//업그레이드 스탯

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<FUpgradableStatInfoContainer> StatInfoByLevel;
};

USTRUCT(BlueprintType)
struct FDebuffInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		ECharacterDebuffType Type = ECharacterDebuffType::E_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float TotalTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float Variable = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsPercentage = false;

	FDebuffInfoStruct() : Type(ECharacterDebuffType::E_None), TotalTime(0.0f), Variable(0.0f), bIsPercentage(false) {}
	FDebuffInfoStruct(ECharacterDebuffType NewType, float NewTotalTime, float NewVariable, bool NewIsPercentage)
		: Type(NewType), TotalTime(NewTotalTime), Variable(NewVariable), bIsPercentage(NewIsPercentage) {}
};

USTRUCT(BlueprintType)
struct FRangedWeaponStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	//----- 발사체 관련 Property ------------------
	//무한 탄약인지?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsInfiniteAmmo = false;

	//소지할 수 있는 최대 발사체 개수 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxTotalAmmo = 200;

	//한 번 발사 할 때, 몇 발을 내보낼지?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 ProjectileCountPerFire = 1; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float MultipleMaxFireAngle = 90.0f;
};

USTRUCT(BlueprintType)
struct FWeaponStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 WeaponID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName WeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName Description;

	//무기 이름 (현지화 지원)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FText WeaponNameText;

	//무기 설명 (현지화 지원)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FText DescriptionText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		EWeaponType WeaponType = EWeaponType::E_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UTexture2D* WeaponImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsWeaponOpen = false;
//----- 공통 Stat -------
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

	// Final Impact Strength, Default is zero.  1.0 + this value 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.0f, UIMax = 5.0f))
		float FinalImpactStrength = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float WeaponKnockBackStrength = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FDebuffInfoStruct DebuffInfo;

//----- CraftingStat ------
	//레벨별 스탯 강화 수치 및 요구 재료
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<EWeaponStatType, FUpgradableStatInfoByLevelContainer> UpgradableStatInfoMap;

//----- 원거리 Stat ------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FRangedWeaponStatStruct RangedWeaponStat;
};

USTRUCT(BlueprintType)
struct FRangedWeaponStateStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//현재 탄창에 있는 발사체 수
	UPROPERTY(BlueprintReadOnly)
		int32 CurrentAmmoCount = 0;

	inline bool GetIsEmpty() const { return CurrentAmmoCount == 0; }

	void UpdateAmmoValidation(int32 MaxTotalAmmo)
	{
		CurrentAmmoCount = FMath::Min(MaxTotalAmmo, CurrentAmmoCount);
	}
};

USTRUCT(BlueprintType)
struct FWeaponStateStruct
{
public:						
	GENERATED_USTRUCT_BODY()

	//현재 내구도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 CurrentDurability = 10;

	//temp, meleeonly
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FRotator SlashRotation = FRotator::ZeroRotator;

//-----Weapon Skill------
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FRangedWeaponStateStruct RangedWeaponState;

//-----Weapon Upgraded Stat
	//Upgraded State(in combat area)
	UPROPERTY(BlueprintReadWrite)
		TMap<EWeaponStatType, uint8> UpgradedStatMap;
};

USTRUCT(BlueprintType)
struct FMeleeWeaponAssetInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> HitEffectParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> HitImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> HitEffectParticleLarge;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> HitImpactSoundLarge;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> MeleeTrailParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		FColor MeleeTrailParticleColor = FColor::White;
};

USTRUCT(BlueprintType)
struct FRangedWeaponAssetInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//스폰할 발사체의 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 ProjectileID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UNiagaraSystem> ShootEffectParticle;
};

USTRUCT(BlueprintType)
struct FWeaponAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//아이템 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 WeaponID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		FName WeaponName;

	//무기 이름 (현지화 지원)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Localization")
		FText WeaponNameText;

	//스폰할 아이템 스태틱 메시 정보 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		TSoftObjectPtr<UStaticMesh> WeaponMesh;

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
		TSoftObjectPtr<class UParticleSystem> DestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> AttackSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> AttackFailSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged")
		FRangedWeaponAssetInfoStruct RangedWeaponAssetInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee")
		FMeleeWeaponAssetInfoStruct MeleeWeaponAssetInfo;
};