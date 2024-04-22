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
	E_Weapon			UMETA(DisplayName = "Weapon")
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
struct FSkillGradeStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	//Whether skill uses skill gauge
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsGradeValid = false;
	
	//Variables to be used as properties for skills according to Common grades.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> CommonVariableMap;

	//Variables to be used as properties for skills according to Rare grades.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> RareVariableMap;

	//Variables to be used as properties for skills according to Legend grades.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> LegendVariableMap;

	//Variables to be used as properties for skills according to Mythic grades.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> MythicVariableMap;

	//common skill gauge requirement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CommonGaugeReq = 1.0;

	// rare skill gauge requirement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RareGaugeReq = 2.0;

	// Legend skill gauge requirement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float LegendGaugeReq = 3.0;

	// mythic skill gauge requirement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MythicGaugeReq = 4.0;
		
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
		bool bSkillBlocked = true;
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

	UPROPERTY(BlueprintReadWrite)
		bool bHidenInGame = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bSkillLifeSpanWithCauser = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillGradeStruct SkillGradeStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FSkillAssetStruct SkillAssetStruct;

	//Skill causer character reference
	UPROPERTY(BlueprintReadWrite)
		TWeakObjectPtr<class ACharacterBase> Causer;
};
