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

	E_BurnResist			UMETA(DisplayName = "BurnResist"),
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
	E_DebuffTime			UMETA(DisplayName = "NormalDebuffTime"),	//[0, 1] (2.5sec + val)
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float MaxValue = 0.0f;

	// �߰��� Ȯ�� ������ �ʿ�����? 
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
	inline bool IsValid() { return DefaultValue > 0.0f; }
	inline void Reset() { FStatValueGroup(); }
	inline void SetDefaultValue(float NewValue)
	{
		if (NewValue < 0.0f) return;
		DefaultValue = NewValue;
		UpdateTotalValue();
	}
	inline void UpdateTotalValue()
	{
		TotalValue = DefaultValue != 0.0f
					? DefaultValue + DefaultValue * (AbilityRateValue + SkillRateValue - DebuffRateValue)
					: (AbilityRateValue + SkillRateValue - DebuffRateValue);

		if (MaxValue != 0.0f)
		{
			TotalValue = FMath::Min(TotalValue, MaxValue);
		}

		if (TotalValue < 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("FStatValueGroup::UpdateTotalValue T : %.2lf, D : %.2lf, A : %.2lf, S : %.2lf, DF: %.2lf")
						, TotalValue, DefaultValue, AbilityRateValue, SkillRateValue, DebuffRateValue);
		}
	}
	inline float GetTotalValue() { return TotalValue; }

	FStatValueGroup(ECharacterStatType NewType) : StatType(NewType), DefaultValue(0.0f), MaxValue(0.0f), bIsContainProbability(false)
					, ProbabilityValue(1.0f), DebuffRateValue(0.0f), AbilityRateValue(0.0f), SkillRateValue(0.0f), TotalValue(0.0f) {}

	FStatValueGroup() : StatType(ECharacterStatType::E_None), DefaultValue(0.0f), MaxValue(0.0f), bIsContainProbability(false)
					, ProbabilityValue(1.0f), DebuffRateValue(0.0f), AbilityRateValue(0.0f), SkillRateValue(0.0f), TotalValue(0.0f) {}
};


USTRUCT(BlueprintType)
struct FCharacterDefaultStat : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//======= Default Stat ======================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
		int32 CharacterID = 0;
 
	//���� ������ / ���� ź�� (Enemy �⺻ ����)
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 0.8f), Category = "Default")
		float DefaultBurnResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 0.8f), Category = "Default")
		float DefaultPoisonResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 0.8f), Category = "Default")
		float DefaultSlowResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 0.8f), Category = "Default")
		float DefaultStunResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultKnockBackResist = 0.0f;
};

