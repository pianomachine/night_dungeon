// Copyright GanBowen 2022-2024. All Rights Reserved.

#include "GBWPowerfulStateComponent.h"

#include <string>

#include "AIController.h"
#include "GBWPowerfulToolsFuncLib.h"
#include "GBWPSFuncLib.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Launch/Resources/Version.h"
#include "StateTree/GBWPS_STSchema.h"
#include "StateTreeExecutionTypes.h"
#include "Subsystems/WorldSubsystem.h"

bool FPS_ActionLockState::IsLock(FGameplayTag ForActionTag) const
{
	if (!ForActionTag.IsValid())
	{
		return false;
	}
	
	for (auto Element : ActionToLock)
	{
		if (!Element.IsEmpty())
		{
			if (Element.Matches(ForActionTag.GetSingleTagContainer()))
			{
				return true;
			}
		}
	}
	return false;
}

bool FPS_ActionLockState::IsLock(const FGameplayTagContainer& ForActionTag) const
{
	if (!ForActionTag.IsValid())
	{
		return false;
	}
	
	for (auto Element : ActionToLock)
	{
		if (!Element.IsEmpty())
		{
			if (Element.Matches(ForActionTag))
			{
				return true;
			}
		}
	}
	return false;
}

void FPS_ActionLockState::RemoveActionLock(const FGameplayTagQuery& InActionToLock)
{
	for (int i = 0; i < ActionToLock.Num(); i++)
	{
		if (ActionToLock[i].GetDescription() == InActionToLock.GetDescription())
		{
			ActionToLock.RemoveAt(i);
			return;
		}
	}
}

bool FPS_ActionLockState::IsActionToLockNoChange(TArray<FGameplayTagQuery> InActionToLock) const
{
	if (ActionToLock.Num() != InActionToLock.Num())
	{
		return false;
	}

	for (auto Element : InActionToLock)
	{
		if (!ActionToLockDescription.Contains(Element.GetDescription()))
		{
			return false;
		}
	}

	return true;
}

void FPS_ActionLockState::UpdateLockState(float DeltaTime)
{
	if (ForceLockTime > 0)
	{
		ForceLockTime-=DeltaTime;
	}

	if (ForceLockFrames > 0)
	{
		ForceLockFrames--;
	}

	ActionToLockDescription.Empty();
	for (auto Element : ActionToLock)
	{
		ActionToLockDescription.Add(Element.GetDescription());
	}
}

FPS_ActionStateItem::FPS_ActionStateItem(const FName InSlot, const FPS_ActionLockState& InActionLockState)
{
	Slot = InSlot;
	ActionLockState = InActionLockState;
}

FPS_ActionStateItem::FPS_ActionStateItem(const FName InSlot, const FGameplayTag InCurrentActionTag)
{
	Slot = InSlot;
	CurrentActionTag = InCurrentActionTag;
}

void FPS_ActionStateItem::Update(float DeltaTime)
{
	ActionLockState.UpdateLockState(DeltaTime);
	CurrentActionExecutedTime+=DeltaTime;

	bIsActionChanged = false;
}

void FPS_ActionStateItem::SetCurrentAction(FGameplayTag ActionTag)
{
	LastActionTag = CurrentActionTag;
	LastActionExecutedTime = CurrentActionExecutedTime;

	CurrentActionTag = ActionTag;
	CurrentActionExecutedTime = 0.0f;

	bIsActionChanged = true;
}

int FPS_State::GetKeyIndex(const FName Key, const bool AutoAdd)
{
	if (!Key.IsValid())
	{
		return -1;
	}
	int Index = StateKeyDict.Find(Key);
	if (Index == INDEX_NONE && AutoAdd)
	{
		Index = StateKeyDict.Add(Key);
	}
	return Index;
}

int FPS_State::GetGTKeyIndex(const FGameplayTag Key, const bool AutoAdd)
{
	if (!Key.IsValid())
	{
		return -1;
	}
	int Index = GTKeyDict.Find(Key);
	if (Index == INDEX_NONE && AutoAdd)
	{
		Index = GTKeyDict.Add(Key);
	}
	return Index;
}

FName FPS_State::GetKey(const int Index)
{
	if (StateKeyDict.IsValidIndex(Index))
	{
		return StateKeyDict[Index];
	}
	return NAME_None;
}

FGameplayTag FPS_State::GetGTKey(const int Index)
{
	if (GTKeyDict.IsValidIndex(Index))
	{
		return GTKeyDict[Index];
	}
	return FGameplayTag::EmptyTag;
}

void FPS_State::RemoveState(const FName Key, const float Time)
{
	const int KeyIndex = GetKeyIndex(Key,false);
	if (KeyIndex == INDEX_NONE)
	{
		return;
	}
	
	for (int i=0; i<FState.Num(); i++)
	{
		if (FState[i].Key == KeyIndex)
		{
			FState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_F(EPS_StateEventType::Remove,FState[i]));
			FState.RemoveAt(i);
			i--;
		}
	}

	for (int i=0; i<BState.Num(); i++)
	{
		if (BState[i].Key == KeyIndex)
		{
			BState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_B(EPS_StateEventType::Remove,BState[i]));
			BState.RemoveAt(i);
			i--;
		}
	}

	for (int i=0; i<AState.Num(); i++)
	{
		if (AState[i].Key == KeyIndex)
		{
			AState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_A(EPS_StateEventType::Remove,AState[i]));
			AState.RemoveAt(i);
			i--;
		}
	}

	for (int i=0; i<TextState.Num(); i++)
	{
		if (TextState[i].Key == KeyIndex)
		{
			TextState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_Text(EPS_StateEventType::Remove,TextState[i]));
			TextState.RemoveAt(i);
			i--;
		}
	}

	for (int i=0; i<VState.Num(); i++)
	{
		if (VState[i].Key == KeyIndex)
		{
			VState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_V(EPS_StateEventType::Remove,VState[i]));
			VState.RemoveAt(i);
			i--;
		}
	}
}

void FPS_State::RemoveState(const int KeyIndex, const float Time)
{
	RemoveState(GetKey(KeyIndex),Time);
}

void FPS_State::SetState(const FName Key, const float Value, const float Time, const bool bReset)
{
	const int KeyIndex = GetKeyIndex(Key,true);
	for (int i=0; i<FState.Num(); i++)
	{
		if (FState[i].Key == KeyIndex)
		{
			if (FState[i].Value == Value && !bReset)
			{
				return;
			}
			FState[i].Value = Value;
			FState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_F(EPS_StateEventType::Change,FState[i]));
			return;
		}
	}
	
	FState.Add(FPS_Float(KeyIndex, Value, Time));
	AddStateOperation(FPS_StateOperation_F(EPS_StateEventType::Add,FPS_Float(KeyIndex, Value, Time)));
}

void FPS_State::SetState(const FName Key, const bool Value, const float Time, const bool bReset)
{
	const int KeyIndex = GetKeyIndex(Key,true);
	for (int i=0; i<BState.Num(); i++)
	{
		if (BState[i].Key == KeyIndex)
		{
			if (BState[i].Value == Value && !bReset)
			{
				return;
			}
			BState[i].Value = Value;
			BState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_B(EPS_StateEventType::Change,BState[i]));
			return;
		}
	}
	
	BState.Add(FPS_Bool(KeyIndex, Value, Time));
	AddStateOperation(FPS_StateOperation_B(EPS_StateEventType::Add,FPS_Bool(KeyIndex, Value, Time)));
}

void FPS_State::SetState(const FName Key, AActor* Value, const float Time, const bool bReset)
{
	const int KeyIndex = GetKeyIndex(Key,true);
	for (int i=0; i<AState.Num(); i++)
	{
		if (AState[i].Key == KeyIndex)
		{
			if (AState[i].Value == Value && !bReset)
			{
				return;
			}
			AState[i].Value = Value;
			AState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_A(EPS_StateEventType::Change,AState[i]));
			return;
		}
	}
	
	AState.Add(FPS_Actor(KeyIndex, Value, Time));
	AddStateOperation(FPS_StateOperation_A(EPS_StateEventType::Add,FPS_Actor(KeyIndex, Value, Time)));
}

