// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "GBWPSFuncLib.h"
#include "Animation/AnimInstance.h"

UGBWPowerfulStateComponent* UGBWPSFuncLib::GetPowerfulStateComponent(AActor* InActor, bool bTryGetFromParentActor)
{
	if (!InActor)
	{
		return nullptr;
	}

	UActorComponent* AComp = InActor->GetComponentByClass(UGBWPowerfulStateComponent::StaticClass());
	if (!AComp)
	{
		if (bTryGetFromParentActor)
		{
			AActor* PActor = InActor;
			while (PActor && !AComp)
			{
				PActor = PActor->GetParentActor();
				if (PActor)
				{
					AComp = PActor->GetComponentByClass(UGBWPowerfulStateComponent::StaticClass());	
				}
			}

			PActor = InActor;
			while (PActor && !AComp)
			{
				PActor = PActor->GetAttachParentActor();
				if (PActor)
				{
					AComp = PActor->GetComponentByClass(UGBWPowerfulStateComponent::StaticClass());
				}
			}
		}
	}

	if (UGBWPowerfulStateComponent* PSComp = Cast<UGBWPowerfulStateComponent>(AComp))
	{
		return PSComp;
	}

	return nullptr;
}

void UGBWPSFuncLib::SendSTEvent(AActor* InActor, const FGameplayTag InTag, bool bToServer)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SendSTEvent(InTag, bToServer);
	}
}

void UGBWPSFuncLib::RemovePowerfulState(AActor* InActor, bool bOnlyTriggerInServer, FName Key)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		if (bOnlyTriggerInServer && !InActor->HasAuthority())
		{
			return;
		}
		PSC->RemoveState(Key);
	}
}

void UGBWPSFuncLib::SetPSFloat(AActor* InActor, FName Key, float Value,const bool bReset)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SetStateFloat(Key,Value,bReset);
	}
}

void UGBWPSFuncLib::SetPSVector(AActor* InActor, FName Key, FVector Value,const bool bReset)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SetStateVector(Key,Value,bReset);
	}
}

void UGBWPSFuncLib::SetPSActor(AActor* InActor, FName Key, AActor* Value,const bool bReset)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SetStateActor(Key,Value,bReset);
	}
}

void UGBWPSFuncLib::SetPSText(AActor* InActor, FName Key, const FString Value,const bool bReset)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SetStateText(Key,Value,bReset);
	}
}

void UGBWPSFuncLib::SetPSBool(AActor* InActor, FName Key, bool Value,const bool bReset)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SetStateBool(Key,Value,bReset);
	}
}

