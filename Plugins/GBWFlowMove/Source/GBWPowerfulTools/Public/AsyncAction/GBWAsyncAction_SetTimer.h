// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Tickable.h"
#include "Engine/EngineTypes.h"
#include "GBWAsyncAction_SetTimer.generated.h"

UCLASS(meta = (HideThen = true))
class GBWPOWERFULTOOLS_API UGBWAsyncAction_SetTimer : public UBlueprintAsyncActionBase, public FTickableGameObject
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FTimerHandleDelegate,
		FTimerHandle, TimerHandle,
		UObject*, ParameterObject,
		float, Time,
		float, DeltaTime,
		UGBWAsyncAction_SetTimer*,TimerTask);
	GENERATED_BODY()

public:
	UGBWAsyncAction_SetTimer();
	~UGBWAsyncAction_SetTimer();

	// FTickableGameObject Begin
	virtual void Tick( float DeltaTime ) override;
	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT( FMyTickableThing, STATGROUP_Tickables );
	}
	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}
	virtual bool IsTickableInEditor() const
	{
		return false;
	}
	// FTickableGameObject End


public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject",
		DisplayName = "GBWSetTimer",
		ScriptName = "GBWSetTimer",
		AdvancedDisplay = "InitialStartDelay, InitialStartDelayVariance"),
		Category = "GBW|Tools|AsyncAction")
		static UGBWAsyncAction_SetTimer* SetTimer(const UObject* WorldContextObject,
			float Time,
			bool bLooping,
			float InitialStartDelay = 0.f,
			float InitialStartDelayVariance = 0.f,
			UObject* ParameterObject=nullptr);

public: 
	
	UPROPERTY(BlueprintAssignable)
		FTimerHandleDelegate Then;

	UPROPERTY(BlueprintAssignable)
		FTimerHandleDelegate Update;
	
	UPROPERTY(BlueprintAssignable)
		FTimerHandleDelegate Completed;

	UFUNCTION(BlueprintCallable,Category = "SetTimer")
		void Stop();

private:

	virtual void Activate() override;
	
	UFUNCTION()
		void CompletedEvent();
	
	FTimerHandle SetTimerDelegate(FTimerDynamicDelegate Delegate,
		float Time,
		bool bLooping,
		float InitialStartDelay = 0.f,
		float InitialStartDelayVariance = 0.f);
	
	void PreGarbageCollect();

	// The last frame number we were ticked.
	// We don't want to tick multiple times per frame 
	uint32 LastFrameNumberWeTicked = INDEX_NONE;

private:
	const UObject* WorldContextObject;
	FTimerHandle TimerHandle;
	UObject* ParameterObjectCache;
	float Timer = 0.0f;
	float FrameDeltaTime = 0.0f;
	bool bIsCompleted = false;
	
	UWorld* World;
};