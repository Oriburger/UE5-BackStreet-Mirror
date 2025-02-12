// Fill out your copyright notice in the Description page of Project Settings.

#include "AssetManagerBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "Kismet/KismetMathLibrary.h"

UAssetManagerBase::UAssetManagerBase() {}

void UAssetManagerBase::InitAssetManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;

	SystemSoundAssetTable = GamemodeRef.Get()->SystemSoundAssetTable;
	WeaponSoundAssetTable = GamemodeRef.Get()->WeaponSoundAssetTable;
	CharacterSoundAssetTable = GamemodeRef.Get()->CharacterSoundAssetTable;
	SkillSoundAssetTable = GamemodeRef.Get()->SkillSoundAssetTable;
	PropSoundAssetTable = GamemodeRef.Get()->PropSoundAssetTable;
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
	case ESoundAssetType::E_Prop:
		soundAssetInfo = GetPropSoundMapWithID(TargetID);
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
	FSoundAssetInfoStruct* soundAssetInfoStruct = GetSoundAssetInfo(SoundType, TargetID);
	if (soundAssetInfoStruct && soundAssetInfoStruct->SoundMap.Contains(SoundName))
	{
		soundList = soundAssetInfoStruct->SoundMap.Find(SoundName)->SoundList;
	}
	return soundList;
}

USoundCue* UAssetManagerBase::GetSingleSound(ESoundAssetType SoundType, int32 TargetID, FName SoundName, int32 IndexOverride)
{
	TArray<USoundCue*> soundList = GetSoundList(SoundType, TargetID, SoundName);
	if (!soundList.IsEmpty())
	{
		if (IndexOverride == -1)
		{
			IndexOverride = UKismetMathLibrary::RandomIntegerInRange(0, soundList.Num() - 1);
		}
		return soundList[IndexOverride];
	}
	UE_LOG(LogTemp, Warning, TEXT("UAssetManagerBase::GetSingleSound #4"));
	return nullptr;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetSystemSoundMapWithID(int32 TargetID)
{	
	if (!SystemSoundAssetTable) return nullptr; 

	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = SystemSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);
	checkf(soundAssetInfo != nullptr, TEXT("SoundAssetInfo is not valid"));
	
	return soundAssetInfo;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetWeaponSoundMapWithID(int32 TargetID)
{
	if (!WeaponSoundAssetTable) return nullptr;

	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = WeaponSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);
	checkf(soundAssetInfo != nullptr, TEXT("SoundAssetInfo is not valid"));

	return soundAssetInfo;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetCharacterSoundMapWithID(int32 TargetID)
{
	if (!CharacterSoundAssetTable) return nullptr;

	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = CharacterSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);
	checkf(soundAssetInfo != nullptr, TEXT("SoundAssetInfo is not valid"));

	return soundAssetInfo;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetSkillSoundMapWithID(int32 TargetID)
{
	if (!SkillSoundAssetTable) return nullptr;

	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = SkillSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);
	checkf(soundAssetInfo != nullptr, TEXT("SoundAssetInfo is not valid"));

	return soundAssetInfo;
}

FSoundAssetInfoStruct* UAssetManagerBase::GetPropSoundMapWithID(int32 TargetID)
{
	if (!PropSoundAssetTable) return nullptr;

	// Read from dataTable
	FString rowName = FString::FromInt(TargetID);
	FSoundAssetInfoStruct* soundAssetInfo = PropSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);
	checkf(soundAssetInfo != nullptr, TEXT("SoundAssetInfo is not valid"));

	return soundAssetInfo;
}

void UAssetManagerBase::PlaySingleSound(AActor* TargetActor, ESoundAssetType SoundType, int32 TargetID, FName SoundName, float VolumeMultiplierOverride, float PitchMultiplier, float StartTime)
{
	FSoundAssetInfoStruct* soundAssetInfo = GetSoundAssetInfo(SoundType, TargetID);
	if (soundAssetInfo == nullptr || !soundAssetInfo->SoundMap.Contains(SoundName)) return;

	TArray<USoundCue*> soundList = soundAssetInfo->SoundMap.Find(SoundName)->SoundList;
	TArray<float> volumeList = soundAssetInfo->SoundMap.Find(SoundName)->SoundVolumeList;

	if (soundList.Num() != volumeList.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("UAssetManagerBase::PlaySingleSound, soundList volumeList member count is not same"));
	}

	for (int32 idx = 0; idx < volumeList.Num(); idx++)
	{
		if (!soundList.IsValidIndex(idx) || !volumeList.IsValidIndex(idx)) return;

		const float targetVolume = VolumeMultiplierOverride != 1.0f ? VolumeMultiplierOverride : volumeList[idx];
		if (TargetActor == nullptr)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), soundList[idx], targetVolume, PitchMultiplier, StartTime);
		}
		else
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), soundList[idx], TargetActor->GetActorLocation(), targetVolume, PitchMultiplier, StartTime);
		}
	}
}

void UAssetManagerBase::PlayRandomSound(AActor* TargetActor, ESoundAssetType SoundType, int32 TargetID, FName SoundName, float VolumeMultiplierOverride, float PitchMultiplier, float StartTime)
{
	FSoundAssetInfoStruct* soundAssetInfo = GetSoundAssetInfo(SoundType, TargetID);
	checkf(soundAssetInfo != nullptr, TEXT("SoundAssetInfo is not valid"));
	if (!soundAssetInfo->SoundMap.Contains(SoundName)) return;

	TArray<USoundCue*> soundList = soundAssetInfo->SoundMap.Find(SoundName)->SoundList;
	TArray<float> volumeList = soundAssetInfo->SoundMap.Find(SoundName)->SoundVolumeList;
	if (!soundList.IsValidIndex(0) || !volumeList.IsValidIndex(0)) return;

	int8 randIdx;
	randIdx = UKismetMathLibrary::RandomIntegerInRange(0, soundList.Num()-1);

	const float targetVolume = VolumeMultiplierOverride != 0.0f ? VolumeMultiplierOverride : volumeList[randIdx];

	if (TargetActor == nullptr)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), soundList[randIdx], targetVolume, PitchMultiplier, StartTime);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), soundList[randIdx], TargetActor->GetActorLocation(), targetVolume, PitchMultiplier, StartTime);
	}
}

UAudioComponent* UAssetManagerBase::SpawnSound2D(ESoundAssetType SoundType, int32 TargetID, FName SoundName)
{
	FSoundAssetInfoStruct* soundAssetInfo = GetSoundAssetInfo(SoundType, TargetID);
	checkf(soundAssetInfo != nullptr, TEXT("SoundAssetInfo is not valid"));
	if (!soundAssetInfo->SoundMap.Contains(SoundName)) return nullptr;

	TArray<USoundCue*> soundList = soundAssetInfo->SoundMap.Find(SoundName)->SoundList;
	TArray<float> volumeList = soundAssetInfo->SoundMap.Find(SoundName)->SoundVolumeList;
	if (!soundList.IsValidIndex(0) || !volumeList.IsValidIndex(0)) return nullptr;

	return UGameplayStatics::SpawnSound2D(GetWorld(), soundList[0], volumeList[0]);
}








