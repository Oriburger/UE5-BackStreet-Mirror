#pragma once
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../../Global/public/BackStreet.h"
#include "WaveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateStage, class AStageData*, Target);

UCLASS()
class BACKSTREET_API AWaveManager : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStage StageClearDelegate;

	// Gate 비활성화 및 몬스터 스폰
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStage StartDefenseWaveDelegate;

	// Gate 활성화 및 몬스터 스폰 종료
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStage ClearDefenseWaveDelegate;

public:
	// 델리게이트 바인딩
	UFUNCTION()
		void InitWaveManager(class AChapterManagerBase* Target);

	// 참조 멤버 변수 초기화
	UFUNCTION()
		void InitReference(class AChapterManagerBase* TargetA, class AResourceManager* TargetB);

	// 한 스테이지의 모든 웨이브 클리어 여부 체크
	UFUNCTION()
		bool CheckAllWaveClear(class AStageData* Target);

	// 함수명 고민중.., 몬스터가 죽으면 호출되어 웨이브 클리어 여부 체크 및 몬스터 수 관리
	UFUNCTION()
		void CheckWaveCategoryByType(class AStageData* Target, AEnemyCharacterBase* Enemy);

	// 스테이지 타입별 웨이브 설정
	UFUNCTION()
		void SetWave(class AStageData* Target);

	// 하데스 웨이브 생성
	UFUNCTION()
		void SpawnHadesWave(class AStageData* Target);

	// 디펜스 웨이브 생성
	UFUNCTION()
		void SpawnDefenseWave(class AStageData* Target);

	// 클리어 체크 및 스폰 관련 타이머 설정, 시간되면 Clear Wave 호출 
	UFUNCTION()
		void SetDefenseWaveTimer(class AStageData* Target, float time);

	// 모든 몬스터 삭제, 다음 웨이브 혹은 스테이지 클리어
	UFUNCTION()
		void ClearDefenseWave(class AStageData* Target);

	//  디펜스 웨이브의 시간 계산
	UFUNCTION()
		void CalculateWaveTime();

	/*UFUNCTION()
		void SpawnMonsterWithID(class AStageData* Target, int32 EnemyID);*/

	// 디펜스 웨이브의 스테이지에 존재하는 몬스터 수 관리
	UFUNCTION()
		void ManageDefenseWaveMonsterCount(class AStageData* Target, int32 EnemyID, bool IsSpawn);

public:
	TWeakObjectPtr<class AChapterManagerBase> ChapterManager;

	TWeakObjectPtr<class AResourceManager> ResourceManager;

public:
	UPROPERTY()
		FTimerHandle SpawnTimerHandle;

};
