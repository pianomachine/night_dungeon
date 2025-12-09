// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GBWAnimComponent.h"
#include "GameFramework/Character.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "Engine/HitResult.h"
#include "GBWFlowMoveObjects.generated.h"

struct FGBWFlowMoveState;
class UGBWFlowMoveComponent;

// BEGIN struct
UENUM(BlueprintType)
enum EFMMMoveVectorNormalizeType
{
	NoNormalize,
	AllNormalize,
	HorizontalNormalize
};

UENUM(BlueprintType)
enum EFMMoveControlANSSettingType
{
	SimpleSet,
	StateSelect,
	BlendSpacePositionSelect,
	FMValueConditionSelect,
	FMSpeedModeSelect
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveTraceSetting
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	bool isDebug = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug",
		meta=(EditConditionHides,EditCondition="isDebug"))
	float DebugTime = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug",
		meta=(EditConditionHides,EditCondition="isDebug"))
	bool DrawTraceShape = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug",
		meta=(EditConditionHides,EditCondition="isDebug"))
	bool DrawTraceLine = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	TArray<TEnumAsByte<ETraceTypeQuery>> TraceChannels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreeningConditions")
	bool UseScreeningConditions = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreeningConditions",
		meta=(EditConditionHides,EditCondition="UseScreeningConditions"))
	TArray<TSubclassOf<AActor>> ActorWithClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreeningConditions",
		meta=(EditConditionHides,EditCondition="UseScreeningConditions"))
	TArray<FName> ActorWithTag;

	FGBWFlowMoveTraceSetting(){}
	bool IsMetScreeningConditions(const AActor* InActor);
};

UENUM(BlueprintType)
enum EFlowMoveDirectionType
{
	ActorForward,
	Velocity,
	Controller,
	CurrentMoveToDirection,
	TargetMoveToDirection,
	FlowMoveForward,
	RootMotionVelocity,
	ActorToTarget,
	StartPointToTarget,
	StartMoveToDirection,
	StartTargetMoveToDirection,
	StartActorForward,
	WorldAbsolute
};

UENUM(BlueprintType)
enum EFMBSAxisType
{
	Speed,
	HorizontalDirection,
	PitchDirection,
	MoveVectorLength,
	TargetSpeed,
	TargetYawAngleSpeed,
	SelectByState,
	SelectByValueCondition,
	SelectBySpeedMode,
	PlaneCoordinateSystem
};

USTRUCT(BlueprintType)
struct FFMGetMagicPlayerParam
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	UAnimationAsset* AnimAsset = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TArray<FMagicPlayerLinkedAnim> LinkedAnimAssetSet;
};

UENUM(BlueprintType)
enum EFlowMoveEventType
{
	OnActive,
	OnActiveFailed,
	OnUpdate,
	OnStop,
	OnFocusActorChange,
	OnFootStep
};
UENUM(BlueprintType)
enum EFMViewMode
{
	TP_FreeMode,
	TP_ForwardLockMode,
	TP_ActorLockMode,
	TP_Aim
};
UENUM(BlueprintType)
enum EFMSpeedMode
{
	FM_Slow,
	FM_Medium,
	FM_Fast,
	FM_Highest
};
UENUM(BlueprintType)
enum EFMBodyMode
{
	FM_Prostrate,
	FM_Crouch,
	FM_Stand,
	FM_Float
};
USTRUCT(BlueprintType)
struct FFlowMoveEvent
{
	GENERATED_BODY()

	UPROPERTY()
	int InFrameNum = -1;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TEnumAsByte<EFlowMoveEventType> EventType = EFlowMoveEventType::OnActive;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditConditionHides,EditCondition="EventType == EFlowMoveEventType::OnViewModeChange"))
	TEnumAsByte<EFMViewMode> NewViewMode = EFMViewMode::TP_FreeMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditConditionHides,EditCondition="EventType == EFlowMoveEventType::OnFocusActorChange"))
	USceneComponent* NewFocusActorComponent = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditConditionHides,EditCondition="EventType == EFlowMoveEventType::OnFootStep"))
	FName FootStepType = NAME_None;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditConditionHides,EditCondition="EventType == EFlowMoveEventType::OnFootStep"))
	FHitResult FootStepFloorHit = FHitResult();
	
	FFlowMoveEvent(){}
	FFlowMoveEvent(EFlowMoveEventType TheEventType);
	FFlowMoveEvent(USceneComponent* TheNewFocusActorComponent);
	FFlowMoveEvent(USkeletalMeshComponent* InSKMesh,
		FName InFootStepType,
		FName InFootStepSocket,
		FGBWFlowMoveTraceSetting FloorTraceSetting);
};

UCLASS(Blueprintable, meta = (DisplayName = "GBWFlowMoveObject_Base"))
class GBWFLOWMOVE_API UGBWFlowMoveObject_Base : public UObject
{
	GENERATED_BODY()

	virtual bool IsNameStableForNetworking() const override;
	virtual bool IsSupportedForNetworking() const override;
protected:
	struct FWorldContext* WorldContext;
public:
	UGBWFlowMoveObject_Base();

	UPROPERTY()
	FGuid Guid = FGuid();

	// Allows the Object to use BP_Functions
	UFUNCTION(BlueprintCallable, Category="WorldContext")
	void SetWorldContext(UObject* NewWorldContext);

	UFUNCTION(BlueprintCallable, Category = "WorldContext")
	UObject* GetWorldContextObject();

