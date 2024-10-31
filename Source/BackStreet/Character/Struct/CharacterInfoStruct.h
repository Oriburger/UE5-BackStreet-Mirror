#pragma once

#include "../../Item/Struct/ItemInfoStruct.h"
#include "../../System/MapSystem/Struct/StageInfoStruct.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "CharacterInfoStruct.generated.h"

UENUM(BlueprintType)
enum class EEmotionType : uint8
{
	E_Idle				UMETA(DisplayName = "Idle"),
	E_Angry				UMETA(DisplayName = "Angry"),
	E_Death				UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class ECharacterStatType : uint8
{
	E_None					UMETA(DisplayName = "None"),
	E_MaxHealth				UMETA(DisplayName = "MaxHealth"),
	E_Defense				UMETA(DisplayName = "Defense"),

	E_FlameResist			UMETA(DisplayName = "FlameResist"),
	E_PoisonResist			UMETA(DisplayName = "PoisonResist"),
	E_SlowResist			UMETA(DisplayName = "SlowResist"),
	E_StunResist			UMETA(DisplayName = "StunResist"),

	E_MoveSpeed				UMETA(DisplayName = "MoveSpeed"),
	E_RollDelay				UMETA(DisplayName = "RollDelay"),

	E_NormalPower			UMETA(DisplayName = "NormalPower"),			//[0, 1]
	E_NormalAttackSpeed		UMETA(DisplayName = "NormalSpeed"),			//[0, 1]
	E_NormalFatality		UMETA(DisplayName = "NormalFatality"),		//[0, 1]
	E_NormalFlame			UMETA(DisplayName = "NormalFlame"),			
	E_NormalPoison			UMETA(DisplayName = "NormalPoison"),
	E_NormalSlow			UMETA(DisplayName = "NormalSlow"),
	E_NormalStun			UMETA(DisplayName = "NormalStun"),

	E_DashAttackPower		UMETA(DisplayName = "DashAttackPower"),		//[0, 1]
	E_DashAttackSpeed		UMETA(DisplayName = "DashAttackSpeed"),		//[0, 1]
	E_DashAttackFatality	UMETA(DisplayName = "DashAttackFatality"),	//[0, 1]
	E_DashAttackFlame		UMETA(DisplayName = "DashAttackFlame"),
	E_DashAttackPoison		UMETA(DisplayName = "DashAttackPoison"),
	E_DashAttackSlow		UMETA(DisplayName = "DashAttackSlow"),
	E_DashAttackStun		UMETA(DisplayName = "DashAttackStun"),

	E_JumpAttackPower		UMETA(DisplayName = "JumpAttackPower"),		//[0, 1]
	E_JumpAttackSpeed		UMETA(DisplayName = "JumpAttackSpeed"),		//[0, 1]
	E_JumpAttackFatality	UMETA(DisplayName = "JumpAttackFatality"),	//[0, 1]
	E_JumpAttackFlame		UMETA(DisplayName = "JumpAttackFlame"),
	E_JumpAttackPoison		UMETA(DisplayName = "JumpAttackPoison"),
	E_JumpAttackSlow		UMETA(DisplayName = "JumpAttackSlow"),
	E_JumpAttackStun		UMETA(DisplayName = "JumpAttackStun"),

	E_SkillCoolTime			UMETA(DisplayName = "SkillCoolTime"),
	E_SubWeaponCapacity		UMETA(DisplayName = "SubWeaponCapacity"),
	E_HealItemPerformance	UMETA(DisplayName = "HealItemPerformance"),
	E_LuckyDrop				UMETA(DisplayName = "LuckyDrop")
};

USTRUCT(BlueprintType)
struct FStatValueGroup
{
public:
	GENERATED_USTRUCT_BODY()

//===================================================================
//====== Editable ===================================================
	UPROPERTY(VisibleAnywhere)
		ECharacterStatType StatType; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float DefaultValue = 0.0f;

	// 추가로 확률 정보가 필요한지? 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		bool bIsContainProbability;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (EditCondition = "bIsContainProbability", UIMin = 0.0f, UIMax = 1.0f))
		float ProbabilityValue = 1.0f;
		
//===================================================================
//====== State ======================================================
	UPROPERTY(BlueprintReadOnly)
		float DebuffRateValue;

