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
		TArray<class ASkillBase*> ActivateSkill(AActor* NewCauser, class ACharacterBase* NewTarget);

	UFUNCTION(BlueprintCallable)
		void DestroySkill(TArray<ASkillBase*> UsedSkillList);

	UFUNCTION()
		UDataTable* GetSkillInfoTable() { return SkillInfoTable; }

protected:
	//FSkillSet�� ������ �ҷ��� ������
	UFUNCTION()
		void SetSkillSet(AActor* NewCauser);

	//SkillMap�� ID�� �´� ��ų��ü�� ���ٸ� �߰���
	UFUNCTION()
		ASkillBase* ComposeSkillMap(int32 NewSkillID);
	
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
		AActor* Causer;

	UPROPERTY()
		ACharacterBase* Target;

	//�� ���� ��ų�� ���� ��
	UPROPERTY()
		TMap<int32,FSkillSetInfo> SkillSetInfoMap;

	UPROPERTY()
		TMap<int32, class ASkillBase*> SkillRefMap;

	UPROPERTY()
		TArray<class ASkillBase*> SkillList;

	UPROPERTY()
		FSkillSetInfo SkillSetInfo;

	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	UPROPERTY()
		FTimerHandle SkillTimerHandle;
};