	//~ Begin UObject Interface
	virtual class UWorld* GetWorld() const override final;
	struct FWorldContext* GetWorldContext() const { return WorldContext; };
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveAngle
{
	GENERATED_BODY()
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFlowMoveDirectionType> From = EFlowMoveDirectionType::TargetMoveToDirection;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFlowMoveDirectionType> To = EFlowMoveDirectionType::ActorForward;
	
	FGBWFlowMoveAngle(){}
};

UENUM(BlueprintType)
enum EFMValueType
{
	FM_HorizontalAngle,
	FM_VerticalAngle,
	FM_Speed,
	FM_HorizontalSpeed,
	FM_VerticalSpeed,
	FM_TargetSpeed,
	FM_MoveVectorLength,
	FM_MoveVectorPitch,
	FM_ControlVectorPitch,
	FM_FallingDistance,
	FM_ToTargetPointDistance,
	FM_AngleSpeed_Yaw,
	FM_Gravity,
	FM_BeginTargetPointHeight,
	Specified
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveMove_Value
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFMValueType> Type = EFMValueType::FM_MoveVectorLength;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="Type == EFMValueType::FM_VerticalAngle || Type == EFMValueType::FM_HorizontalAngle"))
	FGBWFlowMoveAngle Angle = FGBWFlowMoveAngle();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="Type == EFMValueType::Specified"))
	float SpecifiedValue = 0.0f;
	
	FGBWFlowMoveMove_Value(){}
	float GetValue(UGBWFlowMoveComponent* FlowMoveComponent) const;
};
UENUM(BlueprintType)
enum EGBWFlowMoveScaleType
{
	BySpecifiedScale,
	ByValueRatio
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveScale
{
	GENERATED_BODY()
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EGBWFlowMoveScaleType> ScaleType = EGBWFlowMoveScaleType::BySpecifiedScale;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ScaleType == EGBWFlowMoveScaleType::BySpecifiedScale"))
	float SpecifiedScale = 1.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ScaleType == EGBWFlowMoveScaleType::ByValueRatio"))
	FGBWFlowMoveMove_Value From = FGBWFlowMoveMove_Value();
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ScaleType == EGBWFlowMoveScaleType::ByValueRatio"))
	FGBWFlowMoveMove_Value To = FGBWFlowMoveMove_Value();
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ScaleType == EGBWFlowMoveScaleType::ByValueRatio"))
	float MinScale = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ScaleType == EGBWFlowMoveScaleType::ByValueRatio"))
	float MaxScale = 1.0f;
	
	FGBWFlowMoveScale(){}
	float GetScale(UGBWFlowMoveComponent* FlowMoveComponent) const;
};
UENUM(BlueprintType)
enum EFMPlaneCoordinateValueType
{
	ForwardValue,
	RightValue,
};
USTRUCT(BlueprintType)
struct FGBWFlowMovePlaneCoordinateValue
{
	GENERATED_BODY()
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFlowMoveDirectionType> ReferenceForwardVector = EFlowMoveDirectionType::ActorForward;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="ReferenceForwardVector == EFlowMoveDirectionType::WorldAbsolute"))
	FVector WorldForwardVector = FVector::ZeroVector;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFlowMoveDirectionType> TargetVector = EFlowMoveDirectionType::CurrentMoveToDirection;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="TargetVector == EFlowMoveDirectionType::WorldAbsolute"))
	FVector WorldTargetVector = FVector::ZeroVector;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveMove_Value VectorLength = FGBWFlowMoveMove_Value();
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFMPlaneCoordinateValueType> ValueType = EFMPlaneCoordinateValueType::ForwardValue;
	
	FGBWFlowMovePlaneCoordinateValue(){}
	float GetValue(UGBWFlowMoveComponent* FlowMoveComponent) const;
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveFootStepSettings
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FName FootstepType = NAME_None;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FName FootSocketName = NAME_None;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FHitResult FloorHit = FHitResult();
	
	FGBWFlowMoveFootStepSettings(){}
};

UENUM(BlueprintType)
enum EGBWFlowMoveAnimPlayRateAdjustType
{
	NoAdjust,
	ByMoveSpeed,
	BySpecifyMoveSpeed,
	BySpecifyActionDuration,
	ByRootMotion,
	ByCurve
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveAnimPlayRateAdjustSettings
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float AdjustAnimPlayRateScale = 1.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float AnimPlayRateSmoothSpeed = 12.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MinPlayRate = 0.2f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MaxPlayRate = 5.0f;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EGBWFlowMoveAnimPlayRateAdjustType> AdjustType = EGBWFlowMoveAnimPlayRateAdjustType::ByMoveSpeed;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByMoveSpeed"))
	float ReferenceSpeed = 600.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
	meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByMoveSpeed"))
	float Min_AnimPlayRate = 0.1f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByMoveSpeed"))
	float Max_AnimPlayRate = 3.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::BySpecifyActionDuration"))
	float SpecifyActionDuration = 1.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::BySpecifyMoveSpeed"))
	float SpecifyMoveSpeed = 200.0f;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion"))
	bool bAdjustWhenTheTargetSpeedLower = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion"))
	bool bAdjustWhenTheTargetSpeedHigher = true;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion && (bAdjustWhenTheTargetSpeedLower || bAdjustWhenTheTargetSpeedHigher)"))
	float AdjustWhenTargetMoveSpeedGreaterThan = 100.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion && (bAdjustWhenTheTargetSpeedLower || bAdjustWhenTheTargetSpeedHigher)"))
	float AdjustWhenSpeedDifferenceGreaterThan = 20.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion"))
	bool bAdjustWhenTheTargetAngleSpeedLower = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion"))
	bool bAdjustWhenTheTargetAngleSpeedHigher = true;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion && (bAdjustWhenTheTargetAngleSpeedLower || bAdjustWhenTheTargetAngleSpeedHigher)"))
	float AdjustByAngleWhenRealMoveSpeedLessThan = 300.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion && (bAdjustWhenTheTargetAngleSpeedLower || bAdjustWhenTheTargetAngleSpeedHigher)"))
	float TargetAngleSpeedPerSec = 360.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion && (bAdjustWhenTheTargetAngleSpeedLower || bAdjustWhenTheTargetAngleSpeedHigher)"))
	float AdjustWhenTargetAngleGreaterThan = 10.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByRootMotion && (bAdjustWhenTheTargetAngleSpeedLower || bAdjustWhenTheTargetAngleSpeedHigher)"))
	float AdjustWhenAngleSpeedDifferenceGreaterThan = 5.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="AdjustType == EGBWFlowMoveAnimPlayRateAdjustType::ByCurve"))
	FRuntimeFloatCurve AnimPlayRateCurve = FRuntimeFloatCurve();
	
	FGBWFlowMoveAnimPlayRateAdjustSettings(){}
};

