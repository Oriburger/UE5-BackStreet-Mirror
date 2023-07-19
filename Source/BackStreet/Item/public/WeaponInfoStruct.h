#pragma once

#include "Engine/DataTable.h"
#include "../../Character/public/CharacterInfoEnum.h"
#include "ProjectileInfoStruct.h"
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
struct FMeleeWeaponStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//----- 근접 관련 Property --------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float WeaponMeleeDamage = 0.2f;

	//근접 공격이 가능한 지? 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bCanMeleeAtk = true;

	//무기는 각 하나의 디버프만 가짐 (임시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		ECharacterDebuffType DebuffType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float DebuffTotalTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float DebuffVariable;
};

USTRUCT(BlueprintType)
struct FRangedWeaponStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	//----- 발사체 관련 Property ------------------
	//무한 탄약인지?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsInfiniteAmmo;

	//공격 시, 발사체가 있는지?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bHasProjectile = false;

	//무한 탄창인지?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bInfiniteMagazine;

	//한 탄창에 최대 발사체 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxAmmoPerMagazine;

	//소지할 수 있는 최대 발사체 개수 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxTotalAmmo = 200;

	//장전 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float LoadingDelayTime;
};

USTRUCT(BlueprintType)
struct FWeaponStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

		UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FName WeaponName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FName Description;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		EWeaponType WeaponType;

	//----- 공통 Stat -------
	//공격 속도 Rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float WeaponAtkSpeedRate = 1.0f;

	//무기 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float WeaponDamageRate = 1.0f;

	// 내구도 PROPERTY 추가
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bInfinite = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxDurability = 10;

	//인벤토리를 차지하는 칸 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		uint8 WeaponWeight = 1;

	//----- 근거리 Stat ------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FMeleeWeaponStatStruct MeleeWeaponStat;

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

	//탄창에 있는 탄환들의 총합
	UPROPERTY(BlueprintReadOnly)
		int32 TotalAmmoCount = 0;
};

USTRUCT(BlueprintType)
struct FWeaponStateStruct
{
public:						
	GENERATED_USTRUCT_BODY()

	//현재 내구도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 CurrentDurability = 10;

	//콤보 수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 ComboCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FRangedWeaponStateStruct RangedWeaponState;
};