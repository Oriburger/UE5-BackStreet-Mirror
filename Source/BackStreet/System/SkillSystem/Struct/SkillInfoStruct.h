#pragma once
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "SkillInfoStruct.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Mobility			UMETA(DisplayName = "E_Mobility"), //이동기
	E_Debuff				UMETA(DisplayName = "E_Debuff	"),  //디버프기
	E_Sword				UMETA(DisplayName = "E_Sword"),   //검 전용기
	E_Enemy				UMETA(DisplayName = "E_Enemy"),   //적 전용기
};
USTRUCT(BlueprintType)
struct FSkillLevelInfoContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> SkillVariableMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<uint8> RequiredMaterial;
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
struct FSkillLevelStatStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsLevelValid = false;

	//레벨별 쿨타임 변수 정보
	UPROPERTY(BlueprintReadWrite)
		float CoolTime = 0;

	//레벨별 변수 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<FSkillLevelInfoContainer> LevelInfo;
};

USTRUCT(BlueprintType)
struct FSkillLevelStateStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
		uint8 SkillLevel = 0;

	//Skill Variable 
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, float> SkillVariableMap;
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
		FSkillAssetStruct SkillAssetStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillLevelStatStruct SkillLevelStatStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillWeaponStruct SkillWeaponStruct;
};

USTRUCT(BlueprintType)
struct FSkillStateStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsBlocked = false;

	//스킬을 월드에서 보일지 여부
	UPROPERTY(BlueprintReadWrite)
		bool bIsHidden = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillLevelStateStruct SkillLevelStateStruct;

	UPROPERTY()
		bool bIsStateValid = false;
};


// 레거시 코드==================================
USTRUCT(BlueprintType)
struct FSkillLevelStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsLevelValid = false;

	UPROPERTY(BlueprintReadWrite)
		uint8 SkillLevel = 0;

	//Cool Time List By Skill Level
	UPROPERTY(BlueprintReadWrite)
		float CoolTime = 0.0f;

	//Skill Variable 
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, float> SkillVariableMap;
};

USTRUCT(BlueprintType)
struct FSkillInfoStruct : public FTableRowBase
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
		ESkillType SkillType = ESkillType::E_None;

	UPROPERTY(BlueprintReadWrite)
		bool bHidenInGame = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bSkillLifeSpanWithCauser = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillLevelStruct SkillLevelStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillAssetStruct SkillAssetStruct;

	//Skill causer character reference
	UPROPERTY(BlueprintReadWrite)
		TWeakObjectPtr<class ACharacterBase> Causer;
};
