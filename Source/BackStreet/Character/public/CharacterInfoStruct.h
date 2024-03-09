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
struct FCharacterStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//캐릭터의 종류를 나타내는 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0, UIMax = 2500))
		int32 CharacterID;

	UPROPERTY(BlueprintReadOnly)
		bool bIsInvincibility = false;

	//무한 내구도 / 무한 탄약 (Enemy 기본 스탯)
	UPROPERTY(BlueprintReadOnly)
		bool bInfinite = false;

	//PlayerMaxHP는 1.0f
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.5f, UIMax = 10.0f))
		float CharacterMaxHP = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.1f, UIMax = 10.0f))
		float CharacterAtkMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.2f, UIMax = 2.0f))
		float CharacterAtkSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 100.0f, UIMax = 1000.0f))
		float CharacterMoveSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.0f, UIMax = 2.0f))
		float CharacterDefense = 0.0f;

	//한번에 발사할 발사체 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 1, UIMax = 1))
		int32 MaxProjectileCount = 1;
};

USTRUCT(BlueprintType)
struct FCharacterStateStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//캐릭터의 디버프 상태 (Bit-Field로 표현)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 CharacterDebuffState = (1 << 10);

	//공격을 할 수 있는 상태인지?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bCanAttack = false;

	//0 : Idle,  1 : Left Turn,  2 : Right Turn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		uint8 TurnDirection = 0;

	//캐릭터의 행동 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		ECharacterActionType CharacterActionState;

	//PlayerMaxHP는 1.0f
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float CharacterCurrHP;

	//Player Combo게이지
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float CharacterCurrSkillGauge;
};

USTRUCT(BlueprintType)
struct FCharacterAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//적 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 CharacterID;

	//적 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName CharacterName;

	//스폰할 적 스켈레탈 메시 정보 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TSoftObjectPtr<USkeletalMesh> CharacterMesh;

	//스폰할 적 머티리얼 정보 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TSoftObjectPtr<UMaterialInstanceConstant> CharacterMeshMaterial;

	//메시의 초기 위치 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialLocation;

	//메시의 초기 회전 정보
	//현재 미사용, -90도로 고정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FRotator InitialRotation;

	//메시의 초기 크기 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialCapsuleComponentScale;

	// Animation 관련
	
	//스폰할 적 스켈레탈 메시 정보 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UAnimBlueprintGeneratedClass* AnimBlueprint;
	
	//근접 공격 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> MeleeAttackAnimMontageList;

	//사격 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ShootAnimMontageList;

	//투척 공격 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ThrowAnimMontageList;

	//재장전 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ReloadAnimMontageList;

	//타격 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> HitAnimMontageList;

	//무기 ID, 스킬 애니메이션/ Map으로 관리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TMap<int32, FSkillAnimMontageStruct> SkillAnimMontageMap;

	//구르기 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> RollAnimMontageList;

	//상호작용 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> InvestigateAnimMontageList;

	//사망 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> DieAnimMontageList;

	//조우 애니메이션 / List로 관리 
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