// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "Components/BoxComponent.h"
#include "GateBase.generated.h"


DECLARE_DELEGATE_OneParam(FDelegateMove, EDirection);

UCLASS()
class BACKSTREET_API AGateBase : public AActor
{
	GENERATED_BODY()
public:
		FDelegateMove  MoveStageDelegate;

//------------ Global / Component -------------------
public:
	// Sets default values for this actor's properties
	AGateBase();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public: 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* OverlapVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Mesh;
	

//----------- 핵심 로직 ---------------
public:
	// Initialize Gate
	UFUNCTION()
		void InitGate();
	// Bind Delegate Related to Gate
	UFUNCTION()
		void BindGateDelegate();
	// Add Gate to Stage Gate Reference List
	UFUNCTION(BlueprintCallable)
		void AddGate();
	// Check Gate Is Active and RequsetMoveStage
	UFUNCTION(BlueprintCallable)
		void EnterGate();
	// BroadCast MoveRequest to TransitionManager
	UFUNCTION()
		void RequestMoveStage();
	// Activate Gate
	UFUNCTION(BlueprintCallable)
		void ActivateGate();
	// Deactivate Gate
	UFUNCTION(BlueprintCallable)
		void DeactivateGate();
	// Activate Chapter Gate (Guarantee ChpaterClear)
	UFUNCTION(BlueprintCallable)
		void ActivateChapterGateAfterCheck();
	// Set ActivateChpaterGateMaterial
	UFUNCTION()
		void ActivateChapterGateMaterial();
	// Set ActivateNormalGateMaterial
	UFUNCTION()
		void ActivateNormalGateMaterial();
	// Set DeactivateGateMaterial
	UFUNCTION()
		void DeactivateGateMaterial();
	// Check If Gate Is Needed
	UFUNCTION(BlueprintCallable)
		void CheckHaveToNeed();

private:
	UPROPERTY()
		bool bIsGateActive;

//-------- 그 외-----------------------
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Material")
		TArray<class UMaterialInterface*> GateMaterialList;

private:
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

};
