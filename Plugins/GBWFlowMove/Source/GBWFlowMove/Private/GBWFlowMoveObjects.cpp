// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "GBWFlowMoveObjects.h"

#include "GBWAnimComponent.h"
#include "GBWAnimFuncLib.h"
#include "GBWFlowMoveComponent.h"
#include "GBWFlowMoveFuncLib.h"
#include "GBWPowerfulPerceptionComponent.h"
#include "GBWPowerfulStateComponent.h"
#include "GBWPSFuncLib.h"
#include "Kismet/KismetMathLibrary.h"

FGBWFlowMoveMoveControlResult_NetPack::FGBWFlowMoveMoveControlResult_NetPack(FGBWFlowMoveMoveControlResult Source)
{
	bIsControlled = Source.bIsControlled;
	TargetRotationX1000 = FVector_NetQuantize(
		FMath::Floor(Source.TargetRotation.Pitch * 1000),
		FMath::Floor(Source.TargetRotation.Yaw * 1000),
		FMath::Floor(Source.TargetRotation.Roll * 1000)
	);
	VelocityX100 = FVector_NetQuantize(
	FMath::Floor(Source.Velocity.X * 100),
	FMath::Floor(Source.Velocity.Y * 100),
	FMath::Floor(Source.Velocity.Z * 100)
		);
	bConstrainToPlane = Source.bConstrainToPlane;
	ConstrainPlaneNormal = FVector_NetQuantizeNormal(Source.ConstrainPlaneNormal);
	ConstrainPlaneOrigin = FVector_NetQuantize(Source.ConstrainPlaneOrigin);
	CapsuleX100 = FVector_NetQuantize(
		Source.CapsuleRadius * 100,
		Source.CapsuleHalfHeight * 100,
		0
	);
	MeshRelative_LocationX100 = FVector_NetQuantize(Source.MeshRelativeTransform.GetLocation() * 100);
	MeshRelative_RotationX1000 = FVector_NetQuantize(
		Source.MeshRelativeTransform.GetRotation().Rotator().Pitch * 1000,
		Source.MeshRelativeTransform.GetRotation().Rotator().Yaw * 1000,
		Source.MeshRelativeTransform.GetRotation().Rotator().Roll * 1000
	);
	MeshRelative_ScaleX1000 = FVector_NetQuantize(Source.MeshRelativeTransform.GetScale3D() * 1000);
}

FGBWFlowMoveMoveControlResult FGBWFlowMoveMoveControlResult_NetPack::GetValue() const
{
	FGBWFlowMoveMoveControlResult Res;
		
	Res.bIsControlled = bIsControlled;
	
	Res.TargetRotation = FRotator(
		TargetRotationX1000.X / 1000.0f,
		TargetRotationX1000.Y / 1000.0f,
		TargetRotationX1000.Z / 1000.0f
	);
	Res.Velocity = VelocityX100 / 100.0f;
	
	Res.bConstrainToPlane = bConstrainToPlane;
	Res.ConstrainPlaneNormal = ConstrainPlaneNormal;
	Res.ConstrainPlaneOrigin = ConstrainPlaneOrigin;

	Res.CapsuleRadius = CapsuleX100.X / 100.f;
	Res.CapsuleHalfHeight = CapsuleX100.Y / 100.f;
	Res.MeshRelativeTransform.SetLocation(MeshRelative_LocationX100 / 100.f);
	Res.MeshRelativeTransform.SetScale3D(MeshRelative_ScaleX1000 / 1000.f);
	Res.MeshRelativeTransform.SetRotation(FRotator(
		MeshRelative_RotationX1000.X / 1000.0f,
		MeshRelative_RotationX1000.Y / 1000.0f,
		MeshRelative_RotationX1000.Z / 1000.0f
	).Quaternion());
		
	return Res;
}

void FGBWFlowMoveInputState::SetMoveVector(const FVector_NetQuantizeNormal& NewMoveVector)
{
	MoveVector = NewMoveVector;
	LastMoveVectorTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
}

void FGBWFlowMoveInputState::SetControlVector(const FVector_NetQuantizeNormal& NewControlVector)
{
	ControlVector = NewControlVector;
	LastControlVectorTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
}

FVector FGBWFlowMoveInputState::CheckGetVelocityTrend(float DeltaTime, bool& bIsOverride, bool& bIsIgnoreGravity)
{
	FVector Res = FVector::ZeroVector;
	bIsOverride = false;
	bIsIgnoreGravity = false;

	int i = 0;
	while (i<VelocityTrend.Num())
	{
		Res = Res + VelocityTrend[i].GetVelocityTrend(FlowMoveComponent);
		if (VelocityTrend[i].AccumulateMode == EGBWFlowMoveVelocityTrendAccumulateMode::Override)
		{
			bIsOverride = true;
		}
		if (VelocityTrend[i].bIsIgnoreGravity)
		{
			bIsIgnoreGravity = true;
		}
		if (!VelocityTrend[i].ValidUpdate(DeltaTime))
		{
			VelocityTrend.RemoveAt(i);
		}
		else
		{
			i++;
		}
	}

	return Res;
}

void FGBWFlowMoveState::UpdateInfo(const ACharacter* OwnerCharacter)
{
	if (OwnerCharacter)
	{
		if (MoveControlParam.MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.bCalculateFallingDistance)
		{
			FallingDistance += (LastActorTransform.GetLocation() - OwnerCharacter->GetActorLocation()).Z;
		}
		else
		{
			FallingDistance = 0.0f;
		}

		LastActorTransform = OwnerCharacter->GetActorTransform();
	}
}

bool UGBWFlowMoveObject_Base::IsNameStableForNetworking() const
{
	return true;
}

bool UGBWFlowMoveObject_Base::IsSupportedForNetworking() const
{
	return true;
}

UGBWFlowMoveObject_Base::UGBWFlowMoveObject_Base()
{
	Guid = FGuid::NewGuid();
}

void UGBWFlowMoveObject_Base::SetWorldContext(UObject* NewWorldContext)
{
	WorldContext = new FWorldContext();
	WorldContext->SetCurrentWorld(NewWorldContext->GetWorld());
}

UObject* UGBWFlowMoveObject_Base::GetWorldContextObject()
{
	if (WorldContext && WorldContext->World())
	{
		return WorldContext->World();
	}
	
	WorldContext = new FWorldContext();
	WorldContext->SetCurrentWorld(GWorld->GetWorld());
	return GWorld->GetWorld();
}

UWorld* UGBWFlowMoveObject_Base::GetWorld() const
{
	if (UWorld* TheWorld = GWorld->GetWorld())
	{
		if (TheWorld->WorldType == EWorldType::Type::Game || TheWorld->WorldType == EWorldType::Type::PIE)
		{
			return TheWorld;
		}
	}
	return nullptr;
}

float FGBWFlowMoveMove_Value::GetValue(UGBWFlowMoveComponent* FlowMoveComponent) const
{
	float Res = 0.0f;

	if (!FlowMoveComponent)
	{
		return Res;
	}

	switch (Type)
	{
	case FM_Speed:
		Res = FlowMoveComponent->InputState.OwnerCharacter->GetVelocity().Length();
		break;
	case FM_HorizontalSpeed:
		Res = FVector(FlowMoveComponent->InputState.OwnerCharacter->GetVelocity().X,FlowMoveComponent->InputState.OwnerCharacter->GetVelocity().Y,0.0f).Length();
		break;
	case FM_VerticalSpeed:
		Res = FVector(0.0f,0.0f,FlowMoveComponent->InputState.OwnerCharacter->GetVelocity().Z).Length();
		break;
	case FM_TargetSpeed:
		Res = FlowMoveComponent->GetMoveControlParam().MoveSpeed;
		break;
	case FM_MoveVectorLength:
		Res = FlowMoveComponent->GetMoveVector().Length();
		break;
	case FM_MoveVectorPitch:
		Res = FlowMoveComponent->GetMoveVector().Rotation().Pitch;
		break;
	case FM_ControlVectorPitch:
		Res = FlowMoveComponent->InputState.ControlVector.Rotation().Pitch;
		break;
	case FM_FallingDistance:
		Res = FlowMoveComponent->TaskState.FallingDistance;
		break;
	case FM_ToTargetPointDistance:
		Res = FlowMoveComponent->CurrentActionTargetSceneInfo.bHasTargetPoint?
			(FlowMoveComponent->InputState.OwnerCharacter->GetActorLocation() - FlowMoveComponent->CurrentActionTargetSceneInfo.TargetPoint).Length()
			: 0.0f;
		break;
	case FM_AngleSpeed_Yaw:
		Res = FlowMoveComponent->MoveControlResult.DeltaRotator.Yaw / FlowMoveComponent->TaskState.FrameDeltaTime;
		break;
	case FM_Gravity:
		Res = FlowMoveComponent->GetMoveControlParam().Gravity;
		break;
	case FM_HorizontalAngle:
		Res = UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(
		FlowMoveComponent->GetDirectionVector(Angle.From,FVector::ZeroVector),
		FlowMoveComponent->GetDirectionVector(Angle.To,FVector::ZeroVector));
		break;
	case FM_VerticalAngle:
		Res = UGBWPowerfulToolsFuncLib::GetAngleInVerticalPlane(
		FlowMoveComponent->GetDirectionVector(Angle.From,FVector::ZeroVector),
		FlowMoveComponent->GetDirectionVector(Angle.To,FVector::ZeroVector));
		break;
	case Specified:
		Res = SpecifiedValue;
		break;
	case FM_BeginTargetPointHeight:
		if (FlowMoveComponent->CurrentActionTargetSceneInfo.bIsValid
			&& FlowMoveComponent->CurrentActionTargetSceneInfo.bHasTargetPoint)
		{
			Res = (FlowMoveComponent->CurrentActionTargetSceneInfo.TargetPoint - FlowMoveComponent->CurrentActionTargetSceneInfo.BeginLocation).Z;
		}
		break;
	default: ;
	}
	
	return Res;
}

