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
    // �ʱ�ȭ
    void Initialize();
    /*
    // ���� ���� ������ ���� �� �ҷ�����
    void SaveProgress();
    void LoadProgress();

    // ���� ������ ���� �Լ�
    void SetTutorialCompletion(bool bNewState);
    bool GetTutorialCompletion();
    void SetFusionCellCount(int32 NewCount);
    int32 GetFusionCellCount();
    void SetGenesiumCount(int32 NewCount);
    int32 GetGenesiumCount();

	class UProgressSaveGame* GetProgressData() const { return ProgressData; }

private:
    // ���� ������
    UPROPERTY()
        class UProgressSaveGame* ProgressData;

    // ���� �̸� �� �ε���
    const FString ProgressSlot = TEXT("GameProgressSlot");
    int32 UserIndex = 0;*/
};
