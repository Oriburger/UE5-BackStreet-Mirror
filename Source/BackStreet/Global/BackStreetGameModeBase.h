	// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BackStreet.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StreamableManager.h"
#include "BackStreetGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateClearResource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDelegateSystemMessage, FName, Message, bool, bIsErrorMessage, FColor, TextColor);

UENUM(BlueprintType)
enum class ESystemMessageType: uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_NotEnoughAP		UMETA(DisplayName = "APInsufficient"),
	E_NoWeapon			UMETA(DisplayName = "NoWeapon"),
	E_NoSubWeapon		UMETA(DisplayName = "NoSubWeapon")
};

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

	UFUNCTION(BlueprintCallable)
		bool SetGlobalTimeDilationWithFlag(float DilationValue, bool bIsGlobalSlowEffect = false, bool bForceApply = false);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void PrintSystemMessage(ESystemMessageType MessageType, bool bIsErrorMessage = false, const FText& TextOverride = FText::GetEmpty());

protected:
	UFUNCTION()
		void OnSaveManagerLoadDone(bool bIsSuccess);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data|ItemInventory")
		UDataTable* ItemInfoTable;

//------ 위젯 ------------------------
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

//------ 세이브 및 로드 관련 ------------------------
public:
	UFUNCTION(BlueprintCallable)
		void RequestOpenLevel(FName MapName);

//------ 그 외 프로퍼티 ---------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ANewChapterManagerBase* GetChapterManagerRef() { return ChapterManagerRef; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UScriptManager* GetScriptManagerRef() { return ScriptManagerRef; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsGlobalSlowEffectActive() const { return bIsGlobalSlowEffectActive; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsReadyToPause() const 
		{
			UE_LOG(LogTemp, Warning, TEXT("ABackStreetGameModeBase::GetIsReadyToPause - bIsReadyToPaused: %s, bIsGlobalSlowEffectActive: %s"), bIsReadyToPaused ? TEXT("True") : TEXT("False"), bIsGlobalSlowEffectActive ? TEXT("True") : TEXT("False"));
			return bIsReadyToPaused && !bIsGlobalSlowEffectActive; 
		}

	UFUNCTION(BlueprintCallable)
		void SetIsReadyToPause(bool bNewPaused) { bIsReadyToPaused = bNewPaused; }

protected:
	UPROPERTY()
		class AMainCharacterBase* PlayerCharacterRef;

	UPROPERTY()
		class UAssetManagerBase* AssetManagerBase; 

	UPROPERTY()
		class ANewChapterManagerBase* ChapterManagerRef;

	UPROPERTY()
		class UScriptManager* ScriptManagerRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Script System Data")
		class UDataTable* ScriptDataTableAsset;

	TWeakObjectPtr<class UBackStreetGameInstance> GameInstanceRef;

public:
	//현재 게임 모드가 인게임인지 트랜지션인지 확인
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsInGame;

protected: 
	//게임 일시정지 여부
	UPROPERTY(BlueprintReadWrite)
		bool bIsReadyToPaused = true;
	
	//글로벌 슬로우 모션 효과 적용 여부 (Mutex 원리)
	UPROPERTY()
		bool bIsGlobalSlowEffectActive = false;

	TMap<int32, UClass*> ItemClassCacheMap;


};
