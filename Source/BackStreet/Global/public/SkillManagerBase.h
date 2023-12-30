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
	
	//SkillManagerBase를 생성
	UFUNCTION()
		void InitSkillManagerBase(class ABackStreetGameModeBase* NewGamemodeRef);

	//SkillManagerBase에서 스킬의 정보를 처리함
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

	//SkillMap에 ID에 맞는 스킬객체가 없다면 추가함
	UFUNCTION()
		ASkillBase* ComposeSkillMap(AActor* NewCauser, int32 NewSkillID);
	
	//SkillMap에 추가할 스킬 객체를 생성함
	UFUNCTION()
		class ASkillBase* MakeSkillBase(int32 NewSkillID);

	//SkillID에 맞추어 스킬간 사용 간격을 지정하고, Delay를 부여함
	UFUNCTION()
		void DelaySkillInterval(uint8 NewIndex);

	//스킬 스탯 테이블
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