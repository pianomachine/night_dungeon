// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "GBWPTBlueprintAsyncActionBase.h"

#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"

UGBWPTAsync_LoadStreamLevelByName::UGBWPTAsync_LoadStreamLevelByName()
{
	WorldContextObject = nullptr;
}

UGBWPTAsync_LoadStreamLevelByName::~UGBWPTAsync_LoadStreamLevelByName()
{
	
}

void UGBWPTAsync_LoadStreamLevelByName::Tick(float DeltaTime)
{
	if ( LastFrameNumberWeTicked == GFrameCounter )
		return;

	if(bIsCompleted)
	{
		this->ConditionalBeginDestroy();
		return;
	}
	
	Timer = Timer + DeltaTime;
	FrameDeltaTime = DeltaTime;
	LastFrameNumberWeTicked = GFrameCounter;

	if (!StreamLevelInstanceNow)
	{
		Failed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
		FailedEvent();
		return;
	}
	
	if (!StreamLevelInstanceNow->HasLoadRequestPending())
	{
		StreamLevelInstanceNow->SetShouldBeLoaded(true);
		StreamLevelInstanceNow->SetShouldBeVisible(bMakeVisibleAfterLoad);
	}

	if (StreamLevelInstanceNow->IsLevelLoaded())
	{
		Completed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
		CompletedEvent();
	}
	else
	{
		Update.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
	}
}

ETickableTickType UGBWPTAsync_LoadStreamLevelByName::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

TStatId UGBWPTAsync_LoadStreamLevelByName::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( FMyTickableThing, STATGROUP_Tickables );
}

bool UGBWPTAsync_LoadStreamLevelByName::IsTickableWhenPaused() const
{
	return true;
}

bool UGBWPTAsync_LoadStreamLevelByName::IsTickableInEditor() const
{
	return true;
}

UGBWPTAsync_LoadStreamLevelByName* UGBWPTAsync_LoadStreamLevelByName::PT_LoadStreamLevelByName(
	const UObject* WorldContextObject,
	FName LevelName,
	bool bMakeVisibleAfterLoad,
	FTransform LevelTransform,
	float Timeout)
{
	if (!WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute LoadStreamLevelByName."), ELogVerbosity::Error);
		return nullptr;
	}
	
	UGBWPTAsync_LoadStreamLevelByName* AsyncNode = NewObject<UGBWPTAsync_LoadStreamLevelByName>();
	AsyncNode->WorldContextObject = WorldContextObject->GetWorld();
	AsyncNode->StreamLevelInstanceNow = UGameplayStatics::GetStreamingLevel(WorldContextObject, LevelName);
	AsyncNode->LevelTransformNow = LevelTransform;
	if (!AsyncNode->StreamLevelInstanceNow)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid StreamLevel. Cannot execute LoadStreamLevelByName."), ELogVerbosity::Error);
		AsyncNode->FailedEvent();
		return AsyncNode;
	}

	AsyncNode->StreamLevelInstanceNow = AsyncNode->StreamLevelInstanceNow->CreateInstance(
		AsyncNode->StreamLevelInstanceNow->GetName() + FGuid::NewGuid().ToString()
		);
	
	AsyncNode->StreamLevelInstanceNow->LevelTransform = LevelTransform;
	
	FTimerDynamicDelegate Delegate;
	Delegate.BindUFunction(AsyncNode, FName("TimeoutEvent"));
	AsyncNode->TimerHandle = AsyncNode->SetTimerDelegate(Delegate, Timeout);

	AsyncNode->bMakeVisibleAfterLoad = bMakeVisibleAfterLoad;

	AsyncNode->RegisterWithGameInstance(const_cast<UObject*>(AsyncNode->WorldContextObject));

	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddUObject(AsyncNode, &UGBWPTAsync_LoadStreamLevelByName::PreGarbageCollect);

	return AsyncNode;
}