void FPS_State::SetState(const FName Key, const FString& Value, const float Time, const bool bReset)
{
	const int KeyIndex = GetKeyIndex(Key,true);
	for (int i=0; i<TextState.Num(); i++)
	{
		if (TextState[i].Key == KeyIndex)
		{
			if (TextState[i].Value == Value && !bReset)
			{
				return;
			}
			TextState[i].Value = Value;
			TextState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_Text(EPS_StateEventType::Change,TextState[i]));
			return;
		}
	}
	
	TextState.Add(FPS_Text(KeyIndex, Value, Time));
	AddStateOperation(FPS_StateOperation_Text(EPS_StateEventType::Add,FPS_Text(KeyIndex, Value, Time)));
}

void FPS_State::SetState(const FName Key, const FVector& Value, float Time, const bool bReset)
{
	const int KeyIndex = GetKeyIndex(Key,true);
	for (int i=0; i<VState.Num(); i++)
	{
		if (VState[i].Key == KeyIndex)
		{
			if (VState[i].Value == Value && !bReset)
			{
				return;
			}
			VState[i].Value = Value;
			VState[i].Time = Time;
			AddStateOperation(FPS_StateOperation_V(EPS_StateEventType::Change,VState[i]));
			return;
		}
	}
	
	VState.Add(FPS_Vector(KeyIndex, Value, Time));
	AddStateOperation(FPS_StateOperation_V(EPS_StateEventType::Add,FPS_Vector(KeyIndex, Value, Time)));
}

bool FPS_State::GetValue(const FName Key, float& Result, float& Time)
{
	const int KeyIndex = GetKeyIndex(Key,false);
	for (int i=0; i<FState.Num(); i++)
	{
		if (FState[i].Key == KeyIndex)
		{
			Result = FState[i].Value;
			Time = FState[i].Time;
			return true;
		}
	}
	return false;
}

bool FPS_State::GetValue(const FName Key, bool& Result, float& Time)
{
	const int KeyIndex = GetKeyIndex(Key,false);
	for (int i=0; i<BState.Num(); i++)
	{
		if (BState[i].Key == KeyIndex)
		{
			Result = BState[i].Value;
			Time = BState[i].Time;
			return true;
		}
	}
	return false;
}

bool FPS_State::GetValue(const FName Key, AActor*& Result, float& Time)
{
	const int KeyIndex = GetKeyIndex(Key,false);
	for (int i=0; i<AState.Num(); i++)
	{
		if (AState[i].Key == KeyIndex)
		{
			Result = AState[i].Value;
			Time = AState[i].Time;
			return true;
		}
	}
	return false;
}

bool FPS_State::GetValue(const FName Key, FString& Result, float& Time)
{
	const int KeyIndex = GetKeyIndex(Key,false);
	for (int i=0; i<TextState.Num(); i++)
	{
		if (TextState[i].Key == KeyIndex)
		{
			Result = TextState[i].Value;
			Time = TextState[i].Time;
			return true;
		}
	}
	return false;
}

bool FPS_State::GetValue(const FName Key, FVector& Result, float& Time)
{
	const int KeyIndex = GetKeyIndex(Key,false);
	for (int i=0; i<VState.Num(); i++)
	{
		if (VState[i].Key == KeyIndex)
		{
			Result = VState[i].Value;
			Time = VState[i].Time;
			return true;
		}
	}
	return false;
}

bool FPS_State::GetValue_History(const FName Key, const int DistanceFromNow, float& Result, float& Time)
{
	if (DistanceFromNow <= 0)
	{
		return GetValue(Key,Result,Time);
	}
	const int KeyIndex = GetKeyIndex(Key,false);
	FPS_StateOperation_F LastOperation;
	FPS_StateOperation_F TargetOperation;
	int DistanceCache = 0;
	bool bIsGet = false;
	for (int i=0; i<FStateOperations.Num(); i++)
	{
		int Index = -1;
		if (FStateOperationIndexNow - 1 - i >= 0)
		{
			Index = FStateOperationIndexNow - i - 1;
		}
		else
		{
			Index = FStateOperationIndexNow - i - 1 + FStateOperations.Num();
		}
		if (FStateOperations.Num() <= Index || Index < 0)
		{
			break;
		}

		if (FStateOperations[Index].StateElement.Key == KeyIndex)
		{
			DistanceCache++;
			if (DistanceCache == DistanceFromNow)
			{
				LastOperation = FStateOperations[Index];
			}
			else if (DistanceCache == DistanceFromNow+1)
			{
				TargetOperation = FStateOperations[Index];
				bIsGet = true;
				break;
			}
		}
	}

	if (bIsGet)
	{
		if (TargetOperation.OperationType == EPS_StateEventType::Add || TargetOperation.OperationType == EPS_StateEventType::Change)
		{
			Result = TargetOperation.StateElement.Value;
			Time = LastOperation.StateElement.Time - TargetOperation.StateElement.Time;
		}
		else
		{
			return false;
		}
	}

	return bIsGet;
}

bool FPS_State::GetValue_History(const FName Key, const int DistanceFromNow, bool& Result, float& Time)
{
	if (DistanceFromNow <= 0)
	{
		return GetValue(Key,Result,Time);
	}
	const int KeyIndex = GetKeyIndex(Key,false);
	FPS_StateOperation_B LastOperation;
	FPS_StateOperation_B TargetOperation;
	int DistanceCache = 0;
	bool bIsGet = false;
	for (int i=0; i<BStateOperations.Num(); i++)
	{
		int Index = -1;
		if (BStateOperationIndexNow - 1 - i >= 0)
		{
			Index = BStateOperationIndexNow - i - 1;
		}
		else
		{
			Index = BStateOperationIndexNow - i - 1 + BStateOperations.Num();
		}
		if (BStateOperations.Num() <= Index || Index < 0)
		{
			break;
		}

		if (BStateOperations[Index].StateElement.Key == KeyIndex)
		{
			DistanceCache++;
			if (DistanceCache == DistanceFromNow)
			{
				LastOperation = BStateOperations[Index];
			}
			else if (DistanceCache == DistanceFromNow+1)
			{
				TargetOperation = BStateOperations[Index];
				bIsGet = true;
				break;
			}
		}
	}

	if (bIsGet)
	{
		if (TargetOperation.OperationType == EPS_StateEventType::Add || TargetOperation.OperationType == EPS_StateEventType::Change)
		{
			Result = TargetOperation.StateElement.Value;
			Time = LastOperation.StateElement.Time - TargetOperation.StateElement.Time;
		}
		else
		{
			return false;
		}
	}

	return bIsGet;
}

bool FPS_State::GetValue_History(const FName Key, const int DistanceFromNow, AActor*& Result, float& Time)
{
	if (DistanceFromNow <= 0)
	{
		return GetValue(Key,Result,Time);
	}
	const int KeyIndex = GetKeyIndex(Key,false);
	FPS_StateOperation_A LastOperation;
	FPS_StateOperation_A TargetOperation;
	int DistanceCache = 0;
	bool bIsGet = false;
	for (int i=0; i<AStateOperations.Num(); i++)
	{
		int Index = -1;
		if (BStateOperationIndexNow - 1 - i >= 0)
		{
			Index = BStateOperationIndexNow - i - 1;
		}
		else
		{
			Index = BStateOperationIndexNow - i - 1 + AStateOperations.Num();
		}
		if (AStateOperations.Num() <= Index || Index < 0)
		{
			break;
		}

		if (AStateOperations[Index].StateElement.Key == KeyIndex)
		{
			DistanceCache++;
			if (DistanceCache == DistanceFromNow)
			{
				LastOperation = AStateOperations[Index];
			}
			else if (DistanceCache == DistanceFromNow+1)
			{
				TargetOperation = AStateOperations[Index];
				bIsGet = true;
				break;
			}
		}
	}

	if (bIsGet)
	{
		if (TargetOperation.OperationType == EPS_StateEventType::Add || TargetOperation.OperationType == EPS_StateEventType::Change)
		{
			Result = TargetOperation.StateElement.Value;
			Time = LastOperation.StateElement.Time - TargetOperation.StateElement.Time;
		}
		else
		{
			return false;
		}
	}

	return bIsGet;
}

