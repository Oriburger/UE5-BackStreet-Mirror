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

//===============================================
//====== Character Common Info  ========================
//===============================================

USTRUCT(BlueprintType)
struct FPlayerInputActionInfo
{
public:
	GENERATED_USTRUCT_BODY()

	// Move Input Action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Basic")
		class UInputAction* MoveAction;

	// Roll Input Action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Basic")
		class UInputAction* RollAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* UpperAttackAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* ThrowReadyAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
		class UInputAction* ThrowAction;

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

	// Look Input Action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Basic")
		class UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Action")
		class UInputAction* SprintAction;

	// Crouch Input Action
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		//	class UInputAction* CrouchAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* PlayerInfoUIAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* ConfirmUIAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* CancelUIAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "UI")
		class UInputAction* MoveUIAction;
};

USTRUCT(BlueprintType)
struct FCharacterStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//캐릭터의 종류를 나타내는 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0, UIMax = 2500))
		int32 CharacterID = 0;

//======= Weapon Stat ======================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
		bool bIsInvincibility = false;

	//무한 내구도 / 무한 탄약 (Enemy 기본 스탯)
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
		bool bInfinite = false;

	//Projectile Count Per Attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 1, UIMax = 1), Category = "Weapon")
		int32 ProjectileCountPerAttack = 1;

//======= Weapon Stat ======================
	//for time-attack stage
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		float ExtraStageTime = 0.0f;

	//Extra wish list in skill
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		int32 MaxKeepingSkillCount = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		bool bInfiniteSkillMaterial = false;

	//Extra percentage for universal material
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		float ExtraPercentageUnivMaterial = false;

//======= Default Stat ======================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultHP = 100.0f;

	//Multipiler Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 10.0f), Category = "Default")
		float DefaultAttack = 1.0f;

	//Multipiler Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 10.0f), Category = "Default")
		float DefaultDefense = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 100.0f), Category = "Default")
		float DefaultAttackSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 100.0f, UIMax = 1000.0f), Category = "Default")
		float DefaultMoveSpeed = 400.0f;
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
		FCharacterStatStruct CharacterStat;

	//Enemy's Default Weapon ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 DefaultWeaponID = 0;

	//Info data of dropping item when enemy dies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FEnemyDropInfoStruct DropInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
		TArray<int32> EnemySkillIDList; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.2f, UIMax = 1.0f))
		float DefaultAttackSpeed = 0.0f;
};
