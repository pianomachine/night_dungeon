// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWFlowMoveObjects.h"
#include "GameFramework/Character.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Demo/GBWPercetionDemo.h"
#include "GBWFlowMoveFuncLib.generated.h"

UCLASS()
class GBWFLOWMOVE_API UGBWFlowMoveFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove")
	static bool GetSKMeshByTag(
		USkeletalMeshComponent*& SKMeshComp,
		ACharacter* Character,
		FName MeshTag,
		TArray<FName> ExcludedMeshTag,
		UAnimationAsset* TargetAnimationAsset);

	UFUNCTION(BlueprintCallable, Category = "GBW|Net")
	static bool IsLocalOwn(AActor* Actor);
	UFUNCTION(BlueprintCallable, Category = "GBW|Net")
	static bool IsAIPlayer(AActor* Actor);

	static FHitResult SweepMulti(
		const UWorld* World,
		TArray<struct FHitResult>& OutHits,
		const FVector& Start,
		const FVector& End,
		const FQuat& Rot,
		FGBWFlowMoveTraceSetting TraceSetting,
		const FCollisionShape& CollisionShape,
		const FCollisionQueryParams& Params = FCollisionQueryParams::DefaultQueryParam);

	static bool OverlapAnyTest(
		const UWorld* World,
		const FVector& Pos,
		const FQuat& Rot,
		FGBWFlowMoveTraceSetting TraceSetting,
		const FCollisionShape& CollisionShape,
		const FCollisionQueryParams& Params = FCollisionQueryParams::DefaultQueryParam);

	static FHitResult LineTrace(
		const UWorld* World,
		TArray<struct FHitResult>& OutHits,
		const FVector& Start,
		const FVector& End,
		FGBWFlowMoveTraceSetting TraceSetting,
		const FCollisionQueryParams& Params = FCollisionQueryParams::DefaultQueryParam);

	static bool GetTheLastValidHitResult(TArray<FHitResult> HitResultArr, FHitResult& Result);
	
	//Math - start
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Math")
	static FVector RadianToDirectionXY(float Radian);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Math")
	static FVector AngleToDirectionXY(float Angle);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Math")
	static float GetVelocityYawAngle(FVector Velocity);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Math")
	static float GetInputYawAngle(FVector InputDirection);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Math")
	static float GetPointToLineDistance(FVector LineStart, FVector LineEnd, FVector Point);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Math")
	static FVector GetPointToLineFoot(const FVector& LineStart, const FVector& LineEnd, const FVector& Point);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Math")
	static float GetMoveDirection(const ACharacter* OwnerCharacter, const FVector& DirectionVector, bool bIsUseActorSpeedVector = false);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Math")
	static FVector GetMoveVectorFromCharacterInputVector(const ACharacter* OwnerCharacter);
	//Math - end

	//FlowMove - start
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static UGBWFlowMoveComponent* FM_GetFlowMoveComponent(AActor* InActor, bool bTryGetFromParentActor = false);

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_IsFlowMoveControlled(AActor* InActor);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Input", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_SetMoveVector(AActor* InActor, FVector NewMoveVector, float ZeroFaultToleranceDuration = 0.1f);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Input", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_SetMoveVector_(AActor* InActor, float Right, float Forward, float Up, EFMMMoveVectorNormalizeType NormalizeType,float ZeroFaultToleranceDuration = 0.1f);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Input", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_SetControlVector(AActor* InActor, FVector NewControlVector);

	//This method is only effective on local players
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Input", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_UpdateInputFromCharacterInput(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_SetFocusActor(AActor* InActor, USceneComponent* FocusActorComponent, bool bSetViewModeAsActorLock);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_SetViewMode(AActor* InActor, EFMViewMode NewViewMode, bool bAsDefaultViewMode);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_SetSpeedMode(AActor* InActor, EFMSpeedMode NewSpeedMode);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static EFMSpeedMode FM_GetSpeedMode(AActor* InActor);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_SetBodyMode(AActor* InActor, EFMBodyMode NewBodyMode);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static EFMBodyMode FM_GetBodyMode(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_SwitchViewMode(AActor* InActor, TEnumAsByte<EFMViewMode>& ViewModeNow);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_SwitchLockActor(AActor* InActor, bool& bIsLockActor,
	FGBWFlowMoveTraceSetting PerceptionSetting, TArray<FName> ComponentWithTags,float PrejudgeDistance, float FuzzyQueryAngleRestrictions,
	TSubclassOf<UUserWidget> ActorTargetWidgetClass,
	USoundBase* Sound_TargetGet,
	USoundBase* Sound_TargetLost);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_GetFocusActor(AActor* InActor, bool& bIsGet, USceneComponent*& FocusedComponent);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_AddVelocityTrend(AActor* InActor, FGBWFlowMoveVelocityTrend VelocityTrend);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_SetMoveControlInitInfo_Simple(AActor* InActor, const bool bHasTargetPoint, const FVector& TargetPoint);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Condition", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_IsHasMoveVector(AActor* InActor);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool FM_Pure_IsHasMoveVector(const UAnimInstance* AnimInstance);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Condition", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_IsHasUpDownMoveVector(AActor* InActor, bool& HasUpMoveVector, bool& HasDownMoveVector);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Condition", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_IsFallingDistance(AActor* InActor,
		float MinValue,
		float MaxValue);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool FM_Pure_IsFallingDistance(const UAnimInstance* AnimInstance,
		float MinValue,
		float MaxValue);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Condition", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_IsCharacterSpeed(AActor* InActor,
		bool OnlyForHorizontalSpeed,
		bool OnlyForMoveToDirectionSpeed,
		float MinValue,
		float MaxValue);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool FM_Pure_IsCharacterSpeed(const UAnimInstance* AnimInstance,
		bool OnlyForHorizontalSpeed,
		bool OnlyForMoveToDirectionSpeed,
		float MinValue,
		float MaxValue);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Condition", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_IsAngle(AActor* InActor,
		EFlowMoveDirectionType From = EFlowMoveDirectionType::TargetMoveToDirection,
		EFlowMoveDirectionType To = EFlowMoveDirectionType::Velocity,
		float MinValue = 0.0f,
		float MaxValue = 0.0f);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool FM_Pure_IsAngle(const UAnimInstance* AnimInstance,
		EFlowMoveDirectionType From = EFlowMoveDirectionType::TargetMoveToDirection,
		EFlowMoveDirectionType To = EFlowMoveDirectionType::Velocity,
		float MinValue = 0.0f,
		float MaxValue = 0.0f);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Condition", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_IsViewMode(AActor* InActor,
		TEnumAsByte<EInclusivityType> ConditionType,
		TSet<TEnumAsByte<EFMViewMode>> ViewModeSet);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool FM_Pure_IsViewMode(const UAnimInstance* AnimInstance,
		TEnumAsByte<EInclusivityType> ConditionType,
		TSet<TEnumAsByte<EFMViewMode>> ViewModeSet);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool FM_GetMoveControlParam(AActor* InActor, FGBWFlowMoveControlParam& MoveControlParam);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State", meta=(DefaultToSelf="InActor", BlueprintThreadSafe, ExpandBoolAsExecs = "bIsGet"))
	static void GetCustomParam(FGBWFlowMoveControlParam MoveControlParam, FName Key, bool& bIsGet);

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|AnimParam", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_GetMoveParamForAnimAsset(
		AActor* InActor,
		UAnimationAsset* ForAnimAsset,
		UAnimationAsset*& AnimAsset,
		float& Direction,
		float& Speed,
		float& PlayRate,
		float& StartPosition,
		float& EndPosition,
		EFlowMoveDirectionType DirectionType = EFlowMoveDirectionType::Velocity,
		EFMBSAxisType BS_HorizontalAxisFrom = EFMBSAxisType::HorizontalDirection,
		EFMBSAxisType BS_VerticalAxisFrom = EFMBSAxisType::Speed,
		float RefMaxSpeed = 900.0f,
		const bool bUseStep = false,
		const float Step = 8.0f);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|AnimParam", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static void FM_Pure_GetMoveParamForAnimAsset(
		const UAnimInstance* AnimInstance,
		UAnimationAsset* ForAnimAsset,
		UAnimationAsset*& AnimAsset,
		float& Direction,
		float& Speed,
		float& PlayRate,
		float& StartPosition,
		float& EndPosition,
		EFlowMoveDirectionType DirectionType = EFlowMoveDirectionType::Velocity,
		EFMBSAxisType BS_HorizontalAxisFrom = EFMBSAxisType::HorizontalDirection,
		EFMBSAxisType BS_VerticalAxisFrom = EFMBSAxisType::Speed,
		float RefMaxSpeed = 900.0f,
		const bool bUseStep = false,
		const float Step = 8.0f);

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|AnimParam", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_GetMagicPlayerParam(
		AActor* InActor,
		UAnimationAsset* ForAnimAsset,
		FMagicPlayerParam& MagicPlayerParam,
		const bool InReplayNow = false,
		const bool bAutoReplayCodeTime = true,
		const float InReplayCodeTime = 0.1f,
		const bool InLoopAnimation = false,
		const float InBlendTime = 0.2f,
		const float InLoopBlendTime = 0.0f,
		EFlowMoveDirectionType DirectionType = EFlowMoveDirectionType::CurrentMoveToDirection,
		EFMBSAxisType BS_HorizontalAxisFrom = EFMBSAxisType::HorizontalDirection,
		EFMBSAxisType BS_VerticalAxisFrom = EFMBSAxisType::Speed,
		float RefMaxSpeed = 900.0f,
		const bool bUseStep = false,
		const float Step = 8.0f);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|AnimParam", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static void FM_Pure_GetMagicPlayerParam(
		const UAnimInstance* AnimInstance,
		UAnimationAsset* ForAnimAsset,
		FMagicPlayerParam& MagicPlayerParam,
		const bool InReplayNow = false,
		const bool bAutoReplayCodeTime = true,
		const float InReplayCodeTime = 0.1f,
		const bool InLoopAnimation = false,
		const float InBlendTime = 0.2f,
		const float InLoopBlendTime = 0.0f,
		EFlowMoveDirectionType DirectionType = EFlowMoveDirectionType::CurrentMoveToDirection,
		EFMBSAxisType BS_HorizontalAxisFrom = EFMBSAxisType::HorizontalDirection,
		EFMBSAxisType BS_VerticalAxisFrom = EFMBSAxisType::Speed,
		float RefMaxSpeed = 900.0f,
		const bool bUseStep = false,
		const float Step = 8.0f);

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|AnimParam", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_GetMoveParam(
		AActor* InActor,
		UAnimationAsset* ForAnimAsset,
		UAnimSequence*& AnimSequence,
		float& PlayRate,
		float& StartPosition,
		float& EndPosition);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|AnimParam", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static void FM_Pure_GetMoveParam(
		const UAnimInstance* AnimInstance,
		UAnimationAsset* ForAnimAsset,
		float& PlayRate,
		float& StartPosition,
		float& EndPosition,
		UAnimSequence*& AnimSequence);

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Input", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_GetAnimExpectedPlayRate(AActor* InActor, float& PlayRate);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|Input", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static void FM_Pure_GetAnimExpectedPlayRate(const UAnimInstance* AnimInstance, float& PlayRate);
	//FlowMove - end

	//Debug - start
	static void DrawDebugShape(
		const UObject* WorldContext,
		const FVector Location,
		const FQuat Rotation,
		const FCollisionShape CollisionShape,
		const FColor Color,
		float Duration = 0.0f,
		float Thickness = 1.0f);
	static void PrintDebugString(TArray<FString> DebugStrArr, FName Key = NAME_None,const FColor Color = FColor::Red, float Duration = 1.0f);
	static void PrintDebugStringNet(
		const AActor* WorldActor,
		TArray<FString> DebugStrArr, 
		FName Key = NAME_None,
		const FColor ServerColor = FColor::Red,
		const FColor ClientColor = FColor::Green,
		float Duration = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Debug", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_Debug(AActor* InActor);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Debug", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static void FM_DrawDebugDirectionArrow(
		UGBWFlowMoveComponent* FlowMoveComponent,
		EFlowMoveDirectionType DirectionType,
		const bool bOnlyXY,
		FVector StartPoint,
		float ZOffset,
		float ZOffset_String,
		float Length,
		FColor Color,
		FString AppendStr);
	//Debug - end

	//Tools - Start
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Tools", meta=(BlueprintThreadSafe))
	static bool FM_TestVisibility(
		const AActor* StartActor,
		const AActor* EndActor,
		const FGBWFlowMoveTraceSetting& TraceSetting,
		FVector& TouchPoint);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Tools", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static USceneComponent* FM_TryGetFocusActor(
		const AActor* InActor,
		const FVector MoveDirection,
		const FVector ControllerDirection,
		FGBWFlowMoveTraceSetting PerceptionSetting,
		TArray<FName> ComponentWithTags,
		float PrejudgeDistance = 2000.0,
		float FuzzyQueryAngleRestrictions = 45.0f);
	//Tools - End
};