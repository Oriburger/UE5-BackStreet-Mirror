// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/AssetManagerBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "../public/BackStreetGameModeBase.h"

UAssetManagerBase::UAssetManagerBase()
{
	//------ Sound Datatable Initiation ---------
	static ConstructorHelpers::FObjectFinder<UDataTable> systemSoundAssetTableFinder(TEXT("/Game/Asset/Data/Sound/D_SystemSoundAsset.D_SystemSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> weaponSoundAssetTableFinder(TEXT("/Game/Asset/Data/Sound/D_WeaponSoundAsset.D_WeaponSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> characterSoundAssetTableFinder(TEXT("/Game/Asset/Data/Sound/D_CharacterSoundAsset.D_CharacterSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> skillSoundAssetTableFinder(TEXT("/Game/Asset/Data/Sound/D_SkillSoundAsset.D_SkillSoundAsset"));

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

FSoundAssetInfoStruct* UAssetManagerBase::GetSoundAssetInfo(ESoundAssetType SoundType, int32 TargetID)
{
	FSoundAssetInfoStruct* soundAssetInfo;

	switch (SoundType)
	{
	case ESoundAssetType::E_None:
		soundAssetInfo = nullptr;
		break;
	case ESoundAssetType::E_System:
		soundAssetInfo = GetSystemSoundMapWithID(TargetID);
		break;
	case ESoundAssetType::E_Weapon:
		soundAssetInfo = GetWeaponSoundMapWithID(TargetID);
		break;
	case ESoundAssetType::E_Character:
		soundAssetInfo = GetCharacterSoundMapWithID(TargetID);
		break;
	case ESoundAssetType::E_Skill:
		soundAssetInfo = GetSkillSoundMapWithID(TargetID);
		break;
	default:
		soundAssetInfo = nullptr;
		break;
	}
	return soundAssetInfo;
}

TArray<USoundCue*> UAssetManagerBase::GetSoundList(ESoundAssetType SoundType, int32 TargetID, FName SoundName)
{
	// Read from dataTable
	TArray<USoundCue*> soundList;
	soundList = GetSoundAssetInfo(SoundType, TargetID)->SoundMap.Find(SoundName)->SoundList;

	return soundList;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetSystemSoundMapWithID(int32 TargetID)
{	
	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = SystemSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);
	
	return soundAssetInfo;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetWeaponSoundMapWithID(int32 TargetID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = WeaponSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	return soundAssetInfo;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetCharacterSoundMapWithID(int32 TargetID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = CharacterSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	return soundAssetInfo;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetSkillSoundMapWithID(int32 TargetID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = SkillSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);
	
	return soundAssetInfo;
}

void UAssetManagerBase::PlaySingleSound(AActor* TargetActor, ESoundAssetType SoundType, int32 TargetID, FName SoundName)
{
	FSoundAssetInfoStruct* soundAssetInfo = GetSoundAssetInfo(SoundType, TargetID);
	if (soundAssetInfo == nullptr) return;
	if (!soundAssetInfo->SoundMap.Contains(SoundName)) return; 
	
	TArray<USoundCue*> soundList = soundAssetInfo->SoundMap.Find(SoundName)->SoundList;
	TArray<float> volumeList = soundAssetInfo->SoundMap.Find(SoundName)->SoundVolumeList;
	if (!soundList.IsValidIndex(0) || !volumeList.IsValidIndex(0)) return;
	
	if (TargetActor == nullptr)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), soundList[0], volumeList[0]);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), soundList[0], TargetActor->GetActorLocation(), volumeList[0]);
	}
}

void UAssetManagerBase::PlayRandomSound(AActor* TargetActor, ESoundAssetType SoundType, int32 TargetID, FName SoundName)
{
	FSoundAssetInfoStruct* soundAssetInfo = GetSoundAssetInfo(SoundType, TargetID);
	if (!soundAssetInfo->SoundMap.Contains(SoundName)) return;

	TArray<USoundCue*> soundList = soundAssetInfo->SoundMap.Find(SoundName)->SoundList;
	TArray<float> volumeList = soundAssetInfo->SoundMap.Find(SoundName)->SoundVolumeList;
	if (!soundList.IsValidIndex(0) || !volumeList.IsValidIndex(0)) return;

	int8 randIdx;
	randIdx = UKismetMathLibrary::RandomIntegerInRange(0, soundList.Num()-1);

	if (TargetActor == nullptr)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), soundList[randIdx], volumeList[randIdx]);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), soundList[randIdx], TargetActor->GetActorLocation(), volumeList[randIdx]);
	}
}

UAudioComponent* UAssetManagerBase::SpawnSound2D(ESoundAssetType SoundType, int32 TargetID, FName SoundName)
{
	FSoundAssetInfoStruct* soundAssetInfo = GetSoundAssetInfo(SoundType, TargetID);
	if (!soundAssetInfo->SoundMap.Contains(SoundName)) return nullptr;

	TArray<USoundCue*> soundList = soundAssetInfo->SoundMap.Find(SoundName)->SoundList;
	TArray<float> volumeList = soundAssetInfo->SoundMap.Find(SoundName)->SoundVolumeList;
	if (!soundList.IsValidIndex(0) || !volumeList.IsValidIndex(0)) return nullptr;

	return UGameplayStatics::SpawnSound2D(GetWorld(), soundList[0], volumeList[0]);
}