float FGBWFlowMoveScale::GetScale(UGBWFlowMoveComponent* FlowMoveComponent) const
{
	if (!FlowMoveComponent)
	{
		return 1.0f;
	}

	float FV = 0.0f;
	float TV = 0.0f;
	switch (ScaleType) {
	case BySpecifiedScale:
		return SpecifiedScale;
	case ByValueRatio:
		FV = From.GetValue(FlowMoveComponent);
		TV = To.GetValue(FlowMoveComponent);
		if (TV == 0.0f)
		{
			return MaxScale;
		}
		else
		{
			return FMath::Clamp(FV / TV,MinScale,MaxScale);
		}
	default:
			return 1.0f;
	}
}

float FGBWFlowMovePlaneCoordinateValue::GetValue(UGBWFlowMoveComponent* FlowMoveComponent) const
{
	if (!FlowMoveComponent)
	{
		return 0.0f;
	}

	FVector F_Vector = FlowMoveComponent->GetDirectionVector(ReferenceForwardVector,WorldForwardVector);
	FVector T_Vector = FlowMoveComponent->GetDirectionVector(TargetVector, WorldTargetVector);

	F_Vector = UKismetMathLibrary::ProjectVectorOnToPlane(F_Vector, FlowMoveComponent->InputState.OwnerCharacter->GetActorUpVector());

	float Res = 0.0f;
	FVector R_Vector = F_Vector;
	if (ValueType == EFMPlaneCoordinateValueType::RightValue)
	{
		R_Vector = UKismetMathLibrary::RotateAngleAxis(F_Vector,90.0f,FlowMoveComponent->InputState.OwnerCharacter->GetActorUpVector());
	}
	
	T_Vector = UKismetMathLibrary::ProjectVectorOnToVector(T_Vector, R_Vector);
	const bool bNegative = T_Vector.X * R_Vector.X <0.0f || T_Vector.Y * R_Vector.Y < 0.0f || T_Vector.Z * R_Vector.Z < 0.0f;

	Res = T_Vector.Length() * VectorLength.GetValue(FlowMoveComponent) * (bNegative? -1.0f : 1.0f);
	return Res;
}

FVector FGBWFlowMoveVectorCurveSetting::GetBlendValue(const float StartTime, const float EndTime,
                                                      const float TimeNow) const
{
	FVector Res = From;
		
	float TheStartTime = StartTime;
	float TheEndTime = EndTime;

	if (TimeRangeType == EGBWFlowMoveBlendCurveTimeRangeType::Ratio)
	{
		TheStartTime = (EndTime - StartTime) * Ratio_Start + StartTime;
		TheEndTime = (EndTime - StartTime) * Ratio_End + StartTime;
	}
	else if (TimeRangeType == EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime)
	{
		TheStartTime = Time_Start;
		TheEndTime = Time_End;
	}
	else if (TimeRangeType == EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime_OnlyEndTime)
	{
		TheEndTime = Time_End;
	}
	else if (TimeRangeType == EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime_OnlyStartTime)
	{
		TheStartTime = Time_Start;
	}

	if (TheEndTime - TheStartTime > 0.0f && TimeNow - TheStartTime >= 0.0f)
	{
		const float InAlpha = (TimeNow - TheStartTime)/(TheEndTime - TheStartTime);
		const float OutAlpha = UGBWPowerfulToolsFuncLib::GetAlphaByBlendOption(BlendOption,InAlpha);

		Res = (To - From) * OutAlpha + From;
	}

	if (TimeNow >= TheEndTime)
	{
		Res = To;
	}
	
	return Res;
}

float FGBWFlowMoveFloatCurveSetting::GetBlendValue(const float StartTime, const float EndTime,
                                                   const float TimeNow) const
{
	float Res = From;
		
	float TheStartTime = StartTime;
	float TheEndTime = EndTime;

	if (TimeRangeType == EGBWFlowMoveBlendCurveTimeRangeType::Ratio)
	{
		TheStartTime = (EndTime - StartTime) * Ratio_Start + StartTime;
		TheEndTime = (EndTime - StartTime) * Ratio_End + StartTime;
	}
	else if (TimeRangeType == EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime)
	{
		TheStartTime = Time_Start + StartTime;
		TheEndTime = Time_End + StartTime;
	}
	else if (TimeRangeType == EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime_OnlyEndTime)
	{
		TheEndTime = Time_End + StartTime;
	}
	else if (TimeRangeType == EGBWFlowMoveBlendCurveTimeRangeType::AbsoluteTime_OnlyStartTime)
	{
		TheStartTime = Time_Start + StartTime;
	}

	if (TheEndTime - TheStartTime > 0.0f && TimeNow - TheStartTime >= 0.0f)
	{
		const float InAlpha = (TimeNow - TheStartTime)/(TheEndTime - TheStartTime);
		const float OutAlpha = UGBWPowerfulToolsFuncLib::GetAlphaByBlendOption(BlendOption,InAlpha);

		Res = (To - From) * OutAlpha + From;
	}

	if (TimeNow >= TheEndTime && TimeNow > 0.0f)
	{
		Res = To;
	}
		
	return Res;
}

float FGBWFlowMoveControlSettings_BlendSpaceAxisControlSetting::GetAxisValue(
	UGBWFlowMoveComponent* FlowMoveComponent,
	const float OldValue,
	const float DeltaTime,
	const float ExecuteTime,
	const FVector2D AxisRange) const
{
	if (!FlowMoveComponent)
	{
		return OldValue;
	}

	if (AxisValue.bIsSpecifyValue)
	{
		return AxisValue.SpecifyValue;
	}

	const float UpdateDurationIntervalTime =
				AxisUpdate.AxisMaxUpdateIntervalTime>0.0f?
				ExecuteTime - static_cast<int>(ExecuteTime / AxisUpdate.AxisMaxUpdateIntervalTime) * AxisUpdate.AxisMaxUpdateIntervalTime
				: ExecuteTime;
	if (AxisUpdate.AxisUpdateDuration > 0.0f
	&& UpdateDurationIntervalTime >= AxisUpdate.AxisUpdateDuration)
	{
		return OldValue;
	}

	float Res = OldValue;
	
	if (AxisValue.ValueFrom == EFMBSAxisType::Speed)
	{
		Res = FlowMoveComponent->InputState.OwnerCharacter->GetVelocity().Length();
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::HorizontalDirection)
	{
		const float DAngle_H = UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(
			FlowMoveComponent->GetDirectionVector(AxisValue.AngleFrom.From,FVector::ZeroVector),
			FlowMoveComponent->GetDirectionVector(AxisValue.AngleFrom.To,FVector::ZeroVector));
		Res = -DAngle_H;
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::PitchDirection)
	{
		const float DAngle_V = UGBWPowerfulToolsFuncLib::GetAngleInVerticalPlane(
			FlowMoveComponent->GetDirectionVector(AxisValue.AngleFrom.From,FVector::ZeroVector),
			FlowMoveComponent->GetDirectionVector(AxisValue.AngleFrom.To,FVector::ZeroVector));
		Res = -DAngle_V;
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::MoveVectorLength)
	{
		Res = FlowMoveComponent->InputState.MoveVector.Length();
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::TargetSpeed)
	{
		Res = FlowMoveComponent->GetMoveControlParam().MoveSpeed;
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::TargetYawAngleSpeed)
	{
		Res = FMath::Abs(FlowMoveComponent->MoveControlResult.DeltaRotator.Yaw) / DeltaTime;
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::SelectByState)
	{
		for (const auto Element : AxisValue.StateSelectValue)
		{
			bool bIsGet = false;
			bool Result = false;
			float Duration = 0.0f;
			UGBWPSFuncLib::GetPSBoolValue(FlowMoveComponent->InputState.OwnerCharacter, Element.Key, bIsGet, Result, Duration);
			if (Result)
			{
				Res = Element.Value;
				break;
			}
		}
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::SelectByValueCondition)
	{
		for (const auto Element : AxisValue.ValueConditionSelectValue)
		{
			if (Element.IsMeet(FlowMoveComponent))
			{
				Res = Element.Value;
				break;
			}
		}
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::SelectBySpeedMode)
	{
		for (const auto Element : AxisValue.SpeedModeSelectValue)
		{
			if (Element.Key == FlowMoveComponent->GetFlowMoveSpeedMode())
			{
				Res = Element.Value;
				break;
			}
		}
	}
	else if (AxisValue.ValueFrom == EFMBSAxisType::PlaneCoordinateSystem)
	{
		Res = AxisValue.PlaneCoordinateValue.GetValue(FlowMoveComponent);
	}

	for (const auto Element : AxisValue.AxisValueAdjust.AxisDeadZone)
	{
		if (Res > Element.DeadZoneRange.X && Res < Element.DeadZoneRange.Y)
		{
			Res = Element.DeadZoneValue;
		}
	}

	return Res;
}

