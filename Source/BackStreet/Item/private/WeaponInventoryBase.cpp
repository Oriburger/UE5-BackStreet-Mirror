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
		UE_LOG(LogTemp, Warning, TEXT("AInventory는 CharacterBase 이외의 클래스에서는 소유할 수 없습니다."));
		return;
	}
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());

	InventoryArray.Empty();

	//미리 클래스 / Id 정보를 초기화 함 
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
		OwnerCharacterRef.Get()->EquipWeapon(GetCurrentWeaponRef());
		SetCurrentIdx(InventoryIdx);
		SyncCurrentWeaponInfo(true);
	}
}

bool AWeaponInventoryBase::AddWeapon(int32 NewWeaponID)
{
	if (!OwnerCharacterRef.IsValid() || !GamemodeRef.IsValid()) return false;
	if (!WeaponClassInfoMap.Contains(NewWeaponID))	return false;

	FWeaponStatStruct newWeaponStat = GamemodeRef.Get()->GetWeaponStatInfoWithID(NewWeaponID);
	FWeaponStateStruct newWeaponState = FWeaponStateStruct();
	newWeaponState.CurrentDurability = newWeaponStat.MaxDurability;
	newWeaponState.RangedWeaponState.CurrentAmmoCount = newWeaponStat.RangedWeaponStat.MaxAmmoPerMagazine;

	//먼저, 원거리 무기의 중복 여부를 판단. 중복된다면 Ammo를 추가
	int32 duplicateIdx = CheckWeaponDuplicate(NewWeaponID);
	if (duplicateIdx != -1)
	{
		if (newWeaponStat.RangedWeaponStat.bHasProjectile && !newWeaponStat.RangedWeaponStat.bIsInfiniteAmmo)
		{
			InventoryArray[duplicateIdx].WeaponState.RangedWeaponState.TotalAmmoCount +=
				InventoryArray[duplicateIdx].WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine;
			if(duplicateIdx == CurrentIdx) SyncCurrentWeaponInfo(true);
			return true;
		}
		else
		{
			GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("동일한 무기가 인벤토리에 있습니다.")), FColor::White);
		}
		return false;
	}
	//그렇지 않다면 무기 용량 체크를 진행하고 무기 추가를 진행
	else if (newWeaponStat.WeaponWeight == 0 || newWeaponStat.WeaponWeight + CurrentCapacity <= MaxCapacity)
	{
		InventoryArray.Add({ NewWeaponID, newWeaponStat, newWeaponState });
		if (GetCurrentWeaponCount() == 1) //기존 인벤토리가 비어있었다면? 바로 장착까지 해줌
		{
			EquipWeapon(InventoryArray.Num() - 1, true);
		}	
		CurrentCapacity += newWeaponStat.WeaponWeight;
		SortInventory();
		OnInventoryIsUpdated.Broadcast(InventoryArray);

		return true;
	}
	GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("무기를 추가할 수 없습니다. 인벤토리를 비우세요.")), FColor::White);
	return false;
}

void AWeaponInventoryBase::RemoveWeapon(int32 WeaponID)
{
	if (!GetWeaponIsContained(WeaponID)) return;

	int32 targetInventoryIdx = GetWeaponInventoryIdx(WeaponID);
	if (!InventoryArray.IsValidIndex(targetInventoryIdx)) return;
	
	CurrentCapacity -= InventoryArray[targetInventoryIdx].WeaponStat.WeaponWeight;
	CurrentWeaponCount -= 1;

	InventoryArray.RemoveAt(targetInventoryIdx);
	const bool bIsCurrentWeapon = targetInventoryIdx == GetCurrentIdx();

	CurrentIdx = bIsCurrentWeapon ? 0 : GetCurrentWeaponCount() - 1;

	if (bIsCurrentWeapon)
	{
		if (IsValid(GetCurrentWeaponRef())) GetCurrentWeaponRef()->Destroy();
		if (GetCurrentWeaponCount() > 0)
		{
			EquipWeapon(CurrentIdx);
		}
	}
	SortInventory();
	OnInventoryIsUpdated.Broadcast(InventoryArray);
}

void AWeaponInventoryBase::RemoveCurrentWeapon()
{
	if (!IsValid(GetCurrentWeaponRef())) return;
	RemoveWeapon(GetCurrentWeaponInfo().WeaponID);
}

bool AWeaponInventoryBase::SwitchToNextWeapon()
{
	const int32 nextIdx = GetNextInventoryIdx();
	if (GetCurrentWeaponCount() <= 1 || nextIdx == INVALID_INVENTORY_IDX) return false;
	if (!InventoryArray.IsValidIndex(nextIdx)) return false;
	if (!OwnerCharacterRef.IsValid()) return false;

	RestoreCurrentWeapon(); //현재 무기를 Destroy하고, 정보만 List에 저장한다.
	EquipWeapon(nextIdx);
	SetCurrentIdx(nextIdx);
	SortInventory();

	return false;
}