	UPROPERTY(BlueprintReadOnly)
		float AbilityRateValue;

	UPROPERTY(BlueprintReadOnly)
		float SkillRateValue;

	UPROPERTY(BlueprintReadOnly)
		float TotalValue;

//===================================================================
//====== Function ===================================================
	bool IsValid() { return DefaultValue > 0.0f; }
	void Reset() { FStatValueGroup(); }
	void SetDefaultValue(float NewValue)
	{
		DefaultValue = NewValue;
		UpdateTotalValue();
	}
	void UpdateTotalValue()
	{
		TotalValue =  DefaultValue
			+ DefaultValue * (AbilityRateValue + SkillRateValue - DebuffRateValue);
	}
	float GetTotalValue() { return TotalValue; }
	
	FStatValueGroup(ECharacterStatType NewType) : StatType(NewType), DefaultValue(0.0f), bIsContainProbability(false), ProbabilityValue(1.0f)
						, DebuffRateValue(0.0f), AbilityRateValue(0.0f), SkillRateValue(0.0f), TotalValue(0.0f) {}

	FStatValueGroup() : StatType(ECharacterStatType::E_None), DefaultValue(0.0f), bIsContainProbability(false), ProbabilityValue(1.0f)
						, DebuffRateValue(0.0f), AbilityRateValue(0.0f), SkillRateValue(0.0f), TotalValue(0.0f) {}
};


USTRUCT(BlueprintType)
struct FCharacterDefaultStat : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//======= Default Stat ======================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
		int32 CharacterID = 0;
 
	//무한 내구도 / 무한 탄약 (Enemy 기본 스탯)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
		bool bInfinite = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
		bool bIsInvincibility = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultHP = 100.0f;

	//Multipiler Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 10.0f), Category = "Default")
		float DefaultAttack = 1.0f;

	//Multipiler Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultDefense = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultAttackSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 100.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultMoveSpeed = 400.0f;
};

USTRUCT(BlueprintType)
struct FCharacterGameplayInfo : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
//==========================================================
//====== Stat ==============================================
	//CharacterBase내 DefaultStat에 의해 초기화가 되는지?
	UPROPERTY(EditDefaultsOnly, Category = "Default")
		bool bUseDefaultStat = false;

	//캐릭터의 종류를 나타내는 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default", meta = (EditCondition = "!bUseDefaultStat", UIMin = 0, UIMax = 2500))
		int32 CharacterID = 0;

	//무한 내구도 / 무한 탄약 (Enemy 기본 스탯)
	UPROPERTY(BlueprintReadWrite, Category = "Stat", meta = (EditCondition = "!bUseDefaultStat"))
		bool bInfinite = false;

	UPROPERTY(BlueprintReadWrite, Category = "Stat", meta = (EditCondition = "!bUseDefaultStat"))
		bool bIsInvincibility = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat", meta = (EditCondition = "!bUseDefaultStat"))
		TArray<FStatValueGroup> StatGroupList;

//=========================================================
//====== State ============================================
		
//MaxHP Attack DashAttack JumpAttack AttackSpeed MoveSpeed Defense;

	//캐릭터의 디버프 상태 (Bit-Field로 표현)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		int32 CharacterDebuffState = (1 << 10);

	//캐릭터의 행동 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		ECharacterActionType CharacterActionState = ECharacterActionType::E_Idle;

	//PlayerMaxHP는 1.0f
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		float CurrentHP = 0.0f;

	//공격을 할 수 있는 상태인지?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		bool bCanAttack = false;

	//Is character sprinting?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		bool bIsAiming = false;

	//Air Atk Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		bool bIsAirAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		bool bIsDownwardAttacking = false;

	//Hit Counter For Knockback Event
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		int32 HitCounter = 0;

//=========================================================
//====== 미분류 ============================================
	//for time-attack stage
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "State")
		float ExtraStageTime = 0.0f;

	//Extra wish list in skill
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "State")
		int32 MaxKeepingSkillCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "State")
		bool bInfiniteSkillMaterial = false;
	
	//Extra percentage for universal material
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "State")
		float ExtraPercentageUnivMaterial = 0.0f;

