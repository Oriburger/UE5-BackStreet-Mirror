#pragma once

#include "../../Global/public/BackStreet.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

UCLASS()
class BACKSTREET_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

	friend class AWeaponInventoryBase;

//----- Global / Component ----------
public:
	// Sets default values for this character's properties
	ACharacterBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly)
		UChildActorComponent* InventoryComponent;	

// ------- Character Action 기본 ------- 
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		int32 CharacterID;

	//Input에 Binding 되어 공격을 시도 (AnimMontage를 호출)
	virtual void TryAttack();

	///Input에 Binding 되어 스킬공격을 시도 (AnimMontage를 호출)
	virtual void TrySkillAttack(ACharacterBase* Target);

	//AnimNotify에 Binding 되어 실제 공격을 수행
	virtual void Attack();

	virtual void StopAttack();

	virtual void TryReload();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
		, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Die();

	//플레이어가 현재 해당 Action을 수행하고 있는지 반환
	UFUNCTION(BlueprintCallable)
		bool GetIsActionActive(ECharacterActionType Type) { return CharacterState.CharacterActionState == Type; }

	//플레이어의 ActionState를 Idle로 전환한다.
	UFUNCTION(BlueprintCallable)
		void ResetActionState(bool bForceReset = false);

	//플레이어가 체력을 회복함 (일회성)
	UFUNCTION()
		void TakeHeal(float HealAmountRate, bool bIsTimerEvent = false, uint8 BuffDebuffType = 0);

	//디버프 데미지를 입힘 (일회성)
	UFUNCTION(BlueprintCallable)
		float TakeDebuffDamage(float DamageAmount, ECharacterDebuffType DebuffType, AActor* Causer);

	UFUNCTION(BlueprintCallable)
		void TakeKnockBack(AActor* Causer, float Strength);

	//공격Action 사이의 Interval을 관리하는 타이머를 해제
	UFUNCTION()
		void ResetAtkIntervalTimer();

// ------- Character Stat/State ------------------------------
public:
	//캐릭터의 상태 정보를 초기화
	UFUNCTION()
		void InitCharacterState();

	//캐릭터의 디버프 정보를 업데이트
	UFUNCTION(BlueprintCallable)
		virtual	bool TryAddNewDebuff(ECharacterDebuffType NewDebuffType, AActor* Causer = nullptr, float TotalTime = 0.0f, float Value = 0.0f);

	//디버프가 활성화 되어있는지 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetDebuffIsActive(ECharacterDebuffType DebuffType);

	//캐릭터의 스탯을 업데이트
	UFUNCTION(BlueprintCallable)
		void UpdateCharacterStat(FCharacterStatStruct NewStat);

	UFUNCTION(BlueprintCallable)
		void UpdateCharacterState(FCharacterStateStruct NewState);

	UFUNCTION(BlueprintCallable)
		void UpdateWeaponStat(FWeaponStatStruct NewStat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FCharacterStatStruct GetCharacterStat() { return CharacterStat; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FCharacterStateStruct GetCharacterState() { return CharacterState; }

// ------ 무기 관련 -------------------------------------------
public:
	UFUNCTION()
		bool EquipWeapon(class AWeaponBase* TargetWeapon);

	//무기를 집는다. 인벤토리가 꽉 찼다면 false를 반환
	virtual bool PickWeapon(int32 NewWeaponID);

	//다음 무기로 전환한다. 전환에 실패하면 false를 반환
	virtual void SwitchToNextWeapon();

	//무기를 Drop한다. (월드에서 아예 사라진다.)
	virtual void DropWeapon();

	UFUNCTION()
		bool TrySwitchToSubWeapon(int32 SubWeaponIdx);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AWeaponInventoryBase* GetInventoryRef();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AWeaponInventoryBase* GetSubInventoryRef();

	//무기 Ref를 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AWeaponBase* GetCurrentWeaponRef();

protected:
	UPROPERTY()
		TSubclassOf<class AWeaponInventoryBase> WeaponInventoryClass;

	UFUNCTION()
		void SwitchWeaponActor(EWeaponType TargetWeaponType);

protected:
	UPROPERTY()
		class AWeaponInventoryBase* InventoryRef;

private:
	UPROPERTY()
		class AWeaponInventoryBase* SubInventoryRef;

	//0번째 : 들고 있는 무기 / 1번째, 숨겨져 있는 다른 타임의무기
	UPROPERTY()
		TArray<class AWeaponBase*> WeaponActorList;

protected:
	UPROPERTY()
		TWeakObjectPtr<class AWeaponBase> CurrentWeaponRef;

	//0번째 : 근접 무기 / 1번째 : 원거리 무기
	//하나의 WeaponBase로 통일을 한다면 이렇게 하지 않아도 될텐데..

private:
	UPROPERTY()
		TArray<TSubclassOf<class AWeaponBase>> WeaponClassList; 

	//초기 무기 액터들을 스폰하고 초기화 한다.
	void InitWeaponActors();

	//무기 액터를 스폰
	AWeaponBase* SpawnWeaponActor(EWeaponType TargetWeaponType);

// ----- 스킬 관련 --------------------------
protected:
	UPROPERTY()
	struct FSkillSetInfo SkillSetInfo;

	UPROPERTY()
	ESkillGrade SkillGrade;

	UPROPERTY()
	class USkillManagerBase* SkillManagerRef;

// ---- Asset -------------------
public:
	// 외부에서 Init하기위해 Call
	UFUNCTION(BlueprintCallable)
		void InitAsset(int32 NewEnemyID);

protected:
	UFUNCTION()
		void SetAsset();

	UFUNCTION()
		bool InitAnimAsset();

	UFUNCTION()
		void InitSoundAsset();

	UFUNCTION()
		void InitVFXAsset();

	UFUNCTION()
		void InitMaterialAsset();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FCharacterAssetInfoStruct GetAssetInfoWithID(const int32 GetEnemyID);

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay")
		FCharacterAssetInfoStruct AssetInfo;

	//적 데이터 테이블 (에셋 정보 포함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		UDataTable* AssetDataInfoTable;

protected:
	//애니메이션, VFX, 사운드큐 등 저장
	UPROPERTY()
		struct FCharacterAnimAssetInfoStruct AnimAssetData;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		class UAnimMontage* PreChaseAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		class UAnimMontage* InvestigateAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		class USoundCue* RollSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		class USoundCue* BuffSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		class USoundCue* DebuffSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundCue* HitImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		TArray<class UNiagaraSystem*> DebuffNiagaraEffectList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Material")
		class UMaterialInterface* NormalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Material")
		class UMaterialInterface* WallThroughMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Material")
		TArray<class UTexture*> EmotionTextureList;

	UPROPERTY()
		class UMaterialInstanceDynamic* CurrentDynamicMaterial;

protected:
	UFUNCTION()
		void InitDynamicMeshMaterial(UMaterialInterface* NewMaterial);

// ------ 그 외 캐릭터 프로퍼티  ---------------
protected:
	//캐릭터의 스탯
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FCharacterStatStruct CharacterStat;

	//캐릭터의 현재 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
		FCharacterStateStruct CharacterState;

	//Gamemode 약 참조
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

// ----- 타이머 관련 ---------------------------------
protected:
	UFUNCTION()
		virtual void ClearAllTimerHandle();

	//공격 간 딜레이 핸들
	UPROPERTY()
		FTimerHandle AtkIntervalHandle;

	UPROPERTY()
		FTimerHandle ReloadTimerHandle;
};