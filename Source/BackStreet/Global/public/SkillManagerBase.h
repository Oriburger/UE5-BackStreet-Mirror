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

	UFUNCTION()
	void InitSkillManagerBase(class ABackStreetGameModeBase* NewGamemodeRef);

	UFUNCTION()
	void ActivateSkill(AActor* Causer, class ACharacterBase* Target);

protected:
	UFUNCTION()
	void SetSkillSet(AActor* Causer);

	UFUNCTION()
	void ComposeSkillMap(int32 SkillID, AActor* Causer, ACharacterBase* Target);

	UFUNCTION()
	class ASkillBase* MakeSkillBase(int32 SkillID);

	UFUNCTION()
	float GetDelayInterval(int32 SkillListIdx, TArray<float> SkillIntervalList);

private:
	//Àû ¸÷ÀÇ ½ºÅ³¼Â Á¤º¸ ¸Ê
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