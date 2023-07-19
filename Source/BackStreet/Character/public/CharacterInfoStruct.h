#pragma once

#include "Engine/DataTable.h"
#include "CharacterInfoEnum.h"
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
		bool bInfinite = false;; 

	//PlayerMaxHP는 1.0f
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.5f, UIMax = 10.0f))
		float CharacterMaxHP = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.1f, UIMax = 10.0f))
		float CharacterAtkMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0.2f, UIMax = 2.0f))
		float CharacterAtkSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 100.0f, UIMax = 1000.0f))
		float CharacterMoveSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = -1.0f, UIMax = 1.0f))
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
	UPROPERTY(BlueprintReadOnly)
		int32 CharacterDebuffState = (1 << 10);

	//공격을 할 수 있는 상태인지?
	UPROPERTY(BlueprintReadOnly)
		bool bCanAttack = false;

	//0 : Idle,  1 : Left Turn,  2 : Right Turn
	UPROPERTY(BlueprintReadOnly)
		uint8 TurnDirection = 0;

	//캐릭터의 행동 정보
	UPROPERTY(BlueprintReadWrite)
		ECharacterActionType CharacterActionState;

	//PlayerMaxHP는 1.0f
	UPROPERTY(BlueprintReadOnly)
		float CharacterCurrHP;
};