float FGBWFlowMoveControlSettings_SpeedAdjust::GetSpeedAdjustRate(const float InRate, const float DeltaTime, UGBWFlowMoveComponent* FlowMoveComponent) const
{
	float RateRes = 1.0f;
	if (bIsSpeedAdjustByMoveDirectionPitch)
	{
		const float MoveDirectionPitch = FlowMoveComponent->InputState.OwnerCharacter->GetVelocity().Rotation().Pitch;
		if (MoveDirectionPitch > 0.0f)
		{
			RateRes = RateRes * (1.0f - MoveDirectionPitch/90.0f * MaxRate_ByUpPitch);
		}
		else if (MoveDirectionPitch < 0.0f)
		{
			RateRes = RateRes * (1.0f - MoveDirectionPitch/90.0f * MaxRate_ByDownPitch);
		}
	}
	if (bIsSpeedAdjustByMoveDirectionHorizontalAngle)
	{
		const float MoveDirectionYaw = FMath::Abs<float>(
			UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(
			FlowMoveComponent->GetDirectionVector(Angle.From,FVector::ZeroVector),
			FlowMoveComponent->GetDirectionVector(Angle.To,FVector::ZeroVector)));
		if (MoveDirectionYaw > HorizontalAngle_Begin
			&& HorizontalAngle_End > HorizontalAngle_Begin)
		{
			float AdjustRate = (MoveDirectionYaw - HorizontalAngle_Begin) / (HorizontalAngle_End - HorizontalAngle_Begin) * MaxRate_ByHorizontalAngle;
			if (MoveDirectionYaw > HorizontalAngle_End)
			{
				AdjustRate = MaxRate_ByHorizontalAngle;
			}
			RateRes = RateRes * (1.0f - AdjustRate);
		}
	}
	float SpeedAdjustRate = 1.0f;
	if (bIsAdjustByMoveVectorLength)
	{
		if (bOnlyByHorizontalMoveVectorLength)
		{
			FVector M = FlowMoveComponent->InputState.MoveVector;
			M.Z = 0.0f;
			SpeedAdjustRate = M.Length();
		}
		else
		{
			SpeedAdjustRate = FlowMoveComponent->InputState.MoveVector.Length();
		}
	}
	RateRes = RateRes * SpeedAdjustRate;

	return FMath::FInterpTo(
		InRate,
		RateRes,
		DeltaTime,
		RateRes >= InRate? MoveSpeedAdjustSmoothSpeed_LowToHigh
		: (MoveSpeedAdjustSmoothSpeed_HighToLow * (1.0f + 9.0f * FMath::Clamp((1.0f - SpeedAdjustRate), 0.0f, 1.0f))));
}

FGBWFlowMoveMoveControlInitInfo::FGBWFlowMoveMoveControlInitInfo(const ACharacter* OwnerCharacter,
		const FVector_NetQuantizeNormal& InBeginTargetMoveToDirection,
		const FVector_NetQuantizeNormal& InBeginMoveToDirection,
		const FVector_NetQuantizeNormal& InBeginVelocityDirection,
		const float InBeginSpeed)
{
	if (OwnerCharacter)
	{
		bIsValid = true;
		BeginLocation = OwnerCharacter->GetTransform().GetLocation();
		bHasTargetPoint = false;
		BeginTargetMoveToDirection = InBeginTargetMoveToDirection;
		BeginMoveToDirection = InBeginMoveToDirection;
		BeginDirection = OwnerCharacter->GetActorForwardVector();
		BeginVelocity = OwnerCharacter->GetVelocity();
		BeginSpeed = InBeginSpeed;
		BeginVelocityDirection = InBeginVelocityDirection;
	}
	else
	{
		bIsValid = false;
	}
}

FGBWFlowMoveMoveControlInitInfo::FGBWFlowMoveMoveControlInitInfo(
		const ACharacter* OwnerCharacter,
		const FVector_NetQuantize100& InTargetPoint,
		const FVector_NetQuantizeNormal& InBeginTargetMoveToDirection,
		const FVector_NetQuantizeNormal& InBeginMoveToDirection,
		const FVector_NetQuantizeNormal& InBeginVelocityDirection,
		const float InBeginSpeed)
{
	if (OwnerCharacter)
	{
		bIsValid = true;
		BeginLocation = OwnerCharacter->GetTransform().GetLocation();
		bHasTargetPoint = true;
		TargetPoint = InTargetPoint;
		BeginTargetMoveToDirection = InBeginTargetMoveToDirection;
		BeginMoveToDirection = InBeginMoveToDirection;
		BeginDirection = OwnerCharacter->GetActorForwardVector();
		BeginVelocity = OwnerCharacter->GetVelocity();
		BeginSpeed = InBeginSpeed;
		BeginVelocityDirection = InBeginVelocityDirection;
	}
	else
	{
		bIsValid = false;
	}
}

void UGBWFlowMoveBrain_Base::Init(UGBWFlowMoveComponent* InFlowMoveComponent)
{
	FlowMoveComponent = InFlowMoveComponent;
	if (FlowMoveComponent)
	{
		OwnerCharacter = FlowMoveComponent->InputState.OwnerCharacter;
	}
}

bool UGBWFlowMoveBrain_Base::CheckValid(UGBWFlowMoveComponent* InFlowMoveComponent)
{
	if (FlowMoveComponent && OwnerCharacter)
	{
		return true;
	}

	Init(InFlowMoveComponent);
	if (FlowMoveComponent && OwnerCharacter)
	{
		return true;
	}
	return false;
}

void UGBWFlowMoveBrain_Base::OnFlowMoveEvent_Implementation(FGBWFlowMoveState FlowMoveState, const FFlowMoveEvent& FlowMoveEvent)
{}

void UGBWFlowMoveBrain_Base::OnFMEvent(
	FGBWFlowMoveState FlowMoveState, const FFlowMoveEvent& FlowMoveEvent)
{
	OnFlowMoveEvent(FlowMoveState, FlowMoveEvent);
}

void UGBWFlowMoveBrain_Base::FM_AdjustFlowMoveResult(
		FGBWFlowMoveControlParam MoveControlParamNow,
		float DeltaTime,
		FVector Pending_Velocity,
		FRotator Pending_DeltaRotation,
		bool& bIsGet,
		FVector& Res_Velocity,
		FRotator& Res_DeltaRotation)
{
	AdjustFlowMoveResult(
		MoveControlParamNow,
		DeltaTime,
		Pending_Velocity,
		Pending_DeltaRotation,
		bIsGet,
		Res_Velocity,
		Res_DeltaRotation);
}

void UGBWFlowMoveBrain_Base::FM_GetMoveControlDirectly(const FGBWFlowMoveControlParam& MoveControlParamNow,
                                                       float DeltaTime,
                                                       const FVector& CurrentVelocity,
                                                       const FRotator& CurrentDeltaRotation,
                                                       bool& bIsGet,
                                                       FVector& Velocity,
                                                       FRotator& DeltaRotation)
{
	GetMoveControlDirectly(
		MoveControlParamNow,DeltaTime,
		CurrentVelocity,
		CurrentDeltaRotation,
		bIsGet,
		Velocity,
		DeltaRotation);
}

void UGBWFlowMoveBrain_Base::FM_GetTarget(const FGBWFlowMoveControlParam& MoveControlParamNow,
                                          float DeltaTime, const FVector& CurrentTargetPoint,
                                          float CurrentToTargetSpeed, bool& bIsGet, FVector& TargetPoint, float& ToTargetSpeed)
{
	GetTarget(
		MoveControlParamNow,DeltaTime,
		CurrentTargetPoint,
		CurrentToTargetSpeed,
		bIsGet,
		TargetPoint,
		ToTargetSpeed);
}

void UGBWFlowMoveBrain_Base::FM_GetMoveSpeed(const FGBWFlowMoveControlParam& MoveControlParamNow,
                                             float DeltaTime, float CurrentMoveSpeed,
                                             bool& bIsGet, float& MoveSpeed)
{
	GetMoveSpeed(
		MoveControlParamNow,DeltaTime,
		CurrentMoveSpeed,
		bIsGet,
		MoveSpeed);
}

void UGBWFlowMoveBrain_Base::FM_GetMoveDirection(const FGBWFlowMoveControlParam& MoveControlParamNow,
                                                 float DeltaTime,
                                                 const FVector& CurrentMoveDirection, bool& bIsGet, TEnumAsByte<EFlowMoveDirectionType>& MovementDirectionType,
                                                 FVector& WorldAbsoluteMoveDirection)
{
	GetMoveDirection(
		MoveControlParamNow,DeltaTime,
		CurrentMoveDirection,
		bIsGet,
		MovementDirectionType,
		WorldAbsoluteMoveDirection);
}

void UGBWFlowMoveBrain_Base::FM_GetGravity(const FGBWFlowMoveControlParam& MoveControlParamNow,
                                           float DeltaTime, float CurrentGravity,
                                           bool CurrentIsCalculateFallingDistance, bool& bIsGet, float& Gravity, bool& bIsCalculateFallingDistance)
{
	GetGravity(
		MoveControlParamNow,DeltaTime,
		CurrentGravity,
		CurrentIsCalculateFallingDistance,
		bIsGet,
		Gravity,
		bIsCalculateFallingDistance);
}

void UGBWFlowMoveBrain_Base::FM_GetPathOffset(const FGBWFlowMoveControlParam& MoveControlParamNow,
                                              float DeltaTime,
                                              const FVector& CurrentPathOffsetDirection, const FVector& CurrentPathOffset, bool& bIsGet,
                                              FVector& PathOffset, TEnumAsByte<EFlowMoveDirectionType>& PathOffsetDirectionType, FVector& WorldAbsolutePathOffsetDirection)
{
	GetPathOffset(
		MoveControlParamNow,DeltaTime,
		CurrentPathOffsetDirection,
		CurrentPathOffset,
		bIsGet,
		PathOffset,
		PathOffsetDirectionType,
		WorldAbsolutePathOffsetDirection);
}

void UGBWFlowMoveBrain_Base::FM_GetRotationYawOffset(const FGBWFlowMoveControlParam& MoveControlParamNow, float DeltaTime,
                                                     float CurrentRotationYawOffset, bool& bIsGet, float& RotationYawOffset)
{
	GetRotationYawOffset(
		MoveControlParamNow,
		DeltaTime,
		CurrentRotationYawOffset,
		bIsGet,
		RotationYawOffset);
}

void UGBWFlowMoveBrain_Base::FM_GetAnimPlayRate(const FGBWFlowMoveControlParam& MoveControlParamNow,
                                                float DeltaTime,
                                                float CurrentAnimPlayRate, bool& bIsGet, float& AnimPlayRate)
{
	GetAnimPlayRate(
		MoveControlParamNow,DeltaTime,
		CurrentAnimPlayRate,
		bIsGet,
		AnimPlayRate);
}

