// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../../Global/BackStreet.h"
#include "UObject/NoExportTypes.h"
#include "TransitionManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSpawnRequest,class AStageData* , Target);

UCLASS()
class BACKSTREET_API UTransitionManager : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSpawnRequest SpawnRequestDelegate;

		FScriptDelegate LoadCompleteDelegate;

		FScriptDelegate UnloadCompleteDelegate;


public:
		UFUNCTION()
			void InitTransitionManager();

		UFUNCTION()
			void InitChapter(TArray<class AStageData*> StageRef);

		UFUNCTION()
			void TryMoveStage(EDirection Dir);

		UFUNCTION()
			void MoveStage();

		UFUNCTION()
			void LoadStage();

		UFUNCTION()
			void UnLoadStage();

		UFUNCTION()
			void SetStage(AStageData* Target);

		UFUNCTION()
			void SetSpawnPoint(AStageData* Target);

		// Stage Level Unload가 끝내면 실행되어, 새로 생성된 스테이지의 Gate들을 초기화함
		UFUNCTION()
			void SetGate();

		UFUNCTION()
			void HideStageWork(EDirection Dir);

		UFUNCTION()
			void TeleportCharacter();

		// Stage Level Load가 완료되며 실행되어, 스테이지 방문여부에 따라 스테이지 세팅을 진행하며 이전 Stage Level의 UnLoadStage 호출한다.
		UFUNCTION()
			void CompleteLoad();

		UFUNCTION(BlueprintCallable)
			class AStageData* GetStage(uint8 XPosition,uint8 YPosition);

private:
	UPROPERTY()
		TArray<class AStageData*> Stages;

		TWeakObjectPtr<class AStageData> HideStage;

		TWeakObjectPtr<class AChapterManagerBase> ChapterManager;

		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	UPROPERTY()
		EDirection MoveDirection;

	UPROPERTY()
		bool IsMoveStage;

	UPROPERTY(VisibleAnywhere)
		FTimerHandle FadeOutEffectHandle;
};