void UGBWPSFuncLib::GetPSFValue(AActor* InActor, FName Key, bool& bIsGet, float& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetFValue(Key,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSVValue(AActor* InActor, FName Key, bool& bIsGet, FVector& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetVValue(Key,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSActorValue(AActor* InActor, FName Key, bool& bIsGet, AActor*& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetActorValue(Key,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSTextValue(AActor* InActor, FName Key, bool& bIsGet, FString& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetTextValue(Key,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSBoolValue(AActor* InActor, FName Key, bool& bIsGet, bool& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetBoolValue(Key,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSFValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet,
                                        float& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetFValue_History(Key,DistanceFromNow,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSVValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet,
                                        FVector& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetVValue_History(Key,DistanceFromNow,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSActorValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet,
                                            AActor*& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetActorValue_History(Key,DistanceFromNow,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSTextValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet,
                                           FString& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetTextValue_History(Key,DistanceFromNow,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::GetPSBoolValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet,
                                           bool& Result, float& Duration)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bIsGet = PSC->GetBoolValue_History(Key,DistanceFromNow,Result,Duration);
		return;
	}

	bIsGet = false;
}

void UGBWPSFuncLib::ActivePSDebug(AActor* InActor, FColor InDebugTextColor, bool InPrintToScreen, bool InPrintToLog)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		return PSC->ActivePSDebug(InDebugTextColor,InPrintToScreen,InPrintToLog);
	}
}

void UGBWPSFuncLib::DeactivePSDebug(AActor* InActor)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		return PSC->DeactivePSDebug();
	}
}

bool UGBWPSFuncLib::PS_GetActionState(AActor* InActor, const FName Slot, FPS_ActionStateItem& ResActionState)
{
	if (const UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		int Index = 0;
		return PSC->GetActionState(Slot,ResActionState,Index);
	}

	return false;
}

void UGBWPSFuncLib::PS_AddActionLockState(AActor* InActor, FName Slot, FGameplayTagQuery ActionToLock)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->AddActionLockState(Slot,ActionToLock);
	}
}

void UGBWPSFuncLib::PS_SetActionLockStates(AActor* InActor, FName Slot, bool bHasActionLock,
	TArray<FGameplayTagQuery> ActionToLock)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SetActionLockStates(Slot,bHasActionLock,ActionToLock);
	}
}

void UGBWPSFuncLib::RemoveActionLockState(AActor* InActor, FName Slot, FGameplayTagQuery ActionToLock)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->RemoveActionLockState(Slot,ActionToLock);
	}
}

void UGBWPSFuncLib::PS_LockAllActionByTime(AActor* InActor, FName Slot, float LockTIme)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->LockAllActionByTime(Slot,LockTIme);
	}
}

void UGBWPSFuncLib::PS_LockAllActionByFrames(AActor* InActor, FName Slot, int LockFrames)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->LockAllActionByFrames(Slot,LockFrames);
	}
}

void UGBWPSFuncLib::PS_SetCurrentActionTag(AActor* InActor, FName Slot, FGameplayTag ActionTag)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SetCurrentActionTag(Slot, ActionTag);
	}
}

bool UGBWPSFuncLib::PS_IsActionLock(AActor* InActor, FName Slot, FGameplayTagContainer ActionToLock)
{
	if (const UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		return PSC->IsActionLock(Slot, ActionToLock);
	}
	return false;
}

bool UGBWPSFuncLib::PS_IsAnyActionLock(AActor* InActor, FName Slot)
{
	if (const UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		return PSC->IsAnyActionLock(Slot);
	}
	return false;
}

void UGBWPSFuncLib::PS_GetCurrentActionTag(AActor* InActor, FName Slot, bool& bIsGet, FGameplayTag& ActionTag)
{
	if (const UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->GetCurrentActionTag(Slot, bIsGet, ActionTag);
	}
}

void UGBWPSFuncLib::PS_KeyPressed(AActor* InActor, const FName Key)
{
	UGBWPSFuncLib::SetPSBool(InActor,Key,true,true);
}

void UGBWPSFuncLib::PS_KeyReleased(AActor* InActor, const FName Key)
{
	UGBWPSFuncLib::SetPSBool(InActor,Key,false,true);
}

void UGBWPSFuncLib::PS_KeyDisable(AActor* InActor, const FName Key)
{
	UGBWPSFuncLib::RemovePowerfulState(InActor,false,Key);
}

bool UGBWPSFuncLib::PS_IsKeyPressed(AActor* InActor, FName Key, float& Duration, float MinDuration,
                                    float MaxDuration)
{
	bool bIsGet_Current = false;
	bool Result_Current = false;
	float Duration_Current = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue(InActor,Key,bIsGet_Current,Result_Current,Duration_Current);

	if (bIsGet_Current && Result_Current)
	{
		Duration = Duration_Current;
		if (Duration_Current < MinDuration)
		{
			return false;
		}
		if (MaxDuration>0 && Duration_Current>MaxDuration)
		{
			return false;
		}

		return true;
	}

	return false;
}