void UGBWFlowMoveBrain_Base::FM_GetActionDuration(const FGBWFlowMoveControlParam& MoveControlParamNow,
                                                  float DeltaTime,
                                                  bool& bIsGet, float& ActionDuration)
{
	GetActionDuration(
	MoveControlParamNow,DeltaTime,
		bIsGet,
		ActionDuration);
}

bool FGBWFlowMoveTraceSetting::IsMetScreeningConditions(const AActor* InActor)
{
	if (!InActor)
	{
		return false;
	}
	if (!UseScreeningConditions
		||(ActorWithClass.IsEmpty() && ActorWithTag.IsEmpty()))
	{
		return true;
	}

	bool bIsMetActorClass = false;
	bool bIsMetActorTag = false;
	for (auto Class: ActorWithClass)
	{
		if (IsValid(Class) && InActor->GetClass()->IsChildOf(Class))
		{
			bIsMetActorClass = true;
			break;
		}
	}
	for (const auto Tag: ActorWithTag)
	{
		if (InActor->ActorHasTag(Tag))
		{
			bIsMetActorTag = true;
			break;
		}
	}

	return bIsMetActorClass || bIsMetActorTag;
}

FFlowMoveEvent::FFlowMoveEvent(EFlowMoveEventType TheEventType)
{
	EventType = TheEventType;
}

FFlowMoveEvent::FFlowMoveEvent(USceneComponent* TheNewFocusActorComponent)
{
	EventType = EFlowMoveEventType::OnFocusActorChange;
	NewFocusActorComponent = TheNewFocusActorComponent;
}

FFlowMoveEvent::FFlowMoveEvent(USkeletalMeshComponent* InSKMesh,
		FName InFootStepType,
		FName InFootStepSocket,
		FGBWFlowMoveTraceSetting FloorTraceSetting)
{
	EventType = EFlowMoveEventType::OnFootStep;
	FootStepType = InFootStepType;

	if (InSKMesh)
	{
		FVector Start = InSKMesh->GetSocketLocation(InFootStepSocket);
		FVector End = Start - FVector::UpVector * 30.0f;
		TArray<struct FHitResult> OutHits;
		FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams::DefaultQueryParam;
		CollisionQueryParams.bTraceComplex = false;
		CollisionQueryParams.AddIgnoredActor(InSKMesh->GetOwner());
		FootStepFloorHit = UGBWFlowMoveFuncLib::LineTrace(
			InSKMesh->GetWorld(),
			OutHits,
			Start,
			End,
			FloorTraceSetting,
			CollisionQueryParams);
	}
}

bool FGBWFlowMoveMove_ValueCondition::IsMeet(UGBWFlowMoveComponent* FlowMoveComponent) const
{
	if (FlowMoveComponent)
	{
		bool Res = false;
		const float TheValue = Value.GetValue(FlowMoveComponent);

		for (const auto Element : Ranges)
		{
			if (TheValue >= Element.X && TheValue <= Element.Y)
			{
				Res = true;
				break;
			}
		}

		return bInvert? !Res : Res;
	}
	
	return false;
}

bool FGBWFlowMoveMoveControlSetting_ValueConditionSelectItem::IsMeet(UGBWFlowMoveComponent* FlowMoveComponent) const
{
	bool Res = false;
	switch (MeetType) {
	case AnyMeet:
		for (const auto Element : ValueConditions)
		{
			Res = Element.IsMeet(FlowMoveComponent);
			if (Res)
			{
				break;
			}
		}
		break;
	case AllMeet:
		for (const auto Element : ValueConditions)
		{
			Res = Element.IsMeet(FlowMoveComponent);
			if (!Res)
			{
				break;
			}
		}
		break;
	case NoMeet:
		for (const auto Element : ValueConditions)
		{
			Res = !Element.IsMeet(FlowMoveComponent);
			if (!Res)
			{
				break;
			}
		}
		break;
	case AnyNoMeet:
		for (const auto Element : ValueConditions)
		{
			Res = !Element.IsMeet(FlowMoveComponent);
			if (Res)
			{
				break;
			}
		}
		break;
	default: ;
	}

	return Res;
}

bool FGBWFlowMoveMoveValue_ValueConditionSelectItem::IsMeet(UGBWFlowMoveComponent* FlowMoveComponent) const
{
	bool Res = false;
	switch (MeetType) {
	case AnyMeet:
		for (const auto Element : ValueConditions)
		{
			Res = Element.IsMeet(FlowMoveComponent);
			if (Res)
			{
				break;
			}
		}
		break;
	case AllMeet:
		for (const auto Element : ValueConditions)
		{
			Res = Element.IsMeet(FlowMoveComponent);
			if (!Res)
			{
				break;
			}
		}
		break;
	case NoMeet:
		for (const auto Element : ValueConditions)
		{
			Res = !Element.IsMeet(FlowMoveComponent);
			if (!Res)
			{
				break;
			}
		}
		break;
	case AnyNoMeet:
		for (const auto Element : ValueConditions)
		{
			Res = !Element.IsMeet(FlowMoveComponent);
			if (Res)
			{
				break;
			}
		}
		break;
	default: ;
	}

	return Res;
}

bool FGBWFlowMoveControlParam::GetToTargetRateCurve(AActor* InActor, float DeltaTime, float& Value, float& DeltaValue) const
{
	if (!MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.bToTargetRate)
	{
		return false;
	}

	bool bIsGet = false;
	const float StartTime = MoveControlSettings.StartTime;
	const float EndTime = MoveControlSettings.EndTime;
	const float TimeNow = (MoveControlSettings.EndTime - MoveControlSettings.StartTime) * MoveControlSettings.Progress + MoveControlSettings.StartTime;
	const float ScaleDeltaTime = (TimeNow > 0.0f? MoveControlSettings.TimeNow / TimeNow : 1.0f) * DeltaTime * AnimPlayRateNow;
	
	if (MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.ToTargetRateCurve.Type == EGBWFlowMoveCurveType::CurveAsset)
	{
		Value = MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.ToTargetRateCurve.Curve.GetRichCurveConst()->Eval(
				TimeNow - StartTime
				);
		DeltaValue = Value - MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.ToTargetRateCurve.Curve.GetRichCurveConst()->Eval(
			TimeNow - StartTime - ScaleDeltaTime
			);
		bIsGet = true;
	}
	else if (MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.ToTargetRateCurve.Type == EGBWFlowMoveCurveType::BlendOption)
	{
		Value = MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.ToTargetRateCurve.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow);
		DeltaValue = Value - MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.ToTargetRateCurve.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow - ScaleDeltaTime);
		bIsGet = true;
	}

	if (!bIsGet)
	{
		Value = 0.0f;
		DeltaValue = 0.0f;
	}
	
	return bIsGet;	
}

bool FGBWFlowMoveControlParam::GetRotationOffsetCurve(AActor* InActor, float DeltaTime, float& Value,
	float& DeltaValue) const
{
	bool bIsGet = false;
	const float StartTime = MoveControlSettings.StartTime;
	const float EndTime = MoveControlSettings.EndTime;
	const float TimeNow = (MoveControlSettings.EndTime - MoveControlSettings.StartTime) * MoveControlSettings.Progress + MoveControlSettings.StartTime;
	const float ScaleDeltaTime = (TimeNow > 0.0f? MoveControlSettings.TimeNow / TimeNow : 1.0f) * DeltaTime * AnimPlayRateNow;
	
	if (MoveControlSettings.RotationSetting.RotationYawOffset.Type == EGBWFlowMoveCurveType::CurveAsset)
	{
		Value = MoveControlSettings.RotationSetting.RotationYawOffset.Curve.GetRichCurveConst()->Eval(
				TimeNow - StartTime
				);
		DeltaValue = Value - MoveControlSettings.RotationSetting.RotationYawOffset.Curve.GetRichCurveConst()->Eval(
			TimeNow - StartTime - ScaleDeltaTime
			);
		bIsGet = true;
	}
	else if (MoveControlSettings.RotationSetting.RotationYawOffset.Type == EGBWFlowMoveCurveType::BlendOption)
	{
		Value = MoveControlSettings.RotationSetting.RotationYawOffset.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow);
		DeltaValue = Value - MoveControlSettings.RotationSetting.RotationYawOffset.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow - ScaleDeltaTime);
		bIsGet = true;
	}

	if (!bIsGet)
	{
		Value = 0.0f;
		DeltaValue = 0.0f;
	}
	
	return bIsGet;	
}