UENUM(BlueprintType)
enum EGBWFlowMoveCurveType
{
	CurveAsset,
	RootMotion,
	BlendOption
};
UENUM(BlueprintType)
enum EGBWFlowMoveControlSettingsType
{
	Simple,
	Curve,
	Range,
	FromRootMotion,
	GetFromPSFloatValue
};
UENUM(BlueprintType)
enum class EGBWFlowMoveBlendCurveTimeRangeType : uint8
{
	Auto,
	Ratio,
	AbsoluteTime,
	AbsoluteTime_OnlyStartTime,
	AbsoluteTime_OnlyEndTime
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveDirection
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFlowMoveDirectionType> Type = EFlowMoveDirectionType::CurrentMoveToDirection;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="Type == EFlowMoveDirectionType::WorldAbsolute"))
	FVector WorldAbsoluteVector = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool OnlyHorizontalDirection = true;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="!OnlyHorizontalDirection"))
	float MinPitch = -70.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="!OnlyHorizontalDirection"))
	float MaxPitch = 70.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MaxHorizontalAngleToActorForward = 180.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(InlineEditConditionToggle))
	bool bInvert = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bInvert"))
	TEnumAsByte<EFlowMoveDirectionType> InvertRefDirectionType = EFlowMoveDirectionType::FlowMoveForward;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bInvert"))
	float InvertWhenAngleToRefDirection= 180.0f;
	
	FGBWFlowMoveDirection(){}
	FGBWFlowMoveDirection(const FVector& DefaultVector)
	{
		Type = EFlowMoveDirectionType::WorldAbsolute;
		WorldAbsoluteVector = DefaultVector;
	}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveVectorCurveSetting
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EGBWFlowMoveCurveType> Type = EGBWFlowMoveCurveType::CurveAsset;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::CurveAsset"))
	FRuntimeVectorCurve Curve = FRuntimeVectorCurve();
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::RootMotion"))
	float RootMotionScale = 1.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption"))
	FVector From = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption"))
	FVector To = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption"))
	EAlphaBlendOption BlendOption = EAlphaBlendOption::CubicInOut;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption"))
	EGBWFlowMoveBlendCurveTimeRangeType TimeRangeType = EGBWFlowMoveBlendCurveTimeRangeType::Auto;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption && TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::Ratio"))
	float Ratio_Start = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption && TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::Ratio"))
	float Ratio_End = 1.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,
			EditCondition="Type==EGBWFlowMoveCurveType::BlendOption && (TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime || TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime_OnlyStartTime)"))
	float Time_Start = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,
			EditCondition="Type==EGBWFlowMoveCurveType::BlendOption && (TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime || TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime_OnlyEndTime)"))
	float Time_End = 1.0f;
	
	FGBWFlowMoveVectorCurveSetting(){}

	FVector GetBlendValue(
		const float StartTime,
		const float EndTime,
		const float TimeNow) const;
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveFloatCurveSetting
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EGBWFlowMoveCurveType> Type = EGBWFlowMoveCurveType::CurveAsset;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::CurveAsset"))
	FRuntimeFloatCurve Curve = FRuntimeFloatCurve();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption"))
	float From = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption"))
	float To = 1.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption"))
	EAlphaBlendOption BlendOption = EAlphaBlendOption::CubicInOut;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption"))
	EGBWFlowMoveBlendCurveTimeRangeType TimeRangeType = EGBWFlowMoveBlendCurveTimeRangeType::Auto;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption && TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::Ratio"))
	float Ratio_Start = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="Type==EGBWFlowMoveCurveType::BlendOption && TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::Ratio"))
	float Ratio_End = 1.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,
			EditCondition="Type==EGBWFlowMoveCurveType::BlendOption && (TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime || TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime_OnlyStartTime)"))
	float Time_Start = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,
			EditCondition="Type==EGBWFlowMoveCurveType::BlendOption && (TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime || TimeRangeType==EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime_OnlyEndTime)"))
	float Time_End = 1.0f;
	
	FGBWFlowMoveFloatCurveSetting(){}
	float GetBlendValue(
		const float StartTime,
		const float EndTime,
		const float TimeNow) const;
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveFloatValueSetting
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EGBWFlowMoveControlSettingsType> Type = EGBWFlowMoveControlSettingsType::Simple;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::Simple"))
	float Target = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::GetFromPSFloatValue"))
	float DefaultValue = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::GetFromPSFloatValue"))
	FName ValueKey = NAME_None;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::Simple || Type == EGBWFlowMoveControlSettingsType::GetFromPSFloatValue"))
	float SmoothSpeed_LowToHigh = 6.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::Simple || Type == EGBWFlowMoveControlSettingsType::GetFromPSFloatValue"))
	float SmoothSpeed_HighToLow = 6.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::Curve"))
	FGBWFlowMoveFloatCurveSetting Curve = FGBWFlowMoveFloatCurveSetting();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
	meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::Range"))
	float BeginValue = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
	meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::Range"))
	float TargetValue = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides, EditCondition="Type == EGBWFlowMoveControlSettingsType::Range"))
	float SmoothSpeed = 6.0f;
	
	FGBWFlowMoveFloatValueSetting(){}
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_SelectUpdateSettings
{
	GENERATED_BODY()

	//AxisUpdateDuration<=0 means that the entire process will update the Axis value of BlendSpace in real time
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float UpdateDuration = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="AxisUpdateDuration > 0"))
	float MaxUpdateIntervalTime = 0.0f;
	
	FGBWFlowMoveControlSettings_SelectUpdateSettings(){}
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AxisDeadZone
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector2D DeadZoneRange = FVector2D::ZeroVector;

	//When entering the DeadZone, use this value directly
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float DeadZoneValue = 0.0f;
	
	FGBWFlowMoveControlSettings_AxisDeadZone(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AxisUpdateSettings
{
	GENERATED_BODY()

	//AxisUpdateDuration<=0 means that the entire process will update the Axis value of BlendSpace in real time
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float AxisUpdateDuration = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="AxisUpdateDuration > 0"))
	float AxisMaxUpdateIntervalTime = 2.0f;
	
	FGBWFlowMoveControlSettings_AxisUpdateSettings(){}
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AxisValueAdjustSettings
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(TitleProperty="{DeadZoneRange}->{DeadZoneValue}"))
	TArray<FGBWFlowMoveControlSettings_AxisDeadZone> AxisDeadZone;
	
	FGBWFlowMoveControlSettings_AxisValueAdjustSettings(){}
};
UENUM(BlueprintType)
enum EFMConditionMeetType
{
	AnyMeet,
	AllMeet,
	NoMeet,
	AnyNoMeet
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveMove_ValueCondition
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveMove_Value Value = FGBWFlowMoveMove_Value();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TArray<FVector2D> Ranges;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bInvert = false;
	
	FGBWFlowMoveMove_ValueCondition(){}

	bool IsMeet(UGBWFlowMoveComponent* FlowMoveComponent) const;
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveMoveValue_ValueConditionSelectItem
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFMConditionMeetType> MeetType = EFMConditionMeetType::AnyMeet;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TArray<FGBWFlowMoveMove_ValueCondition> ValueConditions;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float Value = 0.0f;
	
