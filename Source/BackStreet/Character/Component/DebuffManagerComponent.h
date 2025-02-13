// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "DebuffManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateDebuffAdded, ECharacterDebuffType, DebuffType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UDebuffManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDebuffManagerComponent();

	//Delegate add event
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateDebuffAdded OnDebuffAdded;

	//Delegate remove event
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateDebuffAdded OnDebuffRemoved;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======= User Basic Function =======================
public:
	//Set the debuff timer to owner
	UFUNCTION(BlueprintCallable)
		bool SetDebuffTimer(FDebuffInfoStruct DebuffInfo, AActor* Causer);

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

	UFUNCTION(BlueprintCallable)
		void PrintAllDebuff();

//======= private fucntions =====================
private:
	//init debuff manager 
	UFUNCTION()
		void InitDebuffManager();

	//apply dot damage
	UFUNCTION()
		void ApplyDotDamage(ECharacterDebuffType DebuffType, float DamageAmount, AActor* Causer);

	//get the reference of debuff timer handle
	UFUNCTION()
		FTimerHandle& GetResetTimerHandle(ECharacterDebuffType DebuffType);

	//get the reference of debuff timer handle
	UFUNCTION()
		FTimerHandle& GetDotDamageTimerHandle(ECharacterDebuffType DebuffType);

	//get the reset value of debuff
	UFUNCTION()
		FDebuffInfoStruct GetDebuffResetValue(ECharacterDebuffType DebuffType);

	//reset te state of the debuff
	UFUNCTION()
		void ResetStatDebuffState(ECharacterDebuffType DebuffType, FDebuffInfoStruct DebuffInfo);

//====== Sounds ===========================
private:
	void GetDebuffSound(ECharacterDebuffType DebuffType, USoundCue*& StartSound, USoundCue*& LoopSound);

//====== Property ===========================
private:
	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//Gamemode 약 참조
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//Gamemode 약 참조
	TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;

	//Timerhandle map for debuff
	UPROPERTY()
		TMap<ECharacterDebuffType, FTimerHandle> ResetTimerMap;

	UPROPERTY()
		TMap<ECharacterDebuffType, FTimerHandle> DotDamageTimerMap;

	//Reset value map for resetting debuff
	UPROPERTY()
		TMap<ECharacterDebuffType, FDebuffInfoStruct> ResetValueInfoMap;

	// 디버프별 루프 사운드 관리 맵
	UPROPERTY()
		TMap<ECharacterDebuffType, UAudioComponent*> DebuffLoopAudioMap;
};