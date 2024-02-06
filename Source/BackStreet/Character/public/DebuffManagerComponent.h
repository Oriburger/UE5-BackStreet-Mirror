// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "../public/CharacterBase.h"
#include "Components/ActorComponent.h"
#include "DebuffManagerComponent.generated.h"

DECLARE_DELEGATE_OneParam(FDeleDebuffClear, ACharacterBase*);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UDebuffManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDebuffManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======= User Basic Function =======================
public:
	//Set the debuff timer to owner
	UFUNCTION(BlueprintCallable)
		bool SetDebuffTimer(ECharacterDebuffType DebuffType, AActor* Causer, float TotalTime = 1.0f, float Variable = 0.0f);

	//clear all the debuff timer
	UFUNCTION(BlueprintCallable)
		void ClearDebuffManager();

	//clear the resource of specific debuff timer
	UFUNCTION(BlueprintCallable)
		void ClearDebuffTimer(ECharacterDebuffType DebuffType);

//======= Getter Function =======================
public:
	//Check the debuff is active
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetDebuffIsActive(ECharacterDebuffType DebuffType);

//======= private fucntions =====================
private:
	//init debuff manager 
	UFUNCTION()
		void InitDebuffManager();

	//get the reference of debuff timer handle
	UFUNCTION()
		FTimerHandle& GetResetTimerHandle(ECharacterDebuffType DebuffType);

	//get the reference of debuff timer handle
	UFUNCTION()
		FTimerHandle& GetDotDamageTimerHandle(ECharacterDebuffType DebuffType);

	//get the reset value of debuff
	UFUNCTION()
		float GetDebuffResetValue(ECharacterDebuffType DebuffType);

	//reset te state of the debuff
	UFUNCTION()
		void ResetStatDebuffState(ECharacterDebuffType DebuffType, float ResetVal);

//====== Property ===========================
private:
	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//Timerhandle map for debuff
	UPROPERTY()
		TMap<ECharacterDebuffType, FTimerHandle> ResetTimerMap;

	UPROPERTY()
		TMap<ECharacterDebuffType, FTimerHandle> DotDamageTimerMap;

	//Resetvalue map for resetting debuff
	UPROPERTY()
		TMap<ECharacterDebuffType, float> ResetValueInfoMap;
};