bool FPS_State::GetValue_History(const FName Key, const int DistanceFromNow, FString& Result, float& Time)
{
	if (DistanceFromNow <= 0)
	{
		return GetValue(Key,Result,Time);
	}
	const int KeyIndex = GetKeyIndex(Key,false);
	FPS_StateOperation_Text LastOperation;
	FPS_StateOperation_Text TargetOperation;
	int DistanceCache = 0;
	bool bIsGet = false;
	for (int i=0; i<TStateOperations.Num(); i++)
	{
		int Index = -1;
		if (BStateOperationIndexNow - 1 - i >= 0)
		{
			Index = BStateOperationIndexNow - i - 1;
		}
		else
		{
			Index = BStateOperationIndexNow - i - 1 + TStateOperations.Num();
		}
		if (TStateOperations.Num() <= Index || Index < 0)
		{
			break;
		}

		if (TStateOperations[Index].StateElement.Key == KeyIndex)
		{
			DistanceCache++;
			if (DistanceCache == DistanceFromNow)
			{
				LastOperation = TStateOperations[Index];
			}
			else if (DistanceCache == DistanceFromNow+1)
			{
				TargetOperation = TStateOperations[Index];
				bIsGet = true;
				break;
			}
		}
	}

	if (bIsGet)
	{
		if (TargetOperation.OperationType == EPS_StateEventType::Add || TargetOperation.OperationType == EPS_StateEventType::Change)
		{
			Result = TargetOperation.StateElement.Value;
			Time = LastOperation.StateElement.Time - TargetOperation.StateElement.Time;
		}
		else
		{
			return false;
		}
	}

	return bIsGet;
}

bool FPS_State::GetValue_History(const FName Key, const int DistanceFromNow, FVector& Result, float& Time)
{
	if (DistanceFromNow <= 0)
	{
		return GetValue(Key,Result,Time);
	}
	const int KeyIndex = GetKeyIndex(Key,false);
	FPS_StateOperation_V LastOperation;
	FPS_StateOperation_V TargetOperation;
	int DistanceCache = 0;
	bool bIsGet = false;
	for (int i=0; i<VStateOperations.Num(); i++)
	{
		int Index = -1;
		if (BStateOperationIndexNow - 1 - i >= 0)
		{
			Index = BStateOperationIndexNow - i - 1;
		}
		else
		{
			Index = BStateOperationIndexNow - i - 1 + VStateOperations.Num();
		}
		if (VStateOperations.Num() <= Index || Index < 0)
		{
			break;
		}

		if (VStateOperations[Index].StateElement.Key == KeyIndex)
		{
			DistanceCache++;
			if (DistanceCache == DistanceFromNow)
			{
				LastOperation = VStateOperations[Index];
			}
			else if (DistanceCache == DistanceFromNow+1)
			{
				TargetOperation = VStateOperations[Index];
				bIsGet = true;
				break;
			}
		}
	}

	if (bIsGet)
	{
		if (TargetOperation.OperationType == EPS_StateEventType::Add || TargetOperation.OperationType == EPS_StateEventType::Change)
		{
			Result = TargetOperation.StateElement.Value;
			Time = LastOperation.StateElement.Time - TargetOperation.StateElement.Time;
		}
		else
		{
			return false;
		}
	}

	return bIsGet;
}

void FPS_State::AddStateOperation(const FPS_StateOperation_F& NewStateOperation)
{
	if (FStateOperations.Num() <= MaxStateOperationNum)
	{
		FStateOperationIndexNow = FStateOperations.Num();
		FStateOperations.Add(NewStateOperation);
	}
	else
	{
		if (FStateOperationIndexNow >= MaxStateOperationNum)
		{
			FStateOperationIndexNow = 0;
		}
		FStateOperations[FStateOperationIndexNow] = NewStateOperation;
		FStateOperationIndexNow++;
	}
}

void FPS_State::AddStateOperation(const FPS_StateOperation_B& NewStateOperation)
{
	if (BStateOperations.Num() <= MaxStateOperationNum)
	{
		BStateOperationIndexNow = BStateOperations.Num();
		BStateOperations.Add(NewStateOperation);
	}
	else
	{
		if (BStateOperationIndexNow >= MaxStateOperationNum)
		{
			BStateOperationIndexNow = 0;
		}
		BStateOperations[BStateOperationIndexNow] = NewStateOperation;
		BStateOperationIndexNow++;
	}
}

void FPS_State::AddStateOperation(const FPS_StateOperation_A& NewStateOperation)
{
	if (AStateOperations.Num() <= MaxStateOperationNum)
	{
		AStateOperationIndexNow = AStateOperations.Num();
		AStateOperations.Add(NewStateOperation);
	}
	else
	{
		if (AStateOperationIndexNow >= MaxStateOperationNum)
		{
			AStateOperationIndexNow = 0;
		}
		AStateOperations[AStateOperationIndexNow] = NewStateOperation;
		AStateOperationIndexNow++;
	}
}

void FPS_State::AddStateOperation(const FPS_StateOperation_Text& NewStateOperation)
{
	if (TStateOperations.Num() <= MaxStateOperationNum)
	{
		TStateOperationIndexNow = TStateOperations.Num();
		TStateOperations.Add(NewStateOperation);
	}
	else
	{
		if (TStateOperationIndexNow >= MaxStateOperationNum)
		{
			TStateOperationIndexNow = 0;
		}
		TStateOperations[TStateOperationIndexNow] = NewStateOperation;
		TStateOperationIndexNow++;
	}
}

void FPS_State::AddStateOperation(const FPS_StateOperation_V& NewStateOperation)
{
	if (VStateOperations.Num() <= MaxStateOperationNum)
	{
		VStateOperationIndexNow = VStateOperations.Num();
		VStateOperations.Add(NewStateOperation);
	}
	else
	{
		if (VStateOperationIndexNow >= MaxStateOperationNum)
		{
			VStateOperationIndexNow = 0;
		}
		VStateOperations[VStateOperationIndexNow] = NewStateOperation;
		VStateOperationIndexNow++;
	}
}

bool FPS_State::CheckStateChangeType(const FName Key, const float Value, EPS_StateEventType& ChangeType,
	float& OldValue, float& NewValue)
{
	NewValue = Value;

	float CurrentValue;
	float Time;
	if (GetValue(Key,CurrentValue,Time))
	{
		if (CurrentValue != Value)
		{
			ChangeType = EPS_StateEventType::Change;
			OldValue = CurrentValue;
			return true;
		}
		else
		{
			return false;
		}
	}
	
	ChangeType = EPS_StateEventType::Add;
	OldValue = Value;
	return true;
}

bool FPS_State::CheckStateChangeType(const FName Key, const bool Value, EPS_StateEventType& ChangeType,
	bool& OldValue, bool& NewValue)
{
	NewValue = Value;

	bool CurrentValue;
	float Time;
	if (GetValue(Key,CurrentValue,Time))
	{
		if (CurrentValue != Value)
		{
			ChangeType = EPS_StateEventType::Change;
			OldValue = CurrentValue;
			return true;
		}
		else
		{
			return false;
		}
	}
	
	ChangeType = EPS_StateEventType::Add;
	OldValue = Value;
	return true;
}

bool FPS_State::CheckStateChangeType(const FName Key, AActor* Value, EPS_StateEventType& ChangeType,
	AActor*& OldValue, AActor*& NewValue)
{
	NewValue = Value;

	AActor* CurrentValue;
	float Time;
	if (GetValue(Key,CurrentValue,Time))
	{
		if (CurrentValue != Value)
		{
			ChangeType = EPS_StateEventType::Change;
			OldValue = CurrentValue;
			return true;
		}
		else
		{
			return false;
		}
	}
	
	ChangeType = EPS_StateEventType::Add;
	OldValue = Value;
	return true;
}

