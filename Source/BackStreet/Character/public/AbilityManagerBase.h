// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "TimerManager.h"
#include "AbilityManagerBase.generated.h"

USTRUCT(BlueprintType)
struct FAbilityInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//어빌리티의 ID
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (UIMin = 0, UIMax = 10))
		int32 AbilityId;

	//Ability Type (Multiple Type / this value is for updating stat)
	//ex)  {E_AttackUp, E_DefenseUp}  ->  update character's attack stat var and def stat
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<ECharacterAbilityType> AbilityTypeList;

	//어빌리티명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName AbilityName;

	//어빌리티 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName AbilityDescription;

	//어빌리티의 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UTexture2D* AbilityIcon;
		
	//반복적인 연산이 필요한지? (도트 힐) , 현재 미사용
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		bool bIsRepetitive;

	//Callback 함수명
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		FName FuncName; 

	//어빌리티에 사용할 변수 (증가량)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<float> Variable;

	//Repetitive 연산을 위한 TimerHandle
	UPROPERTY()
		FTimerHandle TimerHandle; 
		FTimerDelegate TimerDelegate;

public:
	inline bool operator==(const FAbilityInfoStruct& other) const
	{
		return AbilityId == other.AbilityId;
	}
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateAbilityInfoList, const TArray<FAbilityInfoStruct>&, AbilityInfoList);

UCLASS()
class BACKSTREET_API UAbilityManagerBase : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	UAbilityManagerBase();
	
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateAbilityInfoList AbilityUpdateDelegate;

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

protected:
	UFUNCTION()
		bool TryUpdateCharacterStat(const FAbilityInfoStruct TargetAbilityInfo, bool bIsReset = false);

	//배열로부터 AbilityInfo를 불러들임
	UFUNCTION()
		FAbilityInfoStruct GetAbilityInfo(int32 AbilityID);

private:
	UFUNCTION()
		bool InitAbilityInfoListFromTable(const UDataTable* AbilityInfoTable);

//--------- Property ----------------------------------------------------
protected:
	//최대 어빌리티 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 1, UIMax = 5))
		int32 MaxAbilityCount = 3;
		
private:
	//현재 플레이어가 소유한 어빌리티의 정보
	UPROPERTY()
		TArray<FAbilityInfoStruct> ActiveAbilityInfoList;
	
	//소유자 캐릭터 약참조
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//모든 어빌리티의 정보
	UPROPERTY()
		TArray<FAbilityInfoStruct> AbilityInfoList;
};