bool UGBWPSFuncLib::PS_IsKeyReleased(AActor* InActor, FName Key, float& Duration, float MinDuration,
	float MaxDuration)
{
	bool bIsGet_Current = false;
	bool Result_Current = false;
	float Duration_Current = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue(InActor,Key,bIsGet_Current,Result_Current,Duration_Current);

	if (bIsGet_Current && !Result_Current)
	{
		Duration = Duration_Current;
		if (Duration_Current < MinDuration)
		{
			return false;
		}
		if (MaxDuration>0 && Duration_Current>MaxDuration)
		{
			return false;
		}

		return true;
	}

	return false;
}

bool UGBWPSFuncLib::PS_IsKeyClick(AActor* InActor, FName Key, float& Duration, float MaxFinishedDuration)
{
	if (UGBWPSFuncLib::PS_IsKeyDoubleClick(InActor,Key,Duration,MaxFinishedDuration))
	{
		return false;
	}
	
	bool bIsGet_Current = false;
	bool Result_Current = false;
	float Duration_Current = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue(InActor,Key,bIsGet_Current,Result_Current,Duration_Current);

	bool bIsGet_History = false;
	bool Result_History = false;
	float Duration_History = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,1,bIsGet_History,Result_History,Duration_History);

	Duration = Duration_Current;
	
	return (bIsGet_Current && !Result_Current && Duration_Current <= MaxFinishedDuration
		&& bIsGet_History && Result_History && Duration_History <= MaxFinishedDuration);
}

bool UGBWPSFuncLib::PS_IsKeyDoubleClick(AActor* InActor, FName Key, float& Duration, float MaxFinishedDuration)
{
	bool bIsGet_Current = false;
	bool Result_Current = false;
	float Duration_Current = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue(InActor,Key,bIsGet_Current,Result_Current,Duration_Current);

	bool bIsGet_History = false;
	bool Result_History = false;
	float Duration_History = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,1,bIsGet_History,Result_History,Duration_History);

	bool bIsGet_History2 = false;
	bool Result_History2 = false;
	float Duration_History2 = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,2,bIsGet_History2,Result_History2,Duration_History2);

	bool bIsGet_History3 = false;
	bool Result_History3 = false;
	float Duration_History3 = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,3,bIsGet_History3,Result_History3,Duration_History3);

	Duration = Duration_Current;
	
	return (bIsGet_Current && !Result_Current && Duration_Current <= MaxFinishedDuration
		&& bIsGet_History && Result_History && Duration_History <= MaxFinishedDuration
		&& bIsGet_History2 && !Result_History2 && Duration_History2 <= MaxFinishedDuration
		&& bIsGet_History3 && Result_History3 && Duration_History3 <= MaxFinishedDuration);
}

bool UGBWPSFuncLib::PS_IsKeyHold(AActor* InActor, FName Key, float& Duration, float MinDuration)
{
	bool bIsGet_Current = false;
	bool Result_Current = false;
	float Duration_Current = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue(InActor,Key,bIsGet_Current,Result_Current,Duration_Current);

	if (bIsGet_Current && Result_Current)
	{
		Duration = Duration_Current;
		if (Duration_Current < MinDuration)
		{
			return false;
		}

		return true;
	}

	return false;
}

bool UGBWPSFuncLib::PS_IsKeyClickAndHold(AActor* InActor, FName Key, float& Duration,
	float ClickMaxFinishedDuration, float MinDuration)
{
	if (!UGBWPSFuncLib::PS_IsKeyHold(InActor,Key,Duration,MinDuration))
	{
		return false;
	}

	bool bIsGet_History = false;
	bool Result_History = false;
	float Duration_History = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,1,bIsGet_History,Result_History,Duration_History);

	bool bIsGet_History2 = false;
	bool Result_History2 = false;
	float Duration_History2 = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,2,bIsGet_History2,Result_History2,Duration_History2);

	bool bIsGet_History3 = false;
	bool Result_History3 = false;
	float Duration_History3 = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,3,bIsGet_History3,Result_History3,Duration_History3);

	return (bIsGet_History && !Result_History && Duration_History<=ClickMaxFinishedDuration
			&& bIsGet_History2 && Result_History2 && Duration_History2<=ClickMaxFinishedDuration
			&& (!bIsGet_History3 ||
				(bIsGet_History3 && !Result_History3 && Duration_History3 > ClickMaxFinishedDuration)));
}

