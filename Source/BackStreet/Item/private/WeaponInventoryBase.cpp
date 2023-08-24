// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/WeaponInventoryBase.h"
#include "../public/WeaponBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#define INVALID_INVENTORY_IDX -1

AWeaponInventoryBase::AWeaponInventoryBase()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	PrimaryActorTick.bCanEverTick = false;
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
		check(0);
	}

	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());

	InventoryArray.Empty();

	//=========================================================
	/*
	//미리 클래스 / Id 정보를 초기화 함 
	for (int32 classIdx = 0; classIdx < WeaponIDList.Num(); classIdx++)
	{
		WeaponClassInfoMap.Add(WeaponIDList[classIdx], WeaponClassList[classIdx]);
	}*/
}

void AWeaponInventoryBase::EquipWeapon(int32 NewWeaponID)
{
	AWeaponBase* newWeapon = SpawnWeaponActor(NewWeaponID);

	if (IsValid(newWeapon) && !newWeapon->IsActorBeingDestroyed())
	{
		newWeapon->InitWeapon(NewWeaponID);
		SetCurrentWeaponRef(newWeapon);
		OwnerCharacterRef.Get()->EquipWeapon(newWeapon);
		SyncCurrentWeaponInfo();
	}


	/*
	if (IsValid(newWeapon))
	{
		SetCurrentWeaponRef(newWeapon);
		OwnerCharacterRef.Get()->EquipWeapon(GetCurrentWeaponRef());
		SetCurrentIdx(InventoryIdx);
	}*/
}

bool AWeaponInventoryBase::AddWeapon(int32 NewWeaponID)
{
	if(!OwnerCharacterRef.IsValid() || !GamemodeRef.IsValid()) return false;

	int32 duplicateIdx = CheckWeaponDuplicate(NewWeaponID);

	//중복이 되지 않는다면?
	if (duplicateIdx == -1)
	{
		//인벤토리가 비어있지 않다면 바로 착용 
		// 최초 PickUp 액션은 반드시 실행된다고 가정 (모든 무기의 무게가 인벤의 용량 최소치보다 작거나 같다)
		if (GetIsInventoryEmpty())
		{
			EquipWeapon(NewWeaponID);
			
			//인벤토리에 정보를 추가
			InventoryArray.Add({ NewWeaponID,
								GetCurrentWeaponRef()->GetWeaponStat(),
								GetCurrentWeaponRef()->GetWeaponState() });
			TotalWeight = GetCurrentWeaponRef()->GetWeaponStat().WeaponWeight;
		}
		else
		{
			//Weapon이 InValid하면 안됨
			ensure(IsValid(GetCurrentWeaponRef()));

			FWeaponStatStruct newStat; 
			FWeaponStateStruct newState;

			//현재 WeaponActor의 데이터테이블로부터 읽어들임
			newStat = GetCurrentWeaponRef()->GetWeaponStatInfoWithID(NewWeaponID);
			newState.CurrentDurability = newStat.MaxDurability;
			newState.RangedWeaponState.CurrentAmmoCount = newStat.RangedWeaponStat.MaxAmmoPerMagazine;

			//무게가 가득찼다면? 시스템 메시지 호출
			if (newStat.WeaponWeight + TotalWeight > MaxCapacity)
			{
				GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("무기를 추가할 수 없습니다. 인벤토리를 비우세요.")), FColor::White);
				return false;
			}
			//그렇지 않다면 인벤토리에 정보를 추가
			else
			{
				InventoryArray.Add({ NewWeaponID, newStat, newState });
				TotalWeight = GetCurrentWeaponRef()->GetWeaponStat().WeaponWeight;
			}
		}
	}
	//중복이 된다면 (발사체인 경우를 체크하고, 탄환을 추가)
	else
	{
		//Weapon이 InValid하면 안됨. 중복이 된다는 것은 인벤토리에 무기가 있다는 것이기 때문
		ensure(IsValid(GetCurrentWeaponRef()));

		//현재 WeaponActor의 데이터테이블로부터 읽어들임
		FWeaponStatStruct duplicateWeaponStat;
		duplicateWeaponStat = GetCurrentWeaponRef()->GetWeaponStatInfoWithID(NewWeaponID);

		//원거리 무기가 아니라면 시스템 메시지 호출 뒤 반환
		if (duplicateWeaponStat.WeaponType != EWeaponType::E_Shoot
			&& duplicateWeaponStat.WeaponType != EWeaponType::E_Throw)
		{
			GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("동일한 무기가 인벤토리에 있습니다.")), FColor::White);
			return false;
		}
		//원거리 무기라면 탄환 수 추가
		else
		{
			InventoryArray[duplicateIdx].WeaponState.RangedWeaponState.ExtraAmmoCount +=
				InventoryArray[duplicateIdx].WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine;

			InventoryArray[duplicateIdx].WeaponState.RangedWeaponState.ExtraAmmoCount %=
				InventoryArray[duplicateIdx].WeaponStat.RangedWeaponStat.MaxTotalAmmo;

			if (duplicateIdx == CurrentIdx) SyncCurrentWeaponInfo(false);
		}
	}

	//인벤토리를 무게에 따라 정렬
	SortInventory();

	// 인벤토리가 업데이트 되었음을 알리는 델리게이트 호출
	if(OnInventoryIsUpdated.IsBound())
		OnInventoryIsUpdated.Broadcast(InventoryArray);
	
	// 무기 습득 델리게이트 호툴
	if (OnAddWeapon.IsBound())
		OnAddWeapon.Broadcast();

	return true;
}

