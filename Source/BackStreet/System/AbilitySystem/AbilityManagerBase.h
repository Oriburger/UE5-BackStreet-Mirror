// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "AbilityManagerBase.generated.h"

USTRUCT(BlueprintType)
struct FAbilityValueInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float Variable = 0.0f;

	// 추가로 확률 정보가 필요한지? 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsContainProbability = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsContainProbability", UIMin = 0.0f, UIMax = 1.0f))
		float ProbabilityValue = 1.0f;

public:
	FAbilityValueInfoStruct() : Variable(0.0f), bIsContainProbability(false), ProbabilityValue(0.0f) {}
};

UENUM(BlueprintType)
enum class EAbilityTierType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Common			UMETA(DisplayName = "Common"),
	E_Rare				UMETA(DisplayName = "Rare"),
	E_Legendary			UMETA(DisplayName = "Legendary"),
	E_Mythic			UMETA(DisplayName = "Mythic"),
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	E_None						UMETA(DisplayName = "None"),
	//Legacy
	E_BasicStat					UMETA(DisplayName = "BasicStat"),
	E_Debuff					UMETA(DisplayName = "Debuff"),
	E_Action					UMETA(DisplayName = "Action"),
	E_Item						UMETA(DisplayName = "Item"),
	E_SpecialAction				UMETA(DisplayName = "SpecialAction"),

	//New 
	E_Flame						UMETA(DisplayName = "Flame"),
	E_Poison					UMETA(DisplayName = "Poison"),
	E_Slow						UMETA(DisplayName = "Slow"),
	E_Stun						UMETA(DisplayName = "Stun"),
};

USTRUCT(BlueprintType)
struct FAbilityInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//어빌리티의 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0, UIMax = 10))
		int32 AbilityId = 0;

	//for Multiple Stat
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<ECharacterStatType, FAbilityValueInfoStruct> TargetStatMap;

	//어빌리티의 분류 (어떤 스탯을 강화할 것인지?)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		EAbilityType AbilityType;

	//어빌리티명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName AbilityName;

	//Ability's Tier
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		EAbilityTierType AbilityTier = EAbilityTierType::E_None;

	//어빌리티 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName AbilityDescription;

	//어빌리티의 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UTexture2D* AbilityIcon = nullptr;
		
	//반복적인 연산이 필요한지? (도트 힐) , 현재 미사용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsRepetitive = false;

	//Callback 함수명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName FuncName; 
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<FAbilityValueInfoStruct> VariableInfo;	

public:
	//Repetitive 연산을 위한 TimerHandle
	FTimerHandle TimerHandle; 
	FTimerDelegate TimerDelegate;

public:
	inline bool operator==(const FAbilityInfoStruct& other) const
	{
		return AbilityId == other.AbilityId;
	}

	inline bool IsValid() const
	{
		return AbilityId > 0;
	}
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateAbilityUpdate, const TArray<FAbilityInfoStruct>&, AbilityInfoList);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKSTREET_API UAbilityManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	UAbilityManagerComponent();

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAbilityUpdate OnAbilityUpdated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//--------- Property ----------------------------------------------------
public:
	UPROPERTY(EditDefaultsOnly)
		UDataTable* AbilityInfoTable;

//--------- Function ----------------------------------------------------
public:
	// Active Ability getter 함수
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<ECharacterAbilityType> GetActiveAbilityList() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FAbilityInfoStruct> GetActiveAbilityInfoList() const;

public:
	//어빌리티 매니저 초기화, 부모 설정
	UFUNCTION()
		void InitAbilityManager(ACharacterBase* NewCharacter);

	//특정 어빌리티를 추가 (실패 시 false)
	UFUNCTION()
		bool TryAddNewAbility(int32 AbilityID);

	//소유하고 있는 어빌리티를 제거 (실패 시 false)
	UFUNCTION()
		bool TryRemoveAbility(int32 AbilityID);

	//모든 어빌리티 초기화
	UFUNCTION()
		void ClearAllAbility();

	//해당 Ability가 Active한지 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsAbilityActive(int32 AbilityID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetMaxAbilityCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FAbilityInfoStruct> GetAbilityInfoList() { return ActiveAbilityInfoList;  }

	//아직 뽑힌적 없는 무작위의 어빌리티를 반환한다. (개수와 타입 지정 가능)
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FAbilityInfoStruct> GetRandomAbilityInfoList(int32 Count, TArray<EAbilityType> TypeList);

protected:
	UFUNCTION()
		bool TryUpdateCharacterStat(const FAbilityInfoStruct TargetAbilityInfo, bool bIsReset = false);

	//배열로부터 AbilityInfo를 불러들임 (활성화 된 것 중에서 찾을지 지정 가능)
	UFUNCTION()
		FAbilityInfoStruct GetAbilityInfo(int32 AbilityID, bool bActiveAbilityOnly = false);

	//배열로부터 AbilityInfoList의 idx를 불러들임 (활성화 된 것 중에서 찾을지 지정 가능)
	UFUNCTION()
		int32 GetAbilityListIdx(int32 AbilityID, bool bActiveAbilityOnly = false);

private:
	bool InitAbilityInfoListFromTable();
	//이전 단계의 어빌리티가 뽑혔는지를 체크한다. 
	bool CanPickAbility(int32 AbilityID);

//--------- Property ----------------------------------------------------
protected:
	//최대 어빌리티 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxAbilityCount = 9999999;

private:
	//현재 플레이어가 소유한 어빌리티의 정보
	UPROPERTY()
		TArray<FAbilityInfoStruct> ActiveAbilityInfoList;
		TArray<bool> AbilityPickedInfoList;
	
	//소유자 캐릭터 약참조
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//모든 어빌리티의 정보
	UPROPERTY()
		TArray<FAbilityInfoStruct> AbilityInfoList;
};
