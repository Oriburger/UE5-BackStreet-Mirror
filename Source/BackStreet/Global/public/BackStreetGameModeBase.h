// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "../public/BackStreet.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StreamableManager.h"
#include "BackStreetGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateClearResource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSingleParam, bool, bGameIsOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateSystemMessage, FName, Message, FColor, TextColor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateStageClear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateMissionClear);

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
		FDelegateClearResource ChapterClearResourceDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSystemMessage PrintSystemMessageDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStageClear StageClearDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStageClear FadeOutDelegate;


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
		AItemBase* SpawnItemToWorld(uint8 ItemType, uint8 ItemID, FVector SpawnLocation);

	UFUNCTION()
		void UpdateCharacterStat(class ACharacterBase* TargetCharacter, FCharacterStatStruct NewStat);

	UFUNCTION()
		void UpdateCharacterStatWithID(class ACharacterBase* TargetCharacter, const uint32 CharacterID);

	UFUNCTION()
		void UpdateWeaponStat(class AWeaponBase* TargetWeapon, FWeaponStatStruct NewStat);

	UFUNCTION()
		void UpdateWeaponStatWithID(class AWeaponBase* TargetWeapon, const uint8 WeaponID);

	UFUNCTION()
		void UpdateProjectileStatWithID(class AProjectileBase* TargetProjectile, const uint8 ProjectileID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStatStruct GetWeaponStatInfoWithID(const uint8 WeaponID);

	UFUNCTION()
		FStageEnemyTypeStruct GetStageTypeInfoWithRow(uint16 rowName);

	UFUNCTION()
		class UDebuffManager* GetGlobalDebuffManagerRef() { return DebuffManager; }

	UFUNCTION(BlueprintCallable)
		class AChapterManagerBase* GetChapterManagerRef() { return ChapterManager; }

	UFUNCTION(BlueprintImplementableEvent)
		void FadeOut();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateMiniMapUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetMiniMapUI();

	UFUNCTION(BlueprintImplementableEvent)
		void PopUpClearUI();


	// ------ Asset Info ----------------------------
public:
	//캐릭터의 애니메이션 에셋 데이터를 읽어들임
	UFUNCTION()
		FCharacterAnimAssetInfoStruct GetCharacterAnimAssetInfoData(const int32 CharacterID);

protected:
	//애니메이션 에셋 경로 저장 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Asset|Data")
		UDataTable* AnimAssetInfoTable;

	//VFX 에셋 경로 저장 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Asset|Data")
		UDataTable* VFXssetInfoTable;

private:
	//추후 따로 뺄 예정
	UPROPERTY()
		FCharacterAssetInfoStruct CachedCharacterAssetInfoData;

// ------ Data Table -----------------------------
protected:
	//적의 스탯 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* EnemyStatTable;
	
	//무기 스탯 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* WeaponStatTable;

	//발사체 스탯 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* ProjectileStatTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* StageTypeTable;

// ----- Class Info ------------------------------------ 
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|VFX")
		TArray<TSubclassOf<UCameraShakeBase> > CameraShakeEffectList;
	
	//EItemCategoryInfo의 값이 Idx
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Item")
		TMap<int32, TSubclassOf<class AItemBase> > ItemClassMap;

//------ 그 외 프로퍼티 ---------------
protected:
	UPROPERTY()
		class AMainCharacterBase* PlayerCharacterRef;

	UPROPERTY()
		class UDebuffManager* DebuffManager;

	UPROPERTY(BlueprintReadWrite)
		class AChapterManagerBase* ChapterManager;

public:
	//현재 게임 모드가 인게임인지 트랜지션인지 확인
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsInGame;

protected: 
	//게임 일시정지 여부
	UPROPERTY(BlueprintReadWrite)
		bool bIsGamePaused = false;


};
