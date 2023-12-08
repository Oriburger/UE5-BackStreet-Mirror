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

//------- 기본 프로퍼티, Action -------------------
public:
	//공격 처리
	virtual void SkillAttack();

	//공격 마무리 처리
	virtual void StopSkillAttack();

	//필수 지정 프로퍼티
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		int32 SkillID;

	//스킬를 초기화 한다. (스탯은 반드시 따로 초기화 해주어야한다.)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void InitSkill(AActor* Causer, class ACharacterBase* Target, int32 NewSkillID, ESkillGrade SkillGrade);

	UFUNCTION()
				void SetSkillManagerRef(class USkillManagerBase* NewSkillManager);

	//--------- 데이터 테이블, 에셋 관련 ----------------------
protected:
	//스킬 에셋 정보 테이블
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* SkillAssetInfoTable;

	virtual void InitSkillAsset();

	//현재 스킬의 에셋 정보
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Asset")
		FSkillAssetInfoStruct SkillAssetInfo;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillAssetInfoStruct GetSkillAssetInfoWithID(int32 TargetSkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillInfoStruct GetSkillInfoWithID(int32 TargetSkillID);

//-------- 그 외 (Ref VFX 등)-------------------------------
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
	//모든 타이머를 해제
	UFUNCTION(BlueprintCallable)
		virtual void ClearAllTimerHandle();

	UPROPERTY()
		FTimerHandle SkillTimerHandle;

protected:
	//게임모드 약 참조
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//소유자 캐릭터 약 참조
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	//
	TWeakObjectPtr<class USkillManagerBase> SkillManagerRef;
};