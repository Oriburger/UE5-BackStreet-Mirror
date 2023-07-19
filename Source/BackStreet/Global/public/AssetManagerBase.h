#pragma once
#include "BackStreet.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "AssetManagerBase.generated.h"
#define MAX_STAGE_CNT 5

//캐릭터의 애니메이션 에셋 경로를 저장하는 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FCharacterAnimAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//캐릭터의 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 CharacterID;

	//근접 공격 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> MeleeAttackAnimMontageList;

	//사격 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> ShootAnimMontageList;

	//투척 공격 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> ThrowAnimMontageList;

	//재장전 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> ReloadAnimMontageList;

	//타격 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> HitAnimMontageList;

	//구르기 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> RollAnimMontageList;

	//상호작용 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> InvestigateAnimMontageList;

	//사망 애니메이션 / List로 관리 -> 랜덤하게 출력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> DieAnimMontageList;
};

//캐릭터의 VFX 에셋 경로를 저장하는 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FCharacterVFXAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//캐릭터의 디버프 나이아가라 리스트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<class UNiagaraSystem*> DebuffNiagaraEffectLis;
};

//캐릭터의 에셋 경로를 저장하는 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FCharacterAssetInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	//캐릭터 애니메이션 에셋 경로 정보
	UPROPERTY()
		TMap<int32, FCharacterAnimAssetInfoStruct> AnimAssetInfoMap;

	//캐릭터 VFX 에셋 경로 정보
	UPROPERTY()
		TMap<int32, FCharacterVFXAssetInfoStruct> VFXAssetInfoMap;
};

UCLASS()
class BACKSTREET_API AAssetManagerBase : public AActor
{
	GENERATED_BODY()

public:
	AAssetManagerBase();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
		TSubclassOf<AEnemyCharacterBase> GetEnemyWithID(int32 EnemyID);

	UFUNCTION()
		ULevelSequence* GetTileTravelEffectSequence() { return TileTravelEffectSequence; }

	UFUNCTION()
		ULevelSequence* GetFadeOutEffectSequence() { return FadeOutEffectSequence; }

	UFUNCTION()
		ULevelSequence* GetFadeInEffectSequence() { return FadeInEffectSequence; }

	UFUNCTION()
		FName GetRandomMap();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AEnemyCharacterBase>> EnemyAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AEnemyCharacterBase>> MissionEnemyAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AItemBoxBase>> ItemBoxAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AWeaponBase>> WeaponAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class ARewardBoxBase>> RewardBoxAssets;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AProjectileBase>> ProjectileAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class AItemBoxBase>> MissionAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|VFX")
		class ULevelSequence* TileTravelEffectSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|VFX")
		class ULevelSequence* FadeOutEffectSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|VFX")
		class ULevelSequence* FadeInEffectSequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FName> MapNames;

private:
	UPROPERTY()
		class ABackStreetGameModeBase* GameModeRef;

};