	FGBWFlowMoveMoveValue_ValueConditionSelectItem(){}
	bool IsMeet(UGBWFlowMoveComponent* FlowMoveComponent) const;
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AxisValueSettings
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsSpecifyValue = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="bIsSpecifyValue"))
	float SpecifyValue = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="!bIsSpecifyValue"))
	TEnumAsByte<EFMBSAxisType> ValueFrom = EFMBSAxisType::HorizontalDirection;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="!bIsSpecifyValue && (ValueFrom == EFMBSAxisType::HorizontalDirection || ValueFrom == EFMBSAxisType::PitchDirection)"))
	FGBWFlowMoveAngle AngleFrom = FGBWFlowMoveAngle();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="!bIsSpecifyValue && (ValueFrom == EFMBSAxisType::PlaneCoordinateSystem)"))
	FGBWFlowMovePlaneCoordinateValue PlaneCoordinateValue = FGBWFlowMovePlaneCoordinateValue();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="!bIsSpecifyValue"))
	FGBWFlowMoveControlSettings_AxisValueAdjustSettings AxisValueAdjust = FGBWFlowMoveControlSettings_AxisValueAdjustSettings();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(ForceInlineRow, EditCondition="!bIsSpecifyValue && ValueFrom == EFMBSAxisType::SelectByState"))
	TMap<FName, float> StateSelectValue;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(ForceInlineRow, EditCondition="!bIsSpecifyValue && ValueFrom == EFMBSAxisType::SelectByValueCondition", TitleProperty="Value"))
	TArray<FGBWFlowMoveMoveValue_ValueConditionSelectItem> ValueConditionSelectValue;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(ForceInlineRow, EditCondition="!bIsSpecifyValue && ValueFrom == EFMBSAxisType::SelectBySpeedMode"))
	TMap<TEnumAsByte<EFMSpeedMode>, float> SpeedModeSelectValue;
	
	FGBWFlowMoveControlSettings_AxisValueSettings(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_BlendSpaceAxisControlSetting
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWFlowMoveControlSettings_AxisValueSettings AxisValue = FGBWFlowMoveControlSettings_AxisValueSettings();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWFlowMoveControlSettings_AxisUpdateSettings AxisUpdate = FGBWFlowMoveControlSettings_AxisUpdateSettings();
	
	FGBWFlowMoveControlSettings_BlendSpaceAxisControlSetting(){}

	float GetAxisValue(
		UGBWFlowMoveComponent* FlowMoveComponent,
		float OldValue,
		const float DeltaTime,
		const float ExecuteTime,
		const FVector2D AxisRange) const;
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_BSReplayZone
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector2D HorizontalAxisRange = FVector2D::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector2D VerticalAxisRange = FVector2D::ZeroVector;
	
	FGBWFlowMoveControlSettings_BSReplayZone(){}
};
UENUM(BlueprintType)
enum EGBWFlowMoveAnimPlayTimeControlType
{
	DefaultControl,
	ByMoveDistance,
	ByTurnRate
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AnimPlayTimeControl
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	TEnumAsByte<EGBWFlowMoveAnimPlayTimeControlType> PlayTimeControl = EGBWFlowMoveAnimPlayTimeControlType::DefaultControl;
	
