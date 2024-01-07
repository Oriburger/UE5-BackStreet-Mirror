// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../SkillSystem/public/SkillInfoStruct.h"
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

//------- Default Property, Action -------------------
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		int32 SkillID;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		AActor* Causer;
		
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		TArray<class ACharacterBase*> TargetList;

public:
	//Reset Skill
	UFUNCTION()
		void InitSkill(AActor* NewCauser, TArray<class ACharacterBase*>& NewTargetList, float NewSkillStartTiming);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void StartSkill();

	UFUNCTION(BlueprintCallable)
		void HideSkill();

	UFUNCTION()
		void SetSkillManagerRef(class USkillManagerBase* NewSkillManager);

//--------- DataTable, Asset ----------------------
protected:
	virtual void InitSkillAsset();

	//Skill Asset Info Table
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* SkillAssetInfoTable;

	//Current Asset info
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Asset")
		FSkillAssetInfoStruct SkillAssetInfo;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillAssetInfoStruct GetSkillAssetInfoWithID(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillInfoStruct GetSkillInfoWithID(int32 TargetSkillID);

//-------- ETC. (Ref,  VFX)-------------------------------
protected:
	UFUNCTION()
		void PlayEffectSound(class USoundCue* EffectSound);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UParticleSystem* DestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* SkillSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* SkillFailSound;

protected:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//Owner Character Soft Ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//SkillManager Weak Pointer
	TWeakObjectPtr<class USkillManagerBase> SkillManagerRef;

//-------- Timer --------------------------------------------
public:
	UFUNCTION()
		void ClearAllTimerHandle();
	
protected:
	UPROPERTY()
		FTimerHandle SkillStartTimingTimerHandle;
};