#pragma once

UENUM(BlueprintType)
enum class ECharacterAbilityType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_MaxHP				UMETA(DisplayName = "MaxHP"),
	E_AutoHeal			UMETA(DisplayName = "AutoHeal"),
	E_AttackUp			UMETA(DisplayName = "AttackUp"),
	E_DefenseUp			UMETA(DisplayName = "DefenseUp"),
	E_SpeedUp			UMETA(DisplayName = "SpeedUp"),
	E_DoubleShot		UMETA(DisplayName = "DoubleShot"),
	E_TripleShot		UMETA(DisplayName = "TripleShot")
};

UENUM(BlueprintType)
enum class ECharacterDebuffType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Flame				UMETA(DisplayName = "Flame"),
	E_Poison			UMETA(DisplayName = "Poison"),
	E_AttackDown		UMETA(DisplayName = "AttackDown"),
	E_DefenseDown		UMETA(DisplayName = "DefenseDown"),
	E_Slow				UMETA(DisplayName = "Slow"),
	E_Stun				UMETA(DisplayName = "Stun"),
};

UENUM(BlueprintType)
enum class EAIBehaviorType : uint8
{
	E_Idle			UMETA(DisplayName = "Idle"),
	E_Patrol		UMETA(DisplayName = "Patrol"),
	E_Detect		UMETA(DisplayName = "Detect"),
	E_Chase			UMETA(DisplayName = "Chase"),
	E_Attack		UMETA(DisplayName = "Attack"),
	E_Return		UMETA(DisplayName = "Return"),
	E_Stun			UMETA(DisplayName = "Stun")
};

UENUM(BlueprintType)
enum class ECharacterActionType : uint8
{
	E_Idle			UMETA(DisplayName = "Idle"),
	E_Attack		UMETA(DisplayName = "Attack"),
	E_Hit			UMETA(DisplayName = "Hit"),
	E_Roll			UMETA(DisplayName = "Roll"),
	E_Jump			UMETA(DisplayName = "Jump"),
	E_Reload		UMETA(DisplayName = "Reload"),
	E_Stun			UMETA(DisplayName = "Stun"),
	E_Die			UMETA(DisplayName = "Die")
};