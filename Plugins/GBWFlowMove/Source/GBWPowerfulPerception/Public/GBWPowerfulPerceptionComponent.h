// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GBWPerceptionBase.h"
#include "Components/ActorComponent.h"
#include "GBWPowerfulPerceptionComponent.generated.h"

USTRUCT(BlueprintType, DisplayName="GBWCognitionItem")
struct FGBWCognitionItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGameplayTag Key = FGameplayTag();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float LastUpdateTime = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float EffectiveTime = 0.1f;
	UPROPERTY()
	FGBWStruct CognitionContent = FGBWStruct();

	FGBWCognitionItem()
	{}
	FGBWCognitionItem(FGameplayTag InKey, const float InEffectiveTime, const FGBWStruct& InCognition, float InLastUpdateTime)
	{
		Key = InKey;
		EffectiveTime = InEffectiveTime;
		CognitionContent = InCognition;
		LastUpdateTime = InLastUpdateTime;
	}

	FString GetDebugStr() const;
	bool IsValidCognition() const;
};
USTRUCT(BlueprintType, DisplayName="GBWCognition")
struct FGBWCognition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TArray<FGBWCognitionItem> Cognition = {};

	FGBWCognition(){}

	void SetCognition(const FGBWCognitionItem& NewCognition);
	bool GetCognition(FGameplayTag Key, FGBWCognitionItem& Result);
	void RemoveCognition(FGameplayTag Key);

	void Clear();

	FString GetDebugStr() const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGBWPPCognitionDlg, FGameplayTag, CognitionKey, FGBWCognitionItem, Cognition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGBWPPCognitionUpdateDlg);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GBWPOWERFULPERCEPTION_API UGBWPowerfulPerceptionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGBWPowerfulPerceptionComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "GBW|Perception")
	FGBWPPCognitionDlg OnCognitionChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "GBW|Perception")
	FGBWPPCognitionDlg OnCognitionSnapshotChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "GBW|Perception")
	FGBWPPCognitionDlg OnEffectivePerception;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "GBW|Perception")
	FGBWPPCognitionUpdateDlg OnUpdate;

	UPROPERTY(Replicated)
	TArray<FGameplayTag> PerceptionKeyDict;
private:
	UPROPERTY(Replicated)
	TArray<AGBWPerceptionBase*> Perceptions;
	
	UPROPERTY(Replicated)
	FGBWCognition Cognition = FGBWCognition();
	
	UPROPERTY()
	FGBWCognition Cognition_Local = FGBWCognition();

	UPROPERTY()
	FGBWCognition EffectivePerceptionNow = FGBWCognition();

	UPROPERTY(Replicated)
	FGBWCognition CognitionSnapshot = FGBWCognition();
	
	UPROPERTY()
	FGBWCognition CognitionSnapshot_Local = FGBWCognition();

