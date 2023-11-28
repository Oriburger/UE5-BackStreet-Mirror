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
	

//----------- �ٽ� ���� ---------------
public:

	UFUNCTION()
		void InitGate();

	UFUNCTION()
		void BindGateDelegate();

	UFUNCTION(BlueprintCallable)
		void AddGate();

	UFUNCTION(BlueprintCallable)
		void EnterGate();

	UFUNCTION()
		void RequestMoveStage();

	UFUNCTION(BlueprintCallable)
		void ActivateGate();

	UFUNCTION(BlueprintCallable)
		void DeactivateGate();

	UFUNCTION(BlueprintCallable)
		void ActivateChapterGateAfterCheck();

	UFUNCTION()
		void ActivateChapterGateMaterial();

	UFUNCTION()
		void ActivateNormalGateMaterial();

	UFUNCTION()
		void DeactivateGateMaterial();
	
	UFUNCTION(BlueprintCallable)
		void CheckHaveToActive();

private:
	UPROPERTY()
		bool bIsGateActive;

//-------- �� ��-----------------------
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Material")
		TArray<class UMaterialInterface*> GateMaterialList;

private:
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

};