bool FGBWFlowMoveControlParam::GetPathOffsetCurve(float DeltaTime, FVector& Value, FVector& DeltaValue) const
{
	Value = FVector::ZeroVector;
	DeltaValue = FVector::ZeroVector;
	if (!MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.bPathOffset
		|| !IsValid(MoveControlSettings.FromAsset)
		|| !MoveControlSettings.FromAsset
		|| !MoveControlSettings.FromAsset->GetClass())
	{
		return false;
	}

	bool bIsGet = false;
	float StartTime = MoveControlSettings.StartTime;
	float EndTime = MoveControlSettings.EndTime;
	float TimeNow = (MoveControlSettings.EndTime - MoveControlSettings.StartTime) * MoveControlSettings.Progress + MoveControlSettings.StartTime;
	float ScaleDeltaTime = (TimeNow > 0.0f? MoveControlSettings.TimeNow / TimeNow : 1.0f) * DeltaTime * AnimPlayRateNow;
	
	if (MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.Type == EGBWFlowMoveCurveType::CurveAsset)
	{
		Value = MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.Curve.GetValue(
		FMath::Clamp<float>(TimeNow - StartTime, 0.0f, EndTime - StartTime));
		DeltaValue = Value - MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.Curve.GetValue(
		FMath::Clamp<float>(TimeNow - StartTime - ScaleDeltaTime, 0.0f, EndTime - StartTime));
		bIsGet = true;
	}
	else if (MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.Type == EGBWFlowMoveCurveType::BlendOption)
	{
		Value = MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow);
		DeltaValue = Value - MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.GetBlendValue(
			StartTime,
			EndTime,
			FMath::Clamp<float>(TimeNow - ScaleDeltaTime, StartTime, EndTime));
		bIsGet = true;
	}
	else if (MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.Type == EGBWFlowMoveCurveType::RootMotion)
	{
		if (const UAnimMontage* Montage = Cast<UAnimMontage>(MoveControlSettings.FromAsset); Montage && Montage->HasRootMotion())
		{
			FRootMotionMovementParams RootMotionMovementParams;
			RootMotionMovementParams.bHasRootMotion = true;
			FRootMotionMovementParams RootMotionMovementParamsAll;
			RootMotionMovementParamsAll.bHasRootMotion = true;

			if (FMagicPlayerPlayState AnimPlayState;
				AnimComponent->GetMagicPlayerState(MoveControlSettings.AnimSlot,AnimPlayState))
			{
				float CurrentSampleDataTime = TimeNow;
				const float MultipliedSampleRateScale = Montage->RateScale;
				const float SampleMoveDelta = ScaleDeltaTime * MultipliedSampleRateScale;

				float PrevSampleDataTime = FMath::Max(0.0f, CurrentSampleDataTime - SampleMoveDelta);

				RootMotionMovementParams.AccumulateWithBlend(
					Montage->ExtractRootMotionFromTrackRange(
						PrevSampleDataTime,
						CurrentSampleDataTime),
					1.0f);
				RootMotionMovementParamsAll.AccumulateWithBlend(
					Montage->ExtractRootMotionFromTrackRange(
						StartTime,
						CurrentSampleDataTime),
					1.0f);
			}
			
			DeltaValue = RootMotionMovementParams.GetRootMotionTransform().GetTranslation();
			Value = RootMotionMovementParamsAll.GetRootMotionTransform().GetTranslation();
			bIsGet = true;
		}
		else if (const UAnimSequence* AnimSequence = Cast<UAnimSequence>(MoveControlSettings.FromAsset); AnimSequence && AnimSequence->HasRootMotion())
		{
			FRootMotionMovementParams RootMotionMovementParams;
			RootMotionMovementParams.bHasRootMotion = true;
			FRootMotionMovementParams RootMotionMovementParamsAll;
			RootMotionMovementParamsAll.bHasRootMotion = true;

			if (FMagicPlayerPlayState AnimPlayState;
				AnimComponent->GetMagicPlayerState(MoveControlSettings.AnimSlot,AnimPlayState))
			{
				float CurrentSampleDataTime = TimeNow;
				const float MultipliedSampleRateScale = AnimSequence->RateScale;
				const float SampleMoveDelta = ScaleDeltaTime * MultipliedSampleRateScale;

				float PrevSampleDataTime = FMath::Max(0.0f, CurrentSampleDataTime - SampleMoveDelta);

				RootMotionMovementParams.AccumulateWithBlend(
					AnimSequence->ExtractRootMotion(
						PrevSampleDataTime,
						SampleMoveDelta,
						AnimPlayState.PlayerParamCache.bLoopAnimation),
					1.0f);
				RootMotionMovementParamsAll.AccumulateWithBlend(
					AnimSequence->ExtractRootMotionFromRange(
						StartTime,
						CurrentSampleDataTime),
					1.0f);
			}
			
			DeltaValue = RootMotionMovementParams.GetRootMotionTransform().GetTranslation();
			Value = RootMotionMovementParamsAll.GetRootMotionTransform().GetTranslation();
			bIsGet = true;
		}
		else if (const UBlendSpace* BlendSpace = Cast<UBlendSpace>(MoveControlSettings.FromAsset))
		{
			if (FMagicPlayerPlayState AnimPlayState;
				AnimComponent->GetMagicPlayerState(MoveControlSettings.AnimSlot,AnimPlayState))
			{
				FRootMotionMovementParams RootMotionMovementParams;
				RootMotionMovementParams.bHasRootMotion = true;
				FRootMotionMovementParams RootMotionMovementParamsAll;
				RootMotionMovementParamsAll.bHasRootMotion = true;
				for (int32 I = 0; I < AnimPlayState.PlayState.LastBlendSampleDataCache.Num(); ++I)
				{
					FBlendSampleData SampleEntry = AnimPlayState.PlayState.LastBlendSampleDataCache[I];
					const int32 SampleDataIndex = SampleEntry.SampleDataIndex;

					// Skip SamplesPoints that has no relevant weight
					if (BlendSpace->GetBlendSamples().IsValidIndex(SampleDataIndex) && (SampleEntry.TotalWeight > ZERO_ANIMWEIGHT_THRESH))
					{
						const FBlendSample& Sample = BlendSpace->GetBlendSamples()[SampleDataIndex];
						if (Sample.Animation)
						{
							float CurrentSampleDataTime = SampleEntry.Time;

							const float MultipliedSampleRateScale = Sample.Animation->RateScale * Sample.RateScale;
							const float SampleMoveDelta = DeltaTime * MultipliedSampleRateScale * AnimPlayRateNow;

							float PrevSampleDataTime = FMath::Max(0.0f, CurrentSampleDataTime - SampleMoveDelta);

							if (UGBWPowerfulToolsFuncLib::CanAnimRootMotion(Sample.Animation,
								PrevSampleDataTime,
								SampleMoveDelta,
								AnimPlayState.PlayerParamCache.bLoopAnimation))
							{
								RootMotionMovementParams.AccumulateWithBlend(
								Sample.Animation->ExtractRootMotion(
									PrevSampleDataTime,
									SampleMoveDelta,
									AnimPlayState.PlayerParamCache.bLoopAnimation),
								SampleEntry.GetClampedWeight());
								RootMotionMovementParamsAll.AccumulateWithBlend(
									Sample.Animation->ExtractRootMotionFromRange(
										StartTime,
										CurrentSampleDataTime),
									SampleEntry.GetClampedWeight());
							}
						}
					}
				}
				DeltaValue = RootMotionMovementParams.GetRootMotionTransform().GetTranslation();
				Value = RootMotionMovementParamsAll.GetRootMotionTransform().GetTranslation();
				bIsGet = true;
			}
		}

		if (bIsGet)
		{
			Value = Value * MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.RootMotionScale;
			DeltaValue = DeltaValue * MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.RootMotionScale;
		}
	}
	
	return bIsGet;	
}

bool FGBWFlowMoveControlParam::GetMoveSpeedCurve(AActor* InActor, FGBWFlowMoveFloatCurveSetting CurveSetting, const float DeltaTime, float& Value, float& DeltaValue) const
{
	if (!MoveControlSettings.Velocity.ControllableVelocity.MoveSpeed.Type == EGBWFlowMoveControlSettingsType::Curve
		|| !MoveControlSettings.FromAsset
		|| !MoveControlSettings.FromAsset->GetClass())
	{
		return false;
	}

	bool bIsGet = false;
	const float StartTime = MoveControlSettings.StartTime;
	const float EndTime = MoveControlSettings.EndTime;
	const float TimeNow = (MoveControlSettings.EndTime - MoveControlSettings.StartTime) * MoveControlSettings.Progress + MoveControlSettings.StartTime;
	const float ScaleDeltaTime = (TimeNow > 0.0f? MoveControlSettings.TimeNow / TimeNow : 1.0f) * DeltaTime * AnimPlayRateNow;
	if (CurveSetting.Type == EGBWFlowMoveCurveType::CurveAsset)
	{
		Value = CurveSetting.Curve.GetRichCurveConst()->Eval(
				TimeNow - StartTime
				);
		DeltaValue = Value - CurveSetting.Curve.GetRichCurveConst()->Eval(
			TimeNow - StartTime - ScaleDeltaTime
			);
		bIsGet = true;
	}
	else if (CurveSetting.Type == EGBWFlowMoveCurveType::BlendOption)
	{
		Value = CurveSetting.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow);
		DeltaValue = Value - CurveSetting.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow - ScaleDeltaTime);
		bIsGet = true;
	}

	if (!bIsGet)
	{
		Value = 0.0f;
		DeltaValue = 0.0f;
	}
	
	return bIsGet;
}

bool FGBWFlowMoveControlParam::GetGravityCurve(AActor* InActor, const float DeltaTime, float& Value, float& DeltaValue) const
{
	if (!MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Type == EGBWFlowMoveControlSettingsType::Curve
		|| !MoveControlSettings.FromAsset
		|| !MoveControlSettings.FromAsset->GetClass())
	{
		return false;
	}

	bool bIsGet = false;
	const float StartTime = MoveControlSettings.StartTime;
	const float EndTime = MoveControlSettings.EndTime;
	const float TimeNow = (MoveControlSettings.EndTime - MoveControlSettings.StartTime) * MoveControlSettings.Progress + MoveControlSettings.StartTime;
	const float ScaleDeltaTime = (TimeNow > 0.0f? MoveControlSettings.TimeNow / TimeNow : 1.0f) * DeltaTime * AnimPlayRateNow;
	if (MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Type == EGBWFlowMoveCurveType::CurveAsset)
	{
		Value = MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Curve.Curve.GetRichCurveConst()->Eval(
				TimeNow - StartTime
				);
		DeltaValue = Value - MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Curve.Curve.GetRichCurveConst()->Eval(
			TimeNow - StartTime - ScaleDeltaTime
			);
		bIsGet = true;
	}
	else if (MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Curve.Type == EGBWFlowMoveCurveType::BlendOption)
	{
		Value = MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Curve.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow);
		DeltaValue = Value - MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Curve.GetBlendValue(
			StartTime,
			EndTime,
			TimeNow - ScaleDeltaTime);
		bIsGet = true;
	}

	if (!bIsGet)
	{
		Value = 0.0f;
		DeltaValue = 0.0f;
	}
	
	return bIsGet;
}