bool UGBWPSFuncLib::PS_IsKeyDoubleClickAndHold(AActor* InActor, FName Key, float& Duration,
	float ClickMaxFinishedDuration, float MinDuration)
{
	if (!UGBWPSFuncLib::PS_IsKeyHold(InActor,Key,Duration,MinDuration))
	{
		return false;
	}

	bool bIsGet_History = false;
	bool Result_History = false;
	float Duration_History = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,1,bIsGet_History,Result_History,Duration_History);

	bool bIsGet_History2 = false;
	bool Result_History2 = false;
	float Duration_History2 = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,2,bIsGet_History2,Result_History2,Duration_History2);

	bool bIsGet_History3 = false;
	bool Result_History3 = false;
	float Duration_History3 = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,3,bIsGet_History3,Result_History3,Duration_History3);

	bool bIsGet_History4 = false;
	bool Result_History4 = false;
	float Duration_History4 = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue_History(InActor,Key,4,bIsGet_History4,Result_History4,Duration_History4);

	return (bIsGet_History && !Result_History && Duration_History<=ClickMaxFinishedDuration
			&& bIsGet_History2 && Result_History2 && Duration_History2<=ClickMaxFinishedDuration
			&& bIsGet_History3 && !Result_History3 && Duration_History3<=ClickMaxFinishedDuration
			&& bIsGet_History4 && Result_History4 && Duration_History4<=ClickMaxFinishedDuration);
}

void UGBWPSFuncLib::PS_SwitchGetBool(AActor* InActor, const FName Key, bool& bIsReset, bool& OldValue, float& OldValueDuration, bool& NewValue, bool InitValue, const float DurationToReset)
{
	bool bIsGet_Current = false;
	bool Result_Current = false;
	float Duration_Current = 0.0f;
	UGBWPSFuncLib::GetPSBoolValue(InActor,Key,bIsGet_Current,Result_Current,Duration_Current);

	if (bIsGet_Current && (DurationToReset<=0 || Duration_Current <= DurationToReset))
	{
		bIsReset = false;
		OldValue = Result_Current;
		NewValue = !Result_Current;
		OldValueDuration = Duration_Current;
	}
	else
	{
		bIsReset = true;
		OldValue = InitValue;
		NewValue = InitValue;
		OldValueDuration = 0.0f;
	}

	UGBWPSFuncLib::SetPSBool(InActor,Key,NewValue,true);
}

void UGBWPSFuncLib::PS_SwitchGetInt(AActor* InActor, const FName Key, bool& bIsReset, int& OldValue, float& OldValueDuration,
	int& NewValue, const int MinValue, const int MaxValue, const float DurationToReset)
{
	bool bIsGet_Current = false;
	float Result_Current = false;
	float Duration_Current = 0.0f;
	UGBWPSFuncLib::GetPSFValue(InActor,Key,bIsGet_Current,Result_Current,Duration_Current);
	
	if (bIsGet_Current && (DurationToReset<=0 || Duration_Current <= DurationToReset))
	{
		bIsReset = false;
		OldValue = Result_Current;
		NewValue = Result_Current+1;
		if (NewValue>MaxValue)
		{
			NewValue = MinValue;
		}
		OldValueDuration = Duration_Current;
	}
	else
	{
		bIsReset = true;
		OldValue = MinValue;
		NewValue = MinValue;
		OldValueDuration = 0.0f;
	}

	UGBWPSFuncLib::SetPSFloat(InActor,Key,NewValue,true);
}