bool FPS_State::CheckStateChangeType(const FName Key, const FString& Value, EPS_StateEventType& ChangeType,
	FString& OldValue, FString& NewValue)
{
	NewValue = Value;

	FString CurrentValue;
	float Time;
	if (GetValue(Key,CurrentValue,Time))
	{
		if (CurrentValue != Value)
		{
			ChangeType = EPS_StateEventType::Change;
			OldValue = CurrentValue;
			return true;
		}
		else
		{
			return false;
		}
	}
	
	ChangeType = EPS_StateEventType::Add;
	OldValue = Value;
	return true;
}

bool FPS_State::CheckStateChangeType(const FName Key, const FVector& Value, EPS_StateEventType& ChangeType,
	FVector& OldValue, FVector& NewValue)
{
	NewValue = Value;

	FVector CurrentValue;
	float Time;
	if (GetValue(Key,CurrentValue,Time))
	{
		if (CurrentValue != Value)
		{
			ChangeType = EPS_StateEventType::Change;
			OldValue = CurrentValue;
			return true;
		}
		else
		{
			return false;
		}
	}
	
	ChangeType = EPS_StateEventType::Add;
	OldValue = Value;
	return true;
}

FString FPS_State::GetString(const UWorld* InWorld, const int StateOperationNum)
{
	float TimeNow = InWorld? InWorld->GetTimeSeconds() : 0.0f;
	FString Res = "";
	for (int i=FState.Num()-1; i>=0; i--)
	{
		if (i==FState.Num()-1)
		{
			Res = Res + "\n\n Float State:";
		}
		Res = Res + FString::Printf(TEXT("\n  %s<%.2f>  %f"),
			*GetKey(FState[i].Key).ToString(),
			TimeNow - FState[i].Time,
			FState[i].Value
			);
	}
	for (int i=BState.Num()-1; i>=0; i--)
	{
		if (i==BState.Num()-1)
		{
			Res = Res + "\n\n Bool State:";
		}
		Res = Res + FString::Printf(TEXT("\n  %s<%.2f>  %s"),
			*GetKey(BState[i].Key).ToString(),
			TimeNow - BState[i].Time,
			*(BState[i].Value? FString("True"):FString("False"))
			);
	}
	for (int i=AState.Num()-1; i>=0; i--)
	{
		if (i==AState.Num()-1)
		{
			Res = Res + "\n\n Actor State:";
		}
		Res = Res + FString::Printf(TEXT("\n  %s<%.2f>  %s"),
			*GetKey(AState[i].Key).ToString(),
			TimeNow - AState[i].Time,
			*(AState[i].Value? AState[i].Value->GetName():FString("None"))
			);
	}
	for (int i=TextState.Num()-1; i>=0; i--)
	{
		if (i==TextState.Num()-1)
		{
			Res = Res + "\n\n Actor State:";
		}
		Res = Res + FString::Printf(TEXT("\n  %s<%.2f>  %s"),
			*GetKey(TextState[i].Key).ToString(),
			TimeNow - TextState[i].Time,
			*TextState[i].Value
			);
	}
	for (int i=VState.Num()-1; i>=0; i--)
	{
		if (i==VState.Num()-1)
		{
			Res = Res + "\n\n Actor State:";
		}
		Res = Res + FString::Printf(TEXT("\n  %s<%.2f>  %s"),
			*GetKey(VState[i].Key).ToString(),
			TimeNow - VState[i].Time,
			*VState[i].Value.ToString()
			);
	}

	if (StateOperationNum > 0)
	{
		if (FStateOperations.Num() > 0)
		{
			Res =  Res + "\n\n ---------Float StateOperation---------";
		}
		for (int i=0; i<StateOperationNum; i++)
		{
			int Index = -1;
			if (FStateOperationIndexNow - i >= 0)
			{
				Index = FStateOperationIndexNow - i;
			}
			else if (FStateOperations.Num()>=StateOperationNum)
			{
				Index = FStateOperationIndexNow - i + FStateOperations.Num();
			}
			if (FStateOperations.Num() <= Index || Index < 0)
			{
				break;
			}
			Res = Res + FString::Printf(TEXT("\n [%d] <%s> %s"),
				Index,
				*(StaticEnum<EPS_StateEventType>()->GetNameStringByIndex( ( int32 ) FStateOperations[Index].OperationType)),
				*FString::Printf(TEXT("%s<%.2f>  %f"),
							*GetKey(FStateOperations[Index].StateElement.Key).ToString(),
							FStateOperations[Index].StateElement.Time,
							FStateOperations[Index].StateElement.Value
							)
				);
		}
		if (FStateOperations.Num() > 0)
		{
			Res =  Res + "\n ---------Float StateOperation---------";
		}

		if (BStateOperations.Num() > 0)
		{
			Res =  Res + "\n\n ---------Bool StateOperation---------";
		}
		for (int i=0; i<StateOperationNum; i++)
		{
			int Index = -1;
			if (BStateOperationIndexNow - i >= 0)
			{
				Index = BStateOperationIndexNow - i;
			}
			else if (BStateOperations.Num()>=StateOperationNum)
			{
				Index = BStateOperationIndexNow - i + BStateOperations.Num();
			}
			if (BStateOperations.Num() <= Index || Index < 0)
			{
				break;
			}
			Res = Res + FString::Printf(TEXT("\n [%d] <%s> %s"),
				Index,
				*(StaticEnum<EPS_StateEventType>()->GetNameStringByIndex( ( int32 ) BStateOperations[Index].OperationType)),
				*FString::Printf(TEXT("%s<%.2f>  %s"),
							*GetKey(BStateOperations[Index].StateElement.Key).ToString(),
							BStateOperations[Index].StateElement.Time,
							*(BStateOperations[Index].StateElement.Value? FString("True"):FString("False"))
							)
				);
		}
		if (BStateOperations.Num() > 0)
		{
			Res =  Res + "\n ---------Bool StateOperation---------";
		}

		if (AStateOperations.Num() > 0)
		{
			Res =  Res + "\n\n ---------Actor StateOperation---------";
		}
		for (int i=0; i<StateOperationNum; i++)
		{
			int Index = -1;
			if (AStateOperationIndexNow - i >= 0)
			{
				Index = AStateOperationIndexNow - i;
			}
			else if (AStateOperations.Num()>=StateOperationNum)
			{
				Index = AStateOperationIndexNow - i + AStateOperations.Num();
			}
			if (AStateOperations.Num() <= Index || Index < 0)
			{
				break;
			}
			Res = Res + FString::Printf(TEXT("\n [%d] <%s> %s"),
				Index,
				*(StaticEnum<EPS_StateEventType>()->GetNameStringByIndex( ( int32 ) AStateOperations[Index].OperationType)),
				*FString::Printf(TEXT("%s<%.2f>  %s"),
							*GetKey(AStateOperations[Index].StateElement.Key).ToString(),
							AStateOperations[Index].StateElement.Time,
							*(AStateOperations[Index].StateElement.Value? AStateOperations[Index].StateElement.Value->GetName():FString("None"))
							)
				);
		}
		if (AStateOperations.Num() > 0)
		{
			Res =  Res + "\n ---------Actor StateOperation---------";
		}

		if (TStateOperations.Num() > 0)
		{
			Res =  Res + "\n\n ---------Text StateOperation---------";
		}
		for (int i=0; i<StateOperationNum; i++)
		{
			int Index = -1;
			if (TStateOperationIndexNow - i >= 0)
			{
				Index = TStateOperationIndexNow - i;
			}
			else if (TStateOperations.Num()>=StateOperationNum)
			{
				Index = TStateOperationIndexNow - i + TStateOperations.Num();
			}
			if (TStateOperations.Num() <= Index || Index < 0)
			{
				break;
			}
			Res = Res + FString::Printf(TEXT("\n [%d] <%s> %s"),
				Index,
				*(StaticEnum<EPS_StateEventType>()->GetNameStringByIndex( ( int32 ) TStateOperations[Index].OperationType)),
				*FString::Printf(TEXT("%s<%.2f>  %s"),
							*GetKey(TStateOperations[Index].StateElement.Key).ToString(),
							TStateOperations[Index].StateElement.Time,
							*TStateOperations[Index].StateElement.Value
							)
				);
		}
		if (TStateOperations.Num() > 0)
		{
			Res =  Res + "\n ---------Text StateOperation---------";
		}

		if (VStateOperations.Num() > 0)
		{
			Res =  Res + "\n\n ---------Vector StateOperation---------";
		}
		for (int i=0; i<StateOperationNum; i++)
		{
			int Index = -1;
			if (VStateOperationIndexNow - i >= 0)
			{
				Index = VStateOperationIndexNow - i;
			}
			else if (VStateOperations.Num()>=StateOperationNum)
			{
				Index = VStateOperationIndexNow - i + VStateOperations.Num();
			}
			if (VStateOperations.Num() <= Index || Index < 0)
			{
				break;
			}
			Res = Res + FString::Printf(TEXT("\n [%d] <%s> %s"),
				Index,
				*(StaticEnum<EPS_StateEventType>()->GetNameStringByIndex( ( int32 ) VStateOperations[Index].OperationType)),
				*FString::Printf(TEXT("%s<%.2f>  %s"),
							*GetKey(VStateOperations[Index].StateElement.Key).ToString(),
							VStateOperations[Index].StateElement.Time,
							*VStateOperations[Index].StateElement.Value.ToString()
							)
				);
		}
		if (VStateOperations.Num() > 0)
		{
			Res =  Res + "\n ---------Vector StateOperation---------";
		}
	}
	return Res;
}