	FGBWFlowMoveControlSettings_AnimPlayTimeControl(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AnimPlay
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(InlineEditConditionToggle))
	bool bIsSpecifyPlayDuration = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="bIsSpecifyPlayDuration"))
	FVector2D PlayDuration = FVector2D(0.0f, 1.0f);
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsStartFromSync = true;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool LoopAnimation = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float BlendTime = 0.2f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float LoopBlendTime = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float StopBlendTime = 0.2f;

	//GBWToDo
	//UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWFlowMoveControlSettings_AnimPlayTimeControl PlayTimeControl = FGBWFlowMoveControlSettings_AnimPlayTimeControl();
	
	//When the Axis value of BlendSpace reaches that area, the animation starts playing from the starting point.
	/*UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="bIsForBlendSpace"))*/
	TArray<FGBWFlowMoveControlSettings_BSReplayZone> BS_ReplayZone;
		
	FGBWFlowMoveControlSettings_AnimPlay(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_Target
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bToTargetRate = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditCondition="bToTargetRate"))
	FGBWFlowMoveFloatCurveSetting ToTargetRateCurve = FGBWFlowMoveFloatCurveSetting();
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditCondition="bToTargetRate"))
	FVector TargetPointOffset = FVector::ZeroVector;
	
	FGBWFlowMoveControlSettings_Target(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_SpeedAdjust
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MaxSpeed = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MoveSpeedAdjustSmoothSpeed_LowToHigh = 6.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MoveSpeedAdjustSmoothSpeed_HighToLow = 0.3f;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bIsSpeedAdjustByMoveDirectionPitch = false;
	//The degree of velocity attenuation when the upward pitch of Velocity reaches 90 degrees.
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsSpeedAdjustByMoveDirectionPitch"))
	float MaxRate_ByUpPitch = 0.8f;
	//The increase in speed when the downward pitch of Velocity reaches 90 degrees.
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsSpeedAdjustByMoveDirectionPitch"))
	float MaxRate_ByDownPitch = 0.8f;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bIsSpeedAdjustByMoveDirectionHorizontalAngle= false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsSpeedAdjustByMoveDirectionHorizontalAngle"))
	FGBWFlowMoveAngle Angle = FGBWFlowMoveAngle();
	//The degree of velocity attenuation when Velocity reaches HorizontalAngle_End in the direction directly in front of the character.
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsSpeedAdjustByMoveDirectionHorizontalAngle"))
	float MaxRate_ByHorizontalAngle = 0.8f;
	//When the horizontal angle between Velocity and the character's forward direction reaches this value, the speed begins to decay.
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsSpeedAdjustByMoveDirectionHorizontalAngle"))
	float HorizontalAngle_Begin = 45.0f;
	//When the horizontal angle between Velocity and the character's forward direction reaches this value, the speed decays to MaxRate_ByHorizontalAngle * TargetSpeed.
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsSpeedAdjustByMoveDirectionHorizontalAngle"))
	float HorizontalAngle_End = 90.0f;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bIsAdjustByMoveVectorLength = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsAdjustByMoveVectorLength"))
	bool bOnlyByHorizontalMoveVectorLength = true;
	
	FGBWFlowMoveControlSettings_SpeedAdjust(){}

	float GetSpeedAdjustRate(float InRate, float DeltaTime, UGBWFlowMoveComponent* FlowMoveComponent) const;
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_PathOffset
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bPathOffset = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bPathOffset"))
	TEnumAsByte<EFlowMoveDirectionType> ForwardDirectionOfRCS = EFlowMoveDirectionType::CurrentMoveToDirection;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bPathOffset"))
	FGBWFlowMoveScale Scale = FGBWFlowMoveScale();
	//x - Forward; y - Right; z - Up;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bPathOffset"))
	FGBWFlowMoveVectorCurveSetting PathOffsetCurve = FGBWFlowMoveVectorCurveSetting();
	
	FGBWFlowMoveControlSettings_PathOffset(){}
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AdjustCapsule
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float SmoothSpeed = 12.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(ClampMin = 10.0f, ForceUnits = "cm"))
	float AdjustTo = 90.0f;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bRadiusAdjust = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditCondition="bRadiusAdjust", ClampMin = 10.0f, ForceUnits = "cm"))
	float RadiusAdjustTo = 35.0f;
	
	FGBWFlowMoveControlSettings_AdjustCapsule(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AdjustBone
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float DelayTime = 0.0f;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FMagicPlayerBoneControlParam BoneControlParam = FMagicPlayerBoneControlParam();
	
	FGBWFlowMoveControlSettings_AdjustBone(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_TiltPostureAdjust
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FName BoneName = FName("root");
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float MaxAdjustAngle = 10.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float FinishedAngleSpeed = 180.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float StartSpeed = 300.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float FinishedSpeed = 700.0f;
	
	FGBWFlowMoveControlSettings_TiltPostureAdjust(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_Adjust
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bAdjustAnimPlayRate = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bAdjustAnimPlayRate"))
	FGBWFlowMoveAnimPlayRateAdjustSettings AnimPlayRateAdjust = FGBWFlowMoveAnimPlayRateAdjustSettings();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bIsUseCapsuleAdjust = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsUseCapsuleAdjust"))
	FGBWFlowMoveControlSettings_AdjustCapsule CapsuleAdjust = FGBWFlowMoveControlSettings_AdjustCapsule();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bIsUseTiltPostureAdjust = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsUseTiltPostureAdjust"))
	FGBWFlowMoveControlSettings_TiltPostureAdjust TiltPostureAdjust = FGBWFlowMoveControlSettings_TiltPostureAdjust();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bIsUseBoneAdjust = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsUseBoneAdjust"))
	FGBWFlowMoveControlSettings_AdjustBone BoneAdjust = FGBWFlowMoveControlSettings_AdjustBone();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bIsConstrainMoveToTargetPlane = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsConstrainMoveToTargetPlane"))
	float ConstrainMoveToTargetPlaneTime = 0.3f;
	
	FGBWFlowMoveControlSettings_Adjust(){}
};

UENUM(BlueprintType)
enum EGBWFlowMoveInertialAttenuationType
{
	ByInterpSpeed,
	ByDuration
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_InertialSetting
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bIsInertialVelocity = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditCondition="bIsInertialVelocity"))
	TEnumAsByte<EGBWFlowMoveInertialAttenuationType> VelocityAttenuationType = EGBWFlowMoveInertialAttenuationType::ByDuration;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsInertialVelocity && VelocityAttenuationType == EGBWFlowMoveInertialAttenuationType::ByInterpSpeed"))
	float InertialVelocityAttenuationSpeed = 3.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsInertialVelocity && VelocityAttenuationType == EGBWFlowMoveInertialAttenuationType::ByDuration"))
	float InertialVelocityAttenuationDuration = 0.3f;
	
	FGBWFlowMoveControlSettings_InertialSetting(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_GravitySetting
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bCalculateFallingDistance = false;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveFloatValueSetting Gravity = FGBWFlowMoveFloatValueSetting();
	
	FGBWFlowMoveControlSettings_GravitySetting(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_AngleConstrain
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFlowMoveDirectionType> ConstrainTo = EFlowMoveDirectionType::ActorForward;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MaxYaw = 180.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MaxPitchUp = 90.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MaxPitchDown = 90.0f;
	
	FGBWFlowMoveControlSettings_AngleConstrain(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_ControlVector
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float InputVectorLengthSmoothSpeed = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float InputVectorLengthDeadZone = 0.1f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(InlineEditConditionToggle))
	bool bUseDefaultInput = false;
	//When the length of the InputVector enters the DeadZone, this value is used as the default InputVector
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bUseDefaultInput"))
	FGBWFlowMoveDirection DefaultInputDirection = FGBWFlowMoveDirection(FVector::ZeroVector);

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MoveToDirectionSmoothSpeed = 3.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_AngleConstrain MoveToDirectionAngleConstrain = FGBWFlowMoveControlSettings_AngleConstrain();
	
	FGBWFlowMoveControlSettings_ControlVector(){}
};
UENUM(BlueprintType)
enum EGBWFlowMoveMoveSpeedType
{
	DirectlySet,
	InheritFromPreActionSpeed,
	InheritFromPreActionVelocity,
	InheritFromPreActionHorizontalVelocity,
	SetBySpeedMode
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_ControllableVelocity
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool VelocityFromRootMotion = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="VelocityFromRootMotion"))
	float RootMotionScale = 1.0f;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="!VelocityFromRootMotion"))
	FGBWFlowMoveDirection MoveDirection = FGBWFlowMoveDirection();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EGBWFlowMoveMoveSpeedType> MoveSpeedType = EGBWFlowMoveMoveSpeedType::DirectlySet;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="MoveSpeedType == EGBWFlowMoveMoveSpeedType::DirectlySet || MoveSpeedType == EGBWFlowMoveMoveSpeedType::SetBySpeedMode"))
	FGBWFlowMoveFloatValueSetting MoveSpeed = FGBWFlowMoveFloatValueSetting();
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="MoveSpeedType == EGBWFlowMoveMoveSpeedType::SetBySpeedMode"))
	TMap<TEnumAsByte<EFMSpeedMode>, FGBWFlowMoveFloatValueSetting> MoveSpeedSet;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bIsSpeedAdjust= false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditCondition="bIsSpeedAdjust"))
	FGBWFlowMoveControlSettings_SpeedAdjust SpeedAdjustSettings = FGBWFlowMoveControlSettings_SpeedAdjust();
	
	FGBWFlowMoveControlSettings_ControllableVelocity(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_UncontrollableVelocity
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_InertialSetting Inertial = FGBWFlowMoveControlSettings_InertialSetting();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_GravitySetting Gravity = FGBWFlowMoveControlSettings_GravitySetting();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_Target ToTarget = FGBWFlowMoveControlSettings_Target();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_PathOffset PathOffset = FGBWFlowMoveControlSettings_PathOffset();
	
	FGBWFlowMoveControlSettings_UncontrollableVelocity(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_Velocity
{
	GENERATED_BODY()
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_ControllableVelocity ControllableVelocity = FGBWFlowMoveControlSettings_ControllableVelocity();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_UncontrollableVelocity UncontrollableVelocity = FGBWFlowMoveControlSettings_UncontrollableVelocity();
	
	FGBWFlowMoveControlSettings_Velocity(){}
};
UENUM(BlueprintType)
enum EFMRotateType
{
	ShortestPath,
	Clockwise,
	Anticlockwise
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettings_RotationSetting
{
	GENERATED_BODY()
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool RotationFromRootMotion = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="RotationFromRootMotion"))
	float RootMotionScale = 1.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="RotationFromRootMotion"))
	bool RootMotionPitch = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="RotationFromRootMotion"))
	bool RootMotionYaw = true;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="RotationFromRootMotion"))
	bool RootMotionRoll = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="RotationFromRootMotion"))
	bool bTryLockTo = false;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="!RotationFromRootMotion"))
	float RotationSmoothSpeed = 12.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="!RotationFromRootMotion"))
	TEnumAsByte<EFMRotateType> RotateType = EFMRotateType::ShortestPath;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="!RotationFromRootMotion"))
	float DelayTime = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="!RotationFromRootMotion || (RotationFromRootMotion && bTryLockTo)"))
	FGBWFlowMoveDirection RotationLockTo = FGBWFlowMoveDirection();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(InlineEditConditionToggle))
	bool bRotationYawOffset = false;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(EditCondition="bRotationYawOffset"))
	FGBWFlowMoveFloatCurveSetting RotationYawOffset = FGBWFlowMoveFloatCurveSetting();
	
	FGBWFlowMoveControlSettings_RotationSetting(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveControlSettingsPro
{
	GENERATED_BODY()

	float StartTime = 0.0f;
	float EndTime = 0.0f;
	float TimeNow = 0.0f;
	float Progress = 0.0f;
	FName AnimSlot = NAME_None;

	UPROPERTY(NotReplicated, BlueprintReadOnly, Category = "Settings")
	bool bIsNew = false;
	UPROPERTY(NotReplicated, BlueprintReadOnly, Category = "Settings")
	UObject* FromAsset = nullptr;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bTryGetMoveControlParamFromInterface = true;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_AnimPlay AnimPlaySetting = FGBWFlowMoveControlSettings_AnimPlay();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_ControlVector ControlVector = FGBWFlowMoveControlSettings_ControlVector();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_Velocity Velocity = FGBWFlowMoveControlSettings_Velocity();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_RotationSetting RotationSetting = FGBWFlowMoveControlSettings_RotationSetting();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettings_Adjust AdjustSettings = FGBWFlowMoveControlSettings_Adjust();

	//You can obtain the configuration here in the interface related to custom control of FlowMove's final result,
	//to facilitate accurate control of FlowMove's final result.
	//For example, to determine which motion animation you are adjusting.
	//Alternatively, decide whether to enable a custom control feature in the custom motion control.
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TArray<FName> CustomParams = {};
		
	FGBWFlowMoveControlSettingsPro(){}
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveMoveControlSetting_StateSelectItem
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGameplayTag StateKey = FGameplayTag();

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettingsPro MoveControlSetting = FGBWFlowMoveControlSettingsPro();
	
	FGBWFlowMoveMoveControlSetting_StateSelectItem(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveMoveControlSetting_PositionSelectItem
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettingsPro MoveControlSetting = FGBWFlowMoveControlSettingsPro();
	
	FGBWFlowMoveMoveControlSetting_PositionSelectItem(){}
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveMoveControlSetting_ValueConditionSelectItem
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFMConditionMeetType> MeetType = EFMConditionMeetType::AnyMeet;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TArray<FGBWFlowMoveMove_ValueCondition> ValueConditions;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettingsPro MoveControlSetting = FGBWFlowMoveControlSettingsPro();
	
	FGBWFlowMoveMoveControlSetting_ValueConditionSelectItem(){}
	bool IsMeet(UGBWFlowMoveComponent* FlowMoveComponent) const;
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveControlParam
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	bool bIsActive = false;
	UPROPERTY(NotReplicated)
	uint64 LastActiveFrame  = 0;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float ActionDuration = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector BlendSpacePosition = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FTransform RootMotionNow = FTransform();

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlSettingsPro MoveControlSettings = FGBWFlowMoveControlSettingsPro();
	
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	UGBWAnimComponent* AnimComponent = nullptr;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float AnimPlayRateNow = 1.0f;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector TargetPoint = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float ToTargetPointRate = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float DeltaToTargetPointRate = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float ToTargetPointSpeed = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float Gravity = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float DeltaGravity = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float MoveSpeedAdjustRate = 1.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float MoveSpeed = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float DeltaMoveSpeed = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector PathOffset = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector DeltaPathOffset = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector PathOffsetDirection = FVector::ZeroVector;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float RotationYawOffset = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float DeltaRotationYawOffset = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector MoveToDirection = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector VelocityDirection = FVector::ZeroVector;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector BeginInertialVelocity = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector InertialVelocity = FVector::ZeroVector;
	
	FGBWFlowMoveControlParam(){}

	bool GetToTargetRateCurve(AActor* InActor, float DeltaTime, float& Value, float& DeltaValue) const;
	bool GetRotationOffsetCurve(AActor* InActor, float DeltaTime, float& Value, float& DeltaValue) const;
	bool GetPathOffsetCurve(float DeltaTime, FVector& Value, FVector& DeltaValue) const;
	bool GetMoveSpeedCurve(AActor* InActor, FGBWFlowMoveFloatCurveSetting CurveSetting, float DeltaTime, float& Value, float& DeltaValue) const;
	bool GetGravityCurve(AActor* InActor, float DeltaTime, float& Value, float& DeltaValue) const;
	void GetActionDurationAndAnimPlayRate(
		float DeltaTime,
		const UGBWFlowMoveComponent* FMComponent,
		const FGBWFlowMoveControlSettingsPro& InMoveControlSettings,
		const bool bSmoothPlayRate,
		float& TheActionDuration,
		float& TheAnimPlayRate) const;
	
	void UpdateMoveControlParam(float DeltaTime, UGBWFlowMoveComponent* FMComponent);
};

UENUM(BlueprintType)
enum EGBWFlowMoveVelocityTrendAccumulateMode
{
	// Sets velocity to this value directly
	Override, 
	// Gets added on top of existing velocity
	Additive
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveVelocityTrend
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FName Slot = FName("Default");
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsIgnoreGravity = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TEnumAsByte<EGBWFlowMoveVelocityTrendAccumulateMode> AccumulateMode = EGBWFlowMoveVelocityTrendAccumulateMode::Additive;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TEnumAsByte<EFlowMoveDirectionType> DirectionType = EFlowMoveDirectionType::WorldAbsolute;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool OnlyHorizontalDirection = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Target_VelocityTrend = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float Duration = 1.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsBlend = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditConditionHides,EditCondition="bIsBlend"))
	FVector Begin_VelocityTrend = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditConditionHides,EditCondition="bIsBlend"))
	EAlphaBlendOption BlendOption = EAlphaBlendOption::Linear;

	UPROPERTY()
	float Timer = 0.0f;
	
	FGBWFlowMoveVelocityTrend(){}
	FGBWFlowMoveVelocityTrend(const FName InSlot, bool InIsIgnoreGravity, const FVector& InTarget_VelocityTrend, const float InDuration)
	{
		Slot = InSlot;
		bIsIgnoreGravity = InIsIgnoreGravity;
		Target_VelocityTrend = InTarget_VelocityTrend;
		Duration = InDuration;
	}

	FVector GetVelocityTrend(UGBWFlowMoveComponent* FMComp) const;

	bool ValidUpdate(float DeltaTime);
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveMoveControlResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	bool bIsControlled = false;
	uint64 LastControlledFrame = 0;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FRotator TargetRotation = FRotator();
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector Velocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FRotator DeltaRotator = FRotator::ZeroRotator;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bConstrainToPlane = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector ConstrainPlaneNormal = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector ConstrainPlaneOrigin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float CapsuleRadius = 35.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float CapsuleHalfHeight = 90.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FTransform MeshRelativeTransform = FTransform();
	
	FGBWFlowMoveMoveControlResult(){}
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveMoveControlResult_NetPack
{
	//Total 22.5 bytes
	GENERATED_BODY()

	UPROPERTY()//1 byte
	uint8 Index = 0;
	UPROPERTY()//1 byte
	uint8 bIsControlled : 1;

	UPROPERTY()//2.5 bytes
	FVector_NetQuantize TargetRotationX1000;
	UPROPERTY()//2.5 bytes
	FVector_NetQuantize VelocityX100;
	
	UPROPERTY()//1 byte
	uint8 bConstrainToPlane : 1;
	UPROPERTY()//2 bytes
	FVector_NetQuantizeNormal ConstrainPlaneNormal;
	UPROPERTY()//2.5 bytes
	FVector_NetQuantize ConstrainPlaneOrigin;
	
	UPROPERTY()//2.5 bytes X:Radius Y:HalfHeight
	FVector_NetQuantize CapsuleX100;
	
	UPROPERTY()//2.5 bytes
	FVector_NetQuantize MeshRelative_LocationX100;
	UPROPERTY()//2.5 bytes
	FVector_NetQuantize MeshRelative_RotationX1000;
	UPROPERTY()//2.5 bytes
	FVector_NetQuantize MeshRelative_ScaleX1000;
	
	FGBWFlowMoveMoveControlResult_NetPack(): bIsControlled(0), bConstrainToPlane(0){}
	FGBWFlowMoveMoveControlResult_NetPack(FGBWFlowMoveMoveControlResult Source);
	FGBWFlowMoveMoveControlResult GetValue() const;
};

