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
	UFUNCTION(BlueprintCallable)
		void ActivateSkill(AActor* NewCauser, TArray<ACharacterBase*> NewTargetList);

	UFUNCTION()
		UDataTable* GetSkillInfoTable() { return SkillInfoTable; }

protected:
	//Set MainCharacter's Skill Grade by Compare SkillGauge
	UFUNCTION()
		void SetMainCharacterSkillGrade(AActor* NewCauser);

	//Set EnemyCharacter's Skill Grade by Compare Defficulty
	UFUNCTION()
		void SetEnemyCharacterSkillGrade(AActor* NewCauser);
	
	//Return is Not E_None
	UFUNCTION()
		bool IsValidGrade(AActor* NewCauser);

	//SkillMap�� ID�� �´� ��ų��ü�� ���ٸ� �߰���
	UFUNCTION()
		ASkillBase* ComposeSkillMap(AActor* NewCauser, int32 NewSkillID);
	
	//SkillMap�� �߰��� ��ų ��ü�� ������
	UFUNCTION()
		class ASkillBase* MakeSkillBase(int32 NewSkillID);

	//SkillID�� ���߾� ��ų�� ��� ������ �����ϰ�, Delay�� �ο���
	UFUNCTION()
		void DelaySkillInterval(uint8 NewIndex);

	//��ų ���� ���̺�
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* SkillInfoTable;

	UFUNCTION()
		void ClearAllTimerHandle();

private:
	UPROPERTY()
		TMap<int32, class ASkillBase*> SkillRefMap;

	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	UPROPERTY()
		FTimerHandle SkillTimerHandle;
};