void UGBWPSFuncLib::AddPSFloat(AActor* InActor, const FName Key, const float Value)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		float OldValue = 0.0f;
		float Duration = 0.0f;
		PSC->GetFValue(Key,OldValue,Duration);
		
		PSC->SetStateFloat(Key,OldValue+Value,false);
	}
}

void UGBWPSFuncLib::GetVectorFromPSFState(
	AActor* InActor,
	FName X_Key, FName Y_Key, FName Z_Key,
	FVector& Result)
{
	Result = FVector::ZeroVector;
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		float XValue = 0.0f;
		float YValue = 0.0f;
		float ZValue = 0.0f;
		float Duration = 0.0f;
		PSC->GetFValue(X_Key,XValue,Duration);
		PSC->GetFValue(Y_Key,YValue,Duration);
		PSC->GetFValue(Z_Key,ZValue,Duration);

		Result = FVector(XValue,YValue,ZValue);
	}
}

void UGBWPSFuncLib::UniqueSetPSBool(AActor* InActor, const FName TargetKey, const TSet<FName> BoolKeySet)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		PSC->SetStateBool(TargetKey,true,false);

		for (auto Element : BoolKeySet)
		{
			if (Element != TargetKey)
			{
				PSC->SetStateBool(Element,false,false);
			}
		}
	}
}

void UGBWPSFuncLib::SwitchPS3BoolState_UP(
	AActor* InActor,
	const FName StateKey1,
	const FName StateKey2,
	const FName StateKey3,
	FName& CurrentState)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bool Result = false;
		float Duration = 0.0f;
		PSC->GetBoolValue(StateKey1, Result, Duration);
		if (Result)
		{
			PSC->SetStateBool(StateKey1,false,false);
			PSC->SetStateBool(StateKey2,true,false);
			PSC->SetStateBool(StateKey3,false,false);
			CurrentState = StateKey2;
			return;
		}
		else
		{
			Result = false;
			PSC->GetBoolValue(StateKey2, Result, Duration);
			if (Result)
			{
				PSC->SetStateBool(StateKey1,false,false);
				PSC->SetStateBool(StateKey2,false,false);
				PSC->SetStateBool(StateKey3,true,false);
				CurrentState = StateKey3;
				return;
			}
			else
			{
				Result = false;
				PSC->GetBoolValue(StateKey3, Result, Duration);
				if (!Result)
				{
					PSC->SetStateBool(StateKey1,false,false);
					PSC->SetStateBool(StateKey2,false,false);
					PSC->SetStateBool(StateKey3,true,false);
				}
				CurrentState = StateKey3;
				return;
			}
		}
	}

	CurrentState = NAME_None;
}

void UGBWPSFuncLib::SwitchPS3BoolState_Down(AActor* InActor, FName StateKey1, FName StateKey2,
	FName StateKey3, FName& CurrentState)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bool Result = false;
		float Duration = 0.0f;
		PSC->GetBoolValue(StateKey3, Result, Duration);
		if (Result)
		{
			PSC->SetStateBool(StateKey1,false,false);
			PSC->SetStateBool(StateKey2,true,false);
			PSC->SetStateBool(StateKey3,false,false);
			CurrentState = StateKey2;
			return;
		}
		else
		{
			Result = false;
			PSC->GetBoolValue(StateKey2, Result, Duration);
			if (Result)
			{
				PSC->SetStateBool(StateKey1,true,false);
				PSC->SetStateBool(StateKey2,false,false);
				PSC->SetStateBool(StateKey3,false,false);
				CurrentState = StateKey1;
				return;
			}
			else
			{
				Result = false;
				PSC->GetBoolValue(StateKey1, Result, Duration);
				if (!Result)
				{
					PSC->SetStateBool(StateKey1,true,false);
					PSC->SetStateBool(StateKey2,false,false);
					PSC->SetStateBool(StateKey3,false,false);
				}
				CurrentState = StateKey1;
				return;
			}
		}
	}

	CurrentState = NAME_None;
}

