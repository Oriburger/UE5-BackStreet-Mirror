#pragma once
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "../../Character/public/CharacterInfoEnum.h"
#include "SkillInfoStruct.generated.h"

USTRUCT(BlueprintType)
struct FSkillInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 SkillID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class ASkillBase> SkillBaseClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> SkillCommonVariableMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> SkillRareVariableMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> SkillLegendVariableMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, float> SkillMythicVariableMap;
};

USTRUCT(BlueprintType)
struct FSkillAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//��ų ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 SkillID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		FName SkillName;

	//������ ��ų ����ƽ �޽� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		TSoftObjectPtr<UStaticMesh> SkillActorMesh;

	//�޽��� �ʱ� ��ġ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialLocation;

	//�޽��� �ʱ� ȸ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FRotator InitialRotation;

	//�޽��� �ʱ� ũ�� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
		FVector InitialScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TArray<TSoftObjectPtr<UNiagaraSystem>> EffectParticleList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<UNiagaraSystem> DestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TArray<TSoftObjectPtr<USoundCue>> SkillSoundList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UTexture2D> SkillIconImage;
};

USTRUCT(BlueprintType)
struct FSkillSetInfo
{
public:
	GENERATED_USTRUCT_BODY()

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32> SkillIDList;
	
	//Depending on the SkillGrade, the animation rate varies
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsAnimPlayRateSyncWithGrade;

	//when IsAnimPlayRateSyncWithGrade == false, CommonSkillAnimRateList gonna be AnimPlayRateList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<float> CommonSkillAnimRateList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<float> RareSkillAnimRateList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<float> LegendSkillAnimRateList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<float> MythicSkillAnimRateList;

	//Rate value(Criterion is TotalAnimPlayTime) which is used to indicate the interval between each skill
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<float> SkillStartTimingRateList;

	UPROPERTY()
		ESkillGrade SkillGrade;

	UPROPERTY()
		float TotalSkillPlayTime;

	UPROPERTY()
		TMap<int32, class ASkillBase*> SkillRefMap;
};

USTRUCT(BlueprintType)
struct FSkillGaugeInfo
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool IsSkillAvailable = 0;
	
	//skill gauge augmentation per one attack 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillGaugeAug = 0.1;

	//common skill gauge requirement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillCommonReq = 1.0;

	// rare skill gauge requirement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillRareReq = 2.0;

	// Legend skill gauge requirement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillLegendReq = 3.0;

	// mythic skill gauge requirement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillMythicReq = 4.0;
};

USTRUCT(BlueprintType)
struct FSkillAnimMontageStruct
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<TSoftObjectPtr<UAnimMontage>> SkillAnimMontageList;
};

USTRUCT(BlueprintType)
struct FSkillAnimAssetMontageStruct
{
public:
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> SkillAnimMontageList;
};