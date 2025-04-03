// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "GameProgressManager.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UGameProgressManager : public UObject
{
	GENERATED_BODY()

public:
    // 초기화
    void Initialize();
    /*
    // 게임 진행 데이터 저장 및 불러오기
    void SaveProgress();
    void LoadProgress();

    // 진행 데이터 조작 함수
    void SetTutorialCompletion(bool bNewState);
    bool GetTutorialCompletion();
    void SetFusionCellCount(int32 NewCount);
    int32 GetFusionCellCount();
    void SetGenesiumCount(int32 NewCount);
    int32 GetGenesiumCount();

	class UProgressSaveGame* GetProgressData() const { return ProgressData; }

private:
    // 내부 데이터
    UPROPERTY()
        class UProgressSaveGame* ProgressData;

    // 슬롯 이름 및 인덱스
    const FString ProgressSlot = TEXT("GameProgressSlot");
    int32 UserIndex = 0;*/
};
