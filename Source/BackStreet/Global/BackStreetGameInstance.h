// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BackStreet.h"
#include "Engine/GameInstance.h"
#include "BackStreetGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UBackStreetGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

//------ 게임 저장 -------------------
public:
	UFUNCTION()
		void SaveGameData();

	UFUNCTION()
		void LoadGameData();

	// 튜토리얼 진행 여부 체크
	UFUNCTION(BlueprintCallable)
		void SetTutorialCompletion(bool bCompleted);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetTutorialCompletion();

//------ PROPERTY ---------------------
private:
	UPROPERTY()
		class UGameProgressManager* GameProgressManager;

	// 내부 데이터
	UPROPERTY()
		class UProgressSaveGame* ProgressData;

	// 슬롯 이름 및 인덱스
	const FString ProgressSlot = TEXT("ProgressSlot");
	int32 UserIndex = 0;
};
