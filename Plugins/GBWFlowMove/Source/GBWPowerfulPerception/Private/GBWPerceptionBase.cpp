// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "GBWPerceptionBase.h"

#include "GBWPowerfulPerceptionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGBWPerceptionBase::AGBWPerceptionBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f; // 20Hz - Performance optimization
	bReplicates = true;
}

void AGBWPerceptionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
    Parameters.bIsPushBased = true;
    Parameters.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PerceptionGuid, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PerceptionKeyIndex, Parameters)
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OwnerActor, Parameters)
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PerceptionParamsStr, Parameters)
}

// Called when the game starts or when spawned
void AGBWPerceptionBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGBWPerceptionBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PerceptionStart(GetOwnerActorNetType());
	PerceptionUpdate(GetOwnerActorNetType(),DeltaTime);
}

EGBWActorNetType AGBWPerceptionBase::GetOwnerActorNetType()
{
	if (OwnerActorNetType == EGBWActorNetType::None)
	{
		OwnerActorNetType = UGBWPowerfulToolsFuncLib::GetActorNetType(OwnerActor);
	}

	return OwnerActorNetType;
}

FGameplayTag AGBWPerceptionBase::GetPerceptionKey() const
{
	if (PPComponent && PPComponent->PerceptionKeyDict.IsValidIndex(PerceptionKeyIndex))
	{
		return PPComponent->PerceptionKeyDict[PerceptionKeyIndex];
	}

	return FGameplayTag::EmptyTag;
}

void AGBWPerceptionBase::EndPerception()
{
	if (UGBWPowerfulPerceptionComponent* PPComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(OwnerActor))
	{
		PPComp->RemovePerception(PerceptionGuid);
	}
	else
	{
		DestroyPerception();
	}
}

AActor* AGBWPerceptionBase::GetOwnerActor() const
{
	return OwnerActor;
}

void AGBWPerceptionBase::DestroyPerception()
{
	PerceptionEnd();
	this->Destroy();
}

void AGBWPerceptionBase::PerceptionUpdate(EGBWActorNetType OwnerNetType, float DeltaTime)
{
	if (!UKismetSystemLibrary::IsValid(GetOwnerActor()))
	{
		EndPerception();
		return;
	}
	OnPerceptionUpdate(OwnerNetType, DeltaTime);
}

void AGBWPerceptionBase::PerceptionEnd()
{
	PerceptionEnd_Server();
}
void AGBWPerceptionBase::PerceptionEnd_Server_Implementation()
{
	PerceptionEnd_Multicast();
}
void AGBWPerceptionBase::PerceptionEnd_Multicast_Implementation()
{
	PerceptionEnd_Imp();
}
void AGBWPerceptionBase::PerceptionEnd_Imp()
{
	OnPerceptionEnd(GetOwnerActorNetType());
}

void AGBWPerceptionBase::PerceptionEvent(const FName Event)
{
	PerceptionEvent_Server(Event);
}
void AGBWPerceptionBase::PerceptionEvent_Server_Implementation(const FName Event)
{
	PerceptionEvent_Multicast(Event);
}
void AGBWPerceptionBase::PerceptionEvent_Multicast_Implementation(const FName Event)
{
	PerceptionEvent_Imp(Event);
}
void AGBWPerceptionBase::PerceptionEvent_Imp(const FName Event)
{
	OnPerceptionEvent(Event);
}

void AGBWPerceptionBase::execInitPerceptionParameters(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	P_GET_UBOOL_REF(Z_Param_Out_bIsSuccess);
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructPtr = Stack.MostRecentPropertyAddress;
	
	P_FINISH;
	P_NATIVE_BEGIN;
	if (const AGBWPerceptionBase* EActor = Cast<AGBWPerceptionBase>(Stack.Object))
	{
		const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
		{
			Z_Param_Out_bIsSuccess = UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(EActor->PerceptionParamsStr, StructPtr, StructProp->Struct);
		}
		else
		{
			Z_Param_Out_bIsSuccess = false;
		}
	}
	else
	{
		Z_Param_Out_bIsSuccess = false;
	}
	P_NATIVE_END;
}

void AGBWPerceptionBase::EnableInputPerception(bool& bIsSuccess)
{
	bool bIsGet = false;
	bool bIsLocalController = false;
	APlayerController* PlayerController = nullptr;
	UGBWPowerfulToolsFuncLib::GetPlayerController(GetOwnerActor(), bIsGet, bIsLocalController,PlayerController);
	if (bIsGet && bIsLocalController)
	{
		EnableInput(PlayerController);
		bIsSuccess = true;
		return;
	}

	bIsSuccess = false;
}

void AGBWPerceptionBase::execUpdateCognition(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	P_GET_PROPERTY(FFloatProperty, Z_Param_EffectiveTime)
	P_GET_UBOOL(Z_Param_bIsReplicated)
	P_GET_UBOOL(Z_Param_bIsEffectivePerception)
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const void* StructPtr = Stack.MostRecentPropertyAddress;
	
	P_FINISH;
	P_NATIVE_BEGIN;
	if (const AGBWPerceptionBase* EActor = Cast<AGBWPerceptionBase>(Stack.Object))
	{
		if (UGBWPowerfulPerceptionComponent* PPComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(EActor->GetOwnerActor()))
		{
			const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
			if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
			{
				PPComp->SetStructAsCognition(EActor->GetPerceptionKey(),Z_Param_EffectiveTime, StructPtr, StructProp->Struct, Z_Param_bIsReplicated, Z_Param_bIsEffectivePerception);
			}
		}
	}
	P_NATIVE_END;
}

void AGBWPerceptionBase::execEffectivePerception(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	P_GET_UBOOL(Z_Pram_bIsReplicated)
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const void* StructPtr = Stack.MostRecentPropertyAddress;
	
	P_FINISH;
	P_NATIVE_BEGIN;
	if (const AGBWPerceptionBase* EActor = Cast<AGBWPerceptionBase>(Stack.Object))
	{
		if (UGBWPowerfulPerceptionComponent* PPComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(EActor->GetOwnerActor()))
		{
			const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
			if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
			{
				PPComp->EffectivePerceptionFromStruct(EActor->GetPerceptionKey(), StructPtr, StructProp->Struct, Z_Pram_bIsReplicated);
			}
		}
	}
	P_NATIVE_END;
}

void AGBWPerceptionBase::ClearCognition()
{
	if (UGBWPowerfulPerceptionComponent* PPComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(GetOwnerActor()))
	{
		PPComp->RemoveCognition(GetPerceptionKey());
	}
}

void AGBWPerceptionBase::GetOwnerCharacter(bool& bIsSuccess, ACharacter*& Character)
{
	if (!OwnerCharacter && GetOwnerActor())
	{
		OwnerCharacter = Cast<ACharacter>(GetOwnerActor());
	}
	
	Character = OwnerCharacter;
	if (Character)
	{
		bIsSuccess = true;
	}
	else
	{
		bIsSuccess = false;
	}
}

void AGBWPerceptionBase::GetOwnerPlayerController(bool& bIsSuccess, APlayerController*& PlayerController)
{
	PlayerController = nullptr;

	if (const APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		PlayerController = Cast<APlayerController>(Pawn->GetController());
	}

	if (PlayerController)
	{
		bIsSuccess = true;
	}
	else
	{
		bIsSuccess = false;
	}
}