UGBWPowerfulStateComponent::UGBWPowerfulStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	bWantsInitializeComponent = true;
	StateTree.bIsRunning = false;
	StateTree.bIsPaused = false;
}

void UGBWPowerfulStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;
	Parameters.Condition = COND_None;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MaxStateOperationNum, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PowerfulState, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ActionState, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PowerfulStateGuid, Parameters)

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bIsDebug, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StateOperationNum, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DebugTextColor, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bPrintToScreen, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bPrintToLog, Parameters)
}

void UGBWPowerfulStateComponent::InitializeComponent()
{
	Super::InitializeComponent();
	for (int i = 0; i < StateTree.StateTreeSet.Num(); i++)
	{
		StateTree.InstanceDataSet.Add(FStateTreeInstanceData());
		if (!StateTree.StateTreeSet[i].IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("%s: StateTree[%d] asset is not set, cannot initialize."), ANSI_TO_TCHAR(__FUNCTION__), i);
		}
	}

	for (int i = 0; i < StateTree.StateTreeSet.Num(); i++)
	{
		if (StateTree.StateTreeSet[i].IsValid())
		{
			const FStateTreeExecutionContext Context(*GetOwner(), *StateTree.StateTreeSet[i].GetStateTree(), StateTree.InstanceDataSet[i]);
			if (!Context.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("%s: Failed to init StateTreeContext[%d]."), ANSI_TO_TCHAR(__FUNCTION__), i);
			}
		}
	}
}

void UGBWPowerfulStateComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

void UGBWPowerfulStateComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PowerfulStateGuid = FGuid::NewGuid();
	}

	if (StateTree.bStartStateTreeAutomatically)
	{
		StartLogic();
	}
}

void UGBWPowerfulStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (int i = 0; i < ActionState.Num(); i++)
	{
		ActionState[i].Update(DeltaTime);
	}

	if (!StateTree.bIsPaused && StateTree.bIsRunning)
	{
		for (int i = 0; i < StateTree.StateTreeSet.Num(); i++)
		{
			if (StateTree.StateTreeSet[i].IsValid())
			{
				FStateTreeExecutionContext Context(*GetOwner(), *StateTree.StateTreeSet[i].GetStateTree(), StateTree.InstanceDataSet[i]);
				if (SetContextRequirements(Context))
				{
					Context.Tick(DeltaTime);
				}
			}
		}
	}
	
	PSDebug();
}

void UGBWPowerfulStateComponent::StartLogic()
{
	for (int i = 0; i < StateTree.StateTreeSet.Num(); i++)
	{
		if (StateTree.StateTreeSet[i].IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("%s: Start StateTree[%d] Logic"), ANSI_TO_TCHAR(__FUNCTION__), i);
			FStateTreeExecutionContext Context(*GetOwner(), *StateTree.StateTreeSet[i].GetStateTree(), StateTree.InstanceDataSet[i]);
			if (SetContextRequirements(Context))
			{
				Context.Start(&StateTree.StateTreeSet[i].GetParameters());
				StateTree.bIsRunning = true;
			}
		}
	}
}

void UGBWPowerfulStateComponent::RestartLogic()
{
	for (int i = 0; i < StateTree.StateTreeSet.Num(); i++)
	{
		if (StateTree.StateTreeSet[i].IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("%s: Restart StateTree[%d] Logic"), ANSI_TO_TCHAR(__FUNCTION__), i);
			FStateTreeExecutionContext Context(*GetOwner(), *StateTree.StateTreeSet[i].GetStateTree(), StateTree.InstanceDataSet[i]);
			if (SetContextRequirements(Context))
			{
				Context.Start(&StateTree.StateTreeSet[i].GetParameters());
				StateTree.bIsRunning = true;
			}
		}
	}
}

void UGBWPowerfulStateComponent::StopLogic(const FString& Reason)
{
	for (int i = 0; i < StateTree.StateTreeSet.Num(); i++)
	{
		if (StateTree.StateTreeSet[i].IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("%s: Stopping StateTree[%d], reason: \'%s\'"), ANSI_TO_TCHAR(__FUNCTION__), i, *Reason);
			FStateTreeExecutionContext Context(*GetOwner(), *StateTree.StateTreeSet[i].GetStateTree(), StateTree.InstanceDataSet[i]);
			if (SetContextRequirements(Context))
			{
				Context.Stop();
				StateTree.bIsRunning = false;
			}
		}
	}
}

void UGBWPowerfulStateComponent::PauseLogic(const FString& Reason)
{
	UE_LOG(LogTemp, Log, TEXT("%s: Execution updates: PAUSED (%s)"), ANSI_TO_TCHAR(__FUNCTION__), *Reason);
	StateTree.bIsPaused = true;
}

EAILogicResuming::Type UGBWPowerfulStateComponent::ResumeLogic(const FString& Reason)
{
	UE_LOG(LogTemp, Log, TEXT("%s: Execution updates: RESUMED (%s)"), ANSI_TO_TCHAR(__FUNCTION__), *Reason);

	const EAILogicResuming::Type SuperResumeResult = EAILogicResuming::Continue;

	if (!!StateTree.bIsPaused)
	{
		StateTree.bIsPaused = false;

		if (SuperResumeResult == EAILogicResuming::Continue)
		{
			// Nop
		}
		else if (SuperResumeResult == EAILogicResuming::RestartedInstead)
		{
			RestartLogic();
		}
	}

	return SuperResumeResult;
}

bool UGBWPowerfulStateComponent::IsRunning() const
{
	return StateTree.bIsRunning;
}

bool UGBWPowerfulStateComponent::IsPaused() const
{
	return StateTree.bIsPaused;
}

