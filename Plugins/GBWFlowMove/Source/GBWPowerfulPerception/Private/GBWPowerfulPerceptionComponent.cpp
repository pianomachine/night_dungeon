// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "GBWPowerfulPerceptionComponent.h"

#include "GBWPowerfulToolsFuncLib.h"
#include "Net/UnrealNetwork.h"

FString FGBWCognitionItem::GetDebugStr() const
{
	return Key.ToString() + " : " + CognitionContent.StructType + " -> " + CognitionContent.StructStr;
}

bool FGBWCognitionItem::IsValidCognition() const
{
	return Key.IsValid() && CognitionContent.IsValidStruct();
}

void FGBWCognition::SetCognition(const FGBWCognitionItem& NewCognition)
{
	if (!NewCognition.Key.IsValid())
	{
		return;
	}
	for (int i = 0; i < Cognition.Num(); i++)
	{
		if (Cognition[i].Key == NewCognition.Key)
		{
			Cognition[i] = NewCognition;
			return;
		}
	}

	Cognition.Add(NewCognition);
}

bool FGBWCognition::GetCognition(const FGameplayTag Key, FGBWCognitionItem& Result)
{
	if (!Key.IsValid())
	{
		return false;
	}
	for (int i = 0; i < Cognition.Num(); i++)
	{
		if (Cognition[i].Key == Key)
		{
			Result = Cognition[i];
			return true;
		}
	}

	return false;
}

void FGBWCognition::RemoveCognition(const FGameplayTag Key)
{
	if (!Key.IsValid())
	{
		return;
	}
	for (int i = 0; i < Cognition.Num(); i++)
	{
		if (Cognition[i].Key == Key)
		{
			Cognition.RemoveAt(i);
			return;
		}
	}
}

void FGBWCognition::Clear()
{
	Cognition.Empty();
}

FString FGBWCognition::GetDebugStr() const
{
	FString Res = "";
	for (auto C : Cognition)
	{
		Res = "\n" + Res + C.GetDebugStr();
	}

	return Res;
}

// Sets default values for this component's properties
UGBWPowerfulPerceptionComponent::UGBWPowerfulPerceptionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UGBWPowerfulPerceptionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;
	Parameters.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Cognition, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Perceptions, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PerceptionKeyDict, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CognitionSnapshot, Parameters)
}


// Called when the game starts
void UGBWPowerfulPerceptionComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UGBWPowerfulPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	OnUpdate.Broadcast();
	EffectivePerceptionNow.Clear();

	CheckCognitionValid();
}

UGBWPowerfulPerceptionComponent* UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(AActor* InActor)
{
	if (!InActor)
	{
		return nullptr;
	}
	UGBWPowerfulPerceptionComponent* AComp = nullptr;
	if (UActorComponent* Component = InActor->GetComponentByClass(UGBWPowerfulPerceptionComponent::StaticClass()))
	{
		AComp = Cast<UGBWPowerfulPerceptionComponent>(Component);
	}

	if (!AComp && InActor->HasAuthority())
	{
		AComp = Cast<UGBWPowerfulPerceptionComponent>(
			InActor->AddComponentByClass(UGBWPowerfulPerceptionComponent::StaticClass(),
				false,
				FTransform(),
				false));
		AComp->SetIsReplicated(true);
	}
	
	return AComp;
}

void UGBWPowerfulPerceptionComponent::execPP_SetCognition(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	P_GET_OBJECT(AActor,Z_Param_InActor);
	P_GET_UBOOL(Z_Param_bIsReplicated);
	P_GET_UBOOL(Z_Param_bIsEffectivePerception)
	P_GET_STRUCT(FGameplayTag,Z_Param_Key);
	P_GET_PROPERTY(FFloatProperty,Z_Param_EffectiveTime);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const void* StructPtr = Stack.MostRecentPropertyAddress;
	FProperty* StructProperty = Stack.MostRecentProperty;
	
	P_FINISH;
	P_NATIVE_BEGIN;
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(Z_Param_InActor))
	{
		const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
		{
			FGBWStruct GS = UGBWPowerfulToolsFuncLib::UserStructToGBWStruct(StructPtr, StructProp->Struct);
			TComp->SetCognition_Imp(FGBWCognitionItem(Z_Param_Key,Z_Param_EffectiveTime,GS,TComp->GetWorld()->GetTimeSeconds()),Z_Param_bIsReplicated,Z_Param_bIsEffectivePerception);
		}
	}
	P_NATIVE_END;
}