void FGBWFlowMoveControlParam::GetActionDurationAndAnimPlayRate(
	const float DeltaTime, const UGBWFlowMoveComponent* FMComponent, const FGBWFlowMoveControlSettingsPro& InMoveControlSettings,const bool bSmoothPlayRate,
	float& TheActionDuration, float& TheAnimPlayRate) const
{
	if (!FMComponent)
	{
		return;
	}

	const float StartTime = InMoveControlSettings.StartTime;
	const float EndTime = InMoveControlSettings.EndTime;
	const float TimeNow = (InMoveControlSettings.EndTime - InMoveControlSettings.StartTime) * InMoveControlSettings.Progress + InMoveControlSettings.StartTime;
	const float ScaleDeltaTime = (TimeNow > 0.0f? InMoveControlSettings.TimeNow / TimeNow : 1.0f) * DeltaTime * AnimPlayRateNow;
	
	//ActionDuration
	bool bIsGetActionDuration = false;
	float Res_ActionDuration = 0.0f;
	if (InMoveControlSettings.bTryGetMoveControlParamFromInterface && FMComponent->FlowMoveBrain)
	{
		FMComponent->FlowMoveBrain->FM_GetActionDuration(
			FMComponent->GetMoveControlParam(),DeltaTime,
			bIsGetActionDuration,
			Res_ActionDuration);
	}
	if (bIsGetActionDuration && Res_ActionDuration > 0.0f)
	{
		TheActionDuration = Res_ActionDuration;
	}
	else if (InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustType == BySpecifyActionDuration
				&& InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.SpecifyActionDuration > 0.0f)
	{
		TheActionDuration = InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.SpecifyActionDuration;
	}
	else
	{
		TheActionDuration = EndTime - StartTime;
	}

	//Anim PlayRate
	bool bIsGetAnimPlayRate = false;
	float Res_AnimPlayRate = 1.0f;
	if (InMoveControlSettings.bTryGetMoveControlParamFromInterface && FMComponent->FlowMoveBrain)
	{
		FMComponent->FlowMoveBrain->FM_GetAnimPlayRate(
			FMComponent->GetMoveControlParam(),DeltaTime,
			FMComponent->GetMoveControlParam().AnimPlayRateNow,
			bIsGetAnimPlayRate,
			Res_AnimPlayRate);
	}
	if (bIsGetAnimPlayRate)
	{
		TheAnimPlayRate = Res_AnimPlayRate;
	}
	else
	{
		float NewRate = 1.0f;
		
		FVector Velocity = FMComponent->InputState.OwnerCharacter->GetVelocity();
		Velocity.Z = 0.0f;
		const float Speed = Velocity.Length()  / FMComponent->GetActorScale();

		if (InMoveControlSettings.AdjustSettings.bAdjustAnimPlayRate)
		{
			if (InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustType == ByMoveSpeed
				&& InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.ReferenceSpeed > 0)
			{
				NewRate = Speed / InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.ReferenceSpeed;
				if (NewRate >= 1)
				{
					NewRate = 1 + FMath::Abs(NewRate-1) * InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustAnimPlayRateScale;
				}
				else
				{
					NewRate = FMath::Max(0.1f, 1 - FMath::Abs(NewRate-1) * InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustAnimPlayRateScale);
				}

				NewRate = UKismetMathLibrary::FClamp(
				NewRate,
				InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.Min_AnimPlayRate,
				InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.Max_AnimPlayRate);
			}
			else if (InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustType == BySpecifyMoveSpeed
				&& InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.SpecifyMoveSpeed > 0.0f)
			{
				NewRate = Speed/InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.SpecifyMoveSpeed;
			}
			else if (InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustType == BySpecifyActionDuration
				&& TheActionDuration > 0.0f)
			{
				NewRate = (EndTime - StartTime) / TheActionDuration;
			}
			else if (InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustType == ByCurve)
			{
				NewRate = InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AnimPlayRateCurve.GetRichCurveConst()->Eval(
				TimeNow - StartTime
				);
			}
			else if (InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustType == ByRootMotion)
			{
				const float TargetAngleSpeed = FMath::Abs(FMComponent->MoveControlResult.DeltaRotator.Yaw) / DeltaTime;
				
				const float RealSpeed = FMComponent->InputState.OwnerCharacter->GetVelocity().Length();
				const float RMSpeed = RootMotionNow.GetTranslation().Length()/DeltaTime;
				const float AngleYawSpeed = FMath::Abs<float>(RootMotionNow.Rotator().Yaw / DeltaTime);
				const float TargetAngle = FMath::Abs<float>(UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(
					 FMComponent->InputState.OwnerCharacter->GetActorForwardVector(), FMComponent->InputState.ForwardVector));
				const float DeltaAngleSpeed = FMath::Abs<float>(TargetAngleSpeed - AngleYawSpeed);

				bool bUp = false;
				bool bDown = false;
				float Rate = 0.01f;

				if (RealSpeed <= InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustByAngleWhenRealMoveSpeedLessThan
					&& TargetAngle >= InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustWhenTargetAngleGreaterThan
					&& ((TargetAngleSpeed - AngleYawSpeed
							>= InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustWhenAngleSpeedDifferenceGreaterThan
						&& InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.bAdjustWhenTheTargetAngleSpeedHigher)
						|| (TargetAngleSpeed - AngleYawSpeed
								<= -InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustWhenAngleSpeedDifferenceGreaterThan
							&& InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.bAdjustWhenTheTargetAngleSpeedLower)))
				{
					Rate = DeltaAngleSpeed / 5.0f * 0.01f;
					if (TargetAngleSpeed > AngleYawSpeed)
					{
						bUp = true;
					}
					else
					{
						bDown = true;
					}
				}
				
				if (MoveSpeed >= InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustWhenTargetMoveSpeedGreaterThan
					&& ((MoveSpeed - RMSpeed >= InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustWhenSpeedDifferenceGreaterThan
						&& InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.bAdjustWhenTheTargetSpeedHigher)
					|| (MoveSpeed - RMSpeed <= -InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AdjustWhenSpeedDifferenceGreaterThan
						&& InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.bAdjustWhenTheTargetSpeedLower)))
				{
					Rate = FMath::Max<float>(FMath::Abs<float>(MoveSpeed - RMSpeed) / 10.0f * 0.01f, Rate);
					if (MoveSpeed > RMSpeed)
					{
						bUp = true;
						bDown = false;
					}
					else if (!bUp)
					{
						bDown = true;
					}
				}
				
				Rate = FMath::Clamp(Rate, 0.01f, 0.5f);
				if (bUp)
				{
					NewRate = AnimPlayRateNow+Rate;
				}
				else if (bDown)
				{
					NewRate = AnimPlayRateNow-Rate;
				}
				else
				{
					if (AnimPlayRateNow < 0.9f || AnimPlayRateNow > 1.1f)
					{
						NewRate = FMath::FInterpTo(AnimPlayRateNow,1.0f,DeltaTime,24.0f);
					}
					else
					{
						NewRate = 1.0f;
					}
				}
			}
		}

		NewRate = FMath::Clamp<float>(NewRate,
			InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.MinPlayRate,
			InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.MaxPlayRate);
		
		if (!bSmoothPlayRate)
		{
			TheAnimPlayRate = NewRate;
		}
		else
		{
			TheAnimPlayRate = UKismetMathLibrary::FInterpTo(
				FMComponent->GetMoveControlParam().AnimPlayRateNow,
				NewRate,
				DeltaTime,
				InMoveControlSettings.AdjustSettings.AnimPlayRateAdjust.AnimPlayRateSmoothSpeed);
		}
	}
}

