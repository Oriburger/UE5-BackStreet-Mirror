// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "ActionTrackingComponent.generated.h"

// 전투 관련 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackBeginDelegate);

UENUM(BlueprintType)
enum class EActionState : uint8
{
	E_None				UMETA(DisplayName = "None"),				 // Error Handling
	E_Ready				UMETA(DisplayName = "Ready"),                // 준비 상태
	E_InProgress		UMETA(DisplayName = "In Progress"),			 // 액션 진행 중
	E_RecentlyFinished	UMETA(DisplayName = "Recently Finished")	 // 최근 종료
};

UENUM(BlueprintType)
enum class EActionCauserType : uint8
{
	E_None				UMETA(DisplayName = "None"),		//Error Handling		
	E_Owner				UMETA(DisplayName = "Owner"),		//Owner Character
	E_Weapon			UMETA(DisplayName = "Weapon"),		//Owner's WeaponComponent
	E_Enemy				UMETA(DisplayName = "Enemy")		//Enemy Character
};

USTRUCT(BlueprintType)
struct FActionInfo
{
	GENERATED_BODY()

//===================================================================
//====== Basic / Config   ===========================================

	// 액션의 이름 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		FString ActionName;

	// 액션의 Causer -> (Delegate Broadcast 주체, 이를 기반으로 바인딩을 진행)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		EActionCauserType ActionCauserType; 

	// 액션의 시작 트리거 델리게이트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		FName StartTriggerName;
		FDelegateHandle StartTrigger;

	// 인스턴트 액션인지? (종료 트리거가 별도로 없는 액션)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		bool bIsInstantAction;

	// 액션의 종료 트리거 델리게이트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		FName EndTriggerName;
		FDelegateHandle EndTrigger;

	// 쿨다운 타임아웃 값 (Ready 로 넘어가는 시간)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float CooldownTimeoutThreshold = 0.5f;

//===================================================================
//====== State ======================================================
	// 현재 액션의 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
		EActionState ActionState;

	// 액션 지속 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
		float Duration;

	// 쿨다운 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
		float CooldownTime;

	// 남은 쿨다운 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
		float RemainingCooldownTime;

//===================================================================
//====== Timer ======================================================
	FTimerHandle ActionResetTimerHandle;
	FTimerHandle GetTimerHandle() { return ActionResetTimerHandle; }

//===================================================================
//====== Function ===================================================
	bool GetActionIsReady() { return ActionState == EActionState::E_Ready; }
	bool GetActionIsActive() { return ActionState == EActionState::E_InProgress; }
	bool GetActionIsRecentlyEnd() { return ActionState == EActionState::E_RecentlyFinished; } 
	bool TrySetActionState(EActionState NewState)
	{
		if (ActionState == NewState || NewState == EActionState::E_None) return false;

		//액션 상태간 전환할 수 있는 그래프를 정의
		switch (NewState)
		{
		case EActionState::E_Ready:
			if (ActionState == EActionState::E_InProgress)
				return false;
			break;
		case EActionState::E_RecentlyFinished:
			if (ActionState == EActionState::E_Ready)
				return false;
			break;
		}
		ActionState = NewState;
		return true;
	}

	FActionInfo() : ActionState(EActionState::E_Ready), Duration(0.0f), CooldownTime(0.0f), RemainingCooldownTime(0.0f) {}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UActionTrackingComponent : public UActorComponent
{
	GENERATED_BODY()
	
//===================================================================
//====== Common, Delegate ===========================================
public:	
	// Sets default values for this component's properties
	UActionTrackingComponent();

//====================================================================
//====== Basic Function ==============================================
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void BindMonitorEvent(); 

	virtual void Activate(bool bReset) override;

	virtual void Deactivate() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

//======================================================================
//====== Monitor Function ==============================================
public:	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsEnabled() { return bIsEnabled; }

//타이머 이벤트 bind 전용 함수
private:
	void SetActionResetTimer(FTimerHandle& TimerHandle, float& TimeoutValue, bool& Target);
	bool bIsEnabled = false;

// delegate bind 전용 내부 함수
private:


//======================================================================
//====== Monitor Property ==============================================
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TMap<FName, FActionInfo> ActionInfoMap;

//=====================================================================
//====== Monitor Config ===============================================
protected:
	//디버그용) 적들에게도 모니터링 타이머를 적용할지 유무 판단
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Debug")
		bool ForceEnableMonitorForEnemy = false;

	//최대 콤보 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (UIMin = 0.0f, UIMax = 1.5f))
		int32 MaxComboCount = 9999;
		
//=====================================================================
//====== etc ==========================================================
protected:
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};
