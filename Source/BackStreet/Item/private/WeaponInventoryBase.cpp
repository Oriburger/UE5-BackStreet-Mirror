// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/WeaponInventoryBase.h"
#include "../public/WeaponBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#define INVALID_INVENTORY_IDX -1

AWeaponInventoryBase::AWeaponInventoryBase()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
}

void AWeaponInventoryBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponInventoryBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponInventoryBase::InitInventory()
{
	if (!IsValid(GetOwner()) || !GetOwner()->ActorHasTag("Character"))
	{
		UE_LOG(LogTemp, Warning, TEXT("AInventory�� CharacterBase �̿��� Ŭ���������� ������ �� �����ϴ�."));
		return;
	}
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());

	InventoryArray.Empty();

	//�̸� Ŭ���� / Id ������ �ʱ�ȭ �� 
	for (int32 classIdx = 0; classIdx < WeaponIDList.Num(); classIdx++)
	{
		WeaponClassInfoMap.Add(WeaponIDList[classIdx], WeaponClassList[classIdx]);
	}
}

void AWeaponInventoryBase::EquipWeapon(int32 InventoryIdx, bool bIsNewWeapon)
{
	if (!InventoryArray.IsValidIndex(InventoryIdx)) return;

	AWeaponBase* newWeapon = SpawnWeaponActor(InventoryArray[InventoryIdx].WeaponID);

	if (IsValid(newWeapon))
	{
		SetCurrentWeaponRef(newWeapon);
		OwnerCharacterRef->EquipWeapon(GetCurrentWeaponRef());
		SetCurrentIdx(InventoryIdx);

		if (bIsNewWeapon) SyncCurrentWeaponInfo(false);
		else SyncCurrentWeaponInfo(true);
	}
}

bool AWeaponInventoryBase::AddWeapon(int32 NewWeaponID)
{
	UE_LOG(LogTemp, Warning, TEXT("ADD #0"));
	if (!IsValid(OwnerCharacterRef)) return false;

	UE_LOG(LogTemp, Warning, TEXT("ADD #1"));
	if (!WeaponClassInfoMap.Contains(NewWeaponID)) return false;

	UE_LOG(LogTemp, Warning, TEXT("ADD #2"));
	FWeaponStatStruct newWeaponStat = GamemodeRef->GetWeaponStatInfoWithID(NewWeaponID);
	FWeaponStateStruct newWeaponState = FWeaponStateStruct();
	newWeaponState.CurrentDurability = newWeaponStat.MaxDurability;

	//����, ���Ÿ� ������ �ߺ� ���θ� �Ǵ�. �ߺ��ȴٸ� Ammo�� �߰�
	int32 duplicateIdx = CheckWeaponDuplicate(NewWeaponID);
	if (newWeaponStat.bHasProjectile && duplicateIdx != -1)
	{
		InventoryArray[duplicateIdx].WeaponState.TotalAmmoCount += 25;
		UE_LOG(LogTemp, Warning, TEXT("ADD #3"));
		return false;
	}
	//�׷��� �ʴٸ� ���� �뷮 üũ�� �����ϰ� ���� �߰��� ����
	else if (newWeaponStat.WeaponWeight == 0 || newWeaponStat.WeaponWeight + CurrentCapacity <= MaxCapacity)
	{
		InventoryArray.Add({ NewWeaponID, newWeaponStat, newWeaponState });
		if (GetCurrentWeaponCount() == 1) //���� �κ��丮�� ����־��ٸ�? �ٷ� �������� ����
		{
			EquipWeapon(InventoryArray.Num() - 1, true);
		}
		CurrentCapacity += newWeaponStat.WeaponWeight;
		SortInventory();
		UE_LOG(LogTemp, Warning, TEXT("ADD #5"));

		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("ADD #6"));
	return false;
}

void AWeaponInventoryBase::RemoveWeapon(int32 InventoryIdx)
{
	if (!InventoryArray.IsValidIndex(InventoryIdx)) return;

	CurrentCapacity -= InventoryArray[GetCurrentIdx()].WeaponStat.WeaponWeight;
	CurrentWeaponCount -= 1;

	InventoryArray.RemoveAt(GetCurrentIdx());
	if (GetCurrentWeaponCount() >= 1)
	{
		SetCurrentIdx(0);
		EquipWeapon(0);
		SortInventory();
	}
}

bool AWeaponInventoryBase::SwitchToNextWeapon()
{
	const int32 nextIdx = GetNextInventoryIdx();
	if (GetCurrentWeaponCount() <= 1 || nextIdx == INVALID_INVENTORY_IDX) return false;
	if (!InventoryArray.IsValidIndex(nextIdx)) return false;
	if (!IsValid(OwnerCharacterRef)) return false;

	RestoreCurrentWeapon(); //���� ���⸦ Destroy�ϰ�, ������ List�� �����Ѵ�.
	EquipWeapon(nextIdx);
	SetCurrentIdx(nextIdx);
	SortInventory();

	return false;
}

