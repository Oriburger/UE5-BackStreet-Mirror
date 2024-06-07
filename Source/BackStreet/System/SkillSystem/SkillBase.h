// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

//======= User Basic Function =======================
public:	
	UFUNCTION()
		void InitSkill(FSkillInfoStruct NewSkillInfo);

	//Must link with parent function in Blueprint
	UFUNCTION(BlueprintNativeEvent)
		void ActivateSkill();

	//Just hide skillBase (for reusing)
	UFUNCTION(BlueprintCallable)
		void DeactivateSkill();

	//Destroy the skill
	UFUNCTION()
		void DestroySkill();

//--------- DataTable, Asset ----------------------
protected:
	UFUNCTION(BlueprintCallable)
		void PlaySingleSound(FName SoundName);

	UFUNCTION(BlueprintCallable)
		float PlayAnimMontage(ACharacter* Target, FName AnimName);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UNiagaraSystem* GetNiagaraEffect(FName EffectName);

public:	
	//Skill Info
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		FSkillInfoStruct SkillInfo;
//-------- ETC. (Ref)-------------------------------
protected:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//SkillManager Soft Ref
	TWeakObjectPtr<class USkillManagerBase> SkillManagerRef;

	//AssetManager Soft Ref
	TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;

//-------- Timer --------------------------------------------

};