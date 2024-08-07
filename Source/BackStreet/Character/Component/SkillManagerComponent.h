// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "SkillManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateEquiped, int32, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateSkillUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSkillActivated, int32, SkillID);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API USkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkillManagerComponent();

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillUpdated OnSkillUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillActivated OnSkillActivated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void InitSkillManager();

	UFUNCTION()
		void InitSkillMap();

//======= User Basic Function =======================
public:
	UFUNCTION(BlueprintCallable)
		bool TrySkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool AddSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool RemoveSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool UpgradeSkill(int32 SkillID, uint8 NewLevel);

	UFUNCTION(BlueprintCallable)
		void UpdateObtainableSkillMap();

	UFUNCTION(BlueprintCallable)
		bool EquipSkill(int32 NewSkillID);

//======== Getter ============================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct GetSkillInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		ESkillType GetSkillTypeInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		ASkillBase* GetOwnSkillBase(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsSkillValid(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct  GetOwnSkillStat(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStateStruct  GetOwnSkillState(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ESkillType, FSkillListContainer> GetSkillInventoryMap() { return SkillInventoryMap; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ESkillType, FObtainableSkillListContainer> GetObtainableSkillMap() { return ObtainableSkillMap; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ESkillType, int32> GetEquipedSkillMap() { return EquipedSkillMap; }

//======= DataTable ==========================
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gamplay|Data")
		UDataTable* SkillStatTable;

//====== Property ===========================
private:
	//현재 가지고 있는 스킬을 SkillType에 따라 분류한 Map
	TMap<ESkillType, FSkillListContainer> SkillInventoryMap;

	//플레어어가 획득할 수 있는 스킬을 SkillType에 따라 분류한 Map(조합 스테이지용)
	TMap<ESkillType, FObtainableSkillListContainer> ObtainableSkillMap;

	//현재 플레이어가 장착한 스킬 Map
	TMap<ESkillType, int32> EquipedSkillMap;


//@@@@@@이런 정보들은 정보 은닉 지켜주세요, private로 전환 바랍니다@@@@@@@@@@@
public:
	//플레어어가 획득하기 위하여 찜해둔 스킬 리스트(조합 스테이지용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<int32> KeepSkillList;

	//스킬제작UI에서 이미 제시되었던 스킬 리스트(조합 스테이지용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<int32> DisplayedSkillList;

private:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};