void AWeaponInventoryBase::SyncCurrentWeaponInfo(bool bIsLoadInfo)
{
	if (!IsValid(OwnerCharacterRef)) return;
	if (GetCurrentWeaponCount() == 0) return;
	if (!InventoryArray.IsValidIndex(GetCurrentIdx()))
	{
		UE_LOG(LogTemp, Warning, TEXT("SyncCurrentWeaponInfo : Invalid Index"));
		return;
	}

	if (!IsValid(GetCurrentWeaponRef()))
	{
		UE_LOG(LogTemp, Warning, TEXT("SyncCurrentWeaponInfo : Weapon Not Found"));
		return;
	}
	if (bIsLoadInfo)
	{
		GetCurrentWeaponRef()->SetWeaponStat( InventoryArray[GetCurrentIdx()].WeaponStat );
		GetCurrentWeaponRef()->SetWeaponState( InventoryArray[GetCurrentIdx()].WeaponState );
	}
	else
	{
		InventoryArray[GetCurrentIdx()].WeaponStat = GetCurrentWeaponRef()->GetWeaponStat();
		InventoryArray[GetCurrentIdx()].WeaponState = GetCurrentWeaponRef()->GetWeaponState();
	}
}

AWeaponBase* AWeaponInventoryBase::SpawnWeaponActor(int32 WeaponID)
{
	if (!WeaponClassInfoMap.Contains(WeaponID)) return nullptr;
	
	UClass* targetClass = *(WeaponClassInfoMap.Find(WeaponID));
	if (!targetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon Class�� Invalid �մϴ�."));
		return nullptr;
	}
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = OwnerCharacterRef;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector spawnLocation = OwnerCharacterRef->GetActorLocation();
	FRotator spawnRotation = FRotator();
	AWeaponBase* newWeapon = Cast<AWeaponBase>(GetWorld()->SpawnActor(targetClass, &spawnLocation, &spawnRotation, spawnParams));

	return newWeapon;
}

void AWeaponInventoryBase::RestoreCurrentWeapon()
{
	if (GetCurrentWeaponCount() <= 1) return;
	if (!IsValid(OwnerCharacterRef)) return;

	SyncCurrentWeaponInfo(false);
	GetCurrentWeaponRef()->Destroy();
}

void AWeaponInventoryBase::ClearInventory()
{
	RemoveCurrentWeapon();
	InventoryArray.Empty();
}

void AWeaponInventoryBase::SortInventory()
{
	if (InventoryArray.Num() <= 1) return;

	//Inventory�� �ִ� Len�� 10�̱� ������
	//O(n^2)�̾ ��� �ð��� ������ ����� ��
	for (int32 selIdx = 0; selIdx < GetCurrentWeaponCount()-1; selIdx++)
	{
		FInventoryItemInfoStruct tempInfo = FInventoryItemInfoStruct();
		for (int32 compIdx = selIdx + 1; compIdx < GetCurrentWeaponCount(); compIdx++)
		{
			const int32 selWeight = InventoryArray[selIdx].WeaponStat.WeaponWeight;
			const int32 compWeight = InventoryArray[compIdx].WeaponStat.WeaponWeight;
			if (selWeight > compWeight)
			{
				tempInfo = InventoryArray[selIdx];
				InventoryArray[selIdx] = InventoryArray[compIdx];
				InventoryArray[compIdx] = tempInfo;

				if (selIdx == GetCurrentIdx())
				{
					SetCurrentIdx(compIdx);
				}
			}
		}
	}
}

void AWeaponInventoryBase::RemoveCurrentWeapon()
{
	if (!IsValid(GetCurrentWeaponRef())) return;
	CurrentWeaponRef->Destroy();
	RemoveWeapon(GetCurrentIdx());
}

int32 AWeaponInventoryBase::CheckWeaponDuplicate(int32 TargetWeaponID)
{
	for (int32 inventoryIdx = 0; inventoryIdx < InventoryArray.Num(); inventoryIdx++)
	{
		auto& weaponInfo = InventoryArray[inventoryIdx];
		if (weaponInfo.WeaponID == TargetWeaponID)
		{
			return inventoryIdx;
		}
	}
	return -1;
}

AWeaponBase* AWeaponInventoryBase::GetCurrentWeaponRef()
{
	if (!IsValid(CurrentWeaponRef)) return nullptr;
	return CurrentWeaponRef;
}

FInventoryItemInfoStruct AWeaponInventoryBase::GetCurrentWeaponInfo()
{
	if(GetCurrentWeaponCount() == 0) return FInventoryItemInfoStruct();
	return InventoryArray[GetCurrentIdx()];
}

int32 AWeaponInventoryBase::GetNextInventoryIdx()
{
	if (GetCurrentWeaponCount() == 0) return 0;
	return (GetCurrentIdx() + 1) % InventoryArray.Num();
}

void AWeaponInventoryBase::SetCurrentWeaponRef(AWeaponBase* NewWeapon)
{
	if (!IsValid(NewWeapon)) return;
	CurrentWeaponRef = NewWeapon;
	CurrentWeaponRef->WeaponDestroyDelegate.BindUFunction(this, FName("RemoveCurrentWeapon"));
	CurrentWeaponRef->SetOwnerCharacter(OwnerCharacterRef);
}