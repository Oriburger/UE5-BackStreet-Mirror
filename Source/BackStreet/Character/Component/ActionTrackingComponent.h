// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "ActionTrackingComponent.generated.h"

// ���� ���� ��������Ʈ ����
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackBeginDelegate);

UENUM(BlueprintType)
enum class EActionState : uint8
{
	E_None				UMETA(DisplayName = "None"),				 // Error Handling
	E_Ready				UMETA(DisplayName = "Ready"),                // �غ� ����
	E_InProgress		UMETA(DisplayName = "In Progress"),			 // �׼� ���� ��
	E_RecentlyFinished	UMETA(DisplayName = "Recently Finished")	 // �ֱ� ����
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

	// �׼��� �̸� 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		FString ActionName;

	// �׼��� Causer -> (Delegate Broadcast ��ü, �̸� ������� ���ε��� ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		EActionCauserType ActionCauserType; 

	// �׼��� ���� Ʈ���� ��������Ʈ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		FName StartTriggerName;
		FDelegateHandle StartTrigger;

	// �ν���Ʈ �׼�����? (���� Ʈ���Ű� ������ ���� �׼�)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		bool bIsInstantAction;

	// �׼��� ���� Ʈ���� ��������Ʈ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		FName EndTriggerName;
		FDelegateHandle EndTrigger;

	// ��ٿ� Ÿ�Ӿƿ� �� (Ready �� �Ѿ�� �ð�)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float CooldownTimeoutThreshold = 0.5f;

//===================================================================
//====== State ======================================================
	// ���� �׼��� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
		EActionState ActionState;

	// �׼� ���� �ð�
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
		float Duration;

	// ��ٿ� �ð�
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
		float CooldownTime;

	// ���� ��ٿ� �ð�
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

		//�׼� ���°� ��ȯ�� �� �ִ� �׷����� ����
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

//Ÿ�̸� �̺�Ʈ bind ���� �Լ�
private:
	void SetActionResetTimer(FTimerHandle& TimerHandle, float& TimeoutValue, bool& Target);
	bool bIsEnabled = false;

// delegate bind ���� ���� �Լ�
private:


//======================================================================
//====== Monitor Property ==============================================
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TMap<FName, FActionInfo> ActionInfoMap;

//=====================================================================
//====== Monitor Config ===============================================
protected:
	//����׿�) ���鿡�Ե� ����͸� Ÿ�̸Ӹ� �������� ���� �Ǵ�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Debug")
		bool ForceEnableMonitorForEnemy = false;

	//�ִ� �޺� ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (UIMin = 0.0f, UIMax = 1.5f))
		int32 MaxComboCount = 9999;
		
//=====================================================================
//====== etc ==========================================================
protected:
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};