void UGBWPowerfulPerceptionComponent::execPP_GetCognitionAsStruct(UObject* Context, FFrame& Stack,
	void* const Z_Param__Result)
{
	P_GET_OBJECT(AActor,Z_Param_InActor);
	P_GET_STRUCT(FGameplayTag,Z_Param_Key);
	P_GET_UBOOL_REF(Z_Param_Out_bIsSuccess);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(Z_Param_InActor))
	{
		FGBWCognitionItem Result;
		bool bIsGet = false;
		TComp->GetCognition(Z_Param_Key, bIsGet, Result);
		if (bIsGet)
		{
			const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
			if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
			{
				Z_Param_Out_bIsSuccess = UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(Result.CognitionContent, StructPtr, StructProp->Struct);
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
	}
	else
	{
		Z_Param_Out_bIsSuccess = false;
	}
}

bool UGBWPowerfulPerceptionComponent::_GetCognitionAsStruct(AActor* InActor, FGameplayTag Key, void* StructPtr,
	UScriptStruct* ScriptStruct)
{
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		return TComp->GetCognitionAsStruct_(Key, StructPtr, ScriptStruct);
	}

	return false;
}

void UGBWPowerfulPerceptionComponent::execPP_CognitionToStruct(UObject* Context, FFrame& Stack,
                                                            void* const Z_Param__Result)
{
	P_GET_STRUCT(FGBWCognitionItem,Z_Param_InCognition);
	P_GET_UBOOL_REF(Z_Param_Out_bIsSuccess);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;

	const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
	{
		Z_Param_Out_bIsSuccess = UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(Z_Param_InCognition.CognitionContent, StructPtr, StructProp->Struct);
	}
	else
	{
		Z_Param_Out_bIsSuccess = false;
	}
}

void UGBWPowerfulPerceptionComponent::SetStructAsCognition(
	FGameplayTag Key, const float EffectiveTime,
	const void* StructPtr,
	const UScriptStruct* ScriptStruct,
	bool bIsReplicated,
	bool bIsEffectivePerception)
{
	SetCognition_Imp(
		FGBWCognitionItem(Key, EffectiveTime, UGBWPowerfulToolsFuncLib::UserStructToGBWStruct(StructPtr, ScriptStruct),GetWorld()->GetTimeSeconds())
		,bIsReplicated
		,bIsEffectivePerception);
}

void UGBWPowerfulPerceptionComponent::CheckCognitionValid()
{
	const float TimeNow = this->GetWorld()->GetTimeSeconds();
	if (this->GetOwner()->HasAuthority())
	{
		FGBWCognition CognitionTemp = Cognition;
		for (const auto Element : CognitionTemp.Cognition)
        {
        	if (TimeNow - Element.LastUpdateTime > Element.EffectiveTime && Element.EffectiveTime>0.0f)
        	{
        		Cognition.RemoveCognition(Element.Key);
        	}
        }
	}

	FGBWCognition CognitionTemp = Cognition_Local;
	for (int i=0; i < CognitionTemp.Cognition.Num(); i++)
	{
		if (TimeNow - CognitionTemp.Cognition[i].LastUpdateTime > CognitionTemp.Cognition[i].EffectiveTime && CognitionTemp.Cognition[i].EffectiveTime>0.0f)
		{
			Cognition_Local.RemoveCognition(CognitionTemp.Cognition[i].Key);
		}
	}
}

void UGBWPowerfulPerceptionComponent::SetCognition_Imp(const FGBWCognitionItem& NewCognition, bool bIsReplicated, bool bIsEffectivePerception)
{
	bool bIsGet = false;
	FGBWCognitionItem OldCognition;
	GetCognition(NewCognition.Key, bIsGet, OldCognition);
	if (bIsGet
		&& OldCognition.CognitionContent.Equals(NewCognition.CognitionContent)
		&& NewCognition.LastUpdateTime - OldCognition.LastUpdateTime <= OldCognition.EffectiveTime)
	{
		return;
	}
	
	if (bIsReplicated)
	{
		SetCognition_Server(NewCognition);
	}
	else
	{
		SetCognition_Local(NewCognition);
	}

	if (bIsEffectivePerception)
	{
		EffectivePerception_Imp(NewCognition, bIsReplicated);
	}
}
void UGBWPowerfulPerceptionComponent::SetCognition_Server_Implementation(const FGBWCognitionItem& NewCognition)
{
	Cognition.SetCognition(NewCognition);
	SetCognition_Multicast(NewCognition);
}

void UGBWPowerfulPerceptionComponent::SetCognition_Multicast_Implementation(const FGBWCognitionItem& NewCognition)
{
	OnCognitionChanged.Broadcast(NewCognition.Key, NewCognition);
}

void UGBWPowerfulPerceptionComponent::SetCognition_Local(const FGBWCognitionItem& NewCognition)
{
	Cognition_Local.SetCognition(NewCognition);
	OnCognitionChanged.Broadcast(NewCognition.Key, NewCognition);
}

void UGBWPowerfulPerceptionComponent::GetCognition(const FGameplayTag Key, bool& bIsGet, FGBWCognitionItem& Result)
{
	bIsGet = Cognition.GetCognition(Key, Result);
	if (!bIsGet)
	{
		bIsGet = Cognition_Local.GetCognition(Key, Result);
	}
}

bool UGBWPowerfulPerceptionComponent::GetCognitionAsStruct_(const FGameplayTag Key, void* StructPtr, UScriptStruct* ScriptStruct)
{
	bool bIsGet = false;
	FGBWCognitionItem Result;
	GetCognition(Key,bIsGet,Result);
	if (bIsGet)
	{
		return UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(Result.CognitionContent, StructPtr, ScriptStruct);
	}
	return false;
}

FString UGBWPowerfulPerceptionComponent::GetCognitionDebugStr() const
{
	return FString("Net : ") + Cognition.GetDebugStr() + FString("\n Local : ") + Cognition_Local.GetDebugStr();
}

void UGBWPowerfulPerceptionComponent::PP_RemoveCognition(AActor* InActor, FGameplayTag Key)
{
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		TComp->RemoveCognition(Key);
	}
}

void UGBWPowerfulPerceptionComponent::RemoveCognition(FGameplayTag Key)
{
	Cognition_Local.RemoveCognition(Key);
	RemoveCognition_Server(Key);
}

void UGBWPowerfulPerceptionComponent::PP_SnapshotCognition(AActor* InActor, const FGameplayTag Key, bool bReset, const bool bIsReplicated,
                                                           const bool bIsEffectivePerception)
{
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		TComp->SnapshotCognition(Key, bReset, bIsReplicated, bIsEffectivePerception);
	}
}

bool UGBWPowerfulPerceptionComponent::PP_HasCognitionSnapshot(AActor* InActor, FGameplayTag Key)
{
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		return TComp->HasCognitionSnapshot(Key);
	}

	return false;
}

void UGBWPowerfulPerceptionComponent::execPP_GetCognitionSnapshotAsStruct(UObject* Context, FFrame& Stack,
                                                                          void* const Z_Param__Result)
{
	P_GET_OBJECT(AActor,Z_Param_InActor);
	P_GET_STRUCT(FGameplayTag,Z_Param_Key);
	P_GET_UBOOL_REF(Z_Param_Out_bIsSuccess);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(Z_Param_InActor))
	{
		FGBWCognitionItem Result;
		bool bIsGet = false;
		TComp->GetCognitionSnapshot(Z_Param_Key, bIsGet, Result);
		if (bIsGet)
		{
			const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
			if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
			{
				Z_Param_Out_bIsSuccess = UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(Result.CognitionContent, StructPtr, StructProp->Struct);
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
	}
	else
	{
		Z_Param_Out_bIsSuccess = false;
	}
}

bool UGBWPowerfulPerceptionComponent::_GetCognitionSnapshotAsStruct(AActor* InActor, FGameplayTag Key, void* StructPtr,
                                                                    UScriptStruct* ScriptStruct)
{
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		return TComp->GetCognitionSnapshotAsStruct_(Key, StructPtr, ScriptStruct);
	}

	return false;
}

void UGBWPowerfulPerceptionComponent::SetCognitionSnapshot_Local(const FGBWCognitionItem& NewCognition)
{
	CognitionSnapshot_Local.SetCognition(NewCognition);
	OnCognitionSnapshotChanged.Broadcast(NewCognition.Key, NewCognition);
}

void UGBWPowerfulPerceptionComponent::SnapshotCognition(const FGameplayTag Key, bool bReset, bool bIsReplicated, bool bIsEffectivePerception)
{
	bool bIsGetSnapshot = false;
	FGBWCognitionItem OldCognitionSnapshot;
	GetCognitionSnapshot(Key, bIsGetSnapshot, OldCognitionSnapshot);

	bool bIsGet = false;
	FGBWCognitionItem OldCognition;
	GetCognition(Key, bIsGet, OldCognition);
	
	if (!bIsGet)
	{
		return;
	}
	
	if (bIsGetSnapshot && !bReset && OldCognitionSnapshot.CognitionContent.Equals(OldCognition.CognitionContent))
	{
		return;
	}
	
	if (bIsReplicated)
	{
		SetCognitionSnapshot_Server(OldCognition);
	}
	else
	{
		SetCognitionSnapshot_Local(OldCognition);
	}

	if (bIsEffectivePerception)
	{
		EffectivePerception_Imp(OldCognition, bIsReplicated);
	}
}

bool UGBWPowerfulPerceptionComponent::HasCognitionSnapshot(const FGameplayTag Key)
{
	bool bIsGet = false;
	FGBWCognitionItem Result;
	GetCognitionSnapshot(Key, bIsGet, Result);
	return bIsGet;
}

void UGBWPowerfulPerceptionComponent::GetCognitionSnapshot(FGameplayTag Key, bool& bIsGet, FGBWCognitionItem& Result)
{
	bIsGet = CognitionSnapshot.GetCognition(Key, Result);
	if (!bIsGet)
	{
		bIsGet = CognitionSnapshot_Local.GetCognition(Key, Result);
	}
}

bool UGBWPowerfulPerceptionComponent::GetCognitionSnapshotAsStruct_(FGameplayTag Key, void* StructPtr,
	UScriptStruct* ScriptStruct)
{
	bool bIsGet = false;
	FGBWCognitionItem Result;
	GetCognitionSnapshot(Key,bIsGet,Result);
	if (bIsGet)
	{
		return UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(Result.CognitionContent, StructPtr, ScriptStruct);
	}
	return false;
}

FString UGBWPowerfulPerceptionComponent::GetCognitionSnapshotDebugStr() const
{
	return FString("Net : ") + CognitionSnapshot.GetDebugStr() + FString("\n Local : ") + CognitionSnapshot_Local.GetDebugStr();
}

void UGBWPowerfulPerceptionComponent::PP_RemoveCognitionSnapshot(AActor* InActor, FGameplayTag Key)
{
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		TComp->RemoveCognitionSnapshot(Key);
	}
}

void UGBWPowerfulPerceptionComponent::RemoveCognitionSnapshot(FGameplayTag Key)
{
	CognitionSnapshot_Local.RemoveCognition(Key);
	RemoveCognitionSnapshot_Server(Key);
}

void UGBWPowerfulPerceptionComponent::RemoveCognitionSnapshot_Server_Implementation(FGameplayTag Key)
{
	CognitionSnapshot.RemoveCognition(Key);
}

void UGBWPowerfulPerceptionComponent::SetCognitionSnapshot_Multicast_Implementation(
	const FGBWCognitionItem& NewCognition)
{
	OnCognitionSnapshotChanged.Broadcast(NewCognition.Key, NewCognition);
}

void UGBWPowerfulPerceptionComponent::SetCognitionSnapshot_Server_Implementation(const FGBWCognitionItem& NewCognition)
{
	CognitionSnapshot.SetCognition(NewCognition);
	SetCognitionSnapshot_Multicast(NewCognition);
}

void UGBWPowerfulPerceptionComponent::RemoveCognition_Server_Implementation(FGameplayTag Key)
{
	Cognition.RemoveCognition(Key);
}

bool UGBWPowerfulPerceptionComponent::HasPerception(
	FGameplayTag PerceptionKey,
	TSubclassOf<AGBWPerceptionBase> PerceptionClass)
{
	for (int i=0; i < Perceptions.Num(); i++)
	{
		if (Perceptions[i])
		{
			if (Perceptions[i]->GetPerceptionKey() == PerceptionKey)
			{
				if (!PerceptionClass || PerceptionClass == Perceptions[i]->GetClass())
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool UGBWPowerfulPerceptionComponent::PP_HasPerception(AActor* InActor, FGameplayTag PerceptionKey,
	TSubclassOf<AGBWPerceptionBase> PerceptionClass)
{
	if (UGBWPowerfulPerceptionComponent* PPComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		return PPComp->HasPerception(PerceptionKey,PerceptionClass);
	}
	return false;
}

void UGBWPowerfulPerceptionComponent::GetPerception(
	FGameplayTag PerceptionKey,
	bool& bIsGet,
	AGBWPerceptionBase*& Perception)
{
	for (int i=0; i < Perceptions.Num(); i++)
	{
		if (Perceptions[i])
		{
			if (Perceptions[i]->GetPerceptionKey() == PerceptionKey)
			{
				Perception = Perceptions[i];
				bIsGet = true;
				return;
			}
		}
	}

	bIsGet = false;
}

void UGBWPowerfulPerceptionComponent::GetPerception(const int PerceptionKeyIndex, bool& bIsGet, AGBWPerceptionBase*& Perception)
{
	if (PerceptionKeyDict.IsValidIndex(PerceptionKeyIndex))
	{
		GetPerception(PerceptionKeyDict[PerceptionKeyIndex],bIsGet,Perception);
	}
	else
	{
		bIsGet = false;
	}
}

void UGBWPowerfulPerceptionComponent::PP_GetPerception(
	AActor* InActor,
	FGameplayTag PerceptionKey,
	bool& bIsGet,
	AGBWPerceptionBase*& Perception)
{
	if (UGBWPowerfulPerceptionComponent* PPComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		PPComp->GetPerception(PerceptionKey, bIsGet, Perception);
	}
}

void UGBWPowerfulPerceptionComponent::ExecutePerception(
	FGuid& PerceptionGuid,
	FGameplayTag PerceptionKey,
	TSubclassOf<AGBWPerceptionBase> PerceptionClass,
	const FGBWStruct& PerceptionParamsStr)
{
	if (!PerceptionClass || !PerceptionKey.IsValid())
	{
		return;
	}

	PerceptionGuid = FGuid::NewGuid();
	ExecutePerception_Server(PerceptionGuid,PerceptionKey,PerceptionClass,PerceptionParamsStr);
}
void UGBWPowerfulPerceptionComponent::ExecutePerception_Server_Implementation(
	const FGuid& PerceptionGuid,
	FGameplayTag PerceptionKey,
	TSubclassOf<AGBWPerceptionBase> PerceptionClass,
	const FGBWStruct& PerceptionParamsStr)
{
	int KeyIndex = PerceptionKeyDict.Find(PerceptionKey);
	if (KeyIndex == INDEX_NONE)
	{
		KeyIndex = PerceptionKeyDict.Add(PerceptionKey);
	}
	ExecutePerception_Imp(PerceptionGuid, KeyIndex, PerceptionClass, PerceptionParamsStr);
}
void UGBWPowerfulPerceptionComponent::ExecutePerception_Imp(
	const FGuid& PerceptionGuid,
	const int PerceptionKeyIndex,
	TSubclassOf<AGBWPerceptionBase> PerceptionClass,
	const FGBWStruct& PerceptionParamsStr)
{
	if (!PerceptionClass)
	{
		return;
	}
	
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.Owner = this->GetOwner();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AGBWPerceptionBase* PerceptionActor =
		Cast<AGBWPerceptionBase>(
			this->GetWorld()->SpawnActorAbsolute(
				PerceptionClass,
				FTransform(),
				SpawnParameters)
			)
		)
	{
		PerceptionActor->SetReplicates(true);
		PerceptionActor->PerceptionGuid = PerceptionGuid;
		PerceptionActor->PPComponent = this;
		PerceptionActor->PerceptionKeyIndex = PerceptionKeyIndex;
		PerceptionActor->PerceptionParamsStr = PerceptionParamsStr;
		PerceptionActor->OwnerActor = this->GetOwner();
		
		PerceptionActor->AttachToActor(this->GetOwner(),FAttachmentTransformRules::KeepRelativeTransform);

		PerceptionActor->PerceptionStart(UGBWPowerfulToolsFuncLib::GetActorNetType(this->GetOwner()));
		AddPerception(PerceptionActor);
	}
}

void UGBWPowerfulPerceptionComponent::AddPerception(AGBWPerceptionBase* NewPerception)
{
	Perceptions.AddUnique(NewPerception);
}

void UGBWPowerfulPerceptionComponent::execPP_ExecutePerception(UObject* Context, FFrame& Stack,
	void* const Z_Param__Result)
{
	P_GET_OBJECT(AActor,Z_Param_InActor);
	P_GET_STRUCT_REF(FGuid,Z_Param_Out_PerceptionGuid);
	P_GET_STRUCT(FGameplayTag,Z_Param_Key);
	P_GET_OBJECT(UClass,Z_Param_PerceptionClass);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const void* StructPtr = Stack.MostRecentPropertyAddress;
	FProperty* StructProperty = Stack.MostRecentProperty;
	
	P_GET_UBOOL(Z_Param_bIsReplicated);
	P_GET_UBOOL(Z_Param_bOnlyTriggerFromServer);
	
	P_FINISH;
	P_NATIVE_BEGIN;
	if (UGBWPowerfulPerceptionComponent* PEComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(Z_Param_InActor))
	{
		const FStructProperty* StructProp = CastField<FStructProperty>(StructProperty);
		if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
		{
			const FGBWStruct GS = UGBWPowerfulToolsFuncLib::UserStructToGBWStruct(StructPtr, StructProp->Struct);
			PEComp->ExecutePerception(Z_Param_Out_PerceptionGuid,Z_Param_Key,Z_Param_PerceptionClass,GS);
		}
	}
	P_NATIVE_END;
}

void UGBWPowerfulPerceptionComponent::_ExecutePerception(AActor* InActor, FGameplayTag PerceptionKey,
	TSubclassOf<AGBWPerceptionBase> PerceptionClass, const FGBWStruct& PerceptionParams)
{
	if (UGBWPowerfulPerceptionComponent* PEComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		FGuid Guid;
		PEComp->ExecutePerception(
			Guid,PerceptionKey,PerceptionClass,PerceptionParams);
	}
}

void UGBWPowerfulPerceptionComponent::RemovePerceptionByKey(FGameplayTag PerceptionKey)
{
	TArray<AGBWPerceptionBase*> ThePerceptions = Perceptions;
	for (const auto E : ThePerceptions)
	{
		if (E && E->GetPerceptionKey()==PerceptionKey)
		{
			RemovePerception(E->PerceptionGuid);
		}
	}
}

void UGBWPowerfulPerceptionComponent::RemovePerception(FGuid PerceptionGuid)
{
	RemovePerception_Server(PerceptionGuid);
}
void UGBWPowerfulPerceptionComponent::RemovePerception_Server_Implementation(const FGuid& PerceptionGuid)
{
	RemovePerception_Imp(PerceptionGuid);
}
void UGBWPowerfulPerceptionComponent::RemovePerception_Imp(const FGuid& PerceptionGuid)
{
	for (const auto E : Perceptions)
	{
		if (E && E->PerceptionGuid==PerceptionGuid)
		{
			Perceptions.Remove(E);
			E->DestroyPerception();
			return;
		}
	}
}

void UGBWPowerfulPerceptionComponent::PP_RemovePerception(AActor* InActor, FGameplayTag PerceptionKey)
{
	if (UGBWPowerfulPerceptionComponent* PPComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(InActor))
	{
		PPComp->RemovePerceptionByKey(PerceptionKey);
	}
}

void UGBWPowerfulPerceptionComponent::execPP_EffectivePerception(UObject* Context, FFrame& Stack,
	void* const Z_Param__Result)
{
	P_GET_OBJECT(AActor,Z_Param_InActor);
	P_GET_UBOOL(Z_Param_bIsReplicated);
	P_GET_STRUCT(FGameplayTag,Z_Param_Key);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const void* StructPtr = Stack.MostRecentPropertyAddress;
	FProperty* StructProperty = Stack.MostRecentProperty;
	
	P_FINISH;
	P_NATIVE_BEGIN;
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(Z_Param_InActor))
	{
		const FStructProperty* StructProp = CastField<FStructProperty>(StructProperty);
		if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
		{
			const FGBWStruct GS = UGBWPowerfulToolsFuncLib::UserStructToGBWStruct(StructPtr, StructProp->Struct);
			TComp->EffectivePerception_Imp(FGBWCognitionItem(Z_Param_Key,0.0f,GS,TComp->GetWorld()->GetTimeSeconds()), Z_Param_bIsReplicated);
		}
	}
	P_NATIVE_END;
}

void UGBWPowerfulPerceptionComponent::EffectivePerception_Imp(const FGBWCognitionItem& NewCognition, bool bIsReplicated)
{
	if (!NewCognition.IsValidCognition())
	{
		return;
	}
	if (bIsReplicated)
	{
		EffectivePerception_Server(NewCognition);
	}
	else
	{
		EffectivePerception_Exe(NewCognition);
	}
}

void UGBWPowerfulPerceptionComponent::EffectivePerception_Exe(const FGBWCognitionItem& NewCognition)
{
	EffectivePerceptionNow.SetCognition(NewCognition);
	OnEffectivePerception.Broadcast(NewCognition.Key, NewCognition);	
}

void UGBWPowerfulPerceptionComponent::GetEffectivePerception(FGameplayTag Key, bool& bIsGet, FGBWCognitionItem& Result)
{
	bIsGet = EffectivePerceptionNow.GetCognition(Key, Result);
}

FString UGBWPowerfulPerceptionComponent::GetEffectivePerceptionDebugStr() const
{
	return EffectivePerceptionNow.GetDebugStr();
}

void UGBWPowerfulPerceptionComponent::execPP_GetEffectivePerceptionAsStruct(UObject* Context, FFrame& Stack,
	void* const Z_Param__Result)
{
	P_GET_OBJECT(AActor,Z_Param_InActor);
	P_GET_STRUCT(FGameplayTag,Z_Param_Key);
	P_GET_UBOOL_REF(Z_Param_Out_bIsSuccess);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	if (UGBWPowerfulPerceptionComponent* TComp = UGBWPowerfulPerceptionComponent::PP_GetGBWPerceptionComponent(Z_Param_InActor))
	{
		FGBWCognitionItem Result;
		bool bIsGet = false;
		TComp->GetEffectivePerception(Z_Param_Key, bIsGet, Result);
		if (bIsGet)
		{
			const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
			if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
			{
				Z_Param_Out_bIsSuccess = UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(Result.CognitionContent, StructPtr, StructProp->Struct);
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
	}
	else
	{
		Z_Param_Out_bIsSuccess = false;
	}
}

void UGBWPowerfulPerceptionComponent::EffectivePerceptionFromStruct(FGameplayTag Key, const void* StructPtr,
	const UScriptStruct* ScriptStruct, bool bIsReplicated)
{
	EffectivePerception_Imp(
		FGBWCognitionItem(Key,0.0f, UGBWPowerfulToolsFuncLib::UserStructToGBWStruct(StructPtr, ScriptStruct),GetWorld()->GetTimeSeconds())
		,bIsReplicated);
}

void UGBWPowerfulPerceptionComponent::SendPerceptionEvent(const FGameplayTag PerceptionKey, const FName Event)
{
	if (const int PerceptionKeyIndex = PerceptionKeyDict.Find(PerceptionKey);
		PerceptionKeyIndex != INDEX_NONE)
	{
		SendPerceptionEvent_Server(PerceptionKeyIndex,Event);
	}
}

void UGBWPowerfulPerceptionComponent::PP_SendPerceptionEvent(AActor* InActor, FGameplayTag PerceptionKey, FName Event)
{
	if (UGBWPowerfulPerceptionComponent* TComp = PP_GetGBWPerceptionComponent(InActor))
	{
		TComp->SendPerceptionEvent(PerceptionKey,Event);
	}
}

void UGBWPowerfulPerceptionComponent::SendPerceptionEvent_Server_Implementation(const int PerceptionKeyIndex, const FName Event)
{
	bool bIsGet = false;
	AGBWPerceptionBase* Perception = nullptr;
	GetPerception(PerceptionKeyIndex,bIsGet,Perception);
	if (bIsGet)
	{
		Perception->PerceptionEvent(Event);
	}
}

void UGBWPowerfulPerceptionComponent::EffectivePerception_Server_Implementation(const FGBWCognitionItem& NewCognition)
{
	EffectivePerception_Multicast(NewCognition);
}
void UGBWPowerfulPerceptionComponent::EffectivePerception_Multicast_Implementation(
	const FGBWCognitionItem& NewCognition)
{
	EffectivePerception_Exe(NewCognition);
}
