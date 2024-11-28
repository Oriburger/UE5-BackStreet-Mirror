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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UAnimMontage* SkillAnimation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PrevAction")
		bool bContainPrevAction = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PrevAction", meta = (EditCondition = "bContainPrevAction"))
		TSubclassOf<class ASkillBase> PrevActionClass;

	//Causer의 생명주기를 따라갈 것인지?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsLifeSpanWithCauser = true;

	//조합을 위해 필요한 재료 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 MaterialID;
	
	//조합을 위해 필요한 재료 정보
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

	FSkillInfo() : SkillID(0), SkillName(""), IconImage(nullptr), SkillDescription(""), SkillAnimation(nullptr), bContainPrevAction(false)
		, PrevActionClass(nullptr), bIsLifeSpanWithCauser(false), MaterialID(0), MaterialCount(0), bIsValid(false), PrevActionRef(nullptr) { }
};


UENUM(BlueprintType)
enum class ESkillType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Mobility			UMETA(DisplayName = "E_Mobility"), //이동기
	E_Debuff			UMETA(DisplayName = "E_Debuff"),  //디버프기
	E_Sword				UMETA(DisplayName = "E_Sword"),   //검 전용기
	E_Enemy				UMETA(DisplayName = "E_Enemy"),   //적 전용기
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
struct FSkillListContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<class ASkillBase*> SkillBaseList;
};

USTRUCT(BlueprintType)
struct FObtainableSkillListContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<int32> SkillIDList;
};

USTRUCT(BlueprintType)
struct FSkillUpgradeLevelInfo
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
		uint8 CurrentLevel = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<int32, int32> RequiredMaterialMap; //id, cnt
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float Variable = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsPercentage = true;
	
		//uint8 Type = 0; //Optional variable (like debuff type, )
};

USTRUCT(BlueprintType)
struct FSkillUpgradeLevelInfoContainer
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<FSkillUpgradeLevelInfo> SkillLevelInfoList;
};

USTRUCT(BlueprintType)
struct FSkillEffectInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		UNiagaraSystem* Effect;
};

USTRUCT(BlueprintType)
struct FSkillAnimInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UAnimMontage* AnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		float AnimPlayRate = 1.0f;
};

USTRUCT(BlueprintType)
struct FSkillMaterialInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		TArray<UMaterialInterface*> MaterialList;
};

USTRUCT(BlueprintType)
struct FSkillAssetStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Image")
		class UTexture2D* IconImage;

	//All effect maps to be used in the skill. Can find effect by Find(FName).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TMap<FName, FSkillEffectInfoStruct> EffectMap;

	//All animation maps to be used in the skill. Can find animation by Find(FName).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TMap<FName, FSkillAnimInfoStruct> AnimInfoMap;

	//All material maps to be used in the skill. Can find material by Find(FName).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TMap<FName, FSkillMaterialInfoStruct> MaterialInfoMap;
		
};

USTRUCT(BlueprintType)
struct FSkillWeaponStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsWeaponRequired = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		ESkillType SkillType = ESkillType::E_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<int32> AvailableWeaponIDList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 RandomWeight = 0;
};

USTRUCT(BlueprintType)
struct FSkillStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 SkillID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class ASkillBase> SkillBaseClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsLifeSpanWithCauser = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsLevelValid = false;

	//조합을 위해 필요한 재료 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<int32, int32> CraftMaterial;

	//for upgrade after craft
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<ESkillUpgradeType, FSkillUpgradeLevelInfoContainer> LevelInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillAssetStruct SkillAssetStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillWeaponStruct SkillWeaponStruct;
};

USTRUCT(BlueprintType)
struct FSkillStateStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
		uint8 SkillLevel = 0;

	//Skill Variable 
	UPROPERTY(BlueprintReadWrite)
		TMap<ESkillUpgradeType, FSkillUpgradeLevelInfo> SkillUpgradeInfoMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsBlocked = false;

	//스킬을 월드에서 보일지 여부
	UPROPERTY(BlueprintReadWrite)
		bool bIsHidden = false;

	UPROPERTY()
		bool bIsStateValid = false;
};