bool UGBWPowerfulStateComponent::SetContextRequirements(FStateTreeExecutionContext& Context, bool bLogErrors) const
{
	if (!Context.IsValid())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	// Make sure the actor matches one required.
	AActor* ContextActor = nullptr;
	const UGBWPS_STSchema* Schema = Cast<UGBWPS_STSchema>(Context.GetStateTree()->GetSchema());
	if (Schema)
	{
		if (AAIController* OwnerController = Cast<AAIController>(GetOwner()))
		{
			if (OwnerController && OwnerController->IsA(Schema->GetContextActorClass()))
			{
				ContextActor = OwnerController;
			}
		}
		if (ContextActor == nullptr)
		{
			if (AActor* OwnerActor = GetOwner())
			{
				if (OwnerActor && OwnerActor->IsA(Schema->GetContextActorClass()))
				{
					ContextActor = OwnerActor;
				}
			}
		}
		if (ContextActor == nullptr && bLogErrors)
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Could not find context actor of type %s. StateTree will not update."), ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(Schema->GetContextActorClass()));
		}
	}
	else if (bLogErrors)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Expected StateTree asset to contain StateTreeComponentSchema. StateTree will not update."), ANSI_TO_TCHAR(__FUNCTION__));
	}

	// Set context data by name (for Actor defined in schema)
	Context.SetContextDataByName(FName(TEXT("Actor")), FStateTreeDataView(ContextActor));

	// Set external data callback for subsystems, components, etc.
	AActor* OwnerActor = GetOwner();
	Context.SetCollectExternalDataCallback(FOnCollectStateTreeExternalData::CreateLambda(
		[World, OwnerActor](const FStateTreeExecutionContext& InContext, const UStateTree* StateTree, TArrayView<const FStateTreeExternalDataDesc> ExternalDescs, TArrayView<FStateTreeDataView> OutDataViews)
		{
			check(ExternalDescs.Num() == OutDataViews.Num());
			for (int32 Index = 0; Index < ExternalDescs.Num(); Index++)
			{
				const FStateTreeExternalDataDesc& Desc = ExternalDescs[Index];
				if (Desc.Struct != nullptr)
				{
					if (World != nullptr && Desc.Struct->IsChildOf(UWorldSubsystem::StaticClass()))
					{
						UWorldSubsystem* Subsystem = World->GetSubsystemBase(Cast<UClass>(const_cast<UStruct*>(ToRawPtr(Desc.Struct))));
						OutDataViews[Index] = FStateTreeDataView(Subsystem);
					}
					else if (OwnerActor && Desc.Struct->IsChildOf(UActorComponent::StaticClass()))
					{
						UActorComponent* Component = OwnerActor->FindComponentByClass(Cast<UClass>(const_cast<UStruct*>(ToRawPtr(Desc.Struct))));
						OutDataViews[Index] = FStateTreeDataView(Component);
					}
					else if (Desc.Struct->IsChildOf(APawn::StaticClass()))
					{
						APawn* OwnerPawn = Cast<APawn>(OwnerActor);
						OutDataViews[Index] = FStateTreeDataView(OwnerPawn);
					}
					else if (Desc.Struct->IsChildOf(AAIController::StaticClass()))
					{
						AAIController* OwnerController = Cast<AAIController>(OwnerActor);
						OutDataViews[Index] = FStateTreeDataView(OwnerController);
					}
					else if (Desc.Struct->IsChildOf(AActor::StaticClass()))
					{
						OutDataViews[Index] = FStateTreeDataView(OwnerActor);
					}
				}
			}
			return true;
		})
	);

	bool bResult = Context.AreContextDataViewsValid();

	if (!bResult && bLogErrors)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Missing context data requirements. StateTree will not update."), ANSI_TO_TCHAR(__FUNCTION__));
	}

	return bResult;
}

