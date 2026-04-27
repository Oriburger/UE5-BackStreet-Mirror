#pragma once                  

#include "../../Item/Struct/ItemInfoStruct.h"
#include "../../System/MapSystem/Struct/StageInfoStruct.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "CharacterInfoStruct.generated.h"

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
					? DefaultValue + DefaultValue * (AbilityRateValue + SkillRateValue)
					: (AbilityRateValue + SkillRateValue);
		TotalValue *= (1.0f - DebuffRateValue);

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
	inline float GetDebuffRateValue() { return DebuffRateValue; }
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

	//==========Debuff==============================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultMaxFlameStack = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultMaxPoisonStack = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultMaxFrameDropStack = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultFlame = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultPoison = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultFrameDrop = 0.0f;
	//==============================================

	//Multipiler Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultDefense = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultAttackSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 100.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultMoveSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultBurnResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultPoisonResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultSlowResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultStunResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultKnockBackResist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 1.0f), Category = "Default")
		float DefaultActionLaunchDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 100.0f), Category = "Default")
		float DefaultMaxAP = 1.0f;
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

	//메인 캐릭터 스킬 게이지	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float CurrentAP = 0.0f;
	
	//디버프 스택==============================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float CurrentFlameStack = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float CurrentPoisonStack = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float CurrentframeDropStack = 0.0f;
	//=========================================================
	
	//공격을 할 수 있는 상태인지?
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
	//====== Extra ============================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Extra")
		float TotalDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Extra")
		float SubWeaponTotalDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Extra")
		float TotalHealAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Extra")
		float TotalDamageTaken = 0.0f;

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
	inline float GetDebuffRateValue(ECharacterStatType StatType)
	{
		GetStatGroup(StatType).UpdateTotalValue();
		return FMath::Clamp(GetStatGroup(StatType).GetDebuffRateValue(), 0.0f, 0.99f);
	}
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
    inline void ResetAllAbilityStatInfo()
    {
		for (uint8 typeIdx = static_cast<uint8>(ECharacterStatType::E_None) + 1
				; typeIdx < static_cast<uint8>(ECharacterStatType::Count); ++typeIdx)
		{
			ECharacterStatType type = static_cast<ECharacterStatType>(typeIdx);
			FStatValueGroup& target = GetStatGroup(type);
			target.AbilityRateValue = 0.0f;
			target.UpdateTotalValue();
		}
    }

	static FORCEINLINE ECharacterStatType GetDebuffStatType(bool bIsJumpAttacking, bool bIsDashAttacking, ECharacterDebuffType DebuffType)
	{
		// 기본값은 Normal 공격 유형으로 설정
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
				return ECharacterStatType::E_None; // 디버프 타입이 없을 경우
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
		, CharacterActionState(ECharacterActionType::E_Idle), CurrentHP(0.0f), CurrentAP(0.0f), bCanAttack(false), bCanRoll(false), bIsSprinting(false), bIsAiming(false)
		, bIsAirAttacking(false), bIsDownwardAttacking(false), HitCounter(0), TotalDamage(0.0f), SubWeaponTotalDamage(0.0f), TotalHealAmount(0.0f), TotalDamageTaken(0.0f)
	{
		for (uint8 typeIdx = static_cast<uint8>(ECharacterStatType::E_None)
			; typeIdx < static_cast<uint8>(ECharacterStatType::Count); ++typeIdx)
		{
			ECharacterStatType type = static_cast<ECharacterStatType>(typeIdx);
			StatGroupList.Add(FStatValueGroup(type));
		}
	}
	FCharacterGameplayInfo(FCharacterDefaultStat DefaultStat) : bUseDefaultStat(false), CharacterID(DefaultStat.CharacterID), bInfinite(DefaultStat.bInfinite), bIsInvincibility(DefaultStat.bIsInvincibility)
		, StatGroupList(), CharacterDebuffState(1 << 10), CharacterActionState(ECharacterActionType::E_Idle), CurrentHP(0.0f), CurrentAP(0.0f), bCanAttack(false), bCanRoll(false)
		, bIsSprinting(false), bIsAiming(false), bIsAirAttacking(false), bIsDownwardAttacking(false), HitCounter(0), TotalDamage(0.0f), SubWeaponTotalDamage(0.0f), TotalHealAmount(0.0f), TotalDamageTaken(0.0f)
	{
		for (uint8 typeIdx = static_cast<uint8>(ECharacterStatType::E_None)
			; typeIdx < static_cast<uint8>(ECharacterStatType::Count); ++typeIdx)
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
		StatGroupList[(uint8)ECharacterStatType::E_ActionLaunchDistance].SetDefaultValue(DefaultStat.DefaultActionLaunchDistance);
		StatGroupList[(uint8)ECharacterStatType::E_MaxAP].SetDefaultValue(DefaultStat.DefaultMaxAP);
		
		//Debuff Default Value
		StatGroupList[(uint8)ECharacterStatType::E_NormalFlame].SetDefaultValue(DefaultStat.DefaultFlame);
		StatGroupList[(uint8)ECharacterStatType::E_NormalPoison].SetDefaultValue(DefaultStat.DefaultPoison);
		StatGroupList[(uint8)ECharacterStatType::E_NormalSlow].SetDefaultValue(DefaultStat.DefaultFrameDrop);

		StatGroupList[(uint8)ECharacterStatType::E_MaxFlameStack].SetDefaultValue(DefaultStat.DefaultMaxFlameStack);
		StatGroupList[(uint8)ECharacterStatType::E_MaxPoisonStack].SetDefaultValue(DefaultStat.DefaultMaxPoisonStack);
		StatGroupList[(uint8)ECharacterStatType::E_MaxFrameDropStack].SetDefaultValue(DefaultStat.DefaultMaxFrameDropStack);
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
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Basic")
		class UInputAction* LockOnAction;

//------------------Combat---------------------------------------------------------------------------
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* AttackAction;

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

//=========================================================
//====== Enemy CharacterInfo  =============================
//=========================================================

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

//==========================================================================
//============   Ability   =================================================
//==========================================================================

USTRUCT(BlueprintType)
struct FAbilityValueInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float Variable = 0.0f;

	// 추가로 확률 정보가 필요한지? 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsContainProbability = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsContainProbability", UIMin = 0.0f, UIMax = 1.0f))
		float ProbabilityValue = 1.0f;

public:
	FAbilityValueInfoStruct() : Variable(0.0f), bIsContainProbability(false), ProbabilityValue(0.0f) {}
};

USTRUCT(BlueprintType)
struct FBattleAppSlotUnlockInfo
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EComboType ComboType = EComboType::E_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 TargetSlotIndex = -1;	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EBattleAppSlotState PrevSlotState = EBattleAppSlotState::E_Locked;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EBattleAppSlotState NewSlotState = EBattleAppSlotState::E_Locked;
};

