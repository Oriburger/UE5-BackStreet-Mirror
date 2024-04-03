#pragma once

#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "CharacterInfoEnum.h"
#include "../../SkillSystem/public/SkillInfoStruct.h"
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
		float PercentValue;

	UPROPERTY(BlueprintReadOnly)
		float FixedValue;
};

USTRUCT(BlueprintType)
struct FCharacterStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//ĳ������ ������ ��Ÿ���� ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0, UIMax = 2500))
		int32 CharacterID;

	UPROPERTY(BlueprintReadOnly)
		bool bIsInvincibility = false;

	//���� ������ / ���� ź�� (Enemy �⺻ ����)
	UPROPERTY(BlueprintReadOnly)
		bool bInfinite = false;

	//Projectile Count Per Attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 1, UIMax = 1))
		int32 ProjectileCountPerAttack = 1;

//======= Default Stat ======================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 1.0f, UIMax = 1000.0f))
		float DefaultHP = 100.0f;

	//Multipiler Value
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 10.0f))
		float DefaultAttack = 1.0f;

	//Multipiler Value
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 10.0f))
		float DefaultDefense = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 100.0f))
		float DefaultAttackSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 100.0f, UIMax = 1000.0f))
		float DefaultMoveSpeed = 400.0f;
};

USTRUCT(BlueprintType)
struct FCharacterStateStruct
{
public:
	GENERATED_USTRUCT_BODY()

//====== Character State ===============
	//ĳ������ ����� ���� (Bit-Field�� ǥ��)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 CharacterDebuffState = (1 << 10);

	//������ �� �� �ִ� ��������?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bCanAttack = false;

	//Is character sprinting?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsSprinting = false;

	//0 : Idle,  1 : Left Turn,  2 : Right Turn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		uint8 TurnDirection = 0;

	//ĳ������ �ൿ ����
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		ECharacterActionType CharacterActionState;

//====== Current Stat ===================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TotalHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TotalAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TotalDefense;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TotalMoveSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TotalAttackSpeed;

	//PlayerMaxHP�� 1.0f
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float CurrentHP;

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

//====== Skill =========================
	
	//Player Skill Gauge
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float CharacterCurrSkillGauge;
};

USTRUCT(BlueprintType)
struct FCharacterAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//�� ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 CharacterID;

	//�� �̸�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName CharacterName;

	//������ �� ���̷�Ż �޽� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TSoftObjectPtr<USkeletalMesh> CharacterMesh;

	//Character's Mesh Materials
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TArray<TSoftObjectPtr<UMaterialInstanceConstant> > CharacterMeshMaterialList;

	//�޽��� �ʱ� ��ġ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialLocation;

	//�޽��� �ʱ� ȸ�� ����
	//���� �̻��, -90���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FRotator InitialRotation;

	//�޽��� �ʱ� ũ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialCapsuleComponentScale;

	// Animation ����
	
	//������ �� ���̷�Ż �޽� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UAnimBlueprintGeneratedClass* AnimBlueprint;
	
	//���� ���� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> MeleeAttackAnimMontageList;

	//��� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ShootAnimMontageList;

	//��ô ���� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ThrowAnimMontageList;

	//������ �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ReloadAnimMontageList;

	//Ÿ�� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> HitAnimMontageList;

	//���� ID, ��ų �ִϸ��̼�/ Map���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TMap<int32, FSkillAnimMontageStruct> SkillAnimMontageMap;

	//������ �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> RollAnimMontageList;

	//��ȣ�ۿ� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> InvestigateAnimMontageList;

	//��� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> DieAnimMontageList;

	//���� �ִϸ��̼� / List�� ���� 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> PointMontageList;

	// VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TArray<TSoftObjectPtr<UNiagaraSystem>> DebuffNiagaraEffectList;

	// Material
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		TSoftObjectPtr<UMaterialInterface> NormalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		TSoftObjectPtr<UMaterialInterface> WallThroughMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		TArray<TSoftObjectPtr<UTexture>> EmotionTextureList;

};