void FGBWFlowMoveControlParam::UpdateMoveControlParam(float DeltaTime, UGBWFlowMoveComponent* FMComponent)
{
	if (!bIsActive || !FMComponent)
	{
		MoveToDirection = FMComponent->GetMoveToVector();
		return;
	}

	//InertialVelocity
	if (MoveControlSettings.bIsNew && MoveControlSettings.Velocity.UncontrollableVelocity.Inertial.bIsInertialVelocity)
	{
		//BeginInertialVelocity = FMComponent->CurrentActionTargetSceneInfo.BeginSpeed * FMComponent->CurrentActionTargetSceneInfo.BeginVelocityDirection;
		BeginInertialVelocity = FMComponent->CurrentActionTargetSceneInfo.BeginVelocity;
		InertialVelocity = BeginInertialVelocity;
	}

	if(!MoveControlSettings.Velocity.UncontrollableVelocity.Inertial.bIsInertialVelocity)
	{
		BeginInertialVelocity = FVector::ZeroVector;
		//InertialVelocity = FVector::ZeroVector;
		InertialVelocity = FMath::VInterpTo(
					InertialVelocity, FVector::ZeroVector,
					DeltaTime, 12.0f);
	}
	else if (!MoveControlSettings.bIsNew)
	{
		if (MoveControlSettings.Velocity.UncontrollableVelocity.Inertial.VelocityAttenuationType == ByDuration)
		{
			InertialVelocity = FMath::Lerp(
				BeginInertialVelocity, FVector::ZeroVector,
				FMath::Clamp((MoveControlSettings.TimeNow - MoveControlSettings.StartTime) / MoveControlSettings.Velocity.UncontrollableVelocity.Inertial.InertialVelocityAttenuationDuration, 0.0f, 1.0f));
		}
		else
		{
			InertialVelocity = FMath::VInterpTo(
					InertialVelocity, FVector::ZeroVector,
					DeltaTime, MoveControlSettings.Velocity.UncontrollableVelocity.Inertial.InertialVelocityAttenuationSpeed);
		}
	}

	//AnimComponent Check
	if (!AnimComponent)
	{AnimComponent = UGBWAnimFuncLib::GetGBWAnimComponent(FMComponent->InputState.OwnerCharacter);	}

	//ActionDuration && AnimPlayRate
	GetActionDurationAndAnimPlayRate(
		DeltaTime, FMComponent, MoveControlSettings, !MoveControlSettings.bIsNew,
		ActionDuration, AnimPlayRateNow);
	
	//TargetPoint
	bool bIsGetTargetPoint = false;
	FVector Res_TargetPoint = FVector::ZeroVector;
	float Res_ToTargetSpeed = 0.0f;
	if (MoveControlSettings.bTryGetMoveControlParamFromInterface && FMComponent->FlowMoveBrain)
	{
		FMComponent->FlowMoveBrain->FM_GetTarget(
		FMComponent->GetMoveControlParam(),DeltaTime,
			TargetPoint,
			ToTargetPointSpeed,
			bIsGetTargetPoint,
			Res_TargetPoint,
			Res_ToTargetSpeed);
	}
	if (bIsGetTargetPoint)
	{
		TargetPoint = Res_TargetPoint;
		ToTargetPointSpeed = Res_ToTargetSpeed;	
	}
	else
	{
		if (MoveControlSettings.bIsNew)
		{
			TargetPoint = FVector::ZeroVector;
			ToTargetPointRate = 0.0f;
			DeltaToTargetPointRate = 0.0f;
			ToTargetPointSpeed = 0.0f;
		}
		if (FMComponent->CurrentActionTargetSceneInfo.bIsValid
			&& FMComponent->CurrentActionTargetSceneInfo.bHasTargetPoint)
		{
			TargetPoint =
				FMComponent->CurrentActionTargetSceneInfo.TargetPoint
				+ MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.TargetPointOffset;
			const float LastToTargetRate = ToTargetPointRate;
			GetToTargetRateCurve(FMComponent->InputState.OwnerCharacter, DeltaTime,ToTargetPointRate,DeltaToTargetPointRate);
			DeltaToTargetPointRate = ToTargetPointRate - LastToTargetRate;
			const FVector TargetRealDirection = TargetPoint - FMComponent->CurrentActionTargetSceneInfo.BeginLocation;
			ToTargetPointSpeed = TargetRealDirection.Length() * DeltaToTargetPointRate / DeltaTime;
		}
		else
		{
			TargetPoint = FVector::ZeroVector;
			ToTargetPointRate = 0.0f;
			DeltaToTargetPointRate = 0.0f;
			ToTargetPointSpeed = 0.0f;
		}
	}

	//MoveToDirection
	bool bIsGetMoveToDirection = false;
	TEnumAsByte<EFlowMoveDirectionType> Res_MovementDirectionType;
	FVector Res_WorldAbsoluteMoveDirection;
	if (MoveControlSettings.bTryGetMoveControlParamFromInterface && FMComponent->FlowMoveBrain)
	{
		FMComponent->FlowMoveBrain->FM_GetMoveDirection(
		FMComponent->GetMoveControlParam(),DeltaTime,
			MoveToDirection,
			bIsGetMoveToDirection,
			Res_MovementDirectionType,
			Res_WorldAbsoluteMoveDirection);
	}
	if (bIsGetMoveToDirection)
	{
		if (Res_MovementDirectionType == EFlowMoveDirectionType::WorldAbsolute)
		{MoveToDirection = Res_WorldAbsoluteMoveDirection;}
		else
		{MoveToDirection = FMComponent->GetDirectionVector(Res_MovementDirectionType,FVector::ZeroVector);}
	}
	else
	{
		FVector Velocity = FMComponent->InputState.OwnerCharacter->GetVelocity();
		Velocity.Z = 0.0f;
		
		const FRotator CurrentRt = MoveToDirection.Rotation();
		const FRotator TargetRt = FMComponent->GetMoveToVector().Rotation();

		const FRotator ResRt = UKismetMathLibrary::RInterpTo(
				CurrentRt,TargetRt,DeltaTime,
				Velocity.Length() <= 30.0f? 0.0f : MoveControlSettings.ControlVector.MoveToDirectionSmoothSpeed);
		
		FVector Res_MoveToDirection  = ResRt.Vector();
		
		Res_MoveToDirection.Normalize();

		const float MoveDirectionAngle = UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(
			Res_MoveToDirection,
			FMComponent->GetDirectionVector(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.ConstrainTo,FVector::ZeroVector));
		if (MoveDirectionAngle > FMath::Abs(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.MaxYaw))
		{
			Res_MoveToDirection = UKismetMathLibrary::RotateAngleAxis(
				Res_MoveToDirection,
				MoveDirectionAngle - FMath::Abs(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.MaxYaw),
				FVector::UpVector);
		}
		else if (MoveDirectionAngle < -FMath::Abs(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.MaxYaw))
		{
			Res_MoveToDirection = UKismetMathLibrary::RotateAngleAxis(
				Res_MoveToDirection,
				MoveDirectionAngle + FMath::Abs(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.MaxYaw),
				FVector::UpVector);
		}

		const float MoveDirectionAnglePitch = UGBWPowerfulToolsFuncLib::GetAngleInVerticalPlane(
			Res_MoveToDirection,
			FMComponent->GetDirectionVector(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.ConstrainTo,FVector::ZeroVector));
		if (MoveDirectionAnglePitch > FMath::Abs(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.MaxPitchDown))
		{
			Res_MoveToDirection = FRotator(
				-FMath::Abs(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.MaxPitchDown),
				Res_MoveToDirection.Rotation().Yaw,
				Res_MoveToDirection.Rotation().Roll).Vector();
			Res_MoveToDirection.Normalize();
		}
		else if (MoveDirectionAnglePitch < -FMath::Abs(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.MaxPitchUp))
		{
			Res_MoveToDirection = FRotator(
				FMath::Abs(MoveControlSettings.ControlVector.MoveToDirectionAngleConstrain.MaxPitchUp),
				Res_MoveToDirection.Rotation().Yaw,
				Res_MoveToDirection.Rotation().Roll).Vector();
			Res_MoveToDirection.Normalize();
		}
		
		MoveToDirection = Res_MoveToDirection;
	}

	//Velocity Direction
	VelocityDirection = FMComponent->GetDirectionVectorFrom(MoveControlSettings.Velocity.ControllableVelocity.MoveDirection);
	
	//Gravity
	bool bIsGetGravity = false;
	float Res_Gravity = 0.0f;
	bool Res_IsCalculateFallingDistance = false;
	if (MoveControlSettings.bTryGetMoveControlParamFromInterface && FMComponent->FlowMoveBrain)
	{
		FMComponent->FlowMoveBrain->FM_GetGravity(
		FMComponent->GetMoveControlParam(),DeltaTime,
			Gravity,
			MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.bCalculateFallingDistance,
			bIsGetGravity,
			Res_Gravity,
			Res_IsCalculateFallingDistance);
	}
	if (bIsGetGravity)
	{
		MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.bCalculateFallingDistance = Res_IsCalculateFallingDistance;
		DeltaGravity = Res_Gravity - Gravity;
		Gravity = Res_Gravity;
	}
	else
	{
		if (MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Type == EGBWFlowMoveControlSettingsType::Curve)
		{
			GetGravityCurve(FMComponent->InputState.OwnerCharacter, DeltaTime, Gravity, DeltaGravity);
		}
		else if (MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Type == EGBWFlowMoveControlSettingsType::Simple)
		{
			const float GravityNow = Gravity;
			Gravity = UKismetMathLibrary::FInterpTo(
				Gravity,
				MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Target,
				DeltaTime,
				MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Target>=Gravity?
					MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.SmoothSpeed_LowToHigh
					: MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.SmoothSpeed_HighToLow);
			DeltaGravity = Gravity - GravityNow;
		}
		else if (MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Type == EGBWFlowMoveControlSettingsType::Range)
		{
			if (MoveControlSettings.bIsNew)
			{
				Gravity = MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.BeginValue;
			}
			else
			{
				const float GravityNow = Gravity;
				Gravity = UKismetMathLibrary::FInterpTo(
					Gravity,
					MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.TargetValue,
					DeltaTime,
					MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.SmoothSpeed);
				DeltaGravity = Gravity - GravityNow;	
			}
		}
		else if (MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Type == EGBWFlowMoveControlSettingsType::GetFromPSFloatValue)
		{
			float TargetGravity = MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.DefaultValue;
				
			bool bIsGet = false;
			float Result = 0.0f;
			float Duration = 0.0f;
			UGBWPSFuncLib::GetPSFValue(FMComponent->InputState.OwnerCharacter,
				MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.ValueKey,
				bIsGet, Result, Duration);
			if (bIsGet) {TargetGravity = Result;}
				
			const float GravityNow = Gravity;
			Gravity = UKismetMathLibrary::FInterpTo(
				Gravity,
				TargetGravity,
				DeltaTime,
				MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.Target>=Gravity?
					MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.SmoothSpeed_LowToHigh
					: MoveControlSettings.Velocity.UncontrollableVelocity.Gravity.Gravity.SmoothSpeed_HighToLow);
			DeltaGravity = Gravity - GravityNow;
		}
	}
	
	//MoveSpeed
	bool bIsGetMoveSpeed = false;
	float Res_MoveSpeed = 0.0f;
	if (MoveControlSettings.bTryGetMoveControlParamFromInterface && FMComponent->FlowMoveBrain)
	{
		FMComponent->FlowMoveBrain->FM_GetMoveSpeed(
		FMComponent->GetMoveControlParam(),DeltaTime,
			MoveSpeed,
			bIsGetMoveSpeed,
			Res_MoveSpeed);
	}
	if (bIsGetMoveSpeed)
	{
		DeltaMoveSpeed = Res_MoveSpeed - MoveSpeed;
		MoveSpeed = Res_MoveSpeed;
	}
	else
	{
		if(MoveControlSettings.Velocity.ControllableVelocity.bIsSpeedAdjust)
		{
			MoveSpeedAdjustRate = MoveControlSettings.Velocity.ControllableVelocity.SpeedAdjustSettings.GetSpeedAdjustRate(MoveSpeedAdjustRate,DeltaTime,FMComponent);
		}
		else
		{
			MoveSpeedAdjustRate = 1.0f;
		}
		const float MoveSpeedNow = MoveSpeed;

		if (MoveControlSettings.Velocity.ControllableVelocity.MoveSpeedType == EGBWFlowMoveMoveSpeedType::DirectlySet
			|| MoveControlSettings.Velocity.ControllableVelocity.MoveSpeedType == EGBWFlowMoveMoveSpeedType::SetBySpeedMode)
		{
			FGBWFlowMoveFloatValueSetting MoveSpeedSetting = MoveControlSettings.Velocity.ControllableVelocity.MoveSpeed;
			if (MoveControlSettings.Velocity.ControllableVelocity.MoveSpeedType == EGBWFlowMoveMoveSpeedType::SetBySpeedMode)
			{
				if (MoveControlSettings.Velocity.ControllableVelocity.MoveSpeedSet.Find(FMComponent->GetFlowMoveSpeedMode()))
				{
					MoveSpeedSetting = MoveControlSettings.Velocity.ControllableVelocity.MoveSpeedSet.FindRef(FMComponent->GetFlowMoveSpeedMode());
				}
			}
			
			if (MoveSpeedSetting.Type == EGBWFlowMoveControlSettingsType::Curve)
			{
				GetMoveSpeedCurve(FMComponent->InputState.OwnerCharacter, MoveSpeedSetting.Curve, DeltaTime, MoveSpeed, DeltaMoveSpeed);
				MoveSpeed = MoveSpeedAdjustRate * MoveSpeed;
			}
			else if (MoveSpeedSetting.Type == EGBWFlowMoveControlSettingsType::Simple)
			{
				const float TargetMoveSpeed = MoveSpeedAdjustRate * MoveSpeedSetting.Target;
				MoveSpeed = UKismetMathLibrary::FInterpTo(
					MoveSpeed,
					TargetMoveSpeed,
					DeltaTime,
					TargetMoveSpeed>=MoveSpeed?
					MoveSpeedSetting.SmoothSpeed_LowToHigh
					: MoveSpeedSetting.SmoothSpeed_HighToLow);
			}
			else if (MoveSpeedSetting.Type == EGBWFlowMoveControlSettingsType::Range)
			{
				if (MoveControlSettings.bIsNew)
				{
					MoveSpeed = MoveSpeedSetting.BeginValue;
				}
				else
				{
					const float TargetMoveSpeed = MoveSpeedAdjustRate * MoveSpeedSetting.TargetValue;
					MoveSpeed = UKismetMathLibrary::FInterpTo(
						MoveSpeed,
						TargetMoveSpeed,
						DeltaTime,
						MoveSpeedSetting.SmoothSpeed);
				}
			}
			else if (MoveSpeedSetting.Type == EGBWFlowMoveControlSettingsType::GetFromPSFloatValue)
			{
				float TargetMoveSpeed = MoveSpeedAdjustRate * MoveSpeedSetting.DefaultValue;
				
				bool bIsGet = false;
				float Result = 0.0f;
				float Duration = 0.0f;
				UGBWPSFuncLib::GetPSFValue(FMComponent->InputState.OwnerCharacter,
					MoveSpeedSetting.ValueKey,
					bIsGet, Result, Duration);
				if (bIsGet) {TargetMoveSpeed = Result;}
				
				MoveSpeed = UKismetMathLibrary::FInterpTo(
					MoveSpeed,
					TargetMoveSpeed,
					DeltaTime,
					TargetMoveSpeed>=MoveSpeed?
					MoveSpeedSetting.SmoothSpeed_LowToHigh
					: MoveSpeedSetting.SmoothSpeed_HighToLow);
			}
		}
		else if (MoveControlSettings.bIsNew)
		{
			if (MoveControlSettings.Velocity.ControllableVelocity.MoveSpeedType == EGBWFlowMoveMoveSpeedType::InheritFromPreActionSpeed)
			{
				MoveSpeed = FMComponent->CurrentActionTargetSceneInfo.BeginSpeed;
			}
			else if (MoveControlSettings.Velocity.ControllableVelocity.MoveSpeedType == EGBWFlowMoveMoveSpeedType::InheritFromPreActionVelocity)
			{
				MoveSpeed = FMComponent->CurrentActionTargetSceneInfo.BeginVelocity.Length();
			}
			else if (MoveControlSettings.Velocity.ControllableVelocity.MoveSpeedType == EGBWFlowMoveMoveSpeedType::InheritFromPreActionHorizontalVelocity)
			{
				FVector BeginVelocity = FMComponent->CurrentActionTargetSceneInfo.BeginVelocity;
				BeginVelocity.Z = 0.0f;
				MoveSpeed = BeginVelocity.Length();
			}
		}
		
		DeltaMoveSpeed = MoveSpeed - MoveSpeedNow;
	}
	
	//PathOffset
	bool bIsGetPathOffset = false;
	FVector Res_PathOffset;
	TEnumAsByte<EFlowMoveDirectionType> Res_PathOffsetDirectionType;
	FVector Res_WorldAbsolutePathOffsetDirection;
	if (MoveControlSettings.bTryGetMoveControlParamFromInterface && FMComponent->FlowMoveBrain)
	{
		FMComponent->FlowMoveBrain->FM_GetPathOffset(
		FMComponent->GetMoveControlParam(),DeltaTime,
			PathOffsetDirection,
			PathOffset,
			bIsGetPathOffset,
			Res_PathOffset,
			Res_PathOffsetDirectionType,
			Res_WorldAbsolutePathOffsetDirection);
	}
	if (bIsGetPathOffset)
	{
		DeltaPathOffset = Res_PathOffset - PathOffset;
		PathOffset = Res_PathOffset;

		if (Res_PathOffsetDirectionType == EFlowMoveDirectionType::WorldAbsolute)
		{
			PathOffsetDirection = Res_WorldAbsolutePathOffsetDirection;
		}
		else
		{
			PathOffsetDirection = FMComponent->GetDirectionVector(Res_PathOffsetDirectionType,FVector::ZeroVector);
			PathOffsetDirection.Z = 0.0f;
			PathOffsetDirection.Normalize();
		}
	}
	else
	{
		if (MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.PathOffsetCurve.Type == EGBWFlowMoveCurveType::RootMotion)
		{
			PathOffset = FMComponent->ConvertLocalRootMotionToActor(FTransform(PathOffset)).GetTranslation();
			DeltaPathOffset = FMComponent->ConvertLocalRootMotionToActor(FTransform(DeltaPathOffset)).GetTranslation();
		}
		else
		{
			GetPathOffsetCurve(DeltaTime, PathOffset, DeltaPathOffset);
		}
		const float PathOffsetScale = MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.Scale.GetScale(FMComponent);
		PathOffset = PathOffset * PathOffsetScale;
		DeltaPathOffset = DeltaPathOffset * PathOffsetScale;
		
		PathOffsetDirection = FMComponent->GetDirectionVector(MoveControlSettings.Velocity.UncontrollableVelocity.PathOffset.ForwardDirectionOfRCS,FVector::ZeroVector);
		PathOffsetDirection.Z = 0.0f;
		PathOffsetDirection.Normalize();
	}

	//RotationOffset
	bool bIsGetRotationOffset = false;
	float Res_RotationOffset;
	if (MoveControlSettings.bTryGetMoveControlParamFromInterface && FMComponent->FlowMoveBrain)
	{
		FMComponent->FlowMoveBrain->FM_GetRotationYawOffset(
		FMComponent->GetMoveControlParam(),DeltaTime,
			RotationYawOffset,
			bIsGetRotationOffset,
			Res_RotationOffset);
	}
	if (bIsGetRotationOffset)
	{
		DeltaRotationYawOffset = Res_RotationOffset - RotationYawOffset;
		RotationYawOffset = Res_RotationOffset;
	}
	else
	{
		if (MoveControlSettings.RotationSetting.bRotationYawOffset)
		{
			GetRotationOffsetCurve(FMComponent->InputState.OwnerCharacter, DeltaTime,RotationYawOffset,DeltaRotationYawOffset);
		}
		else
		{
			DeltaRotationYawOffset = 0.0f;
			RotationYawOffset = 0.0f;
		}
	}

	//State Update
	MoveControlSettings.bIsNew = false;
}

FVector FGBWFlowMoveVelocityTrend::GetVelocityTrend(UGBWFlowMoveComponent* FMComp) const
{
	if (!bIsBlend || Duration<=0)
	{
		return Target_VelocityTrend;
	}

	const float InAlpha = Timer/Duration;
	const float OutAlpha = UGBWPowerfulToolsFuncLib::GetAlphaByBlendOption(BlendOption,InAlpha);
	
	FVector Direction = FMComp? FMComp->GetDirectionVector(DirectionType,FVector::ZeroVector) : FVector(1.0f,0.0f,0.0f);
	if (OnlyHorizontalDirection)
	{
		Direction.Z = 0.0f;
		Direction.Normalize();
	}
	return Direction.Rotation().RotateVector((Target_VelocityTrend - Begin_VelocityTrend) * OutAlpha + Begin_VelocityTrend);
}

bool FGBWFlowMoveVelocityTrend::ValidUpdate(float DeltaTime)
{
	Timer+=DeltaTime;
	return Timer < Duration;
}
