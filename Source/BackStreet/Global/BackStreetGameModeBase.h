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

	UFUNCTION()
		void UpdateCharacterStat(class ACharacterBase* TargetCharacter, FCharacterStatStruct NewStat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAssetManagerBase* GetGlobalAssetManagerBaseRef() { return AssetManagerBase; }

// ----- Class Info ------------------------------------ 
public:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Class")
		TArray<TSubclassOf<UCameraShakeBase> > CameraShakeEffectList;
	
	//스폰할 아이템 클래스 (초기화를 위한 데이터테이블이 포함된 BP를 지정해야함)
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
		UDataTable* StatUpgradeInfoTable;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data|SkillManager")
		UDataTable* SkillStatTable;

//------ 위젯 ------------------------
public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<class UCommonUserWidget> MainHUDClass;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UCommonUserWidget* GetMainHUDRef();

	//create combat ui and menu ui
	UFUNCTION(BlueprintCallable)
		void CreateDefaultHUD();

protected:
	//For combat hud widget
	UPROPERTY()
		class UCommonUserWidget* MainHUDRef;

//------ 그 외 프로퍼티 ---------------
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

public:
	//현재 게임 모드가 인게임인지 트랜지션인지 확인
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsInGame;

protected: 
	//게임 일시정지 여부
	UPROPERTY(BlueprintReadWrite)
		bool bIsGamePaused = false;


};
