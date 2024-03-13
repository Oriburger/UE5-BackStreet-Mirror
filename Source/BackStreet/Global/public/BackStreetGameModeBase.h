// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "../public/BackStreet.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StreamableManager.h"
#include "BackStreetGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateClearResource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSingleParam, bool, bGameIsOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateSystemMessage, FName, Message, FColor, TextColor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateUIAnimation, FName, AnimationName);




UCLASS()
class BACKSTREET_API ABackStreetGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSingleParam StartChapterDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSingleParam FinishChapterDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateClearResource ClearResourceDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSystemMessage PrintSystemMessageDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateUIAnimation UIAnimationDelegate;

public:
	ABackStreetGameModeBase();

protected:
	virtual void BeginPlay() override;


// ----- Gameplay Manager -------------------
public:
	UFUNCTION(BlueprintImplementableEvent)
		void CreateChapterManager();

	UFUNCTION(BlueprintCallable)
		void InitializeGame();

	UFUNCTION(BlueprintImplementableEvent)
		void PrintDebugMessage();

	UFUNCTION(BlueprintImplementableEvent)
		void FinishChapter(bool bGameIsOver);

	UFUNCTION(BlueprintCallable)
		void PlayCameraShakeEffect(ECameraShakeType EffectType, FVector Location, float Radius = 100.0f);

	UFUNCTION()
		AItemBase* SpawnItemToWorld(int32 ItemID, FVector SpawnLocation);

	UFUNCTION(BlueprintCallable)
		bool IsLobbyStage();

	UFUNCTION()
		void UpdateCharacterStat(class ACharacterBase* TargetCharacter, FCharacterStatStruct NewStat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAssetManagerBase* GetGlobalAssetManagerBaseRef() { return AssetManagerBase; }

	UFUNCTION(BlueprintCallable)
		class USkillManagerBase* GetGlobalSkillManagerBaseRef() { return SkillManagerBase; }

	UFUNCTION(BlueprintCallable)
		class AChapterManagerBase* GetChapterManagerRef() { return ChapterManager; }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateMiniMapUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetMiniMapUI();

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateWaveInfoUI(int32 CurrWave,int32 EndWave);

	UFUNCTION(BlueprintImplementableEvent)
		void PlayUIAnimation(FName AnimationName);


// ----- Class Info ------------------------------------ 
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|VFX")
		TArray<TSubclassOf<UCameraShakeBase> > CameraShakeEffectList;
	
	//������ ������ Ŭ���� (�ʱ�ȭ�� ���� ���������̺��� ���Ե� BP�� �����ؾ���)
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Item")
		TSubclassOf<class AItemBase> ItemClass;

//------ �� �� ������Ƽ ---------------
protected:
	UPROPERTY()
		class AMainCharacterBase* PlayerCharacterRef;

	UPROPERTY()
		class UAssetManagerBase* AssetManagerBase;

	UPROPERTY()
		class USkillManagerBase* SkillManagerBase;

	UPROPERTY(BlueprintReadWrite)
		class AChapterManagerBase* ChapterManager;

public:
	//���� ���� ��尡 �ΰ������� Ʈ���������� Ȯ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsInGame;

protected: 
	//���� �Ͻ����� ����
	UPROPERTY(BlueprintReadWrite)
		bool bIsGamePaused = false;


};
