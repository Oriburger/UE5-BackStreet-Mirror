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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 SkillID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FName SkillDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class ASkillBase> SkillBaseClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UTexture2D* SkillIconImage;
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
		TSoftObjectPtr<UStaticMesh> SkillMesh;

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
		TSoftObjectPtr<class UParticleSystem> EffectParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TSoftObjectPtr<class UParticleSystem> DestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> SkillSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		TSoftObjectPtr<class USoundCue> SkillFailSound;
};

USTRUCT(BlueprintType)
struct FSkillSetInfo
{
public:
	GENERATED_USTRUCT_BODY()

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32> SkillIDList;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<float> SkillAnimPlayRateList;

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

	//Combo ������ ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillGaugeAug = 0.1;

	//Common ��ų �䱸 ��������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillCommonReq = 1.0;

	// Rare ��ų �䱸 ��������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillRareReq = 2.0;

	// epic ��ų �䱸 ��������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillEpicReq = 3.0;

	// Regend ��ų �䱸 ��������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SkillRegendReq = 4.0;
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