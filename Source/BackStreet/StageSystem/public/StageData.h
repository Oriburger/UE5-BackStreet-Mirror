// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "StageData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateAIContorl);

UCLASS()
class BACKSTREET_API AStageData : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAIContorl AIOnDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAIContorl AIOffDelegate;

public:
	AStageData();

	UFUNCTION()
		int32 GetXPos() { return StageInfo.XPos; }

	UFUNCTION()
		void SetXPos(int32 Pos) { StageInfo.XPos = Pos; }

	UFUNCTION()
		int32 GetYPos() { return StageInfo.YPos; }

	UFUNCTION()
		void SetYPos(int32 Pos) { StageInfo.YPos = Pos; }

	UFUNCTION()
		FVector GetStageLocation() { return StageInfo.StageLocation; }

	UFUNCTION()
		void SetStageLocation(FVector Vector) { StageInfo.StageLocation = Vector; }

	UFUNCTION(BlueprintCallable)
		EStageCategoryInfo GetStageType() { return StageInfo.Type; }

	UFUNCTION()
		void SetStageType(EStageCategoryInfo StageType) { StageInfo.Type = StageType; }

	UFUNCTION()
		FName GetLevelToLoad() { return StageInfo.LevelToLoad; }

	UFUNCTION()
		void SetLevelToLoad(FName Level) { StageInfo.LevelToLoad = Level; }

	UFUNCTION()
		bool GetIsClear() { return StageInfo.bIsClear; }

	UFUNCTION()
		void SetIsClear(bool Clear) { StageInfo.bIsClear = Clear; }

	UFUNCTION()
		bool GetIsValid() { return StageInfo.bIsValid; }

	UFUNCTION()
		void SetIsValid(bool Valid) { StageInfo.bIsValid = Valid; }

	UFUNCTION(BlueprintCallable)
		bool GetIsVisited() { return StageInfo.bIsVisited; }

	UFUNCTION()
		void SetIsVisited(bool Visited) { StageInfo.bIsVisited = Visited; }

	UFUNCTION()
		void AddGateInfo(bool Target) { StageInfo.GateInfo.Add(Target); }

	UFUNCTION(BlueprintCallable)
		bool GetGateInfoDir(EDirection Dir) { return StageInfo.GateInfo[(uint8)(Dir)]; }

	UFUNCTION()
		void SetGateInfoDir(EDirection Dir, bool Info) { StageInfo.GateInfo[(uint8)(Dir)] = Info; }

	UFUNCTION(BlueprintCallable)
		TArray<FVector> GetMonsterSpawnPoints() { return StageInfo.MonsterSpawnPoints; }

	UFUNCTION(BlueprintCallable)
		TArray<FVector> GetItemSpawnPoints() { return StageInfo.ItemSpawnPoints; }

	UFUNCTION(BlueprintCallable)
		TArray<FVector> GetCharacterSpawnPoint() { return StageInfo.CharacterSpawnPoint; }

	UFUNCTION(BlueprintCallable)
		TArray<FVector> GetRewardBoxSpawnPoint() { return StageInfo.RewardBoxSpawnPoint; }

	UFUNCTION()
		void AddMonsterSpawnPoint(FVector Target) { StageInfo.MonsterSpawnPoints.Add(Target); }

	UFUNCTION()
		void AddItemSpawnPoint(FVector Target) { StageInfo.ItemSpawnPoints.Add(Target); }

	UFUNCTION()
		void AddCharacterSpawnPoint(FVector Target) { StageInfo.CharacterSpawnPoint.Add(Target); }

	UFUNCTION()
		void AddRewardBoxSpawnPoint(FVector Target) { StageInfo.RewardBoxSpawnPoint.Add(Target); }

	UFUNCTION()
		class AEnemyCharacterBase* GetMonsterIdx(int32 Idx) { return StageInfo.MonsterList[Idx]; }

	UFUNCTION()
		class AItemBase* GetItemIdx(int32 Idx) { return StageInfo.ItemList[Idx]; }

	UFUNCTION()
		class AItemBoxBase* GetItemBoxIdx(int32 Idx) { return StageInfo.ItemBoxList[Idx]; }

	UFUNCTION(BlueprintCallable)
		TArray<class AEnemyCharacterBase*> GetMonsterList() { return StageInfo.MonsterList; }

	UFUNCTION()
		TArray<class AItemBase*> GetItemList() { return StageInfo.ItemList; }

	UFUNCTION()
		TArray<class AItemBoxBase*> GetItemBoxList() { return StageInfo.ItemBoxList; }

	UFUNCTION()
		TArray<class AGateBase*> GetGateList() { return StageInfo.GateList; }

	UFUNCTION()
		class ARewardBoxBase* GetRewardBox() { return StageInfo.RewardBoxRef; }

	UFUNCTION()
		void SetRewardBox(class ARewardBoxBase* Target) { StageInfo.RewardBoxRef = Target; }

	UFUNCTION()
		void AddMonsterList(class AEnemyCharacterBase* Target) { StageInfo.MonsterList.Add(Target); }

	UFUNCTION()
		void RemoveMonsterList(class AEnemyCharacterBase* Target) { StageInfo.MonsterList.Remove(Target); }

	UFUNCTION()
		void AddItemBoxList(class AItemBoxBase* Target) { StageInfo.ItemBoxList.Add(Target); }

	UFUNCTION()
		void RemoveItemBoxList(class AItemBoxBase* Target) { StageInfo.ItemBoxList.Remove(Target); }

	UFUNCTION()
		void AddItemList(class AItemBase* Target) { StageInfo.ItemList.Add(Target); }

	UFUNCTION()
		void AddGateList(class AGateBase* Target) { StageInfo.GateList.Add(Target); }

	UFUNCTION()
		void EmptyGateList() { StageInfo.GateList.Empty(); }

	UFUNCTION()
		ULevelStreaming* GetLevelRef() { return StageInfo.LevelRef; }

	UFUNCTION()
		void SetLevelRef(ULevelStreaming* Target) { StageInfo.LevelRef = Target; }

private:
	UPROPERTY()
		FStageDataStruct StageInfo;
};
