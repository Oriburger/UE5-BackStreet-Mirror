#pragma once
#include "BackStreet.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "AssetManagerBase.generated.h"

UENUM(BlueprintType)
enum class ESoundAssetType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_System				UMETA(DisplayName = "System"),
	E_Weapon			UMETA(DisplayName = "Weapon"),
	E_Character			UMETA(DisplayName = "Character"),
	E_Skill					UMETA(DisplayName = "Skill"),
};


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

	//스킬 애니메이션 / 무기 ID로 매핑하여 관리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<int32, FSkillAnimAssetMontageStruct> SkillAnimMontageMap;

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

USTRUCT(BlueprintType)
struct FSoundArrayContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		TArray<USoundCue*> SoundList;
};

USTRUCT(BlueprintType)
struct FSoundAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	//The ID of the target to use the sound asset.
	//In SystemSound
	//* _ _ _ : SoundType (None : 0, BGM : 1, UI : 2, etc : 3)
	//_ * * * : Identification Number

	//In WeaponSound
	//* * * * : WeaponID

	//In CharacterSound
	//* * * * : CharacterID

	//In SkillSound
	//* * * * : SkillID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		ESoundAssetType SoundType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 TargetID;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName SoundTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<FName, FSoundArrayContainer> SoundMap;

};

UCLASS()
class BACKSTREET_API UAssetManagerBase : public UObject
{
	GENERATED_BODY()

public:
	UAssetManagerBase();

	UFUNCTION()
		void InitAssetManager(ABackStreetGameModeBase* NewGamemodeRef);

//----- Sound --------
public:
	UFUNCTION(BlueprintCallable)
		TArray<USoundCue*> GetSoundList(ESoundAssetType NewType, int32 NewID, FName NewName);

private:
	UFUNCTION()
		TMap<FName, FSoundArrayContainer> GetSystemSoundMapWithID(int32 NewID);

	UFUNCTION()
		TMap<FName, FSoundArrayContainer> GetWeaponSoundMapWithID(int32 NewID);

	UFUNCTION()
		TMap<FName, FSoundArrayContainer> GetCharacterSoundMapWithID(int32 NewID);

	UFUNCTION()
		TMap<FName, FSoundArrayContainer> GetSkillSoundMapWithID(int32 NewID);

private:
	UPROPERTY()
		UDataTable* SystemSoundAssetTable;

	UPROPERTY()
		UDataTable* WeaponSoundAssetTable;

	UPROPERTY()
		UDataTable* SkillSoundAssetTable;

	UPROPERTY()
		UDataTable* CharacterSoundAssetTable;

	//-------- ETC. (Ref)-------------------------------
private:
	UPROPERTY()
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};

