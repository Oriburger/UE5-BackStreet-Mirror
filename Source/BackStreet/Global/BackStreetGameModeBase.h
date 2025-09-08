	// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BackStreet.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StreamableManager.h"
#include "BackStreetGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateClearResource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateSystemMessage, FName, Message, FColor, TextColor);

class ASaveManager;

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

// ----- Gameplay Manager -------------------
public:
	UFUNCTION(BlueprintCallable)
		void StartGame(int32 ChapterID);

	UFUNCTION(BlueprintCallable)
		void TryContinueGame();

	UFUNCTION(BlueprintCallable)
		void FinishGame(bool bGameIsOver);

	UFUNCTION(BlueprintCallable)
		void RegisterActorToStageManager(AActor* TargetActor);

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAssetManagerBase* GetGlobalAssetManagerBaseRef() { return AssetManagerBase; }

protected:
	UFUNCTION()
		void OnSaveManagerLoadDone(bool bIsSuccess);

// ----- Class Info ------------------------------------ 
public:
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data|ItemInventory")
		UDataTable* ItemInfoTable;

//------ ���� ------------------------
public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<class UUserWidget> MainHUDClass;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UUserWidget* GetMainHUDRef();

	//create combat ui and menu ui
	UFUNCTION(BlueprintCallable)
		void CreateDefaultHUD();

protected:
	//For combat hud widget
	UPROPERTY()
		class UUserWidget* MainHUDRef;

//------ ���̺� �� �ε� ���� ------------------------
public:
	UFUNCTION(BlueprintCallable)
		void RequestOpenLevel(FName MapName, bool bShouldLoadData, FName SaveSlotName = "");

//------ �� �� ������Ƽ ---------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ANewChapterManagerBase* GetChapterManagerRef() { return ChapterManagerRef; }

protected:
	UPROPERTY()
		class AMainCharacterBase* PlayerCharacterRef;

	UPROPERTY()
		class UAssetManagerBase* AssetManagerBase; 

	UPROPERTY()
		class ANewChapterManagerBase* ChapterManagerRef;

	TWeakObjectPtr<class UBackStreetGameInstance> GameInstanceRef;

public:
	//���� ���� ��尡 �ΰ������� Ʈ���������� Ȯ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsInGame;

protected: 
	//���� �Ͻ����� ����
	UPROPERTY(BlueprintReadWrite)
		bool bIsGamePaused = false;
	
	TMap<int32, UClass*> ItemClassCacheMap;


};
