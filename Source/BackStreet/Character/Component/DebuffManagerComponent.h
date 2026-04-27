// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "DebuffManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateDebuffAdded, ECharacterDebuffType, DebuffType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateDebuffStackChanged, ECharacterDebuffType, DebuffType, float, CurrentStack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateDebuffStackLock, ECharacterDebuffType, DebuffType, float, StackLocktime);



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

	//Delegate activate event
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateDebuffAdded OnDebuffActivated;


	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateDebuffAdded OnDebuffDeactivated;


	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateDebuffStackLock OnDebuffStackLock;

	//Delegate stack changed event
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateDebuffStackChanged OnDebuffStackChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

//======= User Basic Function =======================
public:
	UFUNCTION(BlueprintCallable)
		void SetDebuffSound(FDebuffInfoStruct DebuffInfo, bool bIsStrongDebuff);

	UFUNCTION(BlueprintCallable)
		void ApplyDebuffStack(FDebuffInfoStruct DebuffInfo, AActor* Causer);

	UFUNCTION(BlueprintCallable)
		void ActivateDebuff(FDebuffInfoStruct DebuffInfo, bool bIsStrongDebuff);

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

	//Debuff Stack Decay Function
	UFUNCTION()
		void StartStackDecay(ECharacterDebuffType DebuffType);

	//Debuff Stack Decay Function
	UFUNCTION()
		void UpdateStackDecay(ECharacterDebuffType DebuffType);
		
	UFUNCTION()
		TArray<AActor*> CheckSphereOverlap(float Radius);

	//get the reset value of debuff
	UFUNCTION()
		FDebuffInfoStruct GetDebuffResetValue(ECharacterDebuffType DebuffType);

	//calculate the percentage of debuff stack for DebuffStackWidget UI
	UFUNCTION()
		float CalculateDebuffStackPercentage(FCharacterGameplayInfo& CharacterInfo, float StackAmount);

//====== reference of debuff timer handle ===========================
private:
	UFUNCTION()
		FTimerHandle& GetDecayTimerHandle(ECharacterDebuffType DebuffType);

	UFUNCTION()
		FTimerHandle& GetDeActivateDebuffTimerMap(ECharacterDebuffType DebuffType);

	UFUNCTION()
		FTimerHandle& GetResetTimerHandle(ECharacterDebuffType DebuffType);

	UFUNCTION()
		FTimerHandle& GetDotDamageTimerHandle(ECharacterDebuffType DebuffType);
		
	UFUNCTION()
		void OnStackLockTimerFinished();

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

	UPROPERTY()
	FTimerHandle	DebuffStackLockTimerHandle;							//for checking debuff active state

	//Timerhandle map for debuff
	UPROPERTY()
	TMap<ECharacterDebuffType, FTimerHandle> DecayTimerMap;				//for debuff stack decay

	UPROPERTY()
	TMap<ECharacterDebuffType, FTimerHandle> DeactivateDebuffTimerMap;	//for deactivating debuff

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