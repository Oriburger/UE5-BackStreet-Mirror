// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/AssetManagerBase.h"
#include "../public/BackStreetGameModeBase.h"

UAssetManagerBase::UAssetManagerBase()
{
	//------ Sound Datatable Initiation ---------
	static ConstructorHelpers::FObjectFinder<UDataTable> systemSoundAssetTableFinder(TEXT("/Game/Asset/Data/D_SystemSoundAsset.D_SystemSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> weaponSoundAssetTableFinder(TEXT("/Game/Asset/Data/D_WeaponSoundAsset.D_WeaponSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> characterSoundAssetTableFinder(TEXT("/Game/Asset/Data/D_CharacterSoundAsset.D_CharacterSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> skillSoundAssetTableFinder(TEXT("/Game/Asset/Data/D_SkillSoundAsset.D_SkillSoundAsset"));

	checkf(systemSoundAssetTableFinder.Succeeded(), TEXT("SystemSoundAssetTable class discovery failed."));
	checkf(weaponSoundAssetTableFinder.Succeeded(), TEXT("WeaponSoundAssetTable class discovery failed."));
	checkf(characterSoundAssetTableFinder.Succeeded(), TEXT("CharacterSoundAssetTable class discovery failed."));
	checkf(skillSoundAssetTableFinder.Succeeded(), TEXT("SkillSoundAssetTable class discovery failed."));

	SystemSoundAssetTable = systemSoundAssetTableFinder.Object;
	WeaponSoundAssetTable = weaponSoundAssetTableFinder.Object;
	CharacterSoundAssetTable = characterSoundAssetTableFinder.Object;
	SkillSoundAssetTable = skillSoundAssetTableFinder.Object;
}

void UAssetManagerBase::InitAssetManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
}
TMap<FName, FSoundArrayContainer>UAssetManagerBase::GetSoundAssetInfo(ESoundAssetType SoundType, int32 TargetID)
{
	TMap<FName, FSoundArrayContainer> soundMap;

	switch (SoundType)
	{
	case ESoundAssetType::E_None:
		break;
	case ESoundAssetType::E_System:
		soundMap = GetSystemSoundMapWithID(TargetID);
		break;
	case ESoundAssetType::E_Weapon:
		soundMap = GetWeaponSoundMapWithID(TargetID);
		break;
	case ESoundAssetType::E_Character:
		soundMap = GetCharacterSoundMapWithID(TargetID);
		break;
	case ESoundAssetType::E_Skill:
		soundMap = GetSkillSoundMapWithID(TargetID);
		break;
	}
	return soundMap;
}

TArray<USoundCue*> UAssetManagerBase::GetSoundList(ESoundAssetType SoundType, int32 TargetID, FName SoundName)
{
	TArray<USoundCue*> soundList;
	soundList = GetSoundAssetInfo(SoundType, TargetID).Find(SoundName)->SoundList;
	return soundList;
}

TMap<FName, FSoundArrayContainer> UAssetManagerBase::GetSystemSoundMapWithID(int32 TargetID)
{	
	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAsset = SystemSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	TMap<FName, FSoundArrayContainer> soundMap = soundAsset->SoundMap;
	
	return soundMap;	
}

TMap<FName, FSoundArrayContainer> UAssetManagerBase::GetWeaponSoundMapWithID(int32 TargetID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAsset = WeaponSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	TMap<FName, FSoundArrayContainer> soundMap = soundAsset->SoundMap;

	return soundMap;
}

TMap<FName, FSoundArrayContainer> UAssetManagerBase::GetCharacterSoundMapWithID(int32 TargetID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAsset = CharacterSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	TMap<FName, FSoundArrayContainer> soundMap = soundAsset->SoundMap;

	return soundMap;
}

TMap<FName, FSoundArrayContainer> UAssetManagerBase::GetSkillSoundMapWithID(int32 TargetID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAsset = SkillSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	TMap<FName, FSoundArrayContainer> soundMap = soundAsset->SoundMap;

	return soundMap;
}