USTRUCT(BlueprintType)
struct FGBWFlowMoveInputState
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	ACharacter* OwnerCharacter = nullptr;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	UGBWFlowMoveComponent* FlowMoveComponent = nullptr;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFMViewMode> ViewMode = EFMViewMode::TP_FreeMode;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	USceneComponent* FocusActorComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFMSpeedMode> SpeedMode = EFMSpeedMode::FM_Medium;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EFMBodyMode> BodyMode = EFMBodyMode::FM_Stand;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector_NetQuantizeNormal MoveVector = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float LastMoveVectorTime = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float LastZeroMoveVectorTime = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector_NetQuantizeNormal ControlVector = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float LastControlVectorTime = 0.0f;
	
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FVector_NetQuantizeNormal ForwardVector = FVector::ZeroVector;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float ForwardVectorYawAngleSpeed = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TArray<FGBWFlowMoveVelocityTrend> VelocityTrend;

	FGBWFlowMoveInputState(){}
	
	void SetMoveVector(const FVector_NetQuantizeNormal& NewMoveVector);
	void SetControlVector(const FVector_NetQuantizeNormal& NewControlVector);
	FVector CheckGetVelocityTrend(float DeltaTime, bool& bIsOverride, bool& bIsIgnoreGravity);
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	bool bIsActive = false;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float Timer = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float FrameDeltaTime = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	TEnumAsByte<EMovementMode> MovementMode = MOVE_None;
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FTransform LastActorTransform = FTransform();
	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float FallingDistance = 0.0f;

	UPROPERTY(NotReplicated, BlueprintReadOnly, EditAnywhere, Category = "Settings")
	FGBWFlowMoveControlParam MoveControlParam = FGBWFlowMoveControlParam();

	FGBWFlowMoveState(){}

	void UpdateInfo(const ACharacter* OwnerCharacter);
};
USTRUCT(BlueprintType)
struct FGBWFlowMoveMoveControlInitInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	uint8 bIsValid = 0;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float LastValidTime = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector_NetQuantize100 BeginLocation = FVector_NetQuantize10::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector_NetQuantizeNormal BeginDirection = FVector_NetQuantize::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	uint8 bHasTargetPoint = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector_NetQuantize100 TargetPoint = FVector_NetQuantize::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector_NetQuantizeNormal BeginTargetMoveToDirection = FVector_NetQuantize::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector_NetQuantizeNormal BeginMoveToDirection = FVector_NetQuantize::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector_NetQuantizeNormal BeginVelocityDirection = FVector_NetQuantize::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FVector BeginVelocity = FVector_NetQuantize::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float BeginSpeed = 0.0f;
	
	FGBWFlowMoveMoveControlInitInfo(){}
	FGBWFlowMoveMoveControlInitInfo(const ACharacter* OwnerCharacter,
		const FVector_NetQuantizeNormal& InBeginTargetMoveToDirection,
		const FVector_NetQuantizeNormal& InBeginMoveToDirection,
		const FVector_NetQuantizeNormal& InBeginVelocityDirection,
		const float InBeginSpeed);
	FGBWFlowMoveMoveControlInitInfo(
		const ACharacter* OwnerCharacter,
		const FVector_NetQuantize100& InTargetPoint,
		const FVector_NetQuantizeNormal& InBeginTargetMoveToDirection,
		const FVector_NetQuantizeNormal& InBeginMoveToDirection,
		const FVector_NetQuantizeNormal& InBeginVelocityDirection,
		const float InBeginSpeed);
};

