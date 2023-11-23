// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "SkillManagerBase.generated.h"

/*
 */
UCLASS()
class BACKSTREET_API USkillManagerBase : public UObject
{
	GENERATED_BODY()
public:
	USkillManagerBase();
	
	//SkillManagerBase�� ����
	UFUNCTION()
		void InitSkillManagerBase(class ABackStreetGameModeBase* NewGamemodeRef);

	//SkillManagerBase���� ��ų�� ������ ó����
	UFUNCTION()
		void ActivateSkill(AActor* Causer, class ACharacterBase* Target);

protected:
	//FSkillSet�� ������ �ҷ��� ������
	UFUNCTION()
		void SetSkillSet(AActor* Causer);

	//SkillMap�� ID�� �´� ��ų��ü�� ���ٸ� �߰���
	UFUNCTION()
		void ComposeSkillMap(int32 SkillID, AActor* Causer, ACharacterBase* Target);
	
	//SkillMap�� �߰��� ��ų ��ü�� ������
	UFUNCTION()
		class ASkillBase* MakeSkillBase(int32 SkillID);

	//SkillID�� ���߾� ��ų�� ��� ������ �����ϰ�, Delay�� �ο���
	UFUNCTION()
		float GetDelayInterval(int32 SkillListIdx, TArray<float> SkillIntervalList);

private:
	//�� ���� ��ų�� ���� ��
	UPROPERTY()
		TMap<int32,FSkillSetInfo> SkillSetInfoMap;

	UPROPERTY()
		TMap<int32, class ASkillBase*> SkillRefMap;

	UPROPERTY()
		FSkillSetInfo SkillSetInfo;

	UPROPERTY()
		TArray<float> SkillIntervalArray;

	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};