#pragma once
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "RewardBoxBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateInteraction, class AMainCharacterBase*, PlayerCharacterRef);

UCLASS()
class BACKSTREET_API ARewardBoxBase : public AActor
{
	GENERATED_BODY()

/*-------- Delegate ----------------------- */
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateInteraction OnPlayerBeginInteract;

/*-------- 공통, 컴포넌트 ----------------------- */
public:
	// Sets default values for this actor's properties
	ARewardBoxBase();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* OverlapVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UWidgetComponent* InfoWidgetComponent;

/*------- 주요 기능 ---------------------------*/
protected:
	//위젯 컴포넌트 초기화
	UFUNCTION(BlueprintImplementableEvent)
		void InitializeWidgetComponent();

	//위젯 컴포넌트 활성화(비활성화)
	UFUNCTION(BlueprintImplementableEvent)
		void ActivateWidgetComponent(bool bDeactivateFlag);

	//플레이어 오버랩 시, 위젯을 띄움
	UFUNCTION()
		void OnPlayerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
								, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//플레이어 오버랩 종료 시, 위젯을 지움
	UFUNCTION()
		void OnPlayerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
								, int32 OtherBodyIndex);

	//플레이어에게 어빌리티를 주는 것을 시도한다.
	UFUNCTION()
		void AddAbilityToPlayer(class AMainCharacterBase* PlayerCharacterRef);

	//무작위의 어빌리티 타입을 갖고온다.
	UFUNCTION(BlueprintCallable)
		void SelectRandomAbilityIdx();

// ----- Asset -----------------------------------
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|VFX")
		class UParticleSystem* DestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* DestroyEffectSound;

/*------- 프로퍼티 ---------------------------*/
protected:
	//현재의 보상 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		ECharacterAbilityType AbilityType; 

private:
	//위젯을 띄운 다음에 true로 바뀌어 상호작용(어빌리티 추가)이 가능
	UPROPERTY()
		bool bIsReadyToInteract = false;
};