UENUM(BlueprintType)
enum class EAbilityTierType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Common			UMETA(DisplayName = "Common"),
	E_Rare				UMETA(DisplayName = "Rare"),
	E_Legendary			UMETA(DisplayName = "Legendary"),
	E_Mythic			UMETA(DisplayName = "Mythic"),
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	E_None						UMETA(DisplayName = "None"),
	//Legacy
	E_BasicStat					UMETA(DisplayName = "BasicStat"),
	E_Debuff					UMETA(DisplayName = "Debuff"),
	E_Action					UMETA(DisplayName = "Action"),
	E_Item						UMETA(DisplayName = "Item"),
	E_SpecialAction				UMETA(DisplayName = "SpecialAction"),

	//New 
	E_Flame						UMETA(DisplayName = "Flame"),
	E_Poison					UMETA(DisplayName = "Poison"),
	E_Slow						UMETA(DisplayName = "Slow"),
	E_Stun						UMETA(DisplayName = "Stun"),
};

USTRUCT(BlueprintType)
struct FAbilityInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//어빌리티의 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0, UIMax = 10), Category = "Basic")
		int32 AbilityId = 0;

	//for Multiple Stat
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TMap<ECharacterStatType, FAbilityValueInfoStruct> TargetStatMap;

	//어빌리티의 분류
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		EAbilityType AbilityType = EAbilityType::E_None;

	//어빌리티명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic")
		FName AbilityName;

	//어빌리티명 (현지화 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic")
		FText AbilityNameText;

	//Ability's Tier
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		EAbilityTierType AbilityTier = EAbilityTierType::E_None;

	//어빌리티 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic")
		FName AbilityDescription;

	//어빌리티 설명 (현지화 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic")
		FText AbilityDescriptionText;

	//어빌리티의 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Asset")
		UTexture2D* AbilityIcon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Asset")
		class UMediaSource* AbilityPreviewMedia;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Asset")
		class UTexture2D* AbilityPreviewImage;

	//Unlock / Upgrade Condition
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
		int32 RequireItemID = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
		int32 RequireCost = -1;

//------------------ Extra Action Info -----------------------
public: 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction")
		bool bIsRequireExtraAction = false;

	//bIsRequireExtraAction이 true면 편집 가능
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction", meta = (EditCondition = "bIsRequireExtraAction"))
		TSubclassOf<class AAbilityExtraActionBase> ExtraActionClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction", meta = (EditCondition = "bIsRequireExtraAction"))
		bool bIsExtraActionRepetitive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction", meta = (EditCondition = "bIsRequireExtraAction"))
		float ExtraActionCoolTime = -1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction", meta = (EditCondition = "bIsRequireExtraAction"))
		float ExtraActionDuration = -1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction", meta = (EditCondition = "bIsRequireExtraAction"))
		bool bIsOneTimeUse = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction", meta = (EditCondition = "bIsRequireExtraAction"))
		bool bIsAutoTrigger = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ExtraAction", meta = (EditCondition = "bIsRequireE  vd v\xtraAction && !bIsAutoTrigger"))
		ECharacterActionType TriggerActionType = ECharacterActionType::E_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction", meta = (EditCondition = "bIsRequireExtraAction && !bIsAutoTrigger"))
		EActionState TriggerActionState = EActionState::E_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExtraAction", meta = (EditCondition = "bIsRequireExtraAction && !bIsAutoTrigger"))
		TArray<int32> TriggerComboIdxList = {};

