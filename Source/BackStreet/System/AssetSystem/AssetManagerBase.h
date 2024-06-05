#pragma once

#include "../../Global/BackStreet.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "AssetManagerBase.generated.h"

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
	//GetSoundList From SoundAssetTable
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<USoundCue*> GetSoundList(ESoundAssetType SoundType, int32 TargetID, FName SoundName);
	
	//Play Single Sound
	UFUNCTION(BlueprintCallable)
		void PlaySingleSound(AActor* TargetActor, ESoundAssetType SoundType, int32 TargetID, FName SoundName, float VolumeMultiplierOverride = 1.0f, float PitchMultiplier = 1.0f, float StartTime = 0.0f);

	//Play Random Sound
	UFUNCTION(BlueprintCallable)
		void PlayRandomSound(AActor* TargetActor, ESoundAssetType SoundType, int32 TargetID, FName SoundName, float VolumeMultiplierOverride = 0.0f, float PitchMultiplier = 1.0f, float StartTime = 0.0f);

	//Which is for Spawning and Stopping the sound
	UFUNCTION(BlueprintCallable)
		UAudioComponent* SpawnSound2D(ESoundAssetType SoundType, int32 TargetID, FName SoundName);

private:
	FSoundAssetInfoStruct* GetSoundAssetInfo(ESoundAssetType SoundType, int32 TargetID);

	FSoundAssetInfoStruct* GetSystemSoundMapWithID(int32 TargetID);

	FSoundAssetInfoStruct* GetWeaponSoundMapWithID(int32 TargetID);

	FSoundAssetInfoStruct* GetCharacterSoundMapWithID(int32 TargetID);

	FSoundAssetInfoStruct* GetSkillSoundMapWithID(int32 TargetID);

	FSoundAssetInfoStruct* GetPropSoundMapWithID(int32 TargetID);

private:
	UPROPERTY()
		UDataTable* SystemSoundAssetTable;

	UPROPERTY()
		UDataTable* WeaponSoundAssetTable;

	UPROPERTY()
		UDataTable* SkillSoundAssetTable;

	UPROPERTY()
		UDataTable* CharacterSoundAssetTable;

	UPROPERTY()
		UDataTable* PropSoundAssetTable;

	//-------- ETC. (Ref)-------------------------------
private:
	UPROPERTY()
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};

