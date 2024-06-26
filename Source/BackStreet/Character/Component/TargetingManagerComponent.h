// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "TargetingManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateTargetUpdate, APawn*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateTargetingActivate, bool, bIsActivated, APawn*, Target);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UTargetingManagerComponent : public UActorComponent
{
	GENERATED_BODY()
// ============ Delegate ====================
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTargetUpdate OnTargetUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTargetingActivate OnTargetingActivated;

// ============ Basic =======================
public:	
	// Sets default values for this component's properties
	UTargetingManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

// ============ Core Function =======================
public:
	UFUNCTION()	
		AActor* FindNearEnemyToTarget(float Radius = 500.0f);

	UFUNCTION()
		void ForceTargetingToNearestCharacter();

	//update target to candidate
	//and activate camera following event (only in player)
	UFUNCTION()
		void ActivateTargeting();

	UFUNCTION()
		void DeactivateTargeting();

private:
	//this function can be called with timer if the parameter "bAutoTargeting" is set in true
	//otherwise, you have to call this func manually
	UFUNCTION()
		void UpdateTargetedCandidate();

	UFUNCTION()
		void UpdateCameraRotation();

// ============ Property & Getter & Setter =======================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsTargetingActivated() { return bIsTargetingActivated; }

	UFUNCTION(BlueprintCallable)
		void SetIsTargetingActivated(bool NewState) { bIsTargetingActivated = NewState; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ACharacterBase* GetTargetedCharacter();// {  }

	UFUNCTION(BlueprintCallable)
		void SetTargetedCharacter(class ACharacterBase* NewTarget);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool bAutoTargeting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float TargetingRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float MaxFindDistance = 1500.0f;
		
private:
	bool bIsTargetingActivated;

	TWeakObjectPtr<class ACharacterBase> TargetedCharacter;

	TWeakObjectPtr<class ACharacterBase> TargetedCandidate;

	TWeakObjectPtr<class ACharacterBase> OwnerCharacter;

	TWeakObjectPtr<class UCameraComponent> FollowingCameraRef;

// ============ Timer ================================
private:
	FTimerHandle AutoFindTimerHandle;

	FTimerHandle CameraRotateTimerHandle;
};
