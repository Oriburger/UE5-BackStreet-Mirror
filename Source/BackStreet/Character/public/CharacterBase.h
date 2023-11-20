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

// ------- Character Action �⺻ ------- 
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		int32 CharacterID;

	//Input�� Binding �Ǿ� ������ �õ� (AnimMontage�� ȣ��)
	virtual void TryAttack();

	///Input�� Binding �Ǿ� ��ų������ �õ� (AnimMontage�� ȣ��)
	virtual void TrySkillAttack(ACharacterBase* Target);

	//AnimNotify�� Binding �Ǿ� ���� ������ ����
	virtual void Attack();

	virtual void StopAttack();

	virtual void TryReload();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
		, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Die();

	//�÷��̾ ���� �ش� Action�� �����ϰ� �ִ��� ��ȯ
	UFUNCTION(BlueprintCallable)
		bool GetIsActionActive(ECharacterActionType Type) { return CharacterState.CharacterActionState == Type; }

	//�÷��̾��� ActionState�� Idle�� ��ȯ�Ѵ�.
	UFUNCTION(BlueprintCallable)
		void ResetActionState(bool bForceReset = false);

	//�÷��̾ ü���� ȸ���� (��ȸ��)
	UFUNCTION()
		void TakeHeal(float HealAmountRate, bool bIsTimerEvent = false, uint8 BuffDebuffType = 0);

	//����� �������� ���� (��ȸ��)
	UFUNCTION(BlueprintCallable)
		float TakeDebuffDamage(float DamageAmount, ECharacterDebuffType DebuffType, AActor* Causer);

	UFUNCTION(BlueprintCallable)
		void TakeKnockBack(AActor* Causer, float Strength);

	//����Action ������ Interval�� �����ϴ� Ÿ�̸Ӹ� ����
	UFUNCTION()
		void ResetAtkIntervalTimer();

// ------- Character Stat/State ------------------------------
public:
	//ĳ������ ���� ������ �ʱ�ȭ
	UFUNCTION()
		void InitCharacterState();

	//ĳ������ ����� ������ ������Ʈ
	UFUNCTION(BlueprintCallable)
		virtual	bool TryAddNewDebuff(ECharacterDebuffType NewDebuffType, AActor* Causer = nullptr, float TotalTime = 0.0f, float Value = 0.0f);

	//������� Ȱ��ȭ �Ǿ��ִ��� ��ȯ
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetDebuffIsActive(ECharacterDebuffType DebuffType);

	//ĳ������ ������ ������Ʈ
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

// ------ ���� ���� -------------------------------------------
public:
	UFUNCTION()
		bool EquipWeapon(class AWeaponBase* TargetWeapon);

	//���⸦ ���´�. �κ��丮�� �� á�ٸ� false�� ��ȯ
	virtual bool PickWeapon(int32 NewWeaponID);

	//���� ����� ��ȯ�Ѵ�. ��ȯ�� �����ϸ� false�� ��ȯ
	virtual void SwitchToNextWeapon();

	//���⸦ Drop�Ѵ�. (���忡�� �ƿ� �������.)
	virtual void DropWeapon();

	UFUNCTION()
		bool TrySwitchToSubWeapon(int32 SubWeaponIdx);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AWeaponInventoryBase* GetInventoryRef();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AWeaponInventoryBase* GetSubInventoryRef();

	//���� Ref�� ��ȯ
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

	//0��° : ��� �ִ� ���� / 1��°, ������ �ִ� �ٸ� Ÿ���ǹ���
	UPROPERTY()
		TArray<class AWeaponBase*> WeaponActorList;

protected:
	UPROPERTY()
		TWeakObjectPtr<class AWeaponBase> CurrentWeaponRef;

	//0��° : ���� ���� / 1��° : ���Ÿ� ����
	//�ϳ��� WeaponBase�� ������ �Ѵٸ� �̷��� ���� �ʾƵ� ���ٵ�..

private:
	UPROPERTY()
		TArray<TSubclassOf<class AWeaponBase>> WeaponClassList; 

	//�ʱ� ���� ���͵��� �����ϰ� �ʱ�ȭ �Ѵ�.
	void InitWeaponActors();

	//���� ���͸� ����
	AWeaponBase* SpawnWeaponActor(EWeaponType TargetWeaponType);

// ----- ��ų ���� --------------------------
protected:
	UPROPERTY()
	struct FSkillSetInfo SkillSetInfo;

	UPROPERTY()
	ESkillGrade SkillGrade;

	UPROPERTY()
	class USkillManagerBase* SkillManagerRef;

// ---- Asset -------------------
public:
	// �ܺο��� Init�ϱ����� Call
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

	//�� ������ ���̺� (���� ���� ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		UDataTable* AssetDataInfoTable;

protected:
	//�ִϸ��̼�, VFX, ����ť �� ����
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

// ------ �� �� ĳ���� ������Ƽ  ---------------
protected:
	//ĳ������ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FCharacterStatStruct CharacterStat;

	//ĳ������ ���� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
		FCharacterStateStruct CharacterState;

	//Gamemode �� ����
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

// ----- Ÿ�̸� ���� ---------------------------------
protected:
	UFUNCTION()
		virtual void ClearAllTimerHandle();

	//���� �� ������ �ڵ�
	UPROPERTY()
		FTimerHandle AtkIntervalHandle;

	UPROPERTY()
		FTimerHandle ReloadTimerHandle;
};