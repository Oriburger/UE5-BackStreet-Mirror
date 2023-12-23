// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../SkillSystem/public/SkillInfoStruct.h"
#include "SkillBase.generated.h"

UCLASS()
class BACKSTREET_API ASkillBase : public AActor
{
	GENERATED_BODY()

//------ Global, Component -------------------
public:
	// Sets default values for this actor's properties
	ASkillBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleDefaultsOnly)
		USceneComponent* DefaultSceneRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UStaticMeshComponent* SkillMesh;

//------- �⺻ ������Ƽ, Action -------------------
public:
	//�ʼ� ���� ������Ƽ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		int32 SkillID;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		AActor* Causer;
		
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		TArray<class ACharacterBase*> TargetList;

	//Reset Skill
	UFUNCTION()
		void InitSkill(AActor* NewCauser, TArray<class ACharacterBase*>& NewTargetList);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void StartSkill();

	UFUNCTION()
		void SetSkillManagerRef(class USkillManagerBase* NewSkillManager);

//--------- ������ ���̺�, ���� ���� ----------------------
protected:
	//��ų ���� ���� ���̺�
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* SkillAssetInfoTable;

	virtual void InitSkillAsset();

	//���� ��ų�� ���� ����
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Asset")
		FSkillAssetInfoStruct SkillAssetInfo;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillAssetInfoStruct GetSkillAssetInfoWithID(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillInfoStruct GetSkillInfoWithID(int32 TargetSkillID);

//-------- �� �� (Ref VFX ��)-------------------------------
protected:
	UFUNCTION()
		void PlayEffectSound(class USoundCue* EffectSound);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UParticleSystem* DestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* SkillSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* SkillFailSound;

protected:
	UFUNCTION(BlueprintCallable)
		virtual void ClearAllTimerHandle();

	UPROPERTY()
		FTimerHandle SkillTimerHandle;

protected:
	//���Ӹ�� �� ����
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//������ ĳ���� �� ����
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//SkillManager Weak Pointer
	TWeakObjectPtr<class USkillManagerBase> SkillManagerRef;
};