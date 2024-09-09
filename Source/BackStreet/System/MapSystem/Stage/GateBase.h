// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "Components/BoxComponent.h"
#include "GateBase.generated.h"


DECLARE_DELEGATE_OneParam(FDelegateMove, FVector2D);

UCLASS()
class BACKSTREET_API AGateBase : public AActor
{
	GENERATED_BODY()
public:
	FDelegateMove OnEnterRequestReceived;

//------------ Global / Component -------------------
public:
	// Sets default values for this actor's properties
	AGateBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public: 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UInteractiveCollisionComponent* OverlapVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Mesh;
	
	//Is gate that open level manually using level name property
	UPROPERTY(EditInstanceOnly, Category = "Gameplay")
		bool bManualMode = false;

	//Level asset name to open manually
	//If you want to use this parameter, activate bManualMode.
	UPROPERTY(EditInstanceOnly, Category = "Gameplay")
		FName ManualTargetLevelName;

//----------- 핵심 로직 ---------------
public:
	// Initialize Gate
	UFUNCTION(BlueprintNativeEvent)
		void InitGate(FVector2D NewDirection, FStageInfo StageInfo);

	// Check Gate Is Active and RequsetMoveStage
	UFUNCTION(BlueprintCallable)
		void EnterGate();
	
	// Activate Gate
	UFUNCTION(BlueprintCallable)
		void ActivateGate();
	
	// Deactivate Gate
	UFUNCTION(BlueprintCallable)
		void DeactivateGate();

protected:
	//Stage type name for next stage
	UPROPERTY(BlueprintReadOnly)
		EStageCategoryInfo NextStageType;

private:
	bool bIsGateActive;
	//Gate's direction to move on n*n grid chapter
	FVector2D Direction;

//-------- 그 외-----------------------
private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
	TWeakObjectPtr<class UAssetManagerBase> AssetManagerRef;

};
