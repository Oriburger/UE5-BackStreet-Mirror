// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../../Global/public/BackStreet.h"
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
			void MoveStage(EDirection Dir);

		UFUNCTION()
			void LoadStage();

		UFUNCTION()
			void UnLoadStage();

		UFUNCTION()
			void SetStage(AStageData* Target);

		UFUNCTION()
			void SetSpawnPoint(AStageData* Target);

		UFUNCTION()
			void SetGate();

		UFUNCTION()
			void HideStageWork(EDirection Dir);

		UFUNCTION()
			void TeleportCharacter();

		UFUNCTION()
			void CompleteLoad();

		UFUNCTION(BlueprintCallable)
			class AStageData* GetStage(uint8 XPosition,uint8 YPosition);

private:
	UPROPERTY()
		TArray<class AStageData*> Stages;

		TWeakObjectPtr<class AStageData> HideStage;

		TWeakObjectPtr<class AChapterManagerBase> ChapterManager;

	UPROPERTY()
		EDirection MoveDirection;

};
