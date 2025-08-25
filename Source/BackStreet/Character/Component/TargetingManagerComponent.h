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
		AActor* FindNearEnemyToTarget(float RadiusOverride = 0.0f);

	UFUNCTION()
		void ForceTargetingToNearestCharacter();

	//update target to candidate
	//and activate camera following event (only in player)
	UFUNCTION(BlueprintCallable)
		bool ActivateTargeting();

	UFUNCTION(BlueprintCallable)
		void DeactivateTargeting();

private:
	//this function can be called with timer if the parameter "bAutoTargeting" is set in true
	//otherwise, you have to call this func manually
	UFUNCTION()
		void UpdateTargetedCandidate();

	UFUNCTION()
		void UpdateCameraRotation();

	UFUNCTION()
		bool ConeTraceByProfile(const FVector Start, const FVector End, float Radius, FName ProfileName
			, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType
			, TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor = FLinearColor::Red
			, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f);


// ============ Property & Getter & Setter =======================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsTargetingActivated() { return bIsTargetingActivated; }

	UFUNCTION(BlueprintCallable)
		void SetIsTargetingActivated(bool NewState) { bIsTargetingActivated = NewState; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ACharacterBase* GetTargetedCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ACharacterBase* GetTargetedCandidate(); 

	UFUNCTION(BlueprintCallable)
		void SetTargetedCharacter(class ACharacterBase* NewTarget);

	UFUNCTION(BlueprintCallable)
		bool GetIsEnemyInBoundary(FVector EnemyLocation);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool bAutoTargeting = true;

	//타게팅을 유지하기 위한 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float TargetingMaintainThreashold = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float MaxFindDistance = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float TraceRadius = 50.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float AutoTargetingRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bShowDebugSphere = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bUseConeTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0, UIMax = 1.0, EditCondition = "bUseConeTrace"))
		float TraceDensity = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0, UIMax = 90.0, EditCondition = "bUseConeTrace"))
		float TraceAngle = 40.0f; //in degree
		
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
