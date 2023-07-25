// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "DebuffManager.generated.h"

USTRUCT(BlueprintType)
struct FDebuffTimerInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		TMap<ECharacterDebuffType, FTimerHandle> TimerHandleMap;

	UPROPERTY()
		TMap<ECharacterDebuffType, FTimerHandle> DebuffDamageTimerHandleMap;

	UPROPERTY()
		TMap<ECharacterDebuffType, float> ResetValueHandleMap;
};

/** 
 */
UCLASS()
class BACKSTREET_API UDebuffManager : public UObject
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	UDebuffManager();

	//������� �Ǵ�
	UFUNCTION()
		bool SetDebuffTimer(ECharacterDebuffType DebuffType, class ACharacterBase* Target, AActor* Causer, float TotalTime = 1.0f, float Variable = 0.0f);

	//����� ���¸� �ʱ�ȭ�Ѵ�
	UFUNCTION()
		void ResetStatDebuffState(ECharacterDebuffType DebuffType, class ACharacterBase* Target, float ResetVal);

	//Ư�� Debuff�� Ÿ�̸Ӹ� �����Ѵ�.
	UFUNCTION()
		void ClearDebuffTimer(ECharacterDebuffType DebuffType, class ACharacterBase* Target);

	UFUNCTION()
		void InitDebuffManager(class ABackStreetGameModeBase* NewGamemodeRef);

	//��� Debuff�� Ÿ�̸Ӹ� ����
	UFUNCTION()
		void ClearAllDebuffTimer();

	//������� Ȱ��ȭ �Ǿ��ִ��� ��ȯ
	UFUNCTION()
		bool GetDebuffIsActive(ECharacterDebuffType DebuffType, class ACharacterBase* Target);

private:
	//����� Ÿ�̸� �ڵ��� �����ڸ� ��ȯ
	UFUNCTION()
		FTimerHandle& GetDebuffTimerHandleRef(ECharacterDebuffType DebuffType, class ACharacterBase* Target, bool bIsDebuffDamageTimerMap = false);

	//GetResetValueListRef�� ���� (Invalid List)�� �����ϴ� �Լ�
	UFUNCTION()
		float GetDebuffResetValue(ECharacterDebuffType DebuffType, ACharacterBase* Target);

private:
	//TimerInfoMap�� Key�� Target.id�� �����ϴ� Timer Handle List�� Get
	UFUNCTION()
		TMap<ECharacterDebuffType, FTimerHandle>& GetTimerHandleMapRef(class ACharacterBase* Target, bool bIsDebuffDamageTimerMap = false);

	//Target.id�� �����ϴ� ����� ���� �ʱ�ȭ �� �迭�� Get
	UFUNCTION()
		TMap<ECharacterDebuffType, float>& GetResetValueMapRef(class ACharacterBase* Target);

	//Target Pawn�� �����ϴ� ����� Ÿ�̸� ����Ʈ ����
	UFUNCTION()
		void AddNewTimerList(class ACharacterBase* Target);

private:
	//���Ӹ�� Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//ĳ������ Actor id - ����� Ÿ�̸�(+ �ʱ�ȭ ��) ������ ����
	UPROPERTY()
		TMap<int32, FDebuffTimerInfoStruct> TimerInfoMap;
};