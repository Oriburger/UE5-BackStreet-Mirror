// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
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

	virtual void Tick(float DeltaSeconds) override;

public:
	UPROPERTY(VisibleDefaultsOnly)
		USceneComponent* DefaultSceneRoot;

//======= User Basic Function =======================
public:	
	UFUNCTION()
		void InitSkill(FSkillStatStruct NewSkillStat, class USkillManagerComponentBase* NewSkillManagerComponent, FSkillInfo NewSkillInfo = FSkillInfo());

	//Must link with parent function in Blueprint
	UFUNCTION(BlueprintNativeEvent)
		void ActivateSkill();

	//Just hide skillBase (for reusing)
	UFUNCTION(BlueprintCallable)
		void DeactivateSkill();

	UFUNCTION(BlueprintCallable)
		float PlayAnimMontage(ACharacter* Target, FName AnimName);

//======= Getter Function =======================
	UFUNCTION(BlueprintCallable, Blueprintpure)
		ACharacterBase* GetOwnerCharacterRef();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UAnimMontage* GetSkillAnimMontage() { return SkillInfo.SkillAnimation; }

//======= DataTable, Asset =======================
public:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		FSkillInfo SkillInfo;

//======= ETC. (Ref) =======================
protected:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//SkillManagerComponent Soft Ref
	TWeakObjectPtr<class USkillManagerComponentBase> SkillManagerComponentRef;

	//OwnerCharacter Soft Ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterBaseRef;

private:
	//Manage skill's cool time
	FTimerHandle SkillCoolTimeHandle;
};