// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GBWPowerfulStateComponent.h"
#include "GBWPSFuncLib.generated.h"

UCLASS()
class GBWPOWERFULSTATE_API UGBWPSFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS")
	static UGBWPowerfulStateComponent* GetPowerfulStateComponent(AActor* InActor, bool bTryGetFromParentActor = false);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SendSTEvent(AActor* InActor, const FGameplayTag InTag, bool bToServer);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void RemovePowerfulState(AActor* InActor, bool bOnlyTriggerInServer, FName Key);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SetPSFloat(AActor* InActor, FName Key,float Value ,const bool bReset);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SetPSVector(AActor* InActor, FName Key,FVector Value,const bool bReset);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SetPSActor(AActor* InActor, FName Key,AActor* Value,const bool bReset);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SetPSText(AActor* InActor, FName Key, const FString Value,const bool bReset);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SetPSBool(AActor* InActor, FName Key,bool Value,const bool bReset);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSFValue(AActor* InActor, FName Key, bool& bIsGet, float& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSVValue(AActor* InActor, FName Key, bool& bIsGet, FVector& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSActorValue(AActor* InActor, FName Key, bool& bIsGet, AActor*& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSTextValue(AActor* InActor, FName Key, bool& bIsGet, FString& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSBoolValue(AActor* InActor, FName Key, bool& bIsGet, bool& Result, float& Duration);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSFValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet, float& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSVValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet, FVector& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSActorValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet, AActor*& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSTextValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet, FString& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetPSBoolValue_History(AActor* InActor, FName Key, int DistanceFromNow, bool& bIsGet, bool& Result, float& Duration);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void ActivePSDebug(AActor* InActor, FColor InDebugTextColor = FColor::Green, bool InPrintToScreen = true, bool InPrintToLog = true);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void DeactivePSDebug(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static bool PS_GetActionState(AActor* InActor, FName Slot, FPS_ActionStateItem& ResActionState);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void PS_AddActionLockState(
		AActor* InActor, 
		FName Slot,
		FGameplayTagQuery ActionToLock);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void PS_SetActionLockStates(
		AActor* InActor, 
		FName Slot,
		bool bHasActionLock,
		TArray<FGameplayTagQuery> ActionToLock);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void RemoveActionLockState(
		AActor* InActor,
		FName Slot,
		FGameplayTagQuery ActionToLock);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void PS_LockAllActionByTime(AActor* InActor, FName Slot, float LockTIme = 0.1f);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void PS_LockAllActionByFrames(AActor* InActor, FName Slot, int LockFrames = 3);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void PS_SetCurrentActionTag(AActor* InActor, FName Slot, FGameplayTag ActionTag);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static bool PS_IsActionLock(AActor* InActor, FName Slot, FGameplayTagContainer ActionToLock);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static bool PS_IsAnyActionLock(AActor* InActor, FName Slot);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void PS_GetCurrentActionTag(AActor* InActor, FName Slot, bool& bIsGet, FGameplayTag& ActionTag);

//Input Start
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static void PS_KeyPressed(AActor* InActor, FName Key);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static void PS_KeyReleased(AActor* InActor, FName Key);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static void PS_KeyDisable(AActor* InActor, FName Key);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static bool PS_IsKeyPressed(AActor* InActor, FName Key, float& Duration, float MinDuration=0.0f, float MaxDuration=0.0f);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static bool PS_IsKeyReleased(AActor* InActor, FName Key, float& Duration, float MinDuration=0.0f, float MaxDuration=0.0f);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static bool PS_IsKeyClick(AActor* InActor, FName Key, float& Duration, float MaxFinishedDuration=0.3f);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static bool PS_IsKeyDoubleClick(AActor* InActor, FName Key, float& Duration, float MaxFinishedDuration=0.15f);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static bool PS_IsKeyHold(AActor* InActor, FName Key, float& Duration, float MinDuration=0.0f);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static bool PS_IsKeyClickAndHold(AActor* InActor, FName Key, float& Duration, float ClickMaxFinishedDuration=0.2f, float MinDuration=0.0f);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static bool PS_IsKeyDoubleClickAndHold(AActor* InActor, FName Key, float& Duration, float ClickMaxFinishedDuration=0.2f, float MinDuration=0.0f);
//Input End

//StateSwitch Start
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static void PS_SwitchGetBool(AActor* InActor, FName Key, bool& bIsReset, bool& OldValue, float& OldValueDuration, bool& NewValue, bool InitValue=false, float DurationToReset=0.0f);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|KeyInput", meta=(DefaultToSelf="InActor"))
	static void PS_SwitchGetInt(AActor* InActor, FName Key, bool& bIsReset, int& OldValue, float& OldValueDuration, int& NewValue, int MinValue=0, int MaxValue=3, float DurationToReset=0.0f);
//StateSwitch End

	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void AddPSFloat(AActor* InActor, FName Key, float Value);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void GetVectorFromPSFState(
		AActor* InActor,
		FName X_Key,
		FName Y_Key,
		FName Z_Key,
		FVector& Result);
	
	//This method will set the Bool state represented by TargetKey to True
	//and set all Bool states in the BoolKeySet except for TargetKey to False
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void UniqueSetPSBool(AActor* InActor, FName TargetKey, TSet<FName> BoolKeySet);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SwitchPS3BoolState_UP(
		AActor* InActor,
		FName StateKey1,
		FName StateKey2,
		FName StateKey3,
		FName& CurrentState);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SwitchPS3BoolState_Down(
		AActor* InActor,
		FName StateKey1,
		FName StateKey2,
		FName StateKey3,
		FName& CurrentState);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SwitchPS4BoolState_UP(
		AActor* InActor,
		FName StateKey1,
		FName StateKey2,
		FName StateKey3,
		FName StateKey4,
		FName& CurrentState);
	UFUNCTION(BlueprintCallable, Category = "GBW|PS", meta=(DefaultToSelf="InActor"))
	static void SwitchPS4BoolState_Down(
		AActor* InActor,
		FName StateKey1,
		FName StateKey2,
		FName StateKey3,
		FName StateKey4,
		FName& CurrentState);
};