//=========================================================
//====== Function =========================================
	bool IsValid() { return CharacterID > 0; }

	FStatValueGroup& GetStatGroup(ECharacterStatType StatType)
	{
		return StatGroupList[(uint8)StatType];
	}
	float GetSkillStatInfo(ECharacterStatType StatType) { return GetStatGroup(StatType).SkillRateValue; }
	float GetDebuffStatInfo(ECharacterStatType StatType) { return GetStatGroup(StatType).DebuffRateValue; }
	float GetAbilityStatInfo(ECharacterStatType StatType) { return GetStatGroup(StatType).AbilityRateValue; }
	
	float GetTotalValue(ECharacterStatType StatType) 
	{ 
		GetStatGroup(StatType).UpdateTotalValue();
		return GetStatGroup(StatType).GetTotalValue(); 
	}
	
	bool SetDefaultStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		target.DefaultValue = NewValue;
		target.UpdateTotalValue();
		return target.IsValid();
	}
	bool SetSkillStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		target.SkillRateValue = NewValue;
		target.UpdateTotalValue();
		return target.IsValid();
	}
	bool SetDebuffStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		target.DebuffRateValue = NewValue;
		target.UpdateTotalValue();
		return target.IsValid();
	}
	bool SetAbilityStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		target.AbilityRateValue = NewValue;
		target.UpdateTotalValue();
		return target.IsValid();
	}

	FCharacterGameplayInfo() : CharacterID(0) , bInfinite(false) , bIsInvincibility(false), StatGroupList(), CharacterDebuffState(1 << 10)
		, CharacterActionState(ECharacterActionType::E_Idle), CurrentHP(0.0f), bCanAttack(false), bIsSprinting(false), bIsAiming(false)
		, bIsAirAttacking(false), bIsDownwardAttacking(false), HitCounter(0), ExtraStageTime(0.0f), MaxKeepingSkillCount(1)
		, bInfiniteSkillMaterial(false), ExtraPercentageUnivMaterial(0.0f)
	{
		for (uint8 typeIdx = static_cast<uint8>(ECharacterStatType::E_None)
			; typeIdx <= static_cast<uint8>(ECharacterStatType::E_LuckyDrop); ++typeIdx)
		{
			ECharacterStatType type = static_cast<ECharacterStatType>(typeIdx);
			StatGroupList.Add(FStatValueGroup(type));
		}
	}

	FCharacterGameplayInfo(FCharacterDefaultStat DefaultStat) : CharacterID(DefaultStat.CharacterID), bInfinite(DefaultStat.bInfinite), bIsInvincibility(DefaultStat.bIsInvincibility)
		, StatGroupList(), CharacterDebuffState(1 << 10), CharacterActionState(ECharacterActionType::E_Idle), CurrentHP(0.0f), bCanAttack(false)
		, bIsSprinting(false), bIsAiming(false), bIsAirAttacking(false), bIsDownwardAttacking(false), HitCounter(0), ExtraStageTime(0.0f)
		, MaxKeepingSkillCount(1), bInfiniteSkillMaterial(false), ExtraPercentageUnivMaterial(0.0f)
	{
		for (uint8 typeIdx = static_cast<uint8>(ECharacterStatType::E_None)
			; typeIdx <= static_cast<uint8>(ECharacterStatType::E_LuckyDrop); ++typeIdx)
		{
			ECharacterStatType type = static_cast<ECharacterStatType>(typeIdx);
			StatGroupList.Add(FStatValueGroup(type));
		}
		StatGroupList[(uint8)ECharacterStatType::E_MaxHealth].SetDefaultValue(DefaultStat.DefaultHP);
		StatGroupList[(uint8)ECharacterStatType::E_Defense].SetDefaultValue(DefaultStat.DefaultDefense);
		StatGroupList[(uint8)ECharacterStatType::E_NormalPower].SetDefaultValue(DefaultStat.DefaultAttack);
		StatGroupList[(uint8)ECharacterStatType::E_MoveSpeed].SetDefaultValue(DefaultStat.DefaultMoveSpeed);
		StatGroupList[(uint8)ECharacterStatType::E_NormalAttackSpeed].SetDefaultValue(DefaultStat.DefaultAttackSpeed);
	}
};

