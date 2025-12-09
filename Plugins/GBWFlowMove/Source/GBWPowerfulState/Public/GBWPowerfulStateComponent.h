// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AITypes.h"
#include "GameplayTagContainer.h"
#include "StateTreeInstanceData.h"
#include "Components/ActorComponent.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "StateTreeReference.h"
#include "StateTreeExecutionContext.h"
#include "GBWPowerfulStateComponent.generated.h"

USTRUCT(BlueprintType)
struct FPS_ActionLockState
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	int ForceLockFrames = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float ForceLockTime = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bHasLock = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TArray<FGameplayTagQuery> ActionToLock;
	UPROPERTY(BlueprintReadOnly, Category="Settings")
	TArray<FString> ActionToLockDescription;

	FPS_ActionLockState(){}
	FPS_ActionLockState(const bool InHasLock, const FGameplayTagQuery& InActionToLock)
	{
		bHasLock = InHasLock;
		ActionToLock = {InActionToLock};
	}
	FPS_ActionLockState(const bool InHasLock, const TArray<FGameplayTagQuery>& InActionToLock)
	{
		bHasLock = InHasLock;
		ActionToLock = InActionToLock;
	}
	FPS_ActionLockState(const float InForceLockTime)
	{
		ForceLockTime = InForceLockTime;
	}
	FPS_ActionLockState(const int InForceLockFrames)
	{
		ForceLockFrames = InForceLockFrames;
	}
	bool IsLock(FGameplayTag ForActionTag) const;
	bool IsLock(const FGameplayTagContainer& ForActionTag) const;
	void RemoveActionLock(const FGameplayTagQuery& InActionToLock);
	bool IsActionToLockNoChange(TArray<FGameplayTagQuery> InActionToLock) const;
	
	void UpdateLockState(float DeltaTime);
};
USTRUCT(BlueprintType)
struct FPS_ActionStateItem
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FName Slot = FName();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FPS_ActionLockState ActionLockState = FPS_ActionLockState();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FGameplayTag LastActionTag = FGameplayTag();
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float LastActionExecutedTime = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FGameplayTag CurrentActionTag = FGameplayTag();
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float CurrentActionExecutedTime = 0.0f;
	UPROPERTY()
	bool bIsActionChanged = false;

	FPS_ActionStateItem(){}
	FPS_ActionStateItem(const FName InSlot, const FPS_ActionLockState& InActionLockState);
	FPS_ActionStateItem(const FName InSlot, const FGameplayTag InCurrentActionTag);

	void Update(float DeltaTime);
	void SetCurrentAction(FGameplayTag ActionTag);
};