void UGBWPTAsync_LoadStreamLevelByName::Stop()
{
	if(TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	CompletedEvent();
}

void UGBWPTAsync_LoadStreamLevelByName::Activate()
{
	if (!WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute ListenEvent."), ELogVerbosity::Error);
		return;
	}

	Then.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_LoadStreamLevelByName::CompletedEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Completed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_LoadStreamLevelByName::FailedEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Failed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_LoadStreamLevelByName::TimeoutEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Timeout.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

FTimerHandle UGBWPTAsync_LoadStreamLevelByName::SetTimerDelegate(FTimerDynamicDelegate Delegate, float Time)
{
	FTimerHandle Handle;
	if (Delegate.IsBound())
	{
		World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (World)
		{
			FTimerManager& TimerManager = World->GetTimerManager();
			Handle = TimerManager.K2_FindDynamicTimerHandle(Delegate);
			TimerManager.SetTimer(Handle, Delegate, Time, false);
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

void UGBWPTAsync_LoadStreamLevelByName::PreGarbageCollect()
{
	if (bIsCompleted)
	{
		SetReadyToDestroy();
	}
}


UGBWPTAsync_LoadStreamLevel::UGBWPTAsync_LoadStreamLevel()
{
	WorldContextObject = nullptr;
}

UGBWPTAsync_LoadStreamLevel::~UGBWPTAsync_LoadStreamLevel()
{
	
}

void UGBWPTAsync_LoadStreamLevel::Tick(float DeltaTime)
{
	if ( LastFrameNumberWeTicked == GFrameCounter )
		return;

	if(bIsCompleted)
	{
		this->ConditionalBeginDestroy();
		return;
	}
	
	Timer = Timer + DeltaTime;
	FrameDeltaTime = DeltaTime;
	LastFrameNumberWeTicked = GFrameCounter;

	if (!StreamLevelInstanceNow)
	{
		Failed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
		FailedEvent();
		return;
	}
	
	if (!StreamLevelInstanceNow->HasLoadRequestPending())
	{
		StreamLevelInstanceNow->SetShouldBeLoaded(true);
		StreamLevelInstanceNow->SetShouldBeVisible(bMakeVisibleAfterLoad);
	}

	if (StreamLevelInstanceNow->IsLevelLoaded())
	{
		Completed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
		CompletedEvent();
	}
	else
	{
		Update.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
	}
}

ETickableTickType UGBWPTAsync_LoadStreamLevel::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

TStatId UGBWPTAsync_LoadStreamLevel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( FMyTickableThing, STATGROUP_Tickables );
}

bool UGBWPTAsync_LoadStreamLevel::IsTickableWhenPaused() const
{
	return true;
}

bool UGBWPTAsync_LoadStreamLevel::IsTickableInEditor() const
{
	return true;
}

UGBWPTAsync_LoadStreamLevel* UGBWPTAsync_LoadStreamLevel::PT_LoadStreamLevel(
		ULevelStreaming* StreamLevelInstance,
		bool bMakeVisibleAfterLoad,
		bool bSetTransform,
		FTransform LevelTransform,
		float Timeout)
{
	UGBWPTAsync_LoadStreamLevel* AsyncNode = NewObject<UGBWPTAsync_LoadStreamLevel>();
	AsyncNode->WorldContextObject = StreamLevelInstance->GetWorld();
	AsyncNode->StreamLevelInstanceNow = StreamLevelInstance;
	AsyncNode->LevelTransformNow = LevelTransform;
	if (!AsyncNode->StreamLevelInstanceNow)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid StreamLevelInstance. Cannot execute LoadStreamLevel."), ELogVerbosity::Error);
		AsyncNode->FailedEvent();
		return AsyncNode;
	}

	if (bSetTransform)
	{
		AsyncNode->StreamLevelInstanceNow->LevelTransform = LevelTransform;
	}
	
	FTimerDynamicDelegate Delegate;
	Delegate.BindUFunction(AsyncNode, FName("TimeoutEvent"));
	AsyncNode->TimerHandle = AsyncNode->SetTimerDelegate(Delegate, Timeout);

	AsyncNode->bMakeVisibleAfterLoad = bMakeVisibleAfterLoad;

	AsyncNode->RegisterWithGameInstance(const_cast<UObject*>(AsyncNode->WorldContextObject));

	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddUObject(AsyncNode, &UGBWPTAsync_LoadStreamLevel::PreGarbageCollect);

	return AsyncNode;
}

void UGBWPTAsync_LoadStreamLevel::Stop()
{
	if(TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	CompletedEvent();
}

void UGBWPTAsync_LoadStreamLevel::Activate()
{
	if (!WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute ListenEvent."), ELogVerbosity::Error);
		return;
	}

	Then.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_LoadStreamLevel::CompletedEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Completed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_LoadStreamLevel::FailedEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Failed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_LoadStreamLevel::TimeoutEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Timeout.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

FTimerHandle UGBWPTAsync_LoadStreamLevel::SetTimerDelegate(FTimerDynamicDelegate Delegate, float Time)
{
	FTimerHandle Handle;
	if (Delegate.IsBound())
	{
		World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (World)
		{
			FTimerManager& TimerManager = World->GetTimerManager();
			Handle = TimerManager.K2_FindDynamicTimerHandle(Delegate);
			TimerManager.SetTimer(Handle, Delegate, Time, false);
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

void UGBWPTAsync_LoadStreamLevel::PreGarbageCollect()
{
	if (bIsCompleted)
	{
		SetReadyToDestroy();
	}
}


UGBWPTAsync_UnloadStreamLevel::UGBWPTAsync_UnloadStreamLevel()
{
	WorldContextObject = nullptr;
}

UGBWPTAsync_UnloadStreamLevel::~UGBWPTAsync_UnloadStreamLevel()
{
	
}

void UGBWPTAsync_UnloadStreamLevel::Tick(float DeltaTime)
{
	if ( LastFrameNumberWeTicked == GFrameCounter )
		return;

	if(bIsCompleted)
	{
		this->ConditionalBeginDestroy();
		return;
	}
	
	Timer = Timer + DeltaTime;
	FrameDeltaTime = DeltaTime;
	LastFrameNumberWeTicked = GFrameCounter;

	if (!StreamLevelInstanceNow)
	{
		Failed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
		FailedEvent();
		return;
	}
	
	if (!StreamLevelInstanceNow->GetIsRequestingUnloadAndRemoval())
	{
		StreamLevelInstanceNow->SetShouldBeLoaded(false);
		StreamLevelInstanceNow->SetShouldBeVisible(false);
	}

	if (!StreamLevelInstanceNow->IsLevelLoaded())
	{
		Completed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
		CompletedEvent();
	}
	else
	{
		Update.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
	}
}

ETickableTickType UGBWPTAsync_UnloadStreamLevel::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

TStatId UGBWPTAsync_UnloadStreamLevel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( FMyTickableThing, STATGROUP_Tickables );
}

bool UGBWPTAsync_UnloadStreamLevel::IsTickableWhenPaused() const
{
	return true;
}

bool UGBWPTAsync_UnloadStreamLevel::IsTickableInEditor() const
{
	return true;
}

UGBWPTAsync_UnloadStreamLevel* UGBWPTAsync_UnloadStreamLevel::PT_UnloadStreamLevel(
	ULevelStreaming* StreamLevelInstance, float Timeout)
{
	if (!StreamLevelInstance)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid StreamLevelInstance. Cannot execute UnloadStreamLevelByName."), ELogVerbosity::Error);
		return nullptr;
	}
	
	UGBWPTAsync_UnloadStreamLevel* AsyncNode = NewObject<UGBWPTAsync_UnloadStreamLevel>();
	AsyncNode->WorldContextObject = StreamLevelInstance->GetWorld();
	AsyncNode->StreamLevelInstanceNow = StreamLevelInstance;
	
	FTimerDynamicDelegate Delegate;
	Delegate.BindUFunction(AsyncNode, FName("TimeoutEvent"));
	AsyncNode->TimerHandle = AsyncNode->SetTimerDelegate(Delegate, Timeout);

	AsyncNode->RegisterWithGameInstance(const_cast<UObject*>(AsyncNode->WorldContextObject));

	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddUObject(AsyncNode, &UGBWPTAsync_UnloadStreamLevel::PreGarbageCollect);

	return AsyncNode;
}

void UGBWPTAsync_UnloadStreamLevel::Stop()
{
	if(TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	CompletedEvent();
}

void UGBWPTAsync_UnloadStreamLevel::Activate()
{
	if (!WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute ListenEvent."), ELogVerbosity::Error);
		return;
	}

	Then.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_UnloadStreamLevel::CompletedEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Completed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_UnloadStreamLevel::FailedEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Failed.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

void UGBWPTAsync_UnloadStreamLevel::TimeoutEvent()
{
	if (bIsCompleted)
	{
		return;
	}
	bIsCompleted = true;
	
	if (TimerHandle.IsValid())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
	
	Timeout.Broadcast(this,Timer,FrameDeltaTime,StreamLevelInstanceNow);
}

FTimerHandle UGBWPTAsync_UnloadStreamLevel::SetTimerDelegate(FTimerDynamicDelegate Delegate, float Time)
{
	FTimerHandle Handle;
	if (Delegate.IsBound())
	{
		World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (World)
		{
			FTimerManager& TimerManager = World->GetTimerManager();
			Handle = TimerManager.K2_FindDynamicTimerHandle(Delegate);
			TimerManager.SetTimer(Handle, Delegate, Time, false);
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

void UGBWPTAsync_UnloadStreamLevel::PreGarbageCollect()
{
	if (bIsCompleted)
	{
		SetReadyToDestroy();
	}
}