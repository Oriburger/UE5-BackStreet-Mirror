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

	UFUNCTION()
		void InitGate();

	UFUNCTION(BlueprintCallable)
		void AddGate();

	UFUNCTION(BlueprintCallable)
		void EnterGate();

	UFUNCTION()
		void RequestMoveStage();

	UFUNCTION(BlueprintCallable)
		void ActivateChapterGate();

	UFUNCTION(BlueprintCallable)
		void ActivateNormalGate();

	UFUNCTION(BlueprintCallable)
		void DeactivateGate();
	
	UFUNCTION(BlueprintCallable)
		void CheckHaveToActive();



//-------- 그 외-----------------------
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Material")
		TArray<class UMaterialInterface*> GateMaterialList;

	UPROPERTY(VisibleAnywhere)
		FTimerHandle FadeOutEffectHandle;

private:
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

};
