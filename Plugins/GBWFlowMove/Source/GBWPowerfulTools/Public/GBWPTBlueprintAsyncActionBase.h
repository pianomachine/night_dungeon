// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Tickable.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GBWPTBlueprintAsyncActionBase.generated.h"

UCLASS(meta = (HideThen = true))
class GBWPOWERFULTOOLS_API UGBWPTAsync_LoadStreamLevelByName : public UBlueprintAsyncActionBase, public FTickableGameObject
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGBWPTStreamLevelTaskDlg,
		UGBWPTAsync_LoadStreamLevelByName*, Task,
		float, Time,
		float, DeltaTime,
		ULevelStreaming*, StreamLevelInstance);
	
	GENERATED_BODY()
public:

	UGBWPTAsync_LoadStreamLevelByName();
	~UGBWPTAsync_LoadStreamLevelByName();

	// FTickableGameObject Begin
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const;
	virtual bool IsTickableInEditor() const;
	// FTickableGameObject End

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject",
		DisplayName = "PT_LoadStreamLevelByName",
		ScriptName = "PT_LoadStreamLevelByName"),
		Category = "GBW|PT|Task")
	static UGBWPTAsync_LoadStreamLevelByName* PT_LoadStreamLevelByName(
		const UObject* WorldContextObject,
		FName LevelName,
		bool bMakeVisibleAfterLoad = true,
		FTransform LevelTransform = FTransform(),
		float Timeout = -1.0f);

	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Then;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Update;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Completed;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Failed;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Timeout;

	UFUNCTION(BlueprintCallable,Category = "GBW|PT|Task")
		void Stop();

private:

	virtual void Activate() override;
	UFUNCTION()
	void CompletedEvent();
	UFUNCTION()
	void FailedEvent();
	UFUNCTION()
	void TimeoutEvent();
	FTimerHandle SetTimerDelegate(FTimerDynamicDelegate Delegate, float Time);
	
	void PreGarbageCollect();
	// The last frame number we were ticked.
	// We don't want to tick multiple times per frame 
	uint32 LastFrameNumberWeTicked = INDEX_NONE;

	UPROPERTY()
	UWorld* World;
	UPROPERTY()
	UObject* WorldContextObject;
	FTimerHandle TimerHandle;
	float Timer = 0.0f;
	float FrameDeltaTime = 0.0f;
	bool bMakeVisibleAfterLoad = true;
	bool bIsCompleted = false;
	UPROPERTY()
	ULevelStreaming* StreamLevelInstanceNow = nullptr;
	FTransform LevelTransformNow = FTransform();
};

UCLASS(meta = (HideThen = true))
class GBWPOWERFULTOOLS_API UGBWPTAsync_LoadStreamLevel : public UBlueprintAsyncActionBase, public FTickableGameObject
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGBWPTStreamLevelTaskDlg,
		UGBWPTAsync_LoadStreamLevel*, Task,
		float, Time,
		float, DeltaTime,
		ULevelStreaming*, StreamLevelInstance);
	
	GENERATED_BODY()
public:

	UGBWPTAsync_LoadStreamLevel();
	~UGBWPTAsync_LoadStreamLevel();

	// FTickableGameObject Begin
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const;
	virtual bool IsTickableInEditor() const;
	// FTickableGameObject End

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		DisplayName = "PT_LoadStreamLevel",
		ScriptName = "PT_LoadStreamLevel"),
		Category = "GBW|PT|Task")
	static UGBWPTAsync_LoadStreamLevel* PT_LoadStreamLevel(
		ULevelStreaming* StreamLevelInstance,
		bool bMakeVisibleAfterLoad = true,
		bool bSetTransform = false,
		FTransform LevelTransform = FTransform(),
		float Timeout = -1.0f);

	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Then;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Update;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Completed;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Failed;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Timeout;

	UFUNCTION(BlueprintCallable,Category = "GBW|PT|Task")
		void Stop();

private:

	virtual void Activate() override;
	UFUNCTION()
	void CompletedEvent();
	UFUNCTION()
	void FailedEvent();
	UFUNCTION()
	void TimeoutEvent();
	FTimerHandle SetTimerDelegate(FTimerDynamicDelegate Delegate, float Time);
	
	void PreGarbageCollect();
	// The last frame number we were ticked.
	// We don't want to tick multiple times per frame 
	uint32 LastFrameNumberWeTicked = INDEX_NONE;

	UPROPERTY()
	UWorld* World;
	UPROPERTY()
	UObject* WorldContextObject;
	FTimerHandle TimerHandle;
	float Timer = 0.0f;
	float FrameDeltaTime = 0.0f;
	bool bMakeVisibleAfterLoad = true;
	bool bIsCompleted = false;
	UPROPERTY()
	ULevelStreaming* StreamLevelInstanceNow = nullptr;
	FTransform LevelTransformNow = FTransform();
};

UCLASS(meta = (HideThen = true))
class GBWPOWERFULTOOLS_API UGBWPTAsync_UnloadStreamLevel : public UBlueprintAsyncActionBase, public FTickableGameObject
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGBWPTStreamLevelTaskDlg,
		UGBWPTAsync_UnloadStreamLevel*, Task,
		float, Time,
		float, DeltaTime,
		ULevelStreaming*, StreamLevelInstance);
	
	GENERATED_BODY()
public:

	UGBWPTAsync_UnloadStreamLevel();
	~UGBWPTAsync_UnloadStreamLevel();

	// FTickableGameObject Begin
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const;
	virtual bool IsTickableInEditor() const;
	// FTickableGameObject End

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		DisplayName = "PT_UnloadStreamLevel",
		ScriptName = "PT_UnloadStreamLevel"),
		Category = "GBW|PT|Task")
	static UGBWPTAsync_UnloadStreamLevel* PT_UnloadStreamLevel(
		ULevelStreaming* StreamLevelInstance,
		float Timeout = -1.0f);

	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Then;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Update;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Completed;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Failed;
	UPROPERTY(BlueprintAssignable)
		FGBWPTStreamLevelTaskDlg Timeout;

	UFUNCTION(BlueprintCallable,Category = "GBW|PT|Task")
		void Stop();

private:

	virtual void Activate() override;
	UFUNCTION()
	void CompletedEvent();
	UFUNCTION()
	void FailedEvent();
	UFUNCTION()
	void TimeoutEvent();
	FTimerHandle SetTimerDelegate(FTimerDynamicDelegate Delegate, float Time);
	
	void PreGarbageCollect();
	// The last frame number we were ticked.
	// We don't want to tick multiple times per frame 
	uint32 LastFrameNumberWeTicked = INDEX_NONE;

	UPROPERTY()
	UWorld* World;
	UPROPERTY()
	UObject* WorldContextObject;
	FTimerHandle TimerHandle;
	float Timer = 0.0f;
	float FrameDeltaTime = 0.0f;
	bool bIsCompleted = false;
	UPROPERTY()
	ULevelStreaming* StreamLevelInstanceNow = nullptr;
};