// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "GBWToolsComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UGBWToolsComponent::UGBWToolsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UGBWToolsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;
	Parameters.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StructItems, Parameters)
}


// Called when the game starts
void UGBWToolsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGBWToolsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGBWToolsComponent::AddProperty(const FString& Key, const FGBWStruct& Value)
{
	AddProperty_Server(Key, Value);
}
void UGBWToolsComponent::AddProperty_Server_Implementation(const FString& Key, const FGBWStruct& Value)
{
	AddProperty_Imp(Key, Value);
}
void UGBWToolsComponent::AddProperty_Imp(const FString& Key, const FGBWStruct& Value)
{
	if (!Key.IsEmpty() && Value.IsValidStruct())
	{
		for (int i=0; i<StructItems.Num(); i++)
		{
			if (StructItems[i].Key == Key)
			{
				StructItems[i].Struct = Value;
				break;
			}
		}

		StructItems.Add(FGBWStructItem(Key,Value));
	}
}

bool UGBWToolsComponent::GetProperty(const FString& Key, FGBWStruct& Value)
{
	if (!Key.IsEmpty())
	{
		for (int i=0; i<StructItems.Num(); i++)
		{
			if (StructItems[i].Key == Key)
			{
				Value = StructItems[i].Struct;
				return true;
			}
		}
	}
	return false;
}