//-------------------------- etc -----------------------------
public:
	//Repetitive 연산을 위한 TimerHandle
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;

public:
	FString GetActionTriggerName() 
	{
		return UEnum::GetValueAsString(TriggerActionType);
	}

	inline bool operator==(const FAbilityInfoStruct& other) const
	{
		return AbilityId == other.AbilityId;
	}

	inline bool IsValid() const
	{
		return AbilityId > 0;
	}
};

USTRUCT(BlueprintType)
struct FBattleAppEquipInfoStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsEquipped = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsPassiveApp = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EComboType ComboType = EComboType::E_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 EquippedSlotIndex = -1;

public:
	void Update(bool InIsEquipped, bool bInIsPassiveApp, EComboType InComboType, int32 InEquippedSlotIndex)
	{
		bIsEquipped = InIsEquipped;
		bIsPassiveApp = bInIsPassiveApp;
		ComboType = InComboType;
		EquippedSlotIndex = InEquippedSlotIndex;
	}

	void Reset()
	{
		bIsEquipped = false;
		bIsPassiveApp = false;
		ComboType = EComboType::E_None;
		EquippedSlotIndex = -1;
	}

	FBattleAppEquipInfoStruct() : bIsEquipped(false), ComboType(EComboType::E_None) {}
	FBattleAppEquipInfoStruct(bool InIsEquipped) : bIsEquipped(InIsEquipped), bIsPassiveApp(false), ComboType(EComboType::E_None), EquippedSlotIndex(-1) {}
	FBattleAppEquipInfoStruct(bool InIsEquipped, bool bIsPassiveApp, EComboType InComboType) : bIsEquipped(InIsEquipped), bIsPassiveApp(bIsPassiveApp), ComboType(InComboType), EquippedSlotIndex(-1) {}
	FBattleAppEquipInfoStruct(bool InIsEquipped, bool bIsPassiveApp, EComboType InComboType, int32 InEquippedSlotIndex) : bIsEquipped(InIsEquipped), bIsPassiveApp(bIsPassiveApp), ComboType(InComboType), EquippedSlotIndex(InEquippedSlotIndex) {}
};

//=======================================
USTRUCT(BlueprintType)
struct FBattleAppSlotStateList
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<EBattleAppSlotState> SlotStateList; //장착된 어빌리티 ID 리스트, 순서 중요

	//static max_combo_length 정의
	static const int32 MaxLength = 7;

public:
	FBattleAppSlotStateList()
	{
		//길이 7로 초기화
		SlotStateList.Init(EBattleAppSlotState::E_None, MaxLength);
	}

	FBattleAppSlotStateList(float length)
	{
		SlotStateList.Init(EBattleAppSlotState::E_None, length);
	}
};

USTRUCT(BlueprintType)
struct FBattleAppIDList
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<int32> AbilityIDList; //장착된 어빌리티 ID 리스트, 순서 중요

	static const int32 MaxLength = 7;

	FBattleAppIDList(){}
	FBattleAppIDList(float length)
	{
		AbilityIDList.Init(-1, length);
	}
};

USTRUCT(BlueprintType)
struct FAbilityManagerInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//현재 플레이어가 소유한 어빌리티의 정보
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)	 
		TArray<FAbilityInfoStruct> ActiveAbilityInfoList;

	//모든 어빌리티의 정보
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		TArray<FAbilityInfoStruct> AbilityInfoList;

	//모든 어빌리티의 정보 (맵)
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		TMap<int32, FAbilityInfoStruct> AbilityInfoMap;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		TSet<int32> UnequippedAbilityIDSet;

	UPROPERTY()
		TMap<EAbilityType, int32> AbilityTotalTier;
};

USTRUCT(BlueprintType)
struct FBoosterchipManagerInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly)
		TArray<bool> AbilityPickedInfoList;

	UPROPERTY(BlueprintReadOnly)
		FAbilityManagerInfoStruct AbilityManagerInfo;
};

USTRUCT(BlueprintType)
struct FBattleApplicationManagerInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FAbilityManagerInfoStruct AbilityManagerInfo;

	//콤보 타입 별 장착 정보 리스트, 0번째는 Passive BattleApp용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		TArray<FBattleAppIDList> EquipIDListPerComboType;

	//패시브앱 미장착 정보 리스트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		FBattleAppIDList UnequippedPassiveAppIDList;

	//콤보앱 미장착 정보 리스트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		FBattleAppIDList UnequippedComboAppIDList;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		TArray<FBattleAppSlotStateList> SlotStateListPerComboType;
};

//FPermanentUpgradeManagerInfo declare
USTRUCT(BlueprintType)
struct FPermanentUpgradeManagerInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
		bool bIsInitialized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 CoreCount = 0;

	UPROPERTY()
		FAbilityManagerInfoStruct AbilityManagerInfo;

	// 플레이어가 해금한 스킬 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32> UnlockedPlayerSkillIDList;

	//현재 플레이어가 장착하고 있는 파츠의 정보
	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		//TArray<FPartsInfoStruct> EquippedPartInfoList;
};

