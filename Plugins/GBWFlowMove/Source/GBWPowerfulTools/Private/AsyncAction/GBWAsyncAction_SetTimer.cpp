// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "AsyncAction/GBWAsyncAction_SetTimer.h"
#include "Engine.h"

UGBWAsyncAction_SetTimer::UGBWAsyncAction_SetTimer()
{
	WorldContextObject = nullptr;
}


UGBWAsyncAction_SetTimer::~UGBWAsyncAction_SetTimer()
{
	
}

UGBWAsyncAction_SetTimer* UGBWAsyncAction_SetTimer::SetTimer(const UObject* WorldContextObject,
	float Time,
	bool bLooping,
	float InitialStartDelay /*= 0.f*/,
	float InitialStartDelayVariance /*= 0.f*/,
	UObject* ParameterObject/*= nullptr*/)
{
	if (!WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute Set Timer."), ELogVerbosity::Error);
		return nullptr;
	}

	InitialStartDelay += FMath::RandRange(-InitialStartDelayVariance, InitialStartDelayVariance);
	if (Time <= 0.f || ((Time + InitialStartDelay) - InitialStartDelayVariance) < 0.f)
	{
		FFrame::KismetExecutionMessage(TEXT("SetTimer passed a negative or zero time.  The associated timer may fail to fire!  If using InitialStartDelayVariance, be sure it is smaller than (Time + InitialStartDelay)."), ELogVerbosity::Warning);
		return nullptr;
	}

	UGBWAsyncAction_SetTimer* AsyncNode = NewObject<UGBWAsyncAction_SetTimer>();
	AsyncNode->WorldContextObject = WorldContextObject;

	FTimerDynamicDelegate Delegate;
	Delegate.BindUFunction(AsyncNode, FName("CompletedEvent"));
	AsyncNode->TimerHandle = AsyncNode->SetTimerDelegate(Delegate, Time, bLooping, InitialStartDelay, InitialStartDelayVariance);

	AsyncNode->RegisterWithGameInstance((UObject*)WorldContextObject);

	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddUObject(AsyncNode, &UGBWAsyncAction_SetTimer::PreGarbageCollect);

	AsyncNode->ParameterObjectCache = ParameterObject;
	
	return AsyncNode;
}

FTimerHandle UGBWAsyncAction_SetTimer::SetTimerDelegate(FTimerDynamicDelegate Delegate,
	float Time,
	bool bLooping,
	float InitialStartDelay /*= 0.f*/,
	float InitialStartDelayVariance /*= 0.f*/)
{
	FTimerHandle Handle;
	if (Delegate.IsBound())
	{
		World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (World)
		{
			InitialStartDelay += FMath::RandRange(-InitialStartDelayVariance, InitialStartDelayVariance);
			if (Time <= 0.f || ((Time + InitialStartDelay) - InitialStartDelayVariance) < 0.f)
			{
				FFrame::KismetExecutionMessage(TEXT("SetTimer passed a negative or zero time.  The associated timer may fail to fire!  If using InitialStartDelayVariance, be sure it is smaller than (Time + InitialStartDelay)."), ELogVerbosity::Warning);
			}

			FTimerManager& TimerManager = World->GetTimerManager();
			Handle = TimerManager.K2_FindDynamicTimerHandle(Delegate);
			TimerManager.SetTimer(Handle, Delegate, Time, bLooping, (Time + InitialStartDelay));
		}
	}
	else
	{
		UE_LOG(LogBlueprintUserMessages, Warning,
			TEXT("SetTimer passed a bad function (%s) or object (%s)"),
			*Delegate.GetFunctionName().ToString(), *GetNameSafe(Delegate.GetUObject()));
	}

	return Handle;
}


void UGBWAsyncAction_SetTimer::PreGarbageCollect()
{
	if (bIsCompleted) 
	{
		SetReadyToDestroy();
	}
}

void UGBWAsyncAction_SetTimer::Stop()
{
	if(TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
    	TimerManager.ClearTimer(TimerHandle);
	}
	CompletedEvent();
}

void UGBWAsyncAction_SetTimer::Activate()
{
	if (!WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute Set Timer."), ELogVerbosity::Error);
		return;
	}

	Then.Broadcast(TimerHandle, ParameterObjectCache,Timer, FrameDeltaTime,this);
}


void UGBWAsyncAction_SetTimer::CompletedEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (GWorld->GetWorld()!=World)
	{
		ParameterObjectCache = nullptr;
	}
	else
	{
		if(ParameterObjectCache)
		{
			if(IsValid(ParameterObjectCache))
			{
				if (!((UObject*)ParameterObjectCache)->IsValidLowLevel())
				{
					ParameterObjectCache = nullptr;
				}
			}
			else
			{
				ParameterObjectCache = nullptr;
			}
		}
	}
	
	Completed.Broadcast(TimerHandle, ParameterObjectCache,Timer, FrameDeltaTime,this);
}

void UGBWAsyncAction_SetTimer::Tick(float DeltaTime)
{
	if ( LastFrameNumberWeTicked == GFrameCounter )
		return;

	if(bIsCompleted)
		return;

	Timer = Timer + DeltaTime;
	FrameDeltaTime = DeltaTime;
	Update.Broadcast(TimerHandle, ParameterObjectCache,Timer, FrameDeltaTime,this);
	
	LastFrameNumberWeTicked = GFrameCounter;
}