UENUM(BlueprintType)
enum EPS_StateEventType
{
	Add,
	Change,
	Remove,
	Reset
};
USTRUCT(BlueprintType, meta = (DisplayName = "PS_Float"))
struct FPS_Float
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	int Key = -1;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float Value = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float Time = -1.0f;

	inline bool operator==(const FPS_Float& Other) const
	{
		return Key == Other.Key && Value == Other.Value;
	}
	FPS_Float(){}
	FPS_Float(const int InKey, const float InValue, const float InTime)
	{
		Key = InKey;
		Value = InValue;
		Time = InTime;
	}
};
USTRUCT(BlueprintType, meta = (DisplayName = "PS_Bool"))
struct FPS_Bool
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	int Key = -1;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	bool Value = false;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float Time = -1.0f;
	
	inline bool operator==(const FPS_Bool& Other) const
	{
		return Key == Other.Key && Value == Other.Value;
	}
	FPS_Bool(){}
	FPS_Bool(const int InKey, const bool InValue, const float InTime)
	{
		Key = InKey;
		Value = InValue;
		Time = InTime;
	}
};
USTRUCT(BlueprintType, meta = (DisplayName = "PS_Vector"))
struct FPS_Vector
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	int Key = -1;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector Value = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float Time = -1.0f;

	inline bool operator==(const FPS_Vector& Other) const
	{
		return Key == Other.Key && Value == Other.Value;
	}
	FPS_Vector(){}
	FPS_Vector(const int InKey, const FVector& InValue, const float InTime)
	{
		Key = InKey;
		Value = InValue;
		Time = InTime;
	}
};
USTRUCT(BlueprintType, meta = (DisplayName = "PS_Text"))
struct FPS_Text
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	int Key = -1;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FString Value = "";
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float Time = -1.0f;

	inline bool operator==(const FPS_Text& Other) const
	{
		return Key == Other.Key && Value == Other.Value;
	}
	FPS_Text(){}
	FPS_Text(const int InKey, const FString& InValue, const float InTime)
	{
		Key = InKey;
		Value = InValue;
		Time = InTime;
	}
};
USTRUCT(BlueprintType, meta = (DisplayName = "PS_Actor"))
struct FPS_Actor
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	int Key = -1;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	AActor* Value = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float Time = -1.0f;

	inline bool operator==(const FPS_Actor& Other) const
	{
		return Key == Other.Key && Value == Other.Value;
	}
	FPS_Actor(){}
	FPS_Actor(const int InKey, AActor* InValue, const float InTime)
	{
		Key = InKey;
		Value = InValue;
		Time = InTime;
	}
};
USTRUCT(BlueprintType)
struct FPS_StateOperation_F
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EPS_StateEventType> OperationType = EPS_StateEventType::Add;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FPS_Float StateElement = FPS_Float();

	FPS_StateOperation_F(){}
	FPS_StateOperation_F(const EPS_StateEventType InOperationType, const FPS_Float InStateElement)
	{
		OperationType = InOperationType;
		StateElement = InStateElement;
	}
};
USTRUCT(BlueprintType)
struct FPS_StateOperation_B
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EPS_StateEventType> OperationType = EPS_StateEventType::Add;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FPS_Bool StateElement = FPS_Bool();

	FPS_StateOperation_B(){}
	FPS_StateOperation_B(const EPS_StateEventType InOperationType, const FPS_Bool InStateElement)
	{
		OperationType = InOperationType;
		StateElement = InStateElement;
	}
};
USTRUCT(BlueprintType)
struct FPS_StateOperation_A
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EPS_StateEventType> OperationType = EPS_StateEventType::Add;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FPS_Actor StateElement = FPS_Actor();

	FPS_StateOperation_A(){}
	FPS_StateOperation_A(const EPS_StateEventType InOperationType, const FPS_Actor& InStateElement)
	{
		OperationType = InOperationType;
		StateElement = InStateElement;
	}
};
USTRUCT(BlueprintType)
struct FPS_StateOperation_Text
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EPS_StateEventType> OperationType = EPS_StateEventType::Add;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FPS_Text StateElement = FPS_Text();

	FPS_StateOperation_Text(){}
	FPS_StateOperation_Text(const EPS_StateEventType InOperationType, const FPS_Text& InStateElement)
	{
		OperationType = InOperationType;
		StateElement = InStateElement;
	}
};
USTRUCT(BlueprintType)
struct FPS_StateOperation_V
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EPS_StateEventType> OperationType = EPS_StateEventType::Add;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FPS_Vector StateElement = FPS_Vector();

	FPS_StateOperation_V(){}
	FPS_StateOperation_V(const EPS_StateEventType InOperationType, const FPS_Vector& InStateElement)
	{
		OperationType = InOperationType;
		StateElement = InStateElement;
	}
};
USTRUCT(BlueprintType, meta = (DisplayName = "PS_State"))
struct FPS_State
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TArray<FGameplayTag> GTKeyDict;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TArray<FName> StateKeyDict;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TArray<FPS_Float> FState;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TArray<FPS_Bool> BState;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TArray<FPS_Actor> AState;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TArray<FPS_Text> TextState;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TArray<FPS_Vector> VState;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Settings")
	TArray<FPS_StateOperation_F> FStateOperations;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Settings")
	TArray<FPS_StateOperation_B> BStateOperations;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Settings")
	TArray<FPS_StateOperation_A> AStateOperations;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Settings")
	TArray<FPS_StateOperation_Text> TStateOperations;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Settings")
	TArray<FPS_StateOperation_V> VStateOperations;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	int MaxStateOperationNum = 2048;
	
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	int FStateOperationIndexNow = -1;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	int BStateOperationIndexNow = -1;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	int AStateOperationIndexNow = -1;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	int TStateOperationIndexNow = -1;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	int VStateOperationIndexNow = -1;

	int GetKeyIndex(const FName Key, const bool AutoAdd);
	int GetGTKeyIndex(const FGameplayTag Key, const bool AutoAdd);
	FName GetKey(const int Index);
	FGameplayTag GetGTKey(const int Index);

	void RemoveState(const FName Key, const float Time);
	void RemoveState(const int KeyIndex, const float Time);

	void SetState(const FName Key, const float Value, const float Time, const bool bReset);
	void SetState(const FName Key, const bool Value, const float Time, const bool bReset);
	void SetState(FName Key, AActor* Value, float Time, const bool bReset = false);
	void SetState(FName Key, const FString& Value, const float Time, const bool bReset);
	void SetState(FName Key, const FVector& Value, float Time, const bool bReset);

	bool GetValue(const FName Key, float& Result, float& Time);
	bool GetValue(const FName Key, bool& Result, float& Time);
	bool GetValue(const FName Key, AActor*& Result, float& Time);
	bool GetValue(const FName Key, FString& Result, float& Time);
	bool GetValue(const FName Key, FVector& Result, float& Time);

	bool GetValue_History(const FName Key, const int DistanceFromNow, float& Result, float& Time);
	bool GetValue_History(const FName Key, const int DistanceFromNow, bool& Result, float& Time);
	bool GetValue_History(const FName Key, const int DistanceFromNow, AActor*& Result, float& Time);
	bool GetValue_History(const FName Key, const int DistanceFromNow, FString& Result, float& Time);
	bool GetValue_History(const FName Key, const int DistanceFromNow, FVector& Result, float& Time);

	void AddStateOperation(const FPS_StateOperation_F& NewStateOperation);
	void AddStateOperation(const FPS_StateOperation_B& NewStateOperation);
	void AddStateOperation(const FPS_StateOperation_A& NewStateOperation);
	void AddStateOperation(const FPS_StateOperation_Text& NewStateOperation);
	void AddStateOperation(const FPS_StateOperation_V& NewStateOperation);

	bool CheckStateChangeType(const FName Key,const float Value,EPS_StateEventType& ChangeType,float& OldValue,float& NewValue);
	bool CheckStateChangeType(const FName Key,const bool Value,EPS_StateEventType& ChangeType,bool& OldValue,bool& NewValue);
	bool CheckStateChangeType(const FName Key,AActor* Value,EPS_StateEventType& ChangeType,AActor*& OldValue,AActor*& NewValue);
	bool CheckStateChangeType(const FName Key,const FString& Value,EPS_StateEventType& ChangeType,FString& OldValue,FString& NewValue);
	bool CheckStateChangeType(const FName Key,const FVector& Value,EPS_StateEventType& ChangeType,FVector& OldValue,FVector& NewValue);
	
	FString GetString(const UWorld* InWorld, int StateOperationNum);
};

