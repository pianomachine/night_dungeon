// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GBWPowerfulPerceptionComponent.h"
#include "Engine/CancellableAsyncAction.h"
#include "Tickable.h"
#include "GBWAsyncAction_ExecutePerception.generated.h"

UCLASS()
class GBWPOWERFULPERCEPTION_API UGBWAsyncAction_ExecutePerception : public UCancellableAsyncAction, public FTickableGameObject
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGBWPPCognitionDlg, FGameplayTag, CognitionKey, FGBWCognitionItem, Cognition);
	GENERATED_BODY()

public:
	// FTickableGameObject Begin
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;
	// FTickableGameObject End
	
	UFUNCTION(BlueprintCallable, DisplayName="Task_ExecutePerception",
		meta=(DefaultToSelf="InActor", BlueprintInternalUseOnly="true"),
		Category = "GBW|Perception")
	static UGBWAsyncAction_ExecutePerception* GBWAsyncAction_ExecutePerception(
		AActor* InActor,
		FGameplayTag InPerceptionKey,
		TSubclassOf<AGBWPerceptionBase> InPerceptionClass,
		FGBWStruct PerceptionParamStruct);

	UPROPERTY(BlueprintAssignable)
	FGBWPPCognitionDlg OnUpdate;

	UPROPERTY(BlueprintAssignable)
	FGBWPPCognitionDlg OnComplete;
	
	UPROPERTY(BlueprintAssignable)
	FGBWPPCognitionDlg OnFail;
	
	// Start UCancellableAsyncAction Functions
	virtual void Activate() override;
	virtual void Cancel() override;
	// End UCancellableAsyncAction Functions

	// Start UObject Functions
	virtual UWorld* GetWorld() const override
	{
		return ContextWorld.IsValid() ? ContextWorld.Get() : nullptr;
	}
	// End UObject Functions

private:
	/** The context world of this action. */
	TWeakObjectPtr<UWorld> ContextWorld = nullptr;

	UPROPERTY()
	UGBWPowerfulPerceptionComponent* PerceptionComponent = nullptr;
	FGameplayTag PerceptionKey = FGameplayTag();
	TSubclassOf<AGBWPerceptionBase> PerceptionClass = nullptr;
	FGBWStruct PerceptionParamsStr = FGBWStruct();
	bool bIsCompleted = false;
	float Timer = 0.0f;
};