void UGBWPowerfulStateComponent::SendStateTreeEvent(const FStateTreeEvent& Event)
{
	if (!StateTree.bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Trying to send even to a StateTree that is not started yet."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	for (int i = 0; i < StateTree.StateTreeSet.Num(); i++)
	{
		if (StateTree.StateTreeSet[i].IsValid())
		{
			FStateTreeExecutionContext Context(*GetOwner(), *StateTree.StateTreeSet[i].GetStateTree(), StateTree.InstanceDataSet[i]);
			if (SetContextRequirements(Context))
			{
				Context.SendEvent(Event.Tag, Event.Payload, Event.Origin);
			}
		}
	}
}

void UGBWPowerfulStateComponent::SendSTEvent(const FGameplayTag InTag, bool bToServer)
{
	if (!InTag.IsValid())
	{
		return;
	}

	if (!bToServer)
	{
		SendStateTreeEvent(FStateTreeEvent(InTag));
	}
	else
	{
		if (const int TagIndex = PowerfulState.GetGTKeyIndex(InTag,false); TagIndex == INDEX_NONE)
		{
			SendStateTreeEventToServer_Server(InTag);
		}
		else
		{
			SendStateTreeEventToServerFast_Server(TagIndex);
		}
	}
}

void UGBWPowerfulStateComponent::SendStateTreeEventToServer_Server_Implementation(const FGameplayTag& InTag)
{
	SendStateTreeEvent(FStateTreeEvent(InTag));
}

void UGBWPowerfulStateComponent::SendStateTreeEventToServerFast_Server_Implementation(const int EventTagIndex)
{
	SendStateTreeEvent(FStateTreeEvent(PowerfulState.GetGTKey(EventTagIndex)));
}

bool UGBWPowerfulStateComponent::HasNetAuth() const
{
	const EGBWActorNetType ActorNetType = UGBWPowerfulToolsFuncLib::GetActorNetType(GetOwner());
	return !(ActorNetType == EGBWActorNetType::InClient_NotPlayer || ActorNetType == EGBWActorNetType::InClient_RemotePlayer);
}

void UGBWPowerfulStateComponent::RemoveState(const FName Key)
{
	if (!HasNetAuth() || !Key.IsValid())
	{
		return;
	}

	if (const int KeyIndex = PowerfulState.GetKeyIndex(Key,false); KeyIndex != INDEX_NONE)
	{
		RemoveStateFast_Server(KeyIndex);
	}
	else
	{
		RemoveState_Server(Key);
	}
}

void UGBWPowerfulStateComponent::RemoveStateFast_Server_Implementation(const int KeyIndex)
{
	PowerfulState.RemoveState(KeyIndex,this->GetWorld()->GetTimeSeconds());
}

void UGBWPowerfulStateComponent::SetStateFloat(const FName Key, const float Value, const bool bReset)
{
	if (!HasNetAuth())
	{
		return;
	}
	float CurrentValue = 0.0f;
	float Time = -1.0f;
	if (GetFValue(Key,CurrentValue,Time) && CurrentValue == Value && !bReset)
	{
		return;
	}

	if (const int KeyIndex = PowerfulState.GetKeyIndex(Key,false); KeyIndex != INDEX_NONE)
	{
		SetStateFloatFast_Server(KeyIndex,Value,bReset);
	}
	else
	{
		SetStateFloat_Server(Key,Value,bReset);
	}
}
void UGBWPowerfulStateComponent::SetStateFloat_Server_Implementation(const FName Key, const float Value, const bool bReset)
{
	if (Key.IsValid())
	{
		PowerfulState.SetState(Key, Value, GetWorld()->GetTimeSeconds(), bReset);	
	}
}

void UGBWPowerfulStateComponent::SetStateFloatFast_Server_Implementation(const int KeyIndex, const float Value, const bool bReset)
{
	const FName Key = PowerfulState.GetKey(KeyIndex); 
	if (Key.IsValid())
	{
		PowerfulState.SetState(Key, Value, GetWorld()->GetTimeSeconds(), bReset);	
	}
}

void UGBWPowerfulStateComponent::SetStateVector(const FName Key, const FVector Value, const bool bReset)
{
	if (!HasNetAuth())
	{
		return;
	}
	FVector CurrentValue = FVector::ZeroVector;
	float Time = -1.0f;
	if (GetVValue(Key,CurrentValue,Time) && CurrentValue == Value && !bReset)
	{
		return;
	}

	if (const int KeyIndex = PowerfulState.GetKeyIndex(Key,false); KeyIndex != INDEX_NONE)
	{
		SetStateVectorFast_Server(KeyIndex,Value,bReset);
	}
	else
	{
		SetStateVector_Server(Key,Value,bReset);
	}
}
void UGBWPowerfulStateComponent::SetStateVector_Server_Implementation(const FName Key, const FVector Value, const bool bReset)
{
	if (Key.IsValid())
	{
		PowerfulState.SetState(Key, Value, GetWorld()->GetTimeSeconds(), bReset);	
	}
}

void UGBWPowerfulStateComponent::SetStateVectorFast_Server_Implementation(const int KeyIndex, const FVector Value,
                                                                          const bool bReset)
{
	const FName Key = PowerfulState.GetKey(KeyIndex); 
	if (Key.IsValid())
	{
		PowerfulState.SetState(Key, Value, GetWorld()->GetTimeSeconds(), bReset);	
	}
}

void UGBWPowerfulStateComponent::SetStateActor(const FName Key, AActor* Value, const bool bReset)
{
	if (!HasNetAuth())
	{
		return;
	}
	AActor* CurrentValue = nullptr;
	float Time = -1.0f;
	if (GetActorValue(Key,CurrentValue,Time) && CurrentValue == Value && !bReset)
	{
		return;
	}
	if (const int KeyIndex = PowerfulState.GetKeyIndex(Key,false); KeyIndex != INDEX_NONE)
	{
		SetStateActorFast_Server(KeyIndex, Value, bReset);
	}
	else
	{
		SetStateActor_Server(Key, Value, bReset);
	}
}
void UGBWPowerfulStateComponent::SetStateActor_Server_Implementation(const FName Key, AActor* Value, const bool bReset)
{
	if (Key.IsValid())
	{
		PowerfulState.SetState(Key, Value, bReset);	
	}
}

void UGBWPowerfulStateComponent::SetStateActorFast_Server_Implementation(const int KeyIndex, AActor* Value, const bool bReset)
{
	const FName Key = PowerfulState.GetKey(KeyIndex); 
	if (Key.IsValid())
	{
		PowerfulState.SetState(Key, Value, bReset);	
	}
}

void UGBWPowerfulStateComponent::SetStateText(const FName Key, const FString Value, const bool bReset)
{
	if (!HasNetAuth())
	{
		return;
	}
	FString CurrentValue = "";
	float Time = -1.0f;
	if (GetTextValue(Key,CurrentValue,Time) && CurrentValue == Value && !bReset)
	{
		return;
	}

	if (const int KeyIndex = PowerfulState.GetKeyIndex(Key,false); KeyIndex != INDEX_NONE)
	{
		SetStateTextFast_Server(KeyIndex, Value,bReset);
	}
	else
	{
		SetStateText_Server(Key, Value,bReset);
	}
}
void UGBWPowerfulStateComponent::SetStateText_Server_Implementation(const FName Key, const FString& Value, const bool bReset)
{
	if (Key.IsValid())
	{
		PowerfulState.SetState(Key, Value, GetWorld()->GetTimeSeconds(), bReset);	
	}
}

void UGBWPowerfulStateComponent::SetStateTextFast_Server_Implementation(const int Key, const FString& Value,
                                                                        const bool bReset)
{
	const FName KeyGT = PowerfulState.GetKey(Key); 
	if (KeyGT.IsValid())
	{
		PowerfulState.SetState(KeyGT, Value, GetWorld()->GetTimeSeconds(), bReset);	
	}
}

void UGBWPowerfulStateComponent::SetStateBool(const FName Key, const bool Value, const bool bReset)
{
	if (!HasNetAuth())
	{
		return;
	}
	bool CurrentValue = false;
	float Time = -1.0f;
	if (GetBoolValue(Key,CurrentValue,Time) && CurrentValue == Value && !bReset)
	{
		return;
	}

	if (const int KeyIndex = PowerfulState.GetKeyIndex(Key,false); KeyIndex != INDEX_NONE)
	{
		SetStateBoolFast_Server(KeyIndex, Value,bReset);
	}
	else
	{
		SetStateBool_Server(Key, Value,bReset);
	}
}

void UGBWPowerfulStateComponent::SetStateBool_Server_Implementation(const FName Key, const bool Value, const bool bReset)
{
	if (Key.IsValid())
	{
		PowerfulState.SetState(Key, Value, GetWorld()->GetTimeSeconds(), bReset);	
	}
}

void UGBWPowerfulStateComponent::RemoveState_Server_Implementation(const FName Key)
{
	PowerfulState.RemoveState(Key,this->GetWorld()->GetTimeSeconds());
}

void UGBWPowerfulStateComponent::SetStateBoolFast_Server_Implementation(const int Key, const bool Value, const bool bReset)
{
	const FName KeyGT = PowerfulState.GetKey(Key); 
	if (KeyGT.IsValid())
	{
		PowerfulState.SetState(KeyGT, Value, GetWorld()->GetTimeSeconds(), bReset);	
	}
}

bool UGBWPowerfulStateComponent::GetFValue(const FName Key, float& Result, float& Duration)
{
	float Time = -1.0f;
	if(PowerfulState.GetValue(Key,Result,Time))
	{
		Duration = this->GetWorld()->GetTimeSeconds() - Time;
		return true;
	}
	return false;
}

bool UGBWPowerfulStateComponent::GetVValue(const FName Key, FVector& Result, float& Duration)
{
	float Time = -1.0f;
	if(PowerfulState.GetValue(Key,Result,Time))
	{
		Duration = this->GetWorld()->GetTimeSeconds() - Time;
		return true;
	}
	return false;
}

bool UGBWPowerfulStateComponent::GetActorValue(const FName Key, AActor*& Result, float& Duration)
{
	float Time = -1.0f;
	if(PowerfulState.GetValue(Key,Result,Time))
	{
		Duration = this->GetWorld()->GetTimeSeconds() - Time;
		return true;
	}
	return false;
}

bool UGBWPowerfulStateComponent::GetTextValue(const FName Key, FString& Result, float& Duration)
{
	float Time = -1.0f;
	if(PowerfulState.GetValue(Key,Result,Time))
	{
		Duration = this->GetWorld()->GetTimeSeconds() - Time;
		return true;
	}
	return false;
}

bool UGBWPowerfulStateComponent::GetBoolValue(const FName Key, bool& Result, float& Duration)
{
	float Time = -1.0f;
	if(PowerfulState.GetValue(Key,Result,Time))
	{
		Duration = this->GetWorld()->GetTimeSeconds() - Time;
		return true;
	}
	return false;
}

bool UGBWPowerfulStateComponent::GetFValue_History(const FName Key, const int DistanceFromNow, float& Result,
                                                   float& Duration)
{
	return PowerfulState.GetValue_History(Key,DistanceFromNow,Result,Duration);
}

bool UGBWPowerfulStateComponent::GetVValue_History(const FName Key, const int DistanceFromNow, FVector& Result,
                                                   float& Duration)
{
	return PowerfulState.GetValue_History(Key,DistanceFromNow,Result,Duration);
}

bool UGBWPowerfulStateComponent::GetActorValue_History(const FName Key, const int DistanceFromNow, AActor*& Result,
                                                       float& Duration)
{
	return PowerfulState.GetValue_History(Key,DistanceFromNow,Result,Duration);
}

bool UGBWPowerfulStateComponent::GetTextValue_History(const FName Key, const int DistanceFromNow, FString& Result,
                                                      float& Duration)
{
	return PowerfulState.GetValue_History(Key,DistanceFromNow,Result,Duration);
}

bool UGBWPowerfulStateComponent::GetBoolValue_History(const FName Key, const int DistanceFromNow, bool& Result,
                                                      float& Duration)
{
	return PowerfulState.GetValue_History(Key,DistanceFromNow,Result,Duration);
}

bool UGBWPowerfulStateComponent::GetActionState(FName Slot, FPS_ActionStateItem& ResActionState, int& Index) const
{
	for (int i=0; i<ActionState.Num(); i++)
	{
		if (ActionState[i].Slot == Slot)
		{
			ResActionState = ActionState[i];
			Index = i;
			return true;
		}
	}

	return false;
}

void UGBWPowerfulStateComponent::AddActionLockState(FName Slot, FGameplayTagQuery ActionToLock)
{
	AddActionLockState_Server(Slot,ActionToLock);
}
void UGBWPowerfulStateComponent::AddActionLockState_Server_Implementation(FName Slot, FGameplayTagQuery ActionToLock)
{
	AddActionLockState_Imp(Slot,ActionToLock);
}
void UGBWPowerfulStateComponent::AddActionLockState_Imp(FName Slot,
	const FGameplayTagQuery& ActionToLock)
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		ActionState[Index].ActionLockState.bHasLock = !ActionToLock.IsEmpty();
		ActionState[Index].ActionLockState.ActionToLock.Add(ActionToLock);
	}
	else
	{
		ActionState.Add(FPS_ActionStateItem(
        		Slot,
        		FPS_ActionLockState(!ActionToLock.IsEmpty(), ActionToLock)
        	));
	}
}

