#pragma once
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "SkillInfoStruct.generated.h"

USTRUCT(BlueprintType)
struct FSkillInfo : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

//======== Basic Info ==================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 SkillID;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UTexture2D* IconImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillSubDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UAnimMontage* SkillAnimation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PrevAction")
		bool bContainPrevAction = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PrevAction", meta = (EditCondition = "bContainPrevAction"))
		TSubclassOf<class ASkillBase> PrevActionClass;

	//Causer�� �����ֱ⸦ ���� ������?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsLifeSpanWithCauser = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsPlayerSkill = false;

	//������ ���� �ʿ��� ��� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 MaterialID;
	
	//������ ���� �ʿ��� ��� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 MaterialCount;

//======== Function, State ======================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TWeakObjectPtr<AActor> PrevActionRef;

	bool IsValid()
	{
		return bIsValid = (SkillID > 0);
	}

	friend uint32 GetTypeHash(const FSkillInfo& Other)
	{
		return GetTypeHash(Other.SkillID);
	}

	FSkillInfo() : SkillID(0), SkillName(""), IconImage(nullptr), SkillDescription(""), SkillSubDescription(""), SkillAnimation(nullptr), bContainPrevAction(false)
		, PrevActionClass(nullptr), bIsLifeSpanWithCauser(false), bIsPlayerSkill(false), MaterialID(0), MaterialCount(0), bIsValid(false), PrevActionRef(nullptr) { }
};

UENUM(BlueprintType)
enum class ESkillUpgradeType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_AttackPower		UMETA(DisplayName = "E_Mobility"),
	E_Debuff			UMETA(DisplayName = "E_Debuff"),
	E_AttackRange		UMETA(DisplayName = "E_AttackRange"),
	E_AttackCount		UMETA(DisplayName = "E_AttackCount"),
	E_Explosion			UMETA(DisplayName = "E_Explosion"),
	E_CoolTime			UMETA(DisplayName = "E_CoolTime")
};

USTRUCT(BlueprintType)
struct FSkillStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 SkillID;
};
