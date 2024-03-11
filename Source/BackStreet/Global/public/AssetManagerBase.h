#pragma once

#include "BackStreet.h"
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
	UFUNCTION(BlueprintCallable)
		TArray<USoundCue*> GetSoundList(ESoundAssetType SoundType, int32 TargetID, FName SoundName);
	
	//Play Single Sound
	UFUNCTION(BlueprintCallable)
		void PlaySingleSound(AActor* TargetActor, ESoundAssetType SoundType, int32 TargetID, FName SoundName);

	//Choose Sound and Play Randomly
	UFUNCTION(BlueprintCallable)
		void PlayRandomSound(AActor* TargetActor, ESoundAssetType SoundType, int32 TargetID, FName SoundName);

private:
	FSoundAssetInfoStruct* GetSoundAssetInfo(ESoundAssetType SoundType, int32 TargetID);

	FSoundAssetInfoStruct* GetSystemSoundMapWithID(int32 TargetID);

	FSoundAssetInfoStruct* GetWeaponSoundMapWithID(int32 TargetID);

	FSoundAssetInfoStruct* GetCharacterSoundMapWithID(int32 TargetID);

	FSoundAssetInfoStruct* GetSkillSoundMapWithID(int32 TargetID);

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

