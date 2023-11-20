#pragma once
#include "Engine/DataTable.h"
#include "../../Character/public/CharacterInfoEnum.h"
#include "SkillInfoStruct.generated.h"

USTRUCT(BlueprintType)
struct FSkillInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SKillDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ASkillBase> SkillBaseClassRef;

};

USTRUCT(BlueprintType)
struct FSkillSetInfo
{
public:
	GENERATED_USTRUCT_BODY()

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillGrade SkillGrade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> SkillIDList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SkillIntervalList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UAnimMontage* SkillAnimMontage;

};

USTRUCT(BlueprintType)
struct FSkillGaugeInfo
{
public:
	GENERATED_USTRUCT_BODY()

	//Combo 게이지 증가 비율
		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillGaugeAug = 0.1;

	//Common 스킬 요구 게이지량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SkillCommonReq = 1.0;

	// Rare 스킬 요구 게이지량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SkillRareReq = 2.0;

	// epic 스킬 요구 게이지량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SkillEpicReq = 3.0;

	// Regend 스킬 요구 게이지량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SkillRegendReq = 4.0;
};