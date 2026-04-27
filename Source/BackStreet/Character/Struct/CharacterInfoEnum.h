#pragma once

/* MUST BE EDITED aftere IndieGo */
UENUM(BlueprintType)
enum class ECharacterAbilityType : uint8
{
	E_None						UMETA(DisplayName = "None"),
	E_MaxHP						UMETA(DisplayName = "MaxHP"),
	E_AutoHeal					UMETA(DisplayName = "AutoHeal"),
	E_AttackUp					UMETA(DisplayName = "AttackUp"),
	E_DefenseUp					UMETA(DisplayName = "DefenseUp"),
	E_MoveSpeedUp				UMETA(DisplayName = "MoveSpeedUp"),
	E_AtkSpeedUp				UMETA(DisplayName = "AttackSpeedUp"),
	E_MultipleShot				UMETA(DisplayName = "MultipleShot"),
	E_LargeWishList				UMETA(DisplayName = "LargeWishList"),
	E_ExtraTime					UMETA(DisplayName = "ExtraTime"),
	E_LuckyMaterial				UMETA(DisplayName = "LuckyMaterial"),
	E_InfiniteSkillMaterial		UMETA(DisplayName = "InfiniteSkillMaterial"),
};

UENUM(BlueprintType)
enum class ECharacterDebuffType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Temp				UMETA(DisplayName = "Temp"),
	E_Poison			UMETA(DisplayName = "Poison"),
	E_Burn				UMETA(DisplayName = "Burn"),
	E_AttackDown		UMETA(DisplayName = "AttackDown"),
	E_DefenseDown		UMETA(DisplayName = "DefenseDown"),
	E_Slow				UMETA(DisplayName = "Slow"),
	E_Stun				UMETA(DisplayName = "Stun"),
	E_FrameDrop			UMETA(DisplayName = "FrameDrop"),
};

UENUM(BlueprintType)
enum class EAIBehaviorType : uint8
{
	E_Idle				UMETA(DisplayName = "Idle"),
	E_Patrol			UMETA(DisplayName = "Patrol"),
	E_Detect			UMETA(DisplayName = "Detect"),
	E_Chase				UMETA(DisplayName = "Chase"),
	E_Attack			UMETA(DisplayName = "Attack"),
	E_Skill				UMETA(DisplayName = "Skill"),
	E_Return			UMETA(DisplayName = "Return"),
	E_Stun				UMETA(DisplayName = "Stun")
};

UENUM(BlueprintType)
enum class ECharacterActionType : uint8
{
	E_NONE             UMETA(DisplayName = "None"),

	// Idle / Movement
	E_Idle             UMETA(DisplayName = "Idle"),
	E_Move             UMETA(DisplayName = "Move"),
	E_Sprint           UMETA(DisplayName = "Sprint"),
	E_Jump             UMETA(DisplayName = "Jump"),
	E_Roll             UMETA(DisplayName = "Roll"),

	// Combat
	E_Attack           UMETA(DisplayName = "Attack"),
	E_JumpAttack       UMETA(DisplayName = "JumpAttack"),
	E_DashAttack       UMETA(DisplayName = "DashAttack"),
	E_Aim              UMETA(DisplayName = "Aim"),
	E_Shoot            UMETA(DisplayName = "Shoot"),
	E_Hit              UMETA(DisplayName = "Hit"),

	// Skill / Interaction
	E_Skill             UMETA(DisplayName = "Skill"),
	E_Interact          UMETA(DisplayName = "Interact"),
	E_ActionLaunch		UMETA(DisplayName = "ActionLaunch"),

	// State
	E_Stun             UMETA(DisplayName = "Stun"),
	E_KnockedDown      UMETA(DisplayName = "KnockedDown"),
	E_Die              UMETA(DisplayName = "Die"),
	E_Revive           UMETA(DisplayName = "Revive"),
	E_Kill			   UMETA(DisplayName = "Kill"),

	// System
	E_LockOn           UMETA(DisplayName = "LockOn"),
	E_ResetCamera      UMETA(DisplayName = "ResetCamera"),

	COUNT              UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	E_None				UMETA(DisplayName = "None"),				 // Error Handling
	E_Ready				UMETA(DisplayName = "Ready"),                // 준비 상태
	E_InProgress		UMETA(DisplayName = "In Progress"),			 // 액션 진행 중
	E_RecentlyFinished	UMETA(DisplayName = "Recently Finished"),	 // 최근 종료
};

UENUM(BlueprintType)
enum class EActionCauserType : uint8
{
	E_None				UMETA(DisplayName = "None"),		//Error Handling		
	E_Owner				UMETA(DisplayName = "Owner"),		//Owner Character
	E_Weapon			UMETA(DisplayName = "Weapon"),		//Owner's WeaponComponent
	E_Enemy				UMETA(DisplayName = "Enemy")		//Enemy Character
};

UENUM(BlueprintType)
enum class EComboType : uint8
{
	E_None				UMETA(DisplayName = "None"),		//No Combo
	E_Normal			UMETA(DisplayName = "Normal"),		//Normal Combo
	E_Jump				UMETA(DisplayName = "Jump"),		//Jump Combo
	E_Dash				UMETA(DisplayName = "Dash")			//Dash Combo
};

UENUM(BlueprintType)
enum class EBattleAppSlotState : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Disabled			UMETA(DisplayName = "Disabled"),
	E_Locked			UMETA(DisplayName = "Locked"),
	E_Empty				UMETA(DisplayName = "Empty"),
	E_Equipped			UMETA(DisplayName = "Equipped")

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

	E_SkillCoolTime			UMETA(DisplayName = "NONE_SkillCoolTime"), //미사용
	E_SubWeaponCapacity		UMETA(DisplayName = "SubWeaponCapacity"),
	E_HealItemPerformance	UMETA(DisplayName = "HealItemPerformance"),
	E_DebuffTime			UMETA(DisplayName = "NormalDebuffTime"),	//[0, 1] (2.5sec + val)
	E_LuckyDrop				UMETA(DisplayName = "LuckyDrop"),

	E_ActionLaunchDistance	UMETA(DisplayName = "ActionLaunchDistance"), //액션 런치 거리
	E_MaxAP					UMETA(DisplayName = "MaxAP"),
	E_ExtraAPGainRate		UMETA(DisplayName = "APGainRate"), //AP 획득량 증가
	E_DamageToAPRate		UMETA(DisplayName = "DamageToAPRate"), //데미지를 AP로 전환하는 비율
	E_MaxFlameStack			UMETA(DisplayName = "MaxFlameStack"),
	E_MaxPoisonStack		UMETA(DisplayName = "MaxPoisonStack"),
	E_MaxFrameDropStack		UMETA(DisplayName = "MaxFrameDropStack"),

	E_MaxNormalComboIdx		UMETA(DisplayName = "MaxNormalComboIdx"),
	E_MaxDashComboIdx		UMETA(DisplayName = "MaxDashComboIdx"),
	E_MaxAerialComboIdx		UMETA(DisplayName = "MaxAerialComboIdx"),

	E_DamageReduction		UMETA(DisplayName = "DamageReduction"),
	E_InvincibilityTime		UMETA(DisplayName = "InvincibilityTime"),
	E_RichProcessing		UMETA(DisplayName = "RichProcessing"),
	E_SubWeaponRefillChance UMETA(DisplayName = "SubWeaponRefillChance"),
	E_ShopDiscount			UMETA(DisplayName = "ShopDiscount"),

	Count					UMETA(DisplayName = "Count") //Total Count 표시용, 실제로는 미사용
};