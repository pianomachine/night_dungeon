// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "AsyncAction/GBWAsyncAction_ExecutePerception.h"
#include "Engine.h"

void UGBWAsyncAction_ExecutePerception::Tick(float DeltaTime)
{
	if (bIsCompleted)
	{
		return;
	}
	
	Timer += DeltaTime;
	
	if (PerceptionComponent)
	{
		FGBWCognitionItem Result;
		bool bIsGet = false;
		PerceptionComponent->GetCognition(PerceptionKey, bIsGet, Result);

		if (!PerceptionComponent->HasPerception(PerceptionKey,PerceptionClass))
		{
			//Allow network latency of 300ms for clients
			if (PerceptionComponent->GetOwner()->HasAuthority() || Timer >= 0.3f)
			{
				OnComplete.Broadcast(PerceptionKey, Result);
				Cancel();
			}
		}
		else
		{
			OnUpdate.Broadcast(PerceptionKey, Result);
		}
	}
	else
	{
		Cancel();
	}
}

ETickableTickType UGBWAsyncAction_ExecutePerception::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

TStatId UGBWAsyncAction_ExecutePerception::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( FMyTickableThing, STATGROUP_Tickables );
}

bool UGBWAsyncAction_ExecutePerception::IsTickableWhenPaused() const
{
	return true;
}

bool UGBWAsyncAction_ExecutePerception::IsTickableInEditor() const
{
	return false;
}

UGBWAsyncAction_ExecutePerception* UGBWAsyncAction_ExecutePerception::GBWAsyncAction_ExecutePerception(
	AActor* InActor,
	FGameplayTag InPerceptionKey, TSubclassOf<AGBWPerceptionBase> InPerceptionClass,
	FGBWStruct PerceptionParamStruct)
{
	if (!InActor || !InPerceptionKey.IsValid() || !InPerceptionClass)
	{
		return nullptr;
	}
	UWorld* ContextWorld = GEngine->GetWorldFromContextObject(InActor, EGetWorldErrorMode::ReturnNull);
	if(!ensureAlwaysMsgf(IsValid(InActor), TEXT("World Context was not valid.")))
	{
		return nullptr;
	}
	
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		UGBWAsyncAction_ExecutePerception* NewAction = NewObject<UGBWAsyncAction_ExecutePerception>();
		NewAction->ContextWorld = ContextWorld;
		NewAction->PerceptionKey = InPerceptionKey;
		NewAction->PerceptionClass = InPerceptionClass;
		NewAction->PerceptionComponent = TComp;
		NewAction->PerceptionParamsStr = PerceptionParamStruct;
		NewAction->RegisterWithGameInstance(ContextWorld->GetGameInstance());
		
		return NewAction;
	}
	
	return nullptr;
}

void UGBWAsyncAction_ExecutePerception::Activate()
{
	Super::Activate();
    if(PerceptionComponent && PerceptionKey.IsValid() && PerceptionClass)
    {
    	FGuid PerceptionGuid;
    	PerceptionComponent->ExecutePerception(PerceptionGuid, PerceptionKey, PerceptionClass, PerceptionParamsStr);
    	return;
    }

    const FGBWCognitionItem Result;
    OnFail.Broadcast(PerceptionKey, Result);
    Cancel();
}

void UGBWAsyncAction_ExecutePerception::Cancel()
{
	bIsCompleted = true;
	Super::Cancel();
}