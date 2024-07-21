// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BackStreet.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StreamableManager.h"
#include "BackStreetGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateClearResource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateSystemMessage, FName, Message, FColor, TextColor);

UCLASS()
class BACKSTREET_API ABackStreetGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateClearResource ClearResourceDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSystemMessage PrintSystemMessageDelegate;

public:
	ABackStreetGameModeBase();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		void InitialzeGame();

// ----- Gameplay Manager -------------------
public:
	UFUNCTION(BlueprintCallable)
		void StartGame(int32 ChapterID);

	UFUNCTION(BlueprintCallable)
		void FinishGame(bool bGameIsOver);

	UFUNCTION(BlueprintImplementableEvent)
		void PrintDebugMessage();

	UFUNCTION(BlueprintCallable)
		void PlayCameraShakeEffect(ECameraShakeType EffectType, FVector Location, float Radius = 100.0f);

	UFUNCTION(BlueprintCallable)
		void ActivateSlowHitEffect(float DilationValue, float Length = 0.25f);

	UFUNCTION(BlueprintCallable)
		void DeactivateSlowHitEffect();

	UFUNCTION()
		AItemBase* SpawnItemToWorld(int32 ItemID, FVector SpawnLocation);
		bool IsLobbyStage();

	UFUNCTION()
		void UpdateCharacterStat(class ACharacterBase* TargetCharacter, FCharacterStatStruct NewStat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAssetManagerBase* GetGlobalAssetManagerBaseRef() { return AssetManagerBase; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class USkillManagerBase* GetGlobalSkillManagerBaseRef() { return SkillManagerBase; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UCraftingManagerBase* GetGlobalCraftingManagerBaseRef() { return CraftingManagerBase; }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateMiniMapUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetMiniMapUI();

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateWaveInfoUI(int32 CurrWave,int32 EndWave);

	UFUNCTION(BlueprintImplementableEvent)
		void PlayUIAnimation(FName AnimationName);


// ----- Class Info ------------------------------------ 
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Class")
		TArray<TSubclassOf<UCameraShakeBase> > CameraShakeEffectList;
	
	//������ ������ Ŭ���� (�ʱ�ȭ�� ���� ���������̺��� ���Ե� BP�� �����ؾ���)
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Class")
		TSubclassOf<class AItemBase> ItemClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Class")
		TSubclassOf<class ANewChapterManagerBase> ChapterManagerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|AssetManager")
		UDataTable* SystemSoundAssetTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|AssetManager")
		UDataTable* WeaponSoundAssetTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|AssetManager")
		UDataTable* SkillSoundAssetTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|AssetManager")
		UDataTable* CharacterSoundAssetTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|AssetManager")
		UDataTable* PropSoundAssetTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|ItemInventory")
		UDataTable* ItemInfoTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|CraftingManager")
		UDataTable* PlayerActiveSkillTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|CraftingManager")
		UDataTable* StatUpgradeInfoTable;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|SkillManager")
		UDataTable* SkillInfoTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|SkillManager")
		UDataTable* SkillStatTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|SkillManager")
		UDataTable* SkillUpgradeInfoTable;

//------ �� �� ������Ƽ ---------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ANewChapterManagerBase* GetChapterManagerRef() { return ChapterManagerRef; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class USkillManagerBase* GetSkillManagerRef() { return SkillManagerBase; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UUserWidget* GetCombatWidgetRef();

protected:
	UPROPERTY()
		class AMainCharacterBase* PlayerCharacterRef;

	UPROPERTY()
		class UAssetManagerBase* AssetManagerBase;

	UPROPERTY()
		class USkillManagerBase* SkillManagerBase;

	UPROPERTY()
		class UCraftingManagerBase* CraftingManagerBase;

	UPROPERTY()
		class ANewChapterManagerBase* ChapterManagerRef;

public:
	//���� ���� ��尡 �ΰ������� Ʈ���������� Ȯ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsInGame;

protected: 
	//���� �Ͻ����� ����
	UPROPERTY(BlueprintReadWrite)
		bool bIsGamePaused = false;


};