USTRUCT(BlueprintType, meta = (DisplayName = "PS_StateTree"))
struct FPS_StateTree
{
	GENERATED_USTRUCT_BODY()
	
	/** If true, the StateTree logic is started on being play. Otherwise StartLogic() needs to be called. */
	UPROPERTY(EditAnywhere, Category="GBW|PowerfulState")
	bool bStartStateTreeAutomatically = true;
	
	UPROPERTY(EditAnywhere, Category="GBW|PowerfulState", meta=(Schema="/Script/GBWPowerfulState.GBWPS_STSchema"))
	TArray<FStateTreeReference> StateTreeSet;
	
	UPROPERTY(Transient)
	TArray<FStateTreeInstanceData> InstanceDataSet;
	/** if set, state tree execution is allowed */
	uint8 bIsRunning : 1;
	/** if set, execution requests will be postponed */
	uint8 bIsPaused : 1;

	FPS_StateTree(): bIsRunning(0), bIsPaused(0)
	{
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GBWPOWERFULSTATE_API UGBWPowerfulStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGBWPowerfulStateComponent();
	// BEGIN UActorComponent overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	// END UActorComponent overrides

	// BEGIN UBrainComponent overrides
	void StartLogic();
	void RestartLogic();
	void StopLogic(const FString& Reason);
	void PauseLogic(const FString& Reason);
	EAILogicResuming::Type ResumeLogic(const FString& Reason);
	bool IsRunning() const;
	bool IsPaused() const;
	// END UBrainComponent overrides

	bool SetContextRequirements(FStateTreeExecutionContext& Context, bool bLogErrors = false) const;
	
	/** Sends event to the running StateTree. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay|StateTree")
	void SendStateTreeEvent(const FStateTreeEvent& Event);
	UFUNCTION(BlueprintCallable, Category = "Gameplay|StateTree")
	void SendSTEvent(const FGameplayTag InTag, bool bToServer);
	UFUNCTION(Server, Reliable, Category = "Gameplay|StateTree")
	void SendStateTreeEventToServerFast_Server(const int EventTagIndex);
	UFUNCTION(Server, Reliable, Category = "Gameplay|StateTree")
	void SendStateTreeEventToServer_Server(const FGameplayTag& InTag);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="GBW|PowerfulState|Settings")
	int MaxStateOperationNum = 2048;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="GBW|PowerfulState|Debug")
	bool bIsDebug;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="GBW|PowerfulState|Debug")
	int StateOperationNum = 10;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="GBW|PowerfulState|Debug")
	FColor DebugTextColor = FColor::Green;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="GBW|PowerfulState|Debug")
	FColor DebugTextColor2 = FColor::Yellow;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="GBW|PowerfulState|Debug")
	bool bPrintToScreen = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="GBW|PowerfulState|Debug")
	bool bPrintToLog = true;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|PowerfulState")
	FGuid PowerfulStateGuid = FGuid::NewGuid();
	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|PowerfulState")
	FPS_State PowerfulState;
	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|PowerfulState")
	TArray<FPS_ActionStateItem> ActionState;

	UPROPERTY(EditAnywhere, Category="GBW|PowerfulState")
	FPS_StateTree StateTree;
	
//State BEGIN
	bool HasNetAuth() const;

	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	void RemoveState(FName Key);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void RemoveState_Server(FName Key);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void RemoveStateFast_Server(int KeyIndex);

	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	void SetStateFloat(FName Key,float Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateFloat_Server(FName Key,float Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateFloatFast_Server(int KeyIndex,float Value, const bool bReset);
	
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	void SetStateVector(FName Key,FVector Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateVector_Server(FName Key,FVector Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateVectorFast_Server(int KeyIndex, FVector Value, const bool bReset);
	
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	void SetStateActor(FName Key,AActor* Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateActor_Server(FName Key,AActor* Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateActorFast_Server(int KeyIndex,AActor* Value, const bool bReset);
	
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	void SetStateText(FName Key, const FString Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateText_Server(FName Key, const FString& Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateTextFast_Server(int Key, const FString& Value, const bool bReset);

	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	void SetStateBool(FName Key,bool Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateBool_Server(FName Key,bool Value, const bool bReset);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetStateBoolFast_Server(int Key,bool Value, const bool bReset);

	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetFValue(FName Key, float& Result, float& Duration);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetVValue(FName Key, FVector& Result, float& Duration);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetActorValue(FName Key, AActor*& Result, float& Duration);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetTextValue(FName Key, FString& Result, float& Duration);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetBoolValue(FName Key, bool& Result, float& Duration);

	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetFValue_History(FName Key, int DistanceFromNow, float& Result, float& Duration);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetVValue_History(FName Key, int DistanceFromNow, FVector& Result, float& Duration);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetActorValue_History(FName Key, int DistanceFromNow, AActor*& Result, float& Duration);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetTextValue_History(FName Key, int DistanceFromNow, FString& Result, float& Duration);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component")
	bool GetBoolValue_History(FName Key, int DistanceFromNow, bool& Result, float& Duration);
//State END

//ActionState BEGIN
	bool GetActionState(FName Slot, FPS_ActionStateItem& ResActionState, int& Index) const;
	
	UFUNCTION(BlueprintCallable, Category = "GBW|PS|Component")
	void AddActionLockState(
		FName Slot,
		FGameplayTagQuery ActionToLock);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void AddActionLockState_Server(
		FName Slot,
		FGameplayTagQuery ActionToLock);
	void AddActionLockState_Imp(
		FName Slot,
		const FGameplayTagQuery& ActionToLock);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS|Component")
	void SetActionLockStates(
		FName Slot,
		bool bHasActionLock,
		TArray<FGameplayTagQuery> ActionToLock);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void SetActionLockStates_Server(
		FName Slot,
		bool bHasActionLock,
		const TArray<FGameplayTagQuery>& ActionToLock);
	void SetActionLockStates_Imp(
		FName Slot,
		bool bHasActionLock,
		const TArray<FGameplayTagQuery>& ActionToLock);

	UFUNCTION(BlueprintCallable, Category = "GBW|PS|Component")
	void RemoveActionLockState(
		FName Slot,
		FGameplayTagQuery ActionToLock);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component")
	void RemoveActionLockState_Server(
		FName Slot,
		FGameplayTagQuery ActionToLock);
	void RemoveActionLockState_Imp(
		FName Slot,
		const FGameplayTagQuery& ActionToLock);

	UFUNCTION(BlueprintCallable, Category = "BW|PS|Component")
	bool IsActionLock(FName Slot, FGameplayTagContainer ActionToLock) const;
	UFUNCTION(BlueprintCallable, Category = "BW|PS|Component")
	bool IsAnyActionLock(FName Slot) const;

	UFUNCTION(BlueprintCallable, Category = "BW|PS|Component")
	void LockAllActionByTime(FName Slot, float LockTIme = 0.1f);
	UFUNCTION(Reliable,Server,Category = "BW|PS|Component")
	void LockAllActionByTime_Server(FName Slot, float LockTIme = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "BW|PS|Component")
	void LockAllActionByFrames(FName Slot, int LockFrames = 3);
	UFUNCTION(Reliable,Server,Category = "BW|PS|Component")
	void LockAllActionByFrames_Server(FName Slot, int LockFrames = 3);

	UFUNCTION(BlueprintCallable, Category = "BW|PS|Component")
	void SetCurrentActionTag(FName Slot, FGameplayTag ActionTag);
	UFUNCTION(Reliable,Server,Category = "BW|PS|Component")
	void SetCurrentActionTag_Server(FName Slot, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "BW|PS|Component")
	void GetCurrentActionTag(FName Slot, bool& bIsGet, FGameplayTag& ActionTag) const;
//ActionState END

//Debug Begin
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component|Debug")
	void ActivePSDebug(FColor InDebugTextColor = FColor::Green, bool InPrintToScreen = true, bool InPrintToLog = true);
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component|Debug")
	void ActivePSDebug_Server(FColor InDebugTextColor = FColor::Green, bool InPrintToScreen = true, bool InPrintToLog = true);
	UFUNCTION(BlueprintCallable,Category = "GBW|PS|Component|Debug")
	void DeactivePSDebug();
	UFUNCTION(Reliable,Server,Category = "GBW|PS|Component|Debug")
	void DeactivePSDebug_Server();
	
	void PSDebug();
//Debug END
};