USTRUCT(BlueprintType)
struct FCharacterGameplayInfo : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	//==========================================================
	//====== Stat ==============================================
		//CharacterBase�� DefaultStat�� ���� �ʱ�ȭ�� �Ǵ���?
	UPROPERTY(EditDefaultsOnly, Category = "Default")
		bool bUseDefaultStat = false;

	//ĳ������ ������ ��Ÿ���� ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default", meta = (EditCondition = "!bUseDefaultStat", UIMin = 0, UIMax = 2500))
		int32 CharacterID = 0;

	//���� ������ / ���� ź�� (Enemy �⺻ ����)
	UPROPERTY(BlueprintReadWrite, Category = "Stat", meta = (EditCondition = "!bUseDefaultStat"))
		bool bInfinite = false;

	UPROPERTY(BlueprintReadWrite, Category = "Stat", meta = (EditCondition = "!bUseDefaultStat"))
		bool bIsInvincibility = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat", meta = (EditCondition = "!bUseDefaultStat"))
		TArray<FStatValueGroup> StatGroupList;

	//=========================================================
	//====== State ============================================

	//MaxHP Attack DashAttack JumpAttack AttackSpeed MoveSpeed Defense;

		//ĳ������ ����� ���� (Bit-Field�� ǥ��)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		int32 CharacterDebuffState = (1 << 10);

	//ĳ������ �ൿ ����
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		ECharacterActionType CharacterActionState = ECharacterActionType::E_Idle;

	//PlayerMaxHP�� 1.0f
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		float CurrentHP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float CurrentSP = 0.0f;

	//������ �� �� �ִ� ��������?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		bool bCanAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		bool bCanRoll = false;

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
	//====== �̺з� ============================================
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
	inline bool IsValid() { return CharacterID >= 0; }

	inline FStatValueGroup& GetStatGroup(ECharacterStatType StatType)
	{
		return StatGroupList[(uint8)StatType];
	}
	inline void UpdateTotalValues()
	{
		for (FStatValueGroup& target : StatGroupList)
			target.UpdateTotalValue();
	}
	inline float GetSkillStatInfo(ECharacterStatType StatType) { return GetStatGroup(StatType).SkillRateValue; }
	inline float GetDebuffStatInfo(ECharacterStatType StatType) { return GetStatGroup(StatType).DebuffRateValue; }
	inline float GetAbilityStatInfo(ECharacterStatType StatType) { return GetStatGroup(StatType).AbilityRateValue; }
	inline float GetProbabilityStatInfo(ECharacterStatType StatType) { return GetStatGroup(StatType).bIsContainProbability ? GetStatGroup(StatType).ProbabilityValue : -1.0f; }
	inline float GetMaxStatValue(ECharacterStatType StatType){ return GetStatGroup(StatType).MaxValue; }
	inline bool GetIsContainProbability(ECharacterStatType StatType) {	return GetStatGroup(StatType).bIsContainProbability; }
	inline float GetTotalValue(ECharacterStatType StatType) 
	{
		GetStatGroup(StatType).UpdateTotalValue();
		return GetStatGroup(StatType).GetTotalValue(); 
	}
	inline bool SetDefaultStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		target.DefaultValue = NewValue;
		target.UpdateTotalValue();
		return target.IsValid();
	}
	inline bool SetSkillStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		target.SkillRateValue = NewValue;
		target.UpdateTotalValue();
		return target.IsValid();
	}
	inline bool SetDebuffStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		target.DebuffRateValue = NewValue;
		target.UpdateTotalValue();
		return target.IsValid();
	}
	inline bool SetAbilityStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		target.AbilityRateValue = NewValue;
		target.UpdateTotalValue();
		return target.IsValid();
	}
	inline bool SetProbabilityStatInfo(ECharacterStatType StatType, float NewValue)
	{
		FStatValueGroup& target = GetStatGroup(StatType);
		if (!target.bIsContainProbability || NewValue < 0.0f) return false;
		target.ProbabilityValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
		return true;
	}
	static FORCEINLINE ECharacterStatType GetDebuffStatType(bool bIsJumpAttacking, bool bIsDashAttacking, ECharacterDebuffType DebuffType)
	{
		// �⺻���� Normal ���� �������� ����
		if (bIsJumpAttacking)
		{
			switch (DebuffType)
			{
			case ECharacterDebuffType::E_Burn:
				return ECharacterStatType::E_JumpAttackFlame;
			case ECharacterDebuffType::E_Poison:
				return ECharacterStatType::E_JumpAttackPoison;
			case ECharacterDebuffType::E_Stun:
				return ECharacterStatType::E_JumpAttackStun;
			case ECharacterDebuffType::E_Slow:
				return ECharacterStatType::E_JumpAttackSlow;
			default:
				return ECharacterStatType::E_None; // ����� Ÿ���� ���� ���
			}
		}
		else if (bIsDashAttacking)
		{
			switch (DebuffType)
			{
			case ECharacterDebuffType::E_Burn:
				return ECharacterStatType::E_DashAttackFlame;
			case ECharacterDebuffType::E_Poison:
				return ECharacterStatType::E_DashAttackPoison;
			case ECharacterDebuffType::E_Stun:
				return ECharacterStatType::E_DashAttackStun;
			case ECharacterDebuffType::E_Slow:
				return ECharacterStatType::E_DashAttackSlow;
			default:
				return ECharacterStatType::E_None;
			}
		}
		else
		{
			switch (DebuffType)
			{
			case ECharacterDebuffType::E_Burn:
				return ECharacterStatType::E_NormalFlame;
			case ECharacterDebuffType::E_Poison:
				return ECharacterStatType::E_NormalPoison;
			case ECharacterDebuffType::E_Stun:
				return ECharacterStatType::E_NormalStun;
			case ECharacterDebuffType::E_Slow:
				return ECharacterStatType::E_NormalSlow;
			default:
				return ECharacterStatType::E_None;
			}
		}
	}

	FCharacterGameplayInfo() : bUseDefaultStat(false), CharacterID(0) , bInfinite(false) , bIsInvincibility(false), StatGroupList(), CharacterDebuffState(1 << 10)
		, CharacterActionState(ECharacterActionType::E_Idle), CurrentHP(0.0f), CurrentSP(0.0f), bCanAttack(false), bCanRoll(false), bIsSprinting(false), bIsAiming(false)
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

	FCharacterGameplayInfo(FCharacterDefaultStat DefaultStat) : bUseDefaultStat(false), CharacterID(DefaultStat.CharacterID), bInfinite(DefaultStat.bInfinite), bIsInvincibility(DefaultStat.bIsInvincibility)
		, StatGroupList(), CharacterDebuffState(1 << 10), CharacterActionState(ECharacterActionType::E_Idle), CurrentHP(0.0f), CurrentSP(0.0f), bCanAttack(false), bCanRoll(false)
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
		StatGroupList[(uint8)ECharacterStatType::E_BurnResist].SetDefaultValue(DefaultStat.DefaultBurnResist);
		StatGroupList[(uint8)ECharacterStatType::E_PoisonResist].SetDefaultValue(DefaultStat.DefaultPoisonResist);
		StatGroupList[(uint8)ECharacterStatType::E_SlowResist].SetDefaultValue(DefaultStat.DefaultSlowResist);
		StatGroupList[(uint8)ECharacterStatType::E_StunResist].SetDefaultValue(DefaultStat.DefaultStunResist);
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

	// Jump Input Action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* SprintAction;
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