void AWeaponInventoryBase::SyncCurrentWeaponInfo(bool bIsLoadInfo)
{
	if (!OwnerCharacterRef.IsValid()) return;
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
	OnInventoryItemIsUpdated.Broadcast(GetCurrentIdx(), true, InventoryArray[GetCurrentIdx()]);
}

bool AWeaponInventoryBase::GetWeaponIsContained(int32 WeaponID)
{
	for (auto& weaponInfoRef : InventoryArray)
	{
		if (weaponInfoRef.WeaponID == WeaponID) return true;
	}
	return false;
}

bool AWeaponInventoryBase::TryAddAmmoToWeapon(int32 WeaponID, int32 AmmoCount)
{
	const int32 targetInventoryIdx = GetWeaponInventoryIdx(WeaponID);
	if (targetInventoryIdx == -1) return false;
	if (!InventoryArray[targetInventoryIdx].WeaponStat.RangedWeaponStat.bHasProjectile) return false;

	FInventoryItemInfoStruct& itemInfoRef = InventoryArray[targetInventoryIdx];

	int32& currTotalAmmoCount = itemInfoRef.WeaponState.RangedWeaponState.TotalAmmoCount;
	const int32 maxTotalAmmoCount = itemInfoRef.WeaponStat.RangedWeaponStat.MaxTotalAmmo;

	currTotalAmmoCount += AmmoCount;

	if(currTotalAmmoCount < maxTotalAmmoCount)
		currTotalAmmoCount %= maxTotalAmmoCount;
	else
		currTotalAmmoCount = maxTotalAmmoCount;
	
	if (CurrentIdx == targetInventoryIdx) SyncCurrentWeaponInfo(true);
	else OnInventoryItemIsUpdated.Broadcast(targetInventoryIdx, false, InventoryArray[targetInventoryIdx]);

	return true;
}

AWeaponBase* AWeaponInventoryBase::SpawnWeaponActor(int32 WeaponID)
{
	if (!OwnerCharacterRef.IsValid()) return nullptr;
	if (!WeaponClassInfoMap.Contains(WeaponID)) return nullptr;
	
	UClass* targetClass = *(WeaponClassInfoMap.Find(WeaponID));
	if (!targetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon Class가 Invalid 합니다."));
		return nullptr;
	}
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = OwnerCharacterRef.Get();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector spawnLocation = OwnerCharacterRef.Get()->GetActorLocation();
	const FRotator spawnRotation = FRotator();
	const FVector spawnScale3D = OwnerCharacterRef.Get()->ActorHasTag("Player") ? FVector(2.0f) : FVector(1.0f / OwnerCharacterRef.Get()->GetCapsuleComponent()->GetComponentScale().X);
	FTransform spawnTransform = FTransform(spawnRotation, spawnLocation, spawnScale3D);
	AWeaponBase* newWeapon = Cast<AWeaponBase>(GetWorld()->SpawnActor(targetClass, &spawnTransform, spawnParams));

	OwnerCharacterRef.Get()->SetWeaponActorRef(newWeapon);
	CurrentWeaponRef = newWeapon;

	return newWeapon;
}

void AWeaponInventoryBase::RestoreCurrentWeapon()
{
	if (GetCurrentWeaponCount() <= 1) return;
	if (!OwnerCharacterRef.IsValid()) return;

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

	//Inventory의 최대 Len이 10이기 때문에
	//O(n^2)이어도 상수 시간에 근접한 결과를 냄
	for (int32 selIdx = 0; selIdx < GetCurrentWeaponCount() - 1; selIdx++)
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

int32 AWeaponInventoryBase::GetWeaponInventoryIdx(int32 WeaponID)
{
	for (int32 inventoryIdx = 0; inventoryIdx < InventoryArray.Num(); inventoryIdx++)
	{
		FInventoryItemInfoStruct &weaponInfo = InventoryArray[inventoryIdx];
		if (weaponInfo.WeaponID == WeaponID) return inventoryIdx;
	}
	return -1;
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
	if (!CurrentWeaponRef.IsValid()) return nullptr;
	return CurrentWeaponRef.Get();
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
	CurrentWeaponRef.Get()->WeaponDestroyDelegate.BindUFunction(this, FName("RemoveCurrentWeapon"));
	CurrentWeaponRef.Get()->SetOwnerCharacter(OwnerCharacterRef.Get());
}