//======================================================
//====== Character Common Info  ========================
//======================================================

USTRUCT(BlueprintType)
struct FPlayerInputActionInfo
{
//------------------Basic---------------------------------------------------------------------------
public:
	GENERATED_USTRUCT_BODY()

	// Move Input Action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Basic")
		class UInputAction* MoveAction;

	// Roll Input Action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Basic")
		class UInputAction* RollAction;

	// Look Input Action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Basic")
		class UInputAction* LookAction;

//------------------Combat---------------------------------------------------------------------------
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* UpperAttackAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* ShootAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* ZoomAction;

//------------------Action---------------------------------------------------------------------------
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* InvestigateAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* SwitchWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* DropWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* PickSubWeaponAction;

	// Jump Input Action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* JumpAction;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* SprintAction;

//------------------UI---------------------------------------------------------------------------
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* PlayerInfoUIAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* ConfirmUIAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* CancelUIAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* MoveUIAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* SwitchTabUIAction;
};

//===============================================
//====== Enemy CharacterInfo  ==========================
//===============================================

USTRUCT(BlueprintType)
struct FEnemyDropInfoStruct
{
	GENERATED_BODY()

	public:
	//Max item count to spawn after dead event.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem", meta = (UIMin = 0, UIMax = 2))
		int32 MaxSpawnItemCount = 0;

	//Item type list to spawn after dead event. (each item is identified by index)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<EItemCategoryInfo> SpawnItemTypeList;

	//Item ID list to spawn after dead event.(each item is identified by index)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<int32> SpawnItemIDList;

	//Item spawn percentage list.  (0.0f ~ 1.0f), (each item is identified by index)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<float> ItemSpawnProbabilityList;
};

USTRUCT(BlueprintType)
struct FEnemyStatStruct : public FTableRowBase
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditAnywhere)
		int32 EnemyID = 0;

	UPROPERTY(EditAnywhere)
		FName EnemyName = FName("");

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay")
		FCharacterDefaultStat DefaultStat;

	//Enemy's Default Weapon ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 DefaultWeaponID = 0;

	//Info data of dropping item when enemy dies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FEnemyDropInfoStruct DropInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
		TArray<int32> EnemySkillIDList; 
};


/*
USTRUCT(BlueprintType)
struct FStatInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	float PercentValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float FixedValue = 0.0f;
};

USTRUCT(BlueprintType)
struct FCharacterStateStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//====== Character State ===============
		//캐릭터의 디버프 상태 (Bit-Field로 표현)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CharacterDebuffState = (1 << 10);

	//공격을 할 수 있는 상태인지?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bCanAttack = false;

	//Is character sprinting?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAiming = false;

	//캐릭터의 행동 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	ECharacterActionType CharacterActionState = ECharacterActionType::E_Idle;

	//====== Current Stat ===================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TotalHP = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TotalAttack = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TotalDefense = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TotalMoveSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TotalAttackSpeed = 0.0f;

	//PlayerMaxHP는 1.0f
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentHP = 0.0f;

	//======= Additional Stat , Uneditable ======================
	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct AbilityHP;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct AbilityAttack;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct AbilityAttackSpeed;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct AbilityMoveSpeed;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct AbilityDefense;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct SkillHP;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct SkillAttack;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct SkillAttackSpeed;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct SkillMoveSpeed;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct SkillDefense;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct DebuffHP;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct DebuffAttack;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct DebuffAttackSpeed;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct DebuffMoveSpeed;

	UPROPERTY(BlueprintReadWrite)
	FStatInfoStruct DebuffDefense;

	//====== Player ====================================	
		//Air Atk Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsAirAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsDownwardAttacking = false;

	//Hit Counter For Knockback Event
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 HitCounter = 0;
};
*/