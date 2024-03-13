// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../SkillSystem/public/SkillInfoStruct.h"
#include "../../Global/public/AssetInfoStruct.h"
#include "SkillBase.generated.h"

UCLASS()
class BACKSTREET_API ASkillBase : public AActor
{
	GENERATED_BODY()

//------ Global, Component -------------------
public:
	// Sets default values for this actor's properties
	ASkillBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleDefaultsOnly)
		USceneComponent* DefaultSceneRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UStaticMeshComponent* SkillMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		AActor* Causer;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		TArray<class ACharacterBase*> TargetList;

//------- Default Property, Action -------------------
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|Basic")
		int32 SkillID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		TMap<FName, float> SkillGradeVariableMap;

public:
	//Reset Skill (can use both c++, BP)
	UFUNCTION(BlueprintNativeEvent)
		void InitSkill(AActor* NewCauser, TArray<class ACharacterBase*>& NewTargetList, int32 NewSkillID, float NewSkillStartTiming);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void StartSkill();

	UFUNCTION()
		void TrySkill();

	UFUNCTION(BlueprintCallable)
		void HideSkill();

	UFUNCTION()
		void SetSkillManagerRef(class USkillManagerBase* NewSkillManager);

	UFUNCTION()
		ESkillGrade GetSkillGrade();

//--------- DataTable, Asset ----------------------
protected:
	UFUNCTION()
		void InitAsset(int32 NewSkillID);
	
	UFUNCTION()
		void SetAsset();

	//Skill Info Table
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* SkillInfoTable;

	//Skill Asset Info Table
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* SkillAssetInfoTable;

	//Current Skill Info
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		FSkillInfoStruct SkillInfo;

	//Current Skill Asset Info
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Asset")
		FSkillAssetInfoStruct SkillAssetInfo;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillAssetInfoStruct GetSkillAssetInfoWithID(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillInfoStruct GetSkillInfoWithID(int32 TargetSkillID);
	
	UFUNCTION(BlueprintCallable)
		void PlaySingleSound(FName SoundName);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		TArray<class UNiagaraSystem*> SkillEffectParticleList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraSystem* SkillDestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Image")
		class UTexture2D* SkillIconImage;

//-------- ETC. (Ref)-------------------------------
protected:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//Owner Character Soft Ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//SkillManager Soft Ref
	TWeakObjectPtr<class USkillManagerBase> SkillManagerRef;

	//AssetManager Soft Ref
	TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;

//-------- Timer --------------------------------------------
public:
	UFUNCTION()
		void ClearAllTimerHandle();
	
protected:
	UPROPERTY()
		FTimerHandle SkillStartTimingTimerHandle;
};