UENUM(BlueprintType)
enum EFlowMoveNetworkLocationType
{
	Server,
	Client
};
//END struct

UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew, meta = (DisplayName = "GBWFlowMoveBrain_Base"))
class GBWFLOWMOVE_API UGBWFlowMoveBrain_Base : public UGBWFlowMoveObject_Base
{
	GENERATED_BODY()
public:
	UGBWFlowMoveBrain_Base(){}

	UPROPERTY(BlueprintReadOnly, Category = "FlowMove")
	ACharacter* OwnerCharacter = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "FlowMove")
	UGBWFlowMoveComponent* FlowMoveComponent = nullptr;
	void Init(UGBWFlowMoveComponent* InFlowMoveComponent);
	bool CheckValid(UGBWFlowMoveComponent* InFlowMoveComponent);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="FlowMoveSettings|View")
	TEnumAsByte<EFMViewMode> DefaultViewMode = EFMViewMode::TP_ForwardLockMode;

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|FlowMove|Event")
	void OnFlowMoveEvent(
		FGBWFlowMoveState FlowMoveState,
		const FFlowMoveEvent& FlowMoveEvent);
	virtual void OnFlowMoveEvent_Implementation(
		FGBWFlowMoveState FlowMoveState,
		const FFlowMoveEvent& FlowMoveEvent);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Event")
	void OnFMEvent(
		FGBWFlowMoveState FlowMoveState,
		const FFlowMoveEvent& FlowMoveEvent);