void UGBWPowerfulStateComponent::SetActionLockStates(const FName Slot, bool bHasActionLock, TArray<FGameplayTagQuery> ActionToLock)
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		if (ActionState[Index].ActionLockState.bHasLock == bHasActionLock
			&& ActionState[Index].ActionLockState.IsActionToLockNoChange(ActionToLock))
		{
			return;
		}
	}
	SetActionLockStates_Server(Slot,bHasActionLock,ActionToLock);
}
void UGBWPowerfulStateComponent::SetActionLockStates_Server_Implementation(FName Slot, bool bHasActionLock,
	const TArray<FGameplayTagQuery>& ActionToLock)
{
	SetActionLockStates_Imp(Slot,bHasActionLock,ActionToLock);
}
void UGBWPowerfulStateComponent::SetActionLockStates_Imp(FName Slot, bool bHasActionLock,
                                                         const TArray<FGameplayTagQuery>& ActionToLock)
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		ActionState[Index].ActionLockState.bHasLock = bHasActionLock;
		if (bHasActionLock)
		{
			ActionState[Index].ActionLockState.ActionToLock = ActionToLock;
		}
		else
		{
			ActionState[Index].ActionLockState.ActionToLock.Empty();
		}
	}
	else
	{
		ActionState.Add(FPS_ActionStateItem(
				Slot,
				FPS_ActionLockState(bHasActionLock, ActionToLock)
			));
	}
}

void UGBWPowerfulStateComponent::RemoveActionLockState(FName Slot, FGameplayTagQuery ActionToLock)
{
	RemoveActionLockState_Server(Slot, ActionToLock);
}
void UGBWPowerfulStateComponent::RemoveActionLockState_Server_Implementation(FName Slot, FGameplayTagQuery ActionToLock)
{
	RemoveActionLockState_Imp(Slot, ActionToLock);
}
void UGBWPowerfulStateComponent::RemoveActionLockState_Imp(FName Slot, const FGameplayTagQuery& ActionToLock)
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		ActionState[Index].ActionLockState.RemoveActionLock(ActionToLock);
		if (ActionState[Index].ActionLockState.ActionToLock.IsEmpty())
		{
			ActionState[Index].ActionLockState.bHasLock = false;
		}
	}
}

bool UGBWPowerfulStateComponent::IsActionLock(const FName Slot, const FGameplayTagContainer ActionToLock) const
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		if (ResActionLockState.ActionLockState.ForceLockFrames>0 || ResActionLockState.ActionLockState.ForceLockTime>0)
		{
			return true;
		}
		if (ResActionLockState.ActionLockState.ActionToLock.IsEmpty())
		{
			return false;
		}
		return ResActionLockState.ActionLockState.bHasLock
			&& ActionToLock.IsValid()
			&& ResActionLockState.ActionLockState.IsLock(ActionToLock);
	}

	return false;
}

bool UGBWPowerfulStateComponent::IsAnyActionLock(const FName Slot) const
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		return ResActionLockState.ActionLockState.bHasLock
			|| ResActionLockState.ActionLockState.ForceLockFrames>0
			|| ResActionLockState.ActionLockState.ForceLockTime>0;
	}

	return false;
}

void UGBWPowerfulStateComponent::LockAllActionByTime(const FName Slot, const float LockTIme)
{
	LockAllActionByTime_Server(Slot, LockTIme);
}
void UGBWPowerfulStateComponent::LockAllActionByTime_Server_Implementation(const FName Slot, const float LockTIme)
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		ActionState[Index].ActionLockState.ForceLockTime = LockTIme;
	}
	else
	{
		ActionState.Add(FPS_ActionStateItem(
				Slot,
				FPS_ActionLockState(LockTIme)
			));
	}
}

void UGBWPowerfulStateComponent::LockAllActionByFrames(const FName Slot, const int LockFrames)
{
	LockAllActionByFrames_Server(Slot, LockFrames);
}

void UGBWPowerfulStateComponent::SetCurrentActionTag(const FName Slot, const FGameplayTag ActionTag)
{
	SetCurrentActionTag_Server(Slot, ActionTag);
}

void UGBWPowerfulStateComponent::SetCurrentActionTag_Server_Implementation(const FName Slot, const FGameplayTag ActionTag)
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		ActionState[Index].SetCurrentAction(ActionTag);
	}
	else
	{
		ActionState.Add(FPS_ActionStateItem(
				Slot,
				ActionTag
			));
	}
}

void UGBWPowerfulStateComponent::LockAllActionByFrames_Server_Implementation(const FName Slot, const int LockFrames)
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		ActionState[Index].ActionLockState.ForceLockFrames = LockFrames;
	}
	else
	{
		ActionState.Add(FPS_ActionStateItem(
				Slot,
				FPS_ActionLockState(LockFrames)
			));
	}
}

void UGBWPowerfulStateComponent::GetCurrentActionTag(FName Slot, bool& bIsGet, FGameplayTag& ActionTag) const
{
	FPS_ActionStateItem ResActionLockState;
	int Index;
	if (GetActionState(Slot, ResActionLockState, Index))
	{
		bIsGet = true;
		ActionTag = ResActionLockState.CurrentActionTag;
	}
	else
	{
		bIsGet = false;
		ActionTag = FGameplayTag();
	}
}

void UGBWPowerfulStateComponent::ActivePSDebug(const FColor InDebugTextColor, const bool InPrintToScreen, const bool InPrintToLog)
{
	if (!HasNetAuth())
	{
		return;
	}
	ActivePSDebug_Server(InDebugTextColor, InPrintToScreen,InPrintToLog);
}

void UGBWPowerfulStateComponent::ActivePSDebug_Server_Implementation(FColor InDebugTextColor, bool InPrintToScreen,
	bool InPrintToLog)
{
	bIsDebug = true;
	DebugTextColor = InDebugTextColor;
	bPrintToScreen = InPrintToScreen;
	bPrintToLog = InPrintToLog;
}

void UGBWPowerfulStateComponent::DeactivePSDebug()
{
	if (!HasNetAuth())
	{
		return;
	}
	DeactivePSDebug_Server();
}

void UGBWPowerfulStateComponent::DeactivePSDebug_Server_Implementation()
{
	bIsDebug = false;
}

void UGBWPowerfulStateComponent::PSDebug()
{
	if (bIsDebug)
	{
		const UWorld* World = GEngine->GetWorldFromContextObject(GetOwner(), EGetWorldErrorMode::ReturnNull);
		FString Prefix;
		if (World)
		{
			if (World->WorldType == EWorldType::PIE)
			{
				switch(World->GetNetMode())
				{
				case NM_Client:
					Prefix = TEXT("Client: ");
					break;
				case NM_DedicatedServer:
				case NM_ListenServer:
					Prefix = TEXT("Server: ");
					break;
				case NM_Standalone:
					break;
				default: ;
				}
			}
		}

		const FString DebugStr = ">>>>>>>>>>>>Powerful State Debug Start<<<<<<<<<<<<<<<\n(StateGuid : "
			+ PowerfulStateGuid.ToString() + ")"
			+ "\n(OwnerActor : " + GetOwner()->GetName() + ")"
			+ FString::Printf(TEXT("\n(LocalRole-%s   RemoteRole-%s)\n%s"),
						*UEnum::GetValueAsName(GetOwner()->GetLocalRole()).ToString(),
						*UEnum::GetValueAsName(GetOwner()->GetRemoteRole()).ToString(),
						*PowerfulState.GetString(GetOwner()->GetWorld(),StateOperationNum));
		
		UKismetSystemLibrary::PrintString(GetOwner()->GetWorld(),
			DebugStr,
			bPrintToScreen,bPrintToLog,DebugTextColor,0.0f,FName(Prefix + "PSState" + FString::Printf(TEXT("%d"), 0)));
	}
}
