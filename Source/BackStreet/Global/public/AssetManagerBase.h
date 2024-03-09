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


//ĳ������ �ִϸ��̼� ���� ��θ� �����ϴ� ������ ���̺� ����ü
USTRUCT(BlueprintType)
struct FCharacterAnimAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	//ĳ������ ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 CharacterID;

	//���� ���� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> MeleeAttackAnimMontageList;

	//��� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> ShootAnimMontageList;

	//��ô ���� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> ThrowAnimMontageList;

	//������ �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> ReloadAnimMontageList;

	//Ÿ�� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> HitAnimMontageList;

	//��ų �ִϸ��̼� / ���� ID�� �����Ͽ� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<int32, FSkillAnimAssetMontageStruct> SkillAnimMontageMap;

	//������ �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> RollAnimMontageList;

	//��ȣ�ۿ� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<UAnimMontage*> InvestigateAnimMontageList;

	//��� �ִϸ��̼� / List�� ���� -> �����ϰ� ���
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data", meta = (UIMin = 0.0f, UIMax = 10.0f))
		TArray<float> SoundVolumeList;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName SoundAssetDescription;
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
		TArray<USoundCue*> GetSoundList(ESoundAssetType SoundType, int32 TargetID, FName SoundName);

	UFUNCTION(BlueprintCallable)
		TMap<FName, FSoundArrayContainer>GetSoundAssetInfo(ESoundAssetType SoundType, int32 TargetID);
	
	//Play Single Sound
	UFUNCTION(BlueprintCallable)
		void PlaySingleSound(AActor* TargetActor, TMap<FName, FSoundArrayContainer> SoundAssetMap, FName SoundName);

	//Choose Sound and Play Randomly
	UFUNCTION(BlueprintCallable)
		void PlayRandomSound(AActor* TargetActor, TMap<FName, FSoundArrayContainer> SoundAssetMap, FName SoundName);

private:
	UFUNCTION()
		TMap<FName, FSoundArrayContainer> GetSystemSoundMapWithID(int32 TargetID);

	UFUNCTION()
		TMap<FName, FSoundArrayContainer> GetWeaponSoundMapWithID(int32 TargetID);

	UFUNCTION()
		TMap<FName, FSoundArrayContainer> GetCharacterSoundMapWithID(int32 TargetID);

	UFUNCTION()
		TMap<FName, FSoundArrayContainer> GetSkillSoundMapWithID(int32 TargetID);

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

