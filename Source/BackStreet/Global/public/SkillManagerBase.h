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
	UFUNCTION()
		void ActivateSkill(AActor* Causer, class ACharacterBase* Target);

protected:
	//FSkillSet에 정보를 불러와 생성함
	UFUNCTION()
		void SetSkillSet(AActor* Causer);

	//SkillMap에 ID에 맞는 스킬객체가 없다면 추가함
	UFUNCTION()
		void ComposeSkillMap(int32 SkillID, AActor* Causer, ACharacterBase* Target);
	
	//SkillMap에 추가할 스킬 객체를 생성함
	UFUNCTION()
		class ASkillBase* MakeSkillBase(int32 SkillID);

	//SkillID에 맞추어 스킬간 사용 간격을 지정하고, Delay를 부여함
	UFUNCTION()
		float GetDelayInterval(int32 SkillListIdx, TArray<float> SkillIntervalList);

private:
	//적 몹의 스킬셋 정보 맵
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