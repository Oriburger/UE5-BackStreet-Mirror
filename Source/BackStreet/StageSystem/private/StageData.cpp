// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/StageData.h"
#include "../public/GateBase.h"

void AStageData::BeginPlay()
{
	Super::BeginPlay();
}

AStageData::AStageData()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStageData::OpenAllGate()
{
	TArray<AGateBase*> target = GetGateList();
	for (int32 idx = GetGateList().Num() - 1; idx >= 0; idx--)
	{

		if (IsValid(target[idx]))
		{
			UE_LOG(LogTemp, Error, TEXT("AStageData::OpenAllGate -> Gate : %s Is Activate"), *GetGateList()[idx]->GetName());
			GetGateList()[idx]->ActivateGate();
		}
		/*if (GetGateList()[idx] == nullptr)
			GetGateList().RemoveAt(idx);
		else
		{
			GetGateList()[idx]->ActivateGate();
		}*/
	}

	//if (GateOffDelegate.IsBound())
	//{
	//	if(GateOffDelegate.GetAllObjects().Num() == GetGateList().Num())
	//		GateOffDelegate.Broadcast();
	//	else
	//	{
	//		for (AGateBase* target : GetGateList())
	//		{
	//			target->BindGateDelegate();
	//		}
	//		GateOffDelegate.Broadcast();
	//	}
	//}
	//else
	//{
	//	for (AGateBase* target : GetGateList())
	//	{
	//		target->BindGateDelegate();
	//	}
	//	GateOffDelegate.Broadcast();
	//}
}

void AStageData::CloseAllGate()
{
	UE_LOG(LogTemp, Error, TEXT("AStageData::CloseAllGate"));
	TArray<AGateBase*> target = GetGateList();
	for (int32 idx = GetGateList().Num() - 1; idx >= 0; idx--)
	{
		if (IsValid(target[idx]))
		{
			UE_LOG(LogTemp, Error, TEXT("AStageData::CloseAllGate -> Gate : %s Is Deactivate"), *GetGateList()[idx]->GetName());
			GetGateList()[idx]->DeactivateGate();
		}	
	}

	//if (GateOffDelegate.IsBound())
	//{
	//	if(GateOffDelegate.GetAllObjects().Num() == GetGateList().Num())
	//		GateOffDelegate.Broadcast();
	//	else
	//	{
	//		for (AGateBase* target : GetGateList())
	//		{
	//			target->BindGateDelegate();
	//		}
	//		GateOffDelegate.Broadcast();
	//	}
	//}
	//else
	//{
	//	for (AGateBase* target : GetGateList())
	//	{
	//		target->BindGateDelegate();
	//	}
	//	GateOffDelegate.Broadcast();
	//}
}