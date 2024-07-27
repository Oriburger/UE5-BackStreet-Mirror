// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "SkillManagerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API USkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkillManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void InitSkillManager();

	UFUNCTION()
	void InitEquipSkillMap();

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
	UFUNCTION(BlueprintCallable)
		FSkillStatStruct GetSkillInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		ASkillBase* GetOwnSkillBase(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		bool IsSkillValid(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct  GetOwnSkillStat(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStateStruct  GetOwnSkillState(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<ESkillType, FSkillListContainer> GetObtainableSkillMap() { return ObtainableSkillMap; }

//======= DataTable ==========================
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Gamplay|Data")
		UDataTable* SkillStatTable;
//====== Property ===========================
private:
	//���� ������ �ִ� ��ų�� SkillID�� ���� �з��� Map (SkillBase���ٿ�)
	TMap<int32, TWeakObjectPtr<ASkillBase>> SkillMap;

	//���� ������ �ִ� ��ų�� SkillType�� ���� �з��� Map (���ս���������)
	TMap<ESkillType, FSkillListContainer > SkillInventoryMap;

	//�÷��� ȹ���� �� �ִ� ��ų�� SkillType�� ���� �з��� Map(���� ����������)
	TMap<ESkillType, FSkillListContainer>ObtainableSkillMap;

	//���� �÷��̾ ������ ��ų Map
	TMap<ESkillType, int32> EquipedSkillMap;
public:
	//�÷��� ȹ���ϱ� ���Ͽ� ���ص� ��ų ����Ʈ(���� ����������)
	TArray<int32> KeepSkillList;

	//��ų����UI���� �̹� ���õǾ��� ��ų ����Ʈ(���� ����������)
	TArray<int32> DisplayedSkillList;

private:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};