private:
	//Cognition - Start
	void CheckCognitionValid();
	
	void SetCognition_Imp(const FGBWCognitionItem& NewCognition, bool bIsReplicated, bool bIsEffectivePerception);
	UFUNCTION(Reliable, Server)
	void SetCognition_Server(const FGBWCognitionItem& NewCognition);
	UFUNCTION(Reliable, NetMulticast)
	void SetCognition_Multicast(const FGBWCognitionItem& NewCognition);
	void SetCognition_Local(const FGBWCognitionItem& NewCognition);
	
	UFUNCTION(Reliable, Server)
	void RemoveCognition_Server(FGameplayTag Key);
	//Cognition - End

	//CognitionSnapshot - Start
	UFUNCTION(Reliable, Server)
	void SetCognitionSnapshot_Server(const FGBWCognitionItem& NewCognition);
	UFUNCTION(Reliable, NetMulticast)
	void SetCognitionSnapshot_Multicast(const FGBWCognitionItem& NewCognition);
	void SetCognitionSnapshot_Local(const FGBWCognitionItem& NewCognition);
	
	UFUNCTION(Reliable, Server)
	void RemoveCognitionSnapshot_Server(FGameplayTag Key);
	//CognitionSnapshot - End

	//Perception - Start
	UFUNCTION(Reliable, Server, Category = "GBW|Perception")
	void ExecutePerception_Server(
		const FGuid& PerceptionGuid,
		FGameplayTag PerceptionKey,
		TSubclassOf<AGBWPerceptionBase> PerceptionClass,
		const FGBWStruct& PerceptionParamsStr = FGBWStruct());
	void ExecutePerception_Imp(
		const FGuid& PerceptionGuid,
		const int PerceptionKeyIndex,
		TSubclassOf<AGBWPerceptionBase> PerceptionClass,
		const FGBWStruct& PerceptionParamsStr = FGBWStruct());

	void AddPerception(AGBWPerceptionBase* NewPerception);
	
	UFUNCTION(Reliable, Server, Category = "GBW|Perception")
	void RemovePerception_Server(const FGuid& PerceptionGuid);
	void RemovePerception_Imp(const FGuid& PerceptionGuid);
	//Perception - End

	//EffectivePerception - Start
	void EffectivePerception_Imp(const FGBWCognitionItem& NewCognition, bool bIsReplicated);
	UFUNCTION(Reliable, Server)
	void EffectivePerception_Server(const FGBWCognitionItem& NewCognition);
	UFUNCTION(Reliable, NetMulticast)
	void EffectivePerception_Multicast(const FGBWCognitionItem& NewCognition);
	void EffectivePerception_Exe(const FGBWCognitionItem& NewCognition);

	void GetEffectivePerception(FGameplayTag Key, bool& bIsGet, FGBWCognitionItem& Result);
	//EffectivePerception - End
	