void AWeaponInventoryBase::RemoveWeapon(int32 WeaponID)
{
	if (!GetWeaponIsContained(WeaponID)) return;

	int32 targetInventoryIdx = GetWeaponInventoryIdx(WeaponID);
	if (!InventoryArray.IsValidIndex(targetInventoryIdx)) return;

	TotalWeight -= InventoryArray[targetInventoryIdx].WeaponStat.WeaponWeight;

	InventoryArray.RemoveAt(targetInventoryIdx);
	const bool bIsCurrentWeapon = targetInventoryIdx == GetCurrentIdx();
	CurrentIdx = bIsCurrentWeapon ? 0 : GetCurrentWeaponCount() - 1;

	if (bIsCurrentWeapon)
	{
		ensure(IsValid(GetCurrentWeaponRef()));
		
		//무기 개수가 0이라면 무기 액터를 제거
		if (GetCurrentWeaponCount() == 0) GetCurrentWeaponRef()->Destroy();

		//그렇지 않다면, 해당 무기 액터를 다시 초기화하여 재활용 
		else
		{
			GetCurrentWeaponRef()->InitWeapon(InventoryArray[CurrentIdx].WeaponID);
			SyncCurrentWeaponInfo();
		}
	}
	SortInventory();
	
	// 무기 드랍 델리게이트 호출
	if (OnDropWeapon.IsBound())
		OnDropWeapon.Broadcast();

	// 인벤토리가 업데이트 되었음을 알리는 델리게이트 호출
	if (OnInventoryIsUpdated.IsBound())
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
	ensure(IsValid(GetCurrentWeaponRef()));

	//기존 무기의 최신 액터 정보를 저장
	SyncCurrentWeaponInfo(true);

	//무기 액터를 다음칸의 무기 정보로 초기화
	GetCurrentWeaponRef()->InitWeapon(InventoryArray[nextIdx].WeaponID);
	
	//새로운 '현재' 인덱스를 등록하고 배열에서 정보를 꺼내옴
	SetCurrentIdx(nextIdx);
	SyncCurrentWeaponInfo(false);

	// 무기 스왑 델리게이트 호출
	if (OnSwapWeapon.IsBound())
		OnSwapWeapon.Broadcast();

	return false;
}

void AWeaponInventoryBase::SyncCurrentWeaponInfo(bool bIsLoadActorInfo)
{
	if (!OwnerCharacterRef.IsValid()) return;
	if (GetCurrentWeaponCount() == 0) return;
	if (!InventoryArray.IsValidIndex(GetCurrentIdx())) return;
	if (!IsValid(GetCurrentWeaponRef())) 	return;

	//액터 정보를 인벤토리 정보로 업데이트
	if (!bIsLoadActorInfo)
	{
		GetCurrentWeaponRef()->SetWeaponStat( InventoryArray[GetCurrentIdx()].WeaponStat );
		GetCurrentWeaponRef()->SetWeaponState( InventoryArray[GetCurrentIdx()].WeaponState );
	}
	//인벤토리 정보를 액터 정보로 업데이트
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
	if (InventoryArray[targetInventoryIdx].WeaponStat.WeaponType != EWeaponType::E_Shoot
		&& InventoryArray[targetInventoryIdx].WeaponStat.WeaponType != EWeaponType::E_Throw) return false;

	FInventoryItemInfoStruct& itemInfoRef = InventoryArray[targetInventoryIdx];

	int32& currExtraAmmoCount = itemInfoRef.WeaponState.RangedWeaponState.ExtraAmmoCount;
	const int32 maxExtraAmmoCount = itemInfoRef.WeaponStat.RangedWeaponStat.MaxTotalAmmo;

	currExtraAmmoCount += AmmoCount;

	if(currExtraAmmoCount < maxExtraAmmoCount)
		currExtraAmmoCount %= maxExtraAmmoCount;
	else
		currExtraAmmoCount = maxExtraAmmoCount;
	
	if (CurrentIdx == targetInventoryIdx) SyncCurrentWeaponInfo(true);
	else OnInventoryItemIsUpdated.Broadcast(targetInventoryIdx, false, InventoryArray[targetInventoryIdx]);

	return true;
}

AWeaponBase* AWeaponInventoryBase::SpawnWeaponActor(int32 WeaponID)
{
	if (!OwnerCharacterRef.IsValid()) return nullptr;
	check(WeaponClass != nullptr); //WeaponClass는 반드시 지정 되어있어야함
	
	UClass* targetClass = WeaponClass;
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
	CurrentWeaponRef.Get()->OnWeaponBeginDestroy.AddDynamic(this, &AWeaponInventoryBase::RemoveCurrentWeapon);
	CurrentWeaponRef.Get()->SetOwnerCharacter(OwnerCharacterRef.Get());
}
