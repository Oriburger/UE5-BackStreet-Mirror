#pragma once

#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "AssetInfoStruct.generated.h"

UENUM(BlueprintType)
enum class ESoundAssetType : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_System			UMETA(DisplayName = "System"),
	E_Weapon			UMETA(DisplayName = "Weapon"),
	E_Character			UMETA(DisplayName = "Character"),
	E_Skill				UMETA(DisplayName = "Skill"),
};


USTRUCT(BlueprintType)
struct FAnimAssetSoftInfo : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()


};

USTRUCT(BlueprintType)
struct FSoundArrayContainer : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		TArray<USoundCue*> SoundList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data", meta = (UIMin = 0.001f, UIMax = 10.0f))
		TArray<float> SoundVolumeList;
};

USTRUCT(BlueprintType)
struct FSoundAssetInfoStruct : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	/******************************************************	
	The ID of the target to use the sound asset.
	In SystemSound
	* _ _ _ : SoundType (None : 0, BGM : 1, UI : 2, etc : 3)
	_ * * * : Identification Number

	In WeaponSound
	* * * * : WeaponID

	In CharacterSound
	* * * * : CharacterID

	In SkillSound
	* * * * : SkillID
	*******************************************************/

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

USTRUCT(BlueprintType)
struct FCharacterAssetSoftInfo : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()

//--------------- Common ------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 CharacterID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		FName CharacterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		TSoftObjectPtr<USkeletalMesh> CharacterMeshSoftPtr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FRotator InitialRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
		FVector InitialCapsuleComponentScale;

	//CharacterSkillList
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		TMap<int32, FOwnerSkillInfoStruct> CharacterSkillInfoMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UAnimBlueprintGeneratedClass* AnimBlueprint;

//--------------- Animation  ------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> MeleeAttackAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TSoftObjectPtr<UAnimMontage> UpperAttackAnimMontageSoftPtr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TSoftObjectPtr<UAnimMontage> DownwardAttackAnimMontageSoftPtr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TSoftObjectPtr<UAnimMontage> DashAttackAnimMontageSoftPtr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> AirAttackAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ShootAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ThrowAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> ReloadAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> HitAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> KnockdownAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> RollAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> InvestigateAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> DieAnimMontageSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TArray<TSoftObjectPtr<UAnimMontage>> PointMontageSoftPtrList;

//--------------- etc ------------------------------------
	// VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TArray<TSoftObjectPtr<UNiagaraSystem>> DebuffNiagaraEffectSoftPtrList;

	// Material
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		TArray<TSoftObjectPtr<UMaterialInterface>> DynamicMaterialSoftPtrList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		TArray<TSoftObjectPtr<UTexture>> EmotionTextureSoftPtrList;
};

USTRUCT(BlueprintType)
struct FAnimAssetHardPtrInfo
{
public:
	GENERATED_USTRUCT_BODY()

};

USTRUCT(BlueprintType)
struct FCharacterAssetHardInfo
{
public:
	GENERATED_USTRUCT_BODY()

//------- Common --------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		USkeletalMesh* CharacterMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
		TArray<UNiagaraSystem*> DebuffNiagaraEffectList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		TArray<UMaterialInterface*> DynamicMaterialList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		TArray<UTexture*> EmotionTextureList;

//------- Animation --------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> MeleeAttackAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		UAnimMontage* UpperAttackAnimMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		UAnimMontage* DownwardAttackAnimMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		UAnimMontage* DashAttackAnimMontage;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> AirAttackAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> ShootAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> ThrowAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> ReloadAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> HitAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> KnockdownAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> RollAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> InvestigateAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> DieAnimMontageList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
		TArray<UAnimMontage*> PointMontageList;
};