//MoveControl Interface start
	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveAdjust")
	void AdjustFlowMoveResult(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector Pending_Velocity,
		FRotator Pending_DeltaRotation,
		bool& bIsGet,
		FVector& Res_Velocity,
		FRotator& Res_DeltaRotation);
	virtual void AdjustFlowMoveResult_Implementation(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector Pending_Velocity,
		FRotator Pending_DeltaRotation,
		bool& bIsGet,
		FVector& Res_Velocity,
		FRotator& Res_DeltaRotation){}
	UFUNCTION(BlueprintCallable, Category = "GBW|MoveControl")
	void FM_AdjustFlowMoveResult(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector Pending_Velocity,
		FRotator Pending_DeltaRotation,
		bool& bIsGet,
		FVector& Res_Velocity,
		FRotator& Res_DeltaRotation);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetMoveControlDirectly(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector CurrentVelocity,
		FRotator CurrentDeltaRotation,
		bool& bIsGet,
		FVector& Velocity,
		FRotator& DeltaRotation);
	virtual void GetMoveControlDirectly_Implementation(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector CurrentVelocity,
		FRotator CurrentDeltaRotation,
		bool& bIsGet,
		FVector& Velocity,
		FRotator& DeltaRotation){}
	UFUNCTION()
	void FM_GetMoveControlDirectly(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
		float DeltaTime,
		const FVector& CurrentVelocity,
		const FRotator& CurrentDeltaRotation,
		bool& bIsGet,
		FVector& Velocity,
		FRotator& DeltaRotation);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetTarget(
	FGBWFlowMoveControlParam MoveControlParamNow,
	float DeltaTime,	
		FVector CurrentTargetPoint,
		float CurrentToTargetSpeed,
		bool& bIsGet,
		FVector& TargetPoint,
		float& ToTargetSpeed);
	virtual void GetTarget_Implementation(
	FGBWFlowMoveControlParam MoveControlParamNow,
	float DeltaTime,
		FVector CurrentTargetPoint,
		float CurrentToTargetSpeed,
		bool& bIsGet,
		FVector& TargetPoint,
		float& ToTargetSpeed){}
	UFUNCTION()
	void FM_GetTarget(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
	float DeltaTime,
		const FVector& CurrentTargetPoint,
		float CurrentToTargetSpeed,
		bool& bIsGet,
		FVector& TargetPoint,
		float& ToTargetSpeed);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetMoveSpeed(
	FGBWFlowMoveControlParam MoveControlParamNow,
	float DeltaTime,
		float CurrentMoveSpeed,
		bool& bIsGet,
		float& MoveSpeed);
	virtual void GetMoveSpeed_Implementation(
	FGBWFlowMoveControlParam MoveControlParamNow,
	float DeltaTime,
		float CurrentMoveSpeed,
		bool& bIsGet,
		float& MoveSpeed){}
	UFUNCTION()
	void FM_GetMoveSpeed(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
		float DeltaTime,
		float CurrentMoveSpeed,
		bool& bIsGet,
		float& MoveSpeed);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetMoveDirection(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector CurrentMoveDirection,
		bool& bIsGet,
		TEnumAsByte<EFlowMoveDirectionType>& MovementDirectionType,
		FVector& WorldAbsoluteMoveDirection);
	virtual void GetMoveDirection_Implementation(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector CurrentMoveDirection,
		bool& bIsGet,
		TEnumAsByte<EFlowMoveDirectionType>& MovementDirectionType,
		FVector& WorldAbsoluteMoveDirection){}
	UFUNCTION()
	void FM_GetMoveDirection(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
		float DeltaTime,
		const FVector& CurrentMoveDirection,
		bool& bIsGet,
		TEnumAsByte<EFlowMoveDirectionType>& MovementDirectionType,
		FVector& WorldAbsoluteMoveDirection);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetGravity(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		float CurrentGravity,
		bool CurrentIsCalculateFallingDistance,
		bool& bIsGet,
		float& Gravity,
		bool& bIsCalculateFallingDistance);
	virtual void GetGravity_Implementation(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		float CurrentGravity,
		bool CurrentIsCalculateFallingDistance,
		bool& bIsGet,
		float& Gravity,
		bool& bIsCalculateFallingDistance){}
	UFUNCTION()
	void FM_GetGravity(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
		float DeltaTime,
		float CurrentGravity,
		bool CurrentIsCalculateFallingDistance,
		bool& bIsGet,
		float& Gravity,
		bool& bIsCalculateFallingDistance);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetPathOffset(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector CurrentPathOffsetDirection,
		FVector CurrentPathOffset,
		bool& bIsGet,
		FVector& PathOffset,
		TEnumAsByte<EFlowMoveDirectionType>& PathOffsetDirectionType,
		FVector& WorldAbsolutePathOffsetDirection);
	virtual void GetPathOffset_Implementation(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector CurrentPathOffsetDirection,
		FVector CurrentPathOffset,
		bool& bIsGet,
		FVector& PathOffset,
		TEnumAsByte<EFlowMoveDirectionType>& PathOffsetDirectionType,
		FVector& WorldAbsolutePathOffsetDirection){}
	UFUNCTION()
	void FM_GetPathOffset(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
		float DeltaTime,
		const FVector& CurrentPathOffsetDirection,
		const FVector& CurrentPathOffset,
		bool& bIsGet,
		FVector& PathOffset,
		TEnumAsByte<EFlowMoveDirectionType>& PathOffsetDirectionType,
		FVector& WorldAbsolutePathOffsetDirection);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetRotationYawOffset(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		float CurrentRotationYawOffset,
		bool& bIsGet,
		float& RotationYawOffset);
	virtual void GetRotationYawOffset_Implementation(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		float CurrentRotationYawOffset,
		bool& bIsGet,
		float& RotationYawOffset){}
	UFUNCTION()
	void FM_GetRotationYawOffset(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
		float DeltaTime,
		float CurrentRotationYawOffset,
		bool& bIsGet,
		float& RotationYawOffset);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetAnimPlayRate(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		float CurrentAnimPlayRate,
		bool& bIsGet,
		float& AnimPlayRate);
	virtual void GetAnimPlayRate_Implementation(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		float CurrentAnimPlayRate,
		bool& bIsGet,
		float& AnimPlayRate){}
	UFUNCTION()
	void FM_GetAnimPlayRate(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
		float DeltaTime,
		float CurrentAnimPlayRate,
		bool& bIsGet,
		float& AnimPlayRate);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|MoveControl")
	void GetActionDuration(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		bool& bIsGet,
		float& ActionDuration);
	virtual void GetActionDuration_Implementation(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		bool& bIsGet,
		float& ActionDuration){}
	UFUNCTION()
	void FM_GetActionDuration(
		const FGBWFlowMoveControlParam& MoveControlParamNow,
		float DeltaTime,
		bool& bIsGet,
		float& ActionDuration);
//MoveControl Interface end
	
};