public:
	UFUNCTION(BlueprintCallable, Category = "GBW|Perception", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static UGBWPowerfulPerceptionComponent* PP_GetGBWPerceptionComponent(AActor* InActor);
	
	//Cognition - Start
	UFUNCTION(BlueprintCallable, Category = "GBW|Perception|Cognition",
			CustomThunk,
			meta = (DefaultToSelf="InActor", CustomStructureParam = "CognitionStruct", BlueprintThreadSafe))
	static void PP_SetCognition(
		AActor* InActor,
		bool bIsReplicated,
		bool bIsEffectivePerception,
		FGameplayTag Key, float EffectiveTime,
		const int32& CognitionStruct){}
	DECLARE_FUNCTION(execPP_SetCognition);

	UFUNCTION(BlueprintCallable, 
		CustomThunk,
		meta = (DefaultToSelf="InActor", CustomStructureParam = "CognitionStruct", ExpandBoolAsExecs = "bIsSuccess", BlueprintThreadSafe),
		Category = "GBW|Perception|Cognition")
	static void PP_GetCognitionAsStruct(
		AActor* InActor,
		FGameplayTag Key,
		bool& bIsSuccess, 
		const int32& CognitionStruct) {}
	DECLARE_FUNCTION(execPP_GetCognitionAsStruct);

	static bool _GetCognitionAsStruct(AActor* InActor, FGameplayTag Key, void* StructPtr, UScriptStruct* ScriptStruct);

	UFUNCTION(BlueprintCallable, 
		CustomThunk,
		meta = (CustomStructureParam = "CognitionStruct", ExpandBoolAsExecs = "bIsSuccess", BlueprintThreadSafe),
		Category = "GBW|Perception|Cognition")
	static void PP_CognitionToStruct(
		FGBWCognitionItem InCognition,
		bool& bIsSuccess, 
		const int32& CognitionStruct) {}
	DECLARE_FUNCTION(execPP_CognitionToStruct);

	void SetStructAsCognition(FGameplayTag Key, const float EffectiveTime, const void* StructPtr, const UScriptStruct* ScriptStruct, bool bIsReplicated, bool bIsEffectivePerception);
	
	void GetCognition(FGameplayTag Key, bool& bIsGet, FGBWCognitionItem& Result);
	bool GetCognitionAsStruct_(FGameplayTag Key, void* StructPtr, UScriptStruct* ScriptStruct);
	UFUNCTION(BlueprintCallable, Category="GBW|Perception|Cognition")
	FString GetCognitionDebugStr() const;

	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf="InActor", BlueprintThreadSafe), Category = "GBW|Perception|Cognition")
	static void PP_RemoveCognition(AActor* InActor, FGameplayTag Key);
	UFUNCTION(BlueprintCallable, Category="GBW|Perception|Cognition", meta=(BlueprintThreadSafe))
	void RemoveCognition(FGameplayTag Key);
	//Cognition - End

	//CognitionSnapshot - Start
	UFUNCTION(BlueprintCallable, Category = "GBW|Perception|Cognition", meta = (DefaultToSelf="InActor", BlueprintThreadSafe))
	static void PP_SnapshotCognition(
		AActor* InActor,
		FGameplayTag Key,
		bool bReset,
		bool bIsReplicated,
		bool bIsEffectivePerception);

	UFUNCTION(BlueprintCallable, Category = "GBW|Perception|Cognition", meta = (DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool PP_HasCognitionSnapshot(
		AActor* InActor,
		FGameplayTag Key);

	UFUNCTION(BlueprintCallable, 
		CustomThunk,
		meta = (DefaultToSelf="InActor", CustomStructureParam = "CognitionStruct", ExpandBoolAsExecs = "bIsSuccess", BlueprintThreadSafe),
		Category = "GBW|Perception|Cognition")
	static void PP_GetCognitionSnapshotAsStruct(
		AActor* InActor,
		FGameplayTag Key,
		bool& bIsSuccess, 
		const int32& CognitionStruct) {}
	DECLARE_FUNCTION(execPP_GetCognitionSnapshotAsStruct);

	static bool _GetCognitionSnapshotAsStruct(AActor* InActor, FGameplayTag Key, void* StructPtr, UScriptStruct* ScriptStruct);

	UFUNCTION(BlueprintCallable, Category = "GBW|Perception|Cognition", meta=(BlueprintThreadSafe))
	void SnapshotCognition(const FGameplayTag Key, bool bReset, bool bIsReplicated, bool bIsEffectivePerception);
	UFUNCTION(BlueprintCallable, Category = "GBW|Perception|Cognition", meta=(BlueprintThreadSafe))
	bool HasCognitionSnapshot(const FGameplayTag Key);
	
	void GetCognitionSnapshot(FGameplayTag Key, bool& bIsGet, FGBWCognitionItem& Result);
	bool GetCognitionSnapshotAsStruct_(FGameplayTag Key, void* StructPtr, UScriptStruct* ScriptStruct);
	UFUNCTION(BlueprintCallable, Category="GBW|Perception|Cognition")
	FString GetCognitionSnapshotDebugStr() const;

	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf="InActor", BlueprintThreadSafe), Category = "GBW|Perception|Cognition")
	static void PP_RemoveCognitionSnapshot(AActor* InActor, FGameplayTag Key);
	UFUNCTION(BlueprintCallable, Category="GBW|Perception|Cognition", meta=(BlueprintThreadSafe))
	void RemoveCognitionSnapshot(FGameplayTag Key);
	//CognitionSnapshot - End

	//Perception - Start
	UFUNCTION(BlueprintCallable,Category = "GBW|Perception", meta=(BlueprintThreadSafe))
	bool HasPerception(
		FGameplayTag PerceptionKey,
		TSubclassOf<AGBWPerceptionBase> PerceptionClass);

	UFUNCTION(BlueprintCallable,Category = "GBW|Perception",
		meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool PP_HasPerception(
		AActor* InActor,
		FGameplayTag PerceptionKey,
		TSubclassOf<AGBWPerceptionBase> PerceptionClass);

	UFUNCTION(BlueprintCallable,Category = "GBW|Perception",
		meta=(BlueprintThreadSafe, ExpandBoolAsExecs="bIsGet"))
	void GetPerception(FGameplayTag PerceptionKey, bool& bIsGet, AGBWPerceptionBase*& Perception);
	void GetPerception(int PerceptionKeyIndex, bool& bIsGet, AGBWPerceptionBase*& Perception);

	UFUNCTION(BlueprintCallable,Category = "GBW|Perception",
		meta=(DefaultToSelf="InActor", BlueprintThreadSafe, ExpandBoolAsExecs="bIsGet"))
	static void PP_GetPerception(AActor* InActor, FGameplayTag PerceptionKey, bool& bIsGet, AGBWPerceptionBase*& Perception);

	UFUNCTION(BlueprintCallable, Category = "GBW|Perception",
		CustomThunk,
		meta = (DefaultToSelf="InActor", CustomStructureParam = "PerceptionParamStruct", BlueprintThreadSafe))
	static void PP_ExecutePerception(
		AActor* InActor,
		FGuid& PerceptionGuid,
		FGameplayTag PerceptionKey,
		TSubclassOf<AGBWPerceptionBase> PerceptionClass,
		const int32& PerceptionParamStruct){}
	DECLARE_FUNCTION(execPP_ExecutePerception);
	static void _ExecutePerception(
		AActor* InActor,
		FGameplayTag PerceptionKey,
		TSubclassOf<AGBWPerceptionBase> PerceptionClass,
		const FGBWStruct& PerceptionParams = FGBWStruct());

	void ExecutePerception(
		FGuid& PerceptionGuid,
		FGameplayTag PerceptionKey,
		TSubclassOf<AGBWPerceptionBase> PerceptionClass,
		const FGBWStruct& PerceptionParamsStr = FGBWStruct());

	UFUNCTION(BlueprintCallable,Category = "GBW|Perception", meta=(BlueprintThreadSafe))
	void RemovePerceptionByKey(FGameplayTag PerceptionKey);
	UFUNCTION(BlueprintCallable,Category = "GBW|Perception", meta=(BlueprintThreadSafe))
	void RemovePerception(FGuid PerceptionGuid);
	UFUNCTION(BlueprintCallable,Category = "GBW|Perception", meta = (DefaultToSelf="InActor", BlueprintThreadSafe))
	static void PP_RemovePerception(AActor* InActor, FGameplayTag PerceptionKey);
	//Perception - End

	//EffectivePerception - Start
	UFUNCTION(BlueprintCallable, Category = "GBW|Perception|EffectivePerception",
			CustomThunk,
			meta = (DefaultToSelf="InActor", CustomStructureParam = "CognitionStruct", BlueprintThreadSafe))
	static void PP_EffectivePerception(
		AActor* InActor,
		bool bIsReplicated,
		FGameplayTag Key,
		const int32& CognitionStruct){}
	DECLARE_FUNCTION(execPP_EffectivePerception);

	UFUNCTION(BlueprintCallable, Category="GBW|Perception|EffectivePerception")
	FString GetEffectivePerceptionDebugStr() const;
	
	UFUNCTION(BlueprintCallable, 
		CustomThunk,
		meta = (DefaultToSelf="InActor", CustomStructureParam = "CognitionStruct", ExpandBoolAsExecs = "bIsSuccess", BlueprintThreadSafe),
		Category = "GBW|Perception|EffectivePerception")
	static void PP_GetEffectivePerceptionAsStruct(
		AActor* InActor,
		FGameplayTag Key,
		bool& bIsSuccess, 
		const int32& CognitionStruct) {}
	DECLARE_FUNCTION(execPP_GetEffectivePerceptionAsStruct);

	void EffectivePerceptionFromStruct(FGameplayTag Key, const void* StructPtr, const UScriptStruct* ScriptStruct, bool bIsReplicated);
	//EffectivePerception - End

	//PerceptionEvent - Start
	UFUNCTION(BlueprintCallable, Category = "GBW|Perception|Event", meta = (BlueprintThreadSafe))
	void SendPerceptionEvent(FGameplayTag PerceptionKey, FName Event);
	UFUNCTION(BlueprintCallable, Reliable, Server, Category = "GBW|Perception|Event", meta = (BlueprintThreadSafe))
	void SendPerceptionEvent_Server(int PerceptionKeyIndex, FName Event);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Perception|Event",
			meta = (DefaultToSelf="InActor", BlueprintThreadSafe))
	static void PP_SendPerceptionEvent(
		AActor* InActor,
		FGameplayTag PerceptionKey,
		FName Event);
	//EffectivePerception - End
};