void UGBWPSFuncLib::SwitchPS4BoolState_UP(AActor* InActor, FName StateKey1, FName StateKey2,
	FName StateKey3, FName StateKey4, FName& CurrentState)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bool Result = false;
		float Duration = 0.0f;
		PSC->GetBoolValue(StateKey1, Result, Duration);
		if (Result)
		{
			PSC->SetStateBool(StateKey1,false,false);
			PSC->SetStateBool(StateKey2,true,false);
			PSC->SetStateBool(StateKey3,false,false);
			PSC->SetStateBool(StateKey4,false,false);
			CurrentState = StateKey2;
			return;
		}
		else
		{
			Result = false;
			PSC->GetBoolValue(StateKey2, Result, Duration);
			if (Result)
			{
				PSC->SetStateBool(StateKey1,false,false);
				PSC->SetStateBool(StateKey2,false,false);
				PSC->SetStateBool(StateKey3,true,false);
				PSC->SetStateBool(StateKey4,false,false);
				CurrentState = StateKey3;
				return;
			}
			else
			{
				Result = false;
				PSC->GetBoolValue(StateKey3, Result, Duration);
				if (Result)
				{
					PSC->SetStateBool(StateKey1,false,false);
					PSC->SetStateBool(StateKey2,false,false);
					PSC->SetStateBool(StateKey3,false,false);
					PSC->SetStateBool(StateKey4,true,false);
					CurrentState = StateKey4;
					return;
				}
				else
				{
					Result = false;
					PSC->GetBoolValue(StateKey4, Result, Duration);
					if (!Result)
					{
						PSC->SetStateBool(StateKey1,false,false);
						PSC->SetStateBool(StateKey2,false,false);
						PSC->SetStateBool(StateKey3,false,false);
						PSC->SetStateBool(StateKey4,true,false);
					}
					CurrentState = StateKey4;
					return;
				}
			}
		}
	}

	CurrentState = NAME_None;
}

void UGBWPSFuncLib::SwitchPS4BoolState_Down(AActor* InActor, FName StateKey1, FName StateKey2,
	FName StateKey3, FName StateKey4, FName& CurrentState)
{
	if (UGBWPowerfulStateComponent* PSC = GetPowerfulStateComponent(InActor))
	{
		bool Result = false;
		float Duration = 0.0f;
		PSC->GetBoolValue(StateKey4, Result, Duration);
		if (Result)
		{
			PSC->SetStateBool(StateKey1,false,false);
			PSC->SetStateBool(StateKey2,false,false);
			PSC->SetStateBool(StateKey3,true,false);
			PSC->SetStateBool(StateKey4,false,false);
			CurrentState = StateKey3;
			return;
		}
		else
		{
			PSC->GetBoolValue(StateKey3, Result, Duration);
			if (Result)
			{
				PSC->SetStateBool(StateKey1,false,false);
				PSC->SetStateBool(StateKey2,true,false);
				PSC->SetStateBool(StateKey3,false,false);
				PSC->SetStateBool(StateKey4,false,false);
				CurrentState = StateKey2;
				return;
			}
			else
			{
				Result = false;
				PSC->GetBoolValue(StateKey2, Result, Duration);
				if (Result)
				{
					PSC->SetStateBool(StateKey1,true,false);
					PSC->SetStateBool(StateKey2,false,false);
					PSC->SetStateBool(StateKey3,false,false);
					PSC->SetStateBool(StateKey4,false,false);
					CurrentState = StateKey1;
					return;
				}
				else
				{
					Result = false;
					PSC->GetBoolValue(StateKey1, Result, Duration);
					if (!Result)
					{
						PSC->SetStateBool(StateKey1,true,false);
						PSC->SetStateBool(StateKey2,false,false);
						PSC->SetStateBool(StateKey3,false,false);
						PSC->SetStateBool(StateKey4,false,false);
					}
					CurrentState = StateKey1;
					return;
				}
			}	
		}
	}

	CurrentState = NAME_None;
}
