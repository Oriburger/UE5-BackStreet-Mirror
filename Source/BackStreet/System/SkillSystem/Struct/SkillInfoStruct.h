#pragma once
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "SkillInfoStruct.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Character			UMETA(DisplayName = "Character"),
	E_Weapon			UMETA(DisplayName = "Weapon"),
	E_Weapon0			UMETA(DisplayName = "Weapon0"),
	E_Weapon1			UMETA(DisplayName = "Weapon1"),
	E_Weapon2			UMETA(DisplayName = "Weapon2")
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
		float CoolTime;

	//Skill Variable 
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, float> SkillVariableMap;
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
struct FOwnerSkillInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 SkillID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class ASkillBase> SkillBaseClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		ESkillType SkillType = ESkillType::E_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bSkillBlocked = true;

	UPROPERTY(BlueprintReadWrite)
		uint8 SkillLevel = 0;
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
