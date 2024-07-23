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

//======= Global, Component =======================
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
		void InitSkill(FSkillStatStruct NewSkillStat, USkillManagerComponent* NewSkillManagerComponent);

	//Must link with parent function in Blueprint
	UFUNCTION(BlueprintNativeEvent)
		void ActivateSkill();

	//Just hide skillBase (for reusing)
	UFUNCTION(BlueprintCallable)
		void DeactivateSkill();

	//Destroy the skill
	UFUNCTION()
		void DestroySkill();

//======= DataTable, Asset =======================
protected:
	UFUNCTION(BlueprintCallable)
		void PlaySingleSound(FName SoundName);

	UFUNCTION(BlueprintCallable)
		float PlayAnimMontage(ACharacter* Target, FName AnimName);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UNiagaraSystem* GetNiagaraEffect(FName EffectName);

public:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		FSkillStatStruct SkillStat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		FSkillStateStruct SkillState;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Gamplay|Data")
		UDataTable* SkillStatTable;

//======= ETC. (Ref) =======================
protected:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//SkillManagerComponent Soft Ref
	TWeakObjectPtr<class USkillManagerComponent> SkillManagerComponentRef;

	//AssetManager Soft Ref
	TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;

//======= Timer =======================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetSkillRemainingCoolTime();

protected:
	//Call after activate skill
	UFUNCTION(BlueprintCallable)
		void SetSkillBlockedTimer(float Delay);

	//Reset skill unblocked
	UFUNCTION(BlueprintCallable)
		void ResetSkillBlockedTimer();
private:
	//Manage skill's cool time
	FTimerHandle SkillCoolTimeHandle;
};