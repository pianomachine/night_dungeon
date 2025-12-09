// Copyright GanBowen 2022-2024. All Rights Reserved.

#include "GBWFlowMoveComponent.h"

#include "GBWAnimComponent.h"
#include "GBWAnimFuncLib.h"
#include "GBWFlowMoveFuncLib.h"
#include "GBWPowerfulToolsFuncLib.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimInstance.h"
#include "ANS/GBWFlowMoveANS_MoveControl.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "RMS/GBWRMSBPFuncLib.h"

// Sets default values for this component's properties
UGBWFlowMoveComponent::UGBWFlowMoveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UGBWFlowMoveComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;
	Parameters.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DefaultViewMode, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InputState, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CurrentActionTargetSceneInfo, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, FlowMoveBrainClass, Parameters)
}

void UGBWFlowMoveComponent::BeginPlay()
{
	Super::BeginPlay();
	InputState.OwnerCharacter = Cast<ACharacter>(GetOwner());
	TaskState.Timer = 0.0f;
	InputState.FlowMoveComponent = this;

	if (FlowMoveBrainValid())
	{
		SetFlowMoveViewMode(FlowMoveBrain->DefaultViewMode,true);
	}

	if (IsInServer() && !UGBWAnimFuncLib::GetGBWAnimComponent(InputState.OwnerCharacter))
	{
		if (UGBWAnimComponent* Ac = Cast<UGBWAnimComponent>(InputState.OwnerCharacter->AddComponentByClass(
			UGBWAnimComponent::StaticClass(),
			false,FTransform(),false)))
		{
			Ac->SetIsReplicated(true);
		}
	}
}

void UGBWFlowMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float RealDeltaTime = DeltaTime;
	
	if(FrameNumber == GFrameCounter){return;}

	FlowMoveTick(RealDeltaTime);
	
	TaskState.FrameDeltaTime = RealDeltaTime;
	FrameNumber = GFrameCounter;
}

bool UGBWFlowMoveComponent::GetRootMotionParamDirect(
	FTransform& Result,
	float SimulationTime,
	float MovementTickTime,
	const ACharacter& Character,
	const UCharacterMovementComponent& MoveComponent)
{
	Result = FTransform(MoveControlResult.DeltaRotator,MoveControlResult.Velocity);
	
	if (MoveControlResult.bIsControlled)
	{
		InputState.OwnerCharacter->GetCharacterMovement()->MaxAcceleration = 0.0f;
		return true;
	}
	else
	{
		InputState.OwnerCharacter->GetCharacterMovement()->MaxAcceleration = MovementCompMaxAccelerationCache;
		return false;
	}
}

void UGBWFlowMoveComponent::MagicPlayerTick(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float StartTime, const float EndTime,const float Progress, const FVector BlendSpacePosition,
	const FTransform RootMotion,
	const bool bIsStartTick, const FName AnimSlot)
{
	if (OwnerActor)// && OwnerActor->HasAuthority())
	{
		//Get Anim NotifyState
		UGBWFlowMoveANS_MoveControl* Ansmc = nullptr;
		if (Animation
			&& Animation->GetClass()->IsChildOf(UAnimSequenceBase::StaticClass()))
		{
			if (UAnimSequenceBase* A = Cast<UAnimSequenceBase>(Animation))
			{
				for (const auto Notify : A->Notifies)
				{
					if (Notify.NotifyStateClass
						&& Notify.NotifyStateClass->GetClass()->IsChildOf(UGBWFlowMoveANS_MoveControl::StaticClass()))
					{
						Ansmc = Cast<UGBWFlowMoveANS_MoveControl>(Notify.NotifyStateClass);
						break;
					}
				}
			}
		}
		else if (Animation
			&& Animation->GetClass()->IsChildOf(UBlendSpace::StaticClass()))
		{
			if (const UBlendSpace* B = Cast<UBlendSpace>(Animation))
			{
				float MinDis = -1.0f;
				for (const auto BSSample : B->GetBlendSamples())
				{
					for (const auto Notify : BSSample.Animation->Notifies)
					{
						if (Notify.NotifyStateClass
						&& Notify.NotifyStateClass->GetClass()->IsChildOf(UGBWFlowMoveANS_MoveControl::StaticClass()))
						{
							if (const float Dis = (BlendSpacePosition - BSSample.SampleValue).Length();
								MinDis < 0.0f || MinDis >= Dis)
							{
								MinDis = Dis;
								Ansmc = Cast<UGBWFlowMoveANS_MoveControl>(Notify.NotifyStateClass);
							}
						}
					}
				}
			}
		}

		if (Ansmc)
		{
			bool bNoChange = false;
			if (FGBWFlowMoveControlSettingsPro MoveControlSettings = FGBWFlowMoveControlSettingsPro();
				Ansmc->GetMoveControlSettings(
					InputState.OwnerCharacter,
					FVector2D(BlendSpacePosition.X, BlendSpacePosition.Y),
					DeltaTime,
					bIsStartTick? 0.0f : ExecuteTime,
					EndTime - StartTime,
					bNoChange,
					MoveControlSettings))
			{
				TaskState.MoveControlParam.bIsActive = true;
				TaskState.MoveControlParam.LastActiveFrame = GFrameCounter;

				if (!bNoChange)
				{
					TaskState.MoveControlParam.MoveControlSettings = MoveControlSettings;
				}
		
				TaskState.MoveControlParam.MoveControlSettings.StartTime = StartTime;
				TaskState.MoveControlParam.MoveControlSettings.EndTime = EndTime;
				TaskState.MoveControlParam.MoveControlSettings.TimeNow = ExecuteTime;
				TaskState.MoveControlParam.MoveControlSettings.Progress = Progress;
				TaskState.MoveControlParam.BlendSpacePosition = BlendSpacePosition;
				TaskState.MoveControlParam.RootMotionNow = RootMotion;
				TaskState.MoveControlParam.MoveControlSettings.AnimSlot = AnimSlot;
				TaskState.MoveControlParam.MoveControlSettings.FromAsset = Animation;
				TaskState.MoveControlParam.MoveControlSettings.bIsNew = bIsStartTick;
			}
			else
			{
				TaskState.MoveControlParam.bIsActive = false;
			}
		}
		else
		{
			TaskState.MoveControlParam.bIsActive = false;
		}
	}
}

void UGBWFlowMoveComponent::OnEvent(bool bNetEvent, const FFlowMoveEvent& FlowMoveEvent)
{
	if (bNetEvent && IsInServer())
	{
		OnEvent_Server(FlowMoveEvent);
	}
	else
	{
		OnEvent_Imp(FlowMoveEvent);	
	}
}
void UGBWFlowMoveComponent::OnEvent_Server_Implementation(const FFlowMoveEvent& FlowMoveEvent)
{
	OnEvent_Multicast(FlowMoveEvent);
}
void UGBWFlowMoveComponent::OnEvent_Multicast_Implementation(const FFlowMoveEvent& FlowMoveEvent)
{
	OnEvent_Imp(FlowMoveEvent);
}
void UGBWFlowMoveComponent::OnEvent_Imp(const FFlowMoveEvent& FlowMoveEvent)
{
	OnFMEvent.Broadcast(FlowMoveEvent);
	if (FlowMoveBrainValid())
	{
		FlowMoveBrain->OnFMEvent(TaskState,FlowMoveEvent);
	}
}

void UGBWFlowMoveComponent::SetBrian(const TSubclassOf<UGBWFlowMoveBrain_Base> NewBrain)
{
	if (NewBrain)
	{
		SetBrian_Server(NewBrain);
	}
}

void UGBWFlowMoveComponent::SetBrian_Server_Implementation(const TSubclassOf<UGBWFlowMoveBrain_Base> NewBrain)
{
	FlowMoveBrainClass = NewBrain;
}

void UGBWFlowMoveComponent::SetMoveVector(const FVector NewMoveVector, const float ZeroFaultToleranceDuration)
{
	if (IsLocalOwn())
	{
		const float TimeNow = InputState.OwnerCharacter->GetWorld()->GetTimeSeconds();
		const float DeltaTime = TimeNow - InputState.LastMoveVectorTime;
		bool bCanSet = true;
		
		FVector_NetQuantizeNormal Result = FVector_NetQuantizeNormal(NewMoveVector);
		FVector_NetQuantizeNormal ResultDirection = Result.IsNearlyZero()? InputState.MoveVector : Result;
		ResultDirection.Normalize();
		Result = ResultDirection * UKismetMathLibrary::FInterpTo(
			InputState.MoveVector.Length(),
			Result.Length(),
			DeltaTime,
			GetMoveControlParam().MoveControlSettings.ControlVector.InputVectorLengthSmoothSpeed
		);
		
		if (Result == InputState.MoveVector)
		{
			bCanSet = false;
		}
		if (DeltaTime < 1.0f / MaxMoveVectorUpdateCountPerSecond)
		{
			bCanSet = false;
		}

		if (NewMoveVector.IsNearlyZero()
			&& TimeNow - InputState.LastZeroMoveVectorTime >= ZeroFaultToleranceDuration + TaskState.FrameDeltaTime*2.0f)
		{
			InputState.LastZeroMoveVectorTime = TimeNow;
		}
		if (NewMoveVector.IsNearlyZero()
			&& TimeNow - InputState.LastZeroMoveVectorTime < ZeroFaultToleranceDuration)
		{
			bCanSet = false;
		}

		if (bCanSet)
		{
			InputState.SetMoveVector(Result);
			SetMoveVector_Server(Result);
		}
	}
}
void UGBWFlowMoveComponent::SetMoveVector_Server_Implementation(const FVector_NetQuantizeNormal NewMoveVector)
{
	InputState.SetMoveVector(NewMoveVector);
}

FVector UGBWFlowMoveComponent::GetMoveVector()
{
	FVector Result = InputState.MoveVector;
	
	if (Result.Length() <= GetMoveControlParam().MoveControlSettings.ControlVector.InputVectorLengthDeadZone)
	{
		if (GetMoveControlParam().MoveControlSettings.ControlVector.bUseDefaultInput)
		{
			Result = GetDirectionVectorFrom(GetMoveControlParam().MoveControlSettings.ControlVector.DefaultInputDirection);

			if (GetMoveControlParam().MoveControlSettings.ControlVector.DefaultInputDirection.Type != EFlowMoveDirectionType::WorldAbsolute)
			{
				FTransform T = InputState.OwnerCharacter->GetActorTransform();
				FRotator CRotator = InputState.OwnerCharacter->GetControlRotation();
				CRotator.Roll = 0.0f;
				CRotator.Pitch = 0.0f;
				T.SetRotation(CRotator.Quaternion());

				Result = UKismetMathLibrary::InverseTransformDirection(T, Result);
				Result.Normalize();
			}
		}
		else
		{
			Result = FVector::ZeroVector;
		}
	}
	
	return Result;
}

void UGBWFlowMoveComponent::SetControlVector(const FVector NewControlVector)
{
	if (IsLocalOwn())
	{
		const float DeltaTime = InputState.OwnerCharacter->GetWorld()->GetTimeSeconds() - InputState.LastControlVectorTime;
		bool bCanSet = true;
		if (DeltaTime < 1.0f / MaxMoveVectorUpdateCountPerSecond)
		{
			bCanSet = false;
		}
		if (InputState.ControlVector == FVector_NetQuantizeNormal(NewControlVector))
		{
			bCanSet = false;
		}
		
		if (bCanSet)
		{
			InputState.SetControlVector(FVector_NetQuantizeNormal(NewControlVector));
			SetControlVector_Server(FVector_NetQuantizeNormal(NewControlVector));
		}
	}
}
void UGBWFlowMoveComponent::SetControlVector_Server_Implementation(const FVector_NetQuantizeNormal ControlVector)
{
	InputState.SetControlVector(ControlVector);
}

FVector UGBWFlowMoveComponent::GetControlVector()
{
	return InputState.ControlVector;
}

void UGBWFlowMoveComponent::GetMoveParamForBlendSpace(
	UBlendSpace* ForBS,
	UBlendSpace*& BS,
	float& Horizontal,
	float& Vertical,
	float& PlayRate,
	float& StartPosition,
	float& EndPosition,
	const EFlowMoveDirectionType DirectionType,
	const EFMBSAxisType BS_HorizontalAxisFrom,
	const EFMBSAxisType BS_VerticalAxisFrom,
	const float RefMaxSpeed,
	const bool bUseStep,
	const float Step)
{
	BS = ForBS;
	
	PlayRate = 1.0f;
	Horizontal = 0.0f;
	Vertical = 0.0f;
	StartPosition = 0.0f;
	EndPosition = 0.0f;
	
	if (!ForBS || !InputState.OwnerCharacter)
	{
		return;
	}

	//Speed
	const float Speed = InputState.OwnerCharacter->GetVelocity().Length();

	//HorizontalDirection And VerticalDirection
	const FVector DirectionVector = GetDirectionVector(DirectionType,FVector::ZeroVector);

	const float DPitch = DirectionVector.Rotation().Pitch;
	
	float DAngle_H = UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(DirectionVector,InputState.OwnerCharacter->GetActorForwardVector());
	if (bUseStep && Step>0.0f && Step <= 360.0f)
	{
		DAngle_H = DAngle_H + 180.0f;
		const float StepValue = (360.0f / Step);
		DAngle_H = static_cast<int>(DAngle_H / StepValue + 0.5) * StepValue;
		DAngle_H = DAngle_H - 180.0f;
	}
	
	const FBlendParameter BPX = ForBS->GetBlendParameter(0);
	const FBlendParameter BPY = ForBS->GetBlendParameter(1);

	if (BS_HorizontalAxisFrom == EFMBSAxisType::Speed)
	{
		Horizontal = Speed/(RefMaxSpeed>0? RefMaxSpeed : 900.0f) / GetActorScale() * BPX.GetRange() + BPX.Min;
	}
	else if (BS_HorizontalAxisFrom == EFMBSAxisType::HorizontalDirection)
	{
		Horizontal = (-DAngle_H+180.0f)/360.0f * BPX.GetRange() + BPX.Min;
	}
	else if (BS_HorizontalAxisFrom == EFMBSAxisType::PitchDirection)
	{
		Horizontal = (DPitch+90.0f)/180.0f * BPX.GetRange() + BPX.Min;
	}
	
	if (BS_VerticalAxisFrom == EFMBSAxisType::Speed)
	{
		Vertical = Speed/(RefMaxSpeed>0? RefMaxSpeed : 900.0f) / GetActorScale() * BPY.GetRange() + BPY.Min;
	}
	else if (BS_VerticalAxisFrom == EFMBSAxisType::HorizontalDirection)
	{
		Vertical = (-DAngle_H+180.0f)/360.0f * BPY.GetRange() + BPY.Min;
	}
	else if (BS_VerticalAxisFrom == EFMBSAxisType::PitchDirection)
	{
		Vertical = (DPitch+90.0f)/180.0f * BPY.GetRange() + BPY.Min;
	}
	
	UAnimSequence* AnimSequence;
	GetMoveParam(ForBS, AnimSequence,PlayRate, StartPosition, EndPosition);
}

void UGBWFlowMoveComponent::GetMoveParam(UAnimationAsset* ForAnimAsset, UAnimSequence*& AnimSequence, float& PlayRate, float& StartPosition, float& EndPosition) const
{
	AnimSequence = Cast<UAnimSequence>(ForAnimAsset);
	
	PlayRate = 1.0f;
	StartPosition = 0.0f;
	EndPosition = 0.0f;
	
	if (!TaskState.MoveControlParam.AnimComponent)
	{
		return;
	}

	if (TaskState.MoveControlParam.MoveControlSettings.FromAsset == ForAnimAsset)
	{
		PlayRate = TaskState.MoveControlParam.AnimPlayRateNow;
		StartPosition = TaskState.MoveControlParam.MoveControlSettings.StartTime;
		EndPosition = TaskState.MoveControlParam.MoveControlSettings.EndTime;
	}
}

void UGBWFlowMoveComponent::GetMoveParamForAnimAsset(UAnimationAsset* ForAnimAsset, UAnimationAsset*& AnimAsset,
	float& Horizontal,
	float& Vertical,
	float& PlayRate, float& StartPosition,
	float& EndPosition,
	const EFlowMoveDirectionType DirectionType,
	const EFMBSAxisType BS_HorizontalAxisFrom,
	const EFMBSAxisType BS_VerticalAxisFrom,
	float RefMaxSpeed, const bool bUseStep, const float Step)
{
	AnimAsset = ForAnimAsset;
	if (!AnimAsset)
	{
		return;
	}

	if (AnimAsset->GetClass()->IsChildOf(UBlendSpace::StaticClass()))
	{
		UBlendSpace* ResBS;
		GetMoveParamForBlendSpace(
			Cast<UBlendSpace>(ForAnimAsset),
			ResBS,
			Horizontal, Vertical, PlayRate, StartPosition,EndPosition,
			DirectionType,BS_HorizontalAxisFrom,BS_VerticalAxisFrom,
			RefMaxSpeed, bUseStep, Step);
	}
	else if (AnimAsset->GetClass()->IsChildOf(UAnimSequence::StaticClass()))
	{
		UAnimSequence* AnimSequence;
		GetMoveParam(ForAnimAsset, AnimSequence, PlayRate, StartPosition,EndPosition);
	}
}

void UGBWFlowMoveComponent::GetMagicPlayerParam(UAnimationAsset* ForAnimAsset, FMagicPlayerParam& MagicPlayerParam,
	const bool InReplayNow,
	const bool bAutoReplayCodeTime,
	const float InReplayCodeTime,
	const bool InLoopAnimation,
	const float InBlendTime,
	const float InLoopBlendTime,
	const EFlowMoveDirectionType DirectionType,
	const EFMBSAxisType BS_HorizontalAxisFrom,
	const EFMBSAxisType BS_VerticalAxisFrom,
	float RefMaxSpeed, const bool bUseStep, const float Step)
{
	UAnimationAsset* AnimAsset;
	float H;
	float V;
	float PlayRate;
	float StartPosition;
	float EndPosition;
	
	GetMoveParamForAnimAsset(ForAnimAsset, AnimAsset,
	H, V, PlayRate, StartPosition, EndPosition,
	DirectionType,BS_HorizontalAxisFrom,BS_VerticalAxisFrom,
	RefMaxSpeed, bUseStep, Step);

	MagicPlayerParam = FMagicPlayerParam();
	MagicPlayerParam.AnimAsset = AnimAsset;
	MagicPlayerParam.bReplayNow = InReplayNow;

	if (bAutoReplayCodeTime)
	{
		MagicPlayerParam.ReplayCodeTime = EndPosition - StartPosition - FMath::Max<float>(InBlendTime/4.0f,0.05f);
	}
	else
	{
		MagicPlayerParam.ReplayCodeTime = InReplayCodeTime;
	}
	
	MagicPlayerParam.Horizontal = H;
	MagicPlayerParam.Vertical = V;
	MagicPlayerParam.PlayRate = PlayRate;
	MagicPlayerParam.StartPosition = StartPosition;
	MagicPlayerParam.EndPosition = EndPosition;
	MagicPlayerParam.bLoopAnimation = InLoopAnimation;
	MagicPlayerParam.BlendTime = InBlendTime;
	MagicPlayerParam.LoopBlendTime = InLoopBlendTime;
}

bool UGBWFlowMoveComponent::ActiveReady()
{
	if (TaskState.bIsActive)
	{
		return true;
	}
	
	InputState.OwnerCharacter = Cast<ACharacter>(GetOwner());
	TaskState.Timer = 0.0f;
	InputState.FlowMoveComponent = this;

	if (InputState.OwnerCharacter)
	{
		FlowMoveBrainValid();
		
		TaskState.bIsActive = true;

		if (!bHasSetCapsuleAndMesh)
		{
			CapsuleHalfHeightCache = InputState.OwnerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
			MeshRelativeTransformCache = InputState.OwnerCharacter->GetMesh()->GetRelativeTransform();
			CapsuleRadiusCache = InputState.OwnerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
		}
		
		bUseControllerRotationYawCache = InputState.OwnerCharacter->bUseControllerRotationYaw;
		MovementCompMaxAccelerationCache = InputState.OwnerCharacter->GetCharacterMovement()->GetMaxAcceleration();
		bIsConstrainToPlaneCache = InputState.OwnerCharacter->GetCharacterMovement()->bConstrainToPlane;
		PlaneConstraintNormalCache = InputState.OwnerCharacter->GetCharacterMovement()->GetPlaneConstraintNormal();
		PlaneConstraintOriginCache = InputState.OwnerCharacter->GetCharacterMovement()->GetPlaneConstraintOrigin();
		
		EnsureRMSActivation();
		
		InputState.OwnerCharacter->bUseControllerRotationYaw = false;
		InputState.OwnerCharacter->GetCharacterMovement()->MaxAcceleration = 0.0f;
		
		UpdateMoveControlParam(TaskState.FrameDeltaTime);

		OnEvent(false,FFlowMoveEvent(EFlowMoveEventType::OnActive));
		return true;
	}
	
	OnEvent(false,FFlowMoveEvent(EFlowMoveEventType::OnActiveFailed));
	return false;
}

bool UGBWFlowMoveComponent::DeactivateReady()
{
	if (bHasSetIgnoreClientMovementErrorChecksAndCorrection
		&& IsInServer()
		&& GFrameCounter - MoveControlResult.LastControlledFrame >= 60)
	{
		SetMovementReplication(true);
	}

	if (bHasSetCapsuleAndMesh && GFrameCounter - MoveControlResult.LastControlledFrame >= 6)
	{
		bHasSetCapsuleAndMesh = false;
		InputState.OwnerCharacter->GetCapsuleComponent()->SetCapsuleRadius(CapsuleRadiusCache);
		InputState.OwnerCharacter->GetCapsuleComponent()->SetCapsuleHalfHeight(CapsuleHalfHeightCache);
		InputState.OwnerCharacter->GetMesh()->SetRelativeTransform(MeshRelativeTransformCache);
	}
	
	if (!TaskState.bIsActive)
	{
		return true;
	}

	TaskState.Timer = 0.0f;
	
	InputState.OwnerCharacter->GetCharacterMovement()->MaxAcceleration = MovementCompMaxAccelerationCache;

	InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintEnabled(bIsConstrainToPlaneCache);
	InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintNormal(PlaneConstraintNormalCache);
	InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintOrigin(PlaneConstraintOriginCache);
	
	InputState.OwnerCharacter->bUseControllerRotationYaw = bUseControllerRotationYawCache;

	TaskState.bIsActive = false;
	OnEvent(false,FFlowMoveEvent(EFlowMoveEventType::OnStop));

	return true;
}

void UGBWFlowMoveComponent::SetMovementConstrain(const FVector& PlaneNormal, const FVector& PlaneOrigin) const
{
	if (InputState.OwnerCharacter->GetMovementComponent()->bConstrainToPlane
		&& InputState.OwnerCharacter->GetMovementComponent()->GetPlaneConstraintNormal() == PlaneNormal
		&& InputState.OwnerCharacter->GetMovementComponent()->GetPlaneConstraintOrigin() == PlaneOrigin)
	{
		return;
	}
	if (InputState.OwnerCharacter && InputState.OwnerCharacter->GetMovementComponent())
	{
		InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintEnabled(true);
		InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintNormal(PlaneNormal);
		InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintOrigin(PlaneOrigin);
	}
}

void UGBWFlowMoveComponent::SetMoveControlInitInfo_Simple(const bool bHasTargetPoint, const FVector& TargetPoint)
{
	SetMoveControlInitInfo_Simple_Server(bHasTargetPoint, FVector_NetQuantize100(TargetPoint));
}
void UGBWFlowMoveComponent::SetMoveControlInitInfo_Simple_Server_Implementation(bool bHasTargetPoint, const FVector_NetQuantize100& TargetPoint)
{
	FVector TargetMoveTo = GetControlVector();
	TargetMoveTo.Z = 0.0f;
	TargetMoveTo.Normalize();
	TargetMoveTo = TargetMoveTo.Rotation().RotateVector(GetMoveVector());
	
	if (bHasTargetPoint)
	{
		CurrentActionTargetSceneInfo = FGBWFlowMoveMoveControlInitInfo(
			InputState.OwnerCharacter,
			TargetPoint,
			GetMoveControlParam().MoveToDirection,
			TargetMoveTo,
			GetMoveControlParam().VelocityDirection,
			GetMoveControlParam().MoveSpeed);
	}
	else
	{
		CurrentActionTargetSceneInfo = FGBWFlowMoveMoveControlInitInfo(
			InputState.OwnerCharacter,
			GetMoveControlParam().MoveToDirection,
			TargetMoveTo,
			GetMoveControlParam().VelocityDirection,
			GetMoveControlParam().MoveSpeed);
	}

	CurrentActionTargetSceneInfo.LastValidTime = this->GetWorld()->GetTimeSeconds();
}

FGBWFlowMoveControlParam UGBWFlowMoveComponent::GetMoveControlParam() const
{
	return TaskState.MoveControlParam;
}

FVector UGBWFlowMoveComponent::GetMoveToVector()
{
	FVector InputVector = GetMoveVector();

	if (InputVector.IsNearlyZero())
	{
		return TaskState.MoveControlParam.MoveToDirection;
	}
	
	InputVector.Normalize();
		
	FVector ForwardVector = GetControlVector();
	ForwardVector.Z = 0.0f;
	if (!InputVector.IsNearlyZero())
	{
		if (FVector(InputVector.X,InputVector.Y,0.0f).IsNearlyZero())
		{
			InputVector.X = 0.01f;
		}
		ForwardVector = ForwardVector.Rotation().RotateVector(InputVector);
	}
	ForwardVector.Normalize();
	return ForwardVector;
}

FVector UGBWFlowMoveComponent::GetDirectionVector(
	const EFlowMoveDirectionType Type,
	const FVector WorldAbsoluteVector)
{
	FVector Res = FVector::ZeroVector;
	switch (Type) { case ActorForward:
		Res = InputState.OwnerCharacter->GetActorForwardVector();
		break;
	case Velocity:
		Res = InputState.OwnerCharacter->GetVelocity();
		break;
	case Controller:
		Res = GetControlVector();
		break;
	case CurrentMoveToDirection:
		Res = GetMoveControlParam().MoveToDirection;
		break;
	case TargetMoveToDirection:
		Res = GetControlVector();
		Res.Z = 0.0f;
		Res.Normalize();
		Res = Res.Rotation().RotateVector(GetMoveVector());
		break;
	case ActorToTarget:
		Res = GetMoveControlParam().TargetPoint - InputState.OwnerCharacter->GetActorLocation();
		break;
	case StartPointToTarget:
		Res = GetMoveControlParam().TargetPoint - CurrentActionTargetSceneInfo.BeginLocation;
		break;
	case WorldAbsolute:
		Res = WorldAbsoluteVector;
		break;
	case FlowMoveForward:
		Res = InputState.ForwardVector;
		break;
	case RootMotionVelocity:
		Res = ConvertLocalRootMotionToWorld(GetMoveControlParam().RootMotionNow).GetTranslation();
		break;
	case StartMoveToDirection:
		Res = CurrentActionTargetSceneInfo.BeginMoveToDirection;
		break;
	case StartTargetMoveToDirection:
		Res = CurrentActionTargetSceneInfo.BeginTargetMoveToDirection;
		break;
	case StartActorForward:
		Res = CurrentActionTargetSceneInfo.BeginDirection;
		break;
	default: ;
	}

	Res.Normalize();
	return Res;
}

FVector UGBWFlowMoveComponent::GetDirectionVectorFrom(const FGBWFlowMoveDirection Direction)
{
	FVector Res = GetDirectionVector(Direction.Type,Direction.WorldAbsoluteVector);

	if (FVector(Res.X,Res.Y,0.0f).IsNearlyZero())
	{
		Res.X = GetMoveControlParam().MoveToDirection.X;
		Res.Y = GetMoveControlParam().MoveToDirection.Y;
		Res.Normalize();
	}
	if (FVector(Res.X,Res.Y,0.0f).IsNearlyZero())
	{
		Res.X = InputState.OwnerCharacter->GetActorForwardVector().X;
		Res.Y = InputState.OwnerCharacter->GetActorForwardVector().Y;
		Res.Normalize();
	}

	if (Direction.OnlyHorizontalDirection)
	{
		Res.Z = 0.0f;
	}
	else
	{
		FRotator ResRot = Res.Rotation();
		if (ResRot.Pitch < Direction.MinPitch)
		{
			ResRot.Pitch = Direction.MinPitch;
		}
		if (ResRot.Pitch > Direction.MaxPitch)
		{
			ResRot.Pitch = Direction.MaxPitch;
		}
		
		Res = ResRot.Vector();
	}
	Res.Normalize();

	if (Direction.bInvert)
	{
		if (const FVector InvertRefDirection = GetDirectionVector(Direction.InvertRefDirectionType,Res);
			FMath::Abs<float>(UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(Res,InvertRefDirection)) >= Direction.InvertWhenAngleToRefDirection)
		{
			Res = -Res;
		}
	}
	return Res;
}

void UGBWFlowMoveComponent::AddVelocityTrend(FGBWFlowMoveVelocityTrend VelocityTrend)
{
	AddVelocityTrend_Server(VelocityTrend);
}
void UGBWFlowMoveComponent::AddVelocityTrend_Server_Implementation(FGBWFlowMoveVelocityTrend VelocityTrend)
{
	for (int i = 0; i < InputState.VelocityTrend.Num(); i++)
	{
		if (InputState.VelocityTrend[i].Slot == VelocityTrend.Slot)
		{
			InputState.VelocityTrend[i] = VelocityTrend;
			return;
		}
	}
	InputState.VelocityTrend.Add(VelocityTrend);
}

void UGBWFlowMoveComponent::SendFootStepEvent(USkeletalMeshComponent* InSKMesh,
                                            FName InFootStepType,
                                            FName InFootStepSocket,
                                            FGBWFlowMoveTraceSetting FloorTraceSetting)
{
	OnEvent(false,FFlowMoveEvent(
		InSKMesh,
		InFootStepType,
		InFootStepSocket,
		FloorTraceSetting
	));
}

void UGBWFlowMoveComponent::SetMovementReplication(const bool bReplication)
{
	if (!bReplication)
	{
		if (!bHasSetIgnoreClientMovementErrorChecksAndCorrection)
		{
			bIgnoreClientMovementErrorChecksAndCorrectionCache = InputState.OwnerCharacter->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection;
			bServerAcceptClientAuthoritativePositionCache = InputState.OwnerCharacter->GetCharacterMovement()->bServerAcceptClientAuthoritativePosition;
		}
		
		bHasSetIgnoreClientMovementErrorChecksAndCorrection = true;
		InputState.OwnerCharacter->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;
		InputState.OwnerCharacter->GetCharacterMovement()->bServerAcceptClientAuthoritativePosition = true;
	}
	else if (bHasSetIgnoreClientMovementErrorChecksAndCorrection)
	{
		bHasSetIgnoreClientMovementErrorChecksAndCorrection = false;
		InputState.OwnerCharacter->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = bIgnoreClientMovementErrorChecksAndCorrectionCache;
		InputState.OwnerCharacter->GetCharacterMovement()->bServerAcceptClientAuthoritativePosition = bServerAcceptClientAuthoritativePositionCache;
	}
}

bool UGBWFlowMoveComponent::IsFmControlled() const
{
	return TaskState.MoveControlParam.bIsActive;
}

bool UGBWFlowMoveComponent::IsLocalPlayer(bool bIsReset)
{
	if (!InputState.OwnerCharacter)
	{
		return false;
	}

	if (!bIsLocalPlayerInit || bIsReset)
	{
		const EGBWActorNetType ActorNetType = UGBWPowerfulToolsFuncLib::GetActorNetType(InputState.OwnerCharacter);
		bIsLocalPlayer =
			ActorNetType == EGBWActorNetType::InClient_LocalPlayer
			|| ActorNetType == EGBWActorNetType::InServer_LocalPlayer;
		bIsLocalPlayerInit = true;
	}
	return bIsLocalPlayer;
}

bool UGBWFlowMoveComponent::IsLocalOwn(bool bIsReset)
{
	if (!InputState.OwnerCharacter)
	{
		return false;
	}

	if (!bIsLocalOwnInit || bIsReset)
	{
		const EGBWActorNetType ActorNetType = UGBWPowerfulToolsFuncLib::GetActorNetType(InputState.OwnerCharacter);
		bIsLocalOwn =
			ActorNetType == EGBWActorNetType::InClient_LocalPlayer
			|| ActorNetType == EGBWActorNetType::InServer_LocalPlayer;
		
		bIsLocalOwnInit = true;
	}
	
	return bIsLocalOwn;
}

bool UGBWFlowMoveComponent::IsInServer(bool bIsReset)
{
	if (!bIsInServerInit || bIsReset)
	{
		UGBWPowerfulToolsFuncLib::NetTypeInServer(UGBWPowerfulToolsFuncLib::GetActorNetType(InputState.OwnerCharacter),bIsInServer);
		bIsInServerInit = true;
	}
	return bIsInServer;
}

float UGBWFlowMoveComponent::GetActorScale() const
{
	float Res = 1.0f;
	if (InputState.OwnerCharacter)
	{
		const FVector Scale = InputState.OwnerCharacter->GetActorScale3D();
		Res =  (Scale.X + Scale.Y + Scale.Z) / 3.0f;
	}

	return Res==0.0f? 1.0f : Res;
}

FTransform UGBWFlowMoveComponent::ConvertLocalRootMotionToWorld(const FTransform& InTransform)
{
	if (!InputState.OwnerCharacter)
	{
		return InTransform;
	}

	USkeletalMeshComponent* SKMesh = InputState.OwnerCharacter->GetMesh();
	if (!SKMesh)
	{
		return InTransform;
	}
		
	const FTransform ActorToWorld = SKMesh->GetOwner()->GetTransform();

	const FTransform ComponentToActor = ActorToWorld.GetRelativeTransform(SKMesh->GetComponentTransform());
	const FTransform NewComponentToWorld = InTransform * SKMesh->GetComponentTransform();
	const FTransform NewActorTransform = ComponentToActor * NewComponentToWorld;

	const FVector DeltaWorldTranslation = NewActorTransform.GetTranslation() - ActorToWorld.GetTranslation();

	const FQuat NewWorldRotation = SKMesh->GetComponentTransform().GetRotation() * InTransform.GetRotation();
	const FQuat DeltaWorldRotation = NewWorldRotation * SKMesh->GetComponentTransform().GetRotation().Inverse();
	
	const FTransform DeltaWorldTransform(DeltaWorldRotation, DeltaWorldTranslation);

	return DeltaWorldTransform;
}

FTransform UGBWFlowMoveComponent::ConvertLocalRootMotionToActor(const FTransform& InTransform)
{
	if (!InputState.OwnerCharacter)
	{
		return InTransform;
	}

	USkeletalMeshComponent* SKMesh = InputState.OwnerCharacter->GetMesh();
	if (!SKMesh)
	{
		return InTransform;
	}
		
	const FTransform ActorToWorld = SKMesh->GetOwner()->GetTransform();

	const FTransform ComponentToActor = ActorToWorld.GetRelativeTransform(SKMesh->GetComponentTransform());
	const FTransform NewComponentToWorld = InTransform * SKMesh->GetComponentTransform();
	const FTransform NewActorTransform = ComponentToActor * NewComponentToWorld;

	const FVector DeltaWorldTranslation = NewActorTransform.GetTranslation() - ActorToWorld.GetTranslation();

	const FQuat NewWorldRotation = SKMesh->GetComponentTransform().GetRotation() * InTransform.GetRotation();
	const FQuat DeltaWorldRotation = NewWorldRotation * SKMesh->GetComponentTransform().GetRotation().Inverse();
	
	const FTransform DeltaActorTransform(
		UKismetMathLibrary::InverseTransformRotation(ActorToWorld,DeltaWorldRotation.Rotator()),
		UKismetMathLibrary::InverseTransformDirection(ActorToWorld,DeltaWorldTranslation));
	
	return DeltaActorTransform;
}

bool UGBWFlowMoveComponent::CheckUpdateValid()
{
	if (!InputState.OwnerCharacter)
	{
		InputState.OwnerCharacter = Cast<ACharacter>(GetOwner());
	}
	if (!InputState.OwnerCharacter)
	{
		return false;
	}
	
	FlowMoveBrainValid();
	
	return true;
}

void UGBWFlowMoveComponent::EnsureRMSActivation()
{
	if (!UGBWRMSBPFuncLib::GBWRMS_IsRMSCommonActive(InputState.OwnerCharacter->GetCharacterMovement()))
	{
		FName RMSInsName;
		UGBWRMSBPFuncLib::GBWRMS_Common(
						RMSInsName,
						this,
						InputState.OwnerCharacter->GetCharacterMovement(),
						true);
	}

	FRootMotionSourceGroup RmsArr = InputState.OwnerCharacter->GetCharacterMovement()->CurrentRootMotion;
	for (const auto Rms:RmsArr.RootMotionSources)
	{
		if (Rms->InstanceName != UGBWRMSBPFuncLib::GBWRMS_GetRMSCommonInstanceName())
		{
			InputState.OwnerCharacter->GetCharacterMovement()->RemoveRootMotionSource(Rms->InstanceName);
		}
	}
}

void UGBWFlowMoveComponent::UpdateCharacterState(float DeltaTime)
{
	TaskState.MovementMode = InputState.OwnerCharacter->GetCharacterMovement()->MovementMode;
}

void UGBWFlowMoveComponent::UpdateMoveControlParam(float DeltaTime)
{
	TaskState.UpdateInfo(InputState.OwnerCharacter);
	if (InputState.OwnerCharacter)// && IsInServer())
    {
		//Update MoveControlParam
		TaskState.MoveControlParam.UpdateMoveControlParam(DeltaTime,this);
    }

	if (IsInServer())
	{
		if (this->GetWorld()->GetTimeSeconds() - CurrentActionTargetSceneInfo.LastValidTime <= 0.05f)
		{
			FVector TargetMoveTo = GetControlVector();
			TargetMoveTo.Z = 0.0f;
			TargetMoveTo.Normalize();
			TargetMoveTo = TargetMoveTo.Rotation().RotateVector(GetMoveVector());
			
			CurrentActionTargetSceneInfo.BeginMoveToDirection = GetMoveControlParam().MoveToDirection;
			CurrentActionTargetSceneInfo.BeginTargetMoveToDirection = TargetMoveTo;
		}
	}
}

void UGBWFlowMoveComponent::UpdateRMS(float DeltaTime)
{
	//if (IsInServer())
	{
		if (!TaskState.MoveControlParam.bIsActive)
		{
			MoveControlResult.Velocity = FVector::ZeroVector;
			MoveControlResult.bIsControlled = false;
			return;
		}
		
		if ((TaskState.MoveControlParam.MoveControlSettings.Progress >= 0.9f && GFrameCounter - TaskState.MoveControlParam.LastActiveFrame >= 2)
			|| GFrameCounter - TaskState.MoveControlParam.LastActiveFrame >= 4)
		{
			TaskState.MoveControlParam.bIsActive = false;
		}
		
		MoveControlResult.bIsControlled = true;
		MoveControlResult.LastControlledFrame = GFrameCounter;
		
		FVector NewVelocity = FVector::ZeroVector;
		
		bool bIsVelocityTrendOverride = false;
		bool bIsIgnoreGravity = false;
		FVector VelocityTrend = InputState.CheckGetVelocityTrend(DeltaTime, bIsVelocityTrendOverride,bIsIgnoreGravity);

		if (DeltaTime > SMALL_NUMBER && !bIsVelocityTrendOverride)
		{
			bool bIsGetMoveControlDirectly = false;
			FVector Velocity_Direct;
			if (GetMoveControlParam().MoveControlSettings.bTryGetMoveControlParamFromInterface && FlowMoveBrainValid())
			{
				FRotator DeltaRotation_Direct;
				FlowMoveBrain->FM_GetMoveControlDirectly(
				GetMoveControlParam(),DeltaTime,
				MoveControlResult.Velocity,
				FRotator::ZeroRotator,
				bIsGetMoveControlDirectly,
				Velocity_Direct,
				DeltaRotation_Direct);
			}
			if (bIsGetMoveControlDirectly)
			{
				NewVelocity = Velocity_Direct;
			}
			else
			{
				//Velocity Control
				FVector TargetPoint = GetMoveControlParam().TargetPoint;
				const bool bIsToTargetPoint = GetMoveControlParam().MoveControlSettings.Velocity.UncontrollableVelocity.ToTarget.bToTargetRate;

				FVector TargetRealDirection = TargetPoint - CurrentActionTargetSceneInfo.BeginLocation;
				TargetRealDirection.Normalize();

				const FVector DtPathOffset = GetMoveControlParam().PathOffsetDirection.Rotation().RotateVector(GetMoveControlParam().DeltaPathOffset);
				FVector CurrentTargetDtPathOffset = FVector::ZeroVector;
				if (CurrentActionTargetSceneInfo.bIsValid && bIsToTargetPoint)
				{
					CurrentTargetDtPathOffset = TargetRealDirection * GetMoveControlParam().ToTargetPointSpeed * DeltaTime;
				}

				const float FallDistance = bIsIgnoreGravity? 0.0f : 0.5f * 10000.0f * GetMoveControlParam().Gravity * DeltaTime * DeltaTime;

				FVector DtMove = CurrentTargetDtPathOffset + DtPathOffset + FVector(0,0,FallDistance);

				if (GetMoveControlParam().MoveControlSettings.Velocity.ControllableVelocity.VelocityFromRootMotion)
				{
					const FTransform RootMotionNowWorld = ConvertLocalRootMotionToWorld(GetMoveControlParam().RootMotionNow);
					DtMove = DtMove + RootMotionNowWorld.GetTranslation() * GetMoveControlParam().MoveControlSettings.Velocity.ControllableVelocity.RootMotionScale;
				}
				else
				{
					FVector VelocityDirection = GetMoveControlParam().VelocityDirection;
					{
						const float MoveDirectionAngle = UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(
						VelocityDirection,
						InputState.OwnerCharacter->GetActorForwardVector());
						if (MoveDirectionAngle > FMath::Abs(GetMoveControlParam().MoveControlSettings.Velocity.ControllableVelocity.MoveDirection.MaxHorizontalAngleToActorForward))
						{
							VelocityDirection = UKismetMathLibrary::RotateAngleAxis(
								VelocityDirection,
								MoveDirectionAngle - FMath::Abs(GetMoveControlParam().MoveControlSettings.Velocity.ControllableVelocity.MoveDirection.MaxHorizontalAngleToActorForward),
								FVector::UpVector);
						}
						else if (MoveDirectionAngle < -FMath::Abs(GetMoveControlParam().MoveControlSettings.Velocity.ControllableVelocity.MoveDirection.MaxHorizontalAngleToActorForward))
						{
							VelocityDirection = UKismetMathLibrary::RotateAngleAxis(
								VelocityDirection,
								MoveDirectionAngle + FMath::Abs(GetMoveControlParam().MoveControlSettings.Velocity.ControllableVelocity.MoveDirection.MaxHorizontalAngleToActorForward),
								FVector::UpVector);
						}
					}
				
					DtMove = DtMove +
						VelocityDirection
						* GetMoveControlParam().MoveSpeed * DeltaTime;
				}
								
				FVector Force = DtMove / DeltaTime;

				//Rotation Control
				if (!GetMoveControlParam().MoveControlSettings.RotationSetting.RotationFromRootMotion)
				{
					if (GetMoveControlParam().MoveControlSettings.TimeNow >= GetMoveControlParam().MoveControlSettings.RotationSetting.DelayTime)
					{
						FVector TargetDirection = InputState.ForwardVector;

						FVector DirectionLockTo = GetDirectionVectorFrom(GetMoveControlParam().MoveControlSettings.RotationSetting.RotationLockTo);
						TargetDirection = DirectionLockTo;

						FRotator CurrentRotation = TaskState.LastActorTransform.Rotator();
						FRotator TargetRotation = TargetDirection.Rotation();
						CurrentRotation.Normalize();
						TargetRotation.Normalize();
						switch (GetMoveControlParam().MoveControlSettings.RotationSetting.RotateType)
						{
						case ShortestPath:
							MoveControlResult.TargetRotation = FMath::QInterpTo(
								TaskState.LastActorTransform.Rotator().Quaternion(),
								TargetDirection.Rotation().Quaternion(),
								DeltaTime,
								GetMoveControlParam().MoveControlSettings.RotationSetting.RotationSmoothSpeed
								).Rotator();
							break;
						case Clockwise:
							if (TargetRotation.Pitch < CurrentRotation.Pitch)
							{TargetRotation.Pitch += 360.0f;}
							if (TargetRotation.Yaw < CurrentRotation.Yaw)
							{TargetRotation.Yaw += 360.0f;}
							if (TargetRotation.Roll < CurrentRotation.Roll)
							{TargetRotation.Roll += 360.0f;}

							MoveControlResult.TargetRotation.Pitch = FMath::FInterpTo(
								CurrentRotation.Pitch, TargetRotation.Pitch, DeltaTime,
								GetMoveControlParam().MoveControlSettings.RotationSetting.RotationSmoothSpeed);
							MoveControlResult.TargetRotation.Yaw = FMath::FInterpTo(
								CurrentRotation.Yaw, TargetRotation.Yaw, DeltaTime,
								GetMoveControlParam().MoveControlSettings.RotationSetting.RotationSmoothSpeed);
							MoveControlResult.TargetRotation.Roll = FMath::FInterpTo(
								CurrentRotation.Roll, TargetRotation.Roll, DeltaTime,
								GetMoveControlParam().MoveControlSettings.RotationSetting.RotationSmoothSpeed);
							break;
						case Anticlockwise:
							if (TargetRotation.Pitch > CurrentRotation.Pitch)
							{TargetRotation.Pitch -= 360.0f;}
							if (TargetRotation.Yaw > CurrentRotation.Yaw)
							{TargetRotation.Yaw -= 360.0f;}
							if (TargetRotation.Roll > CurrentRotation.Roll)
							{TargetRotation.Roll -= 360.0f;}

							MoveControlResult.TargetRotation.Pitch = FMath::FInterpTo(
								CurrentRotation.Pitch, TargetRotation.Pitch, DeltaTime,
								GetMoveControlParam().MoveControlSettings.RotationSetting.RotationSmoothSpeed);
							MoveControlResult.TargetRotation.Yaw = FMath::FInterpTo(
								CurrentRotation.Yaw, TargetRotation.Yaw, DeltaTime,
								GetMoveControlParam().MoveControlSettings.RotationSetting.RotationSmoothSpeed);
							MoveControlResult.TargetRotation.Roll = FMath::FInterpTo(
								CurrentRotation.Roll, TargetRotation.Roll, DeltaTime,
								GetMoveControlParam().MoveControlSettings.RotationSetting.RotationSmoothSpeed);
							break;
						default: ;
						}
					}
					MoveControlResult.DeltaRotator = (MoveControlResult.TargetRotation.Quaternion() * InputState.OwnerCharacter->GetTransform().GetRotation().Inverse()).Rotator();
				}
				else
				{
					MoveControlResult.TargetRotation =
						(InputState.OwnerCharacter->GetTransform().GetRotation()
							* (GetMoveControlParam().RootMotionNow.GetRotation().Rotator() * GetMoveControlParam().MoveControlSettings.RotationSetting.RootMotionScale).Quaternion()
							).Rotator();
					MoveControlResult.DeltaRotator = (MoveControlResult.TargetRotation.Quaternion() * InputState.OwnerCharacter->GetTransform().GetRotation().Inverse()).Rotator();
					if (!GetMoveControlParam().MoveControlSettings.RotationSetting.RootMotionPitch)
					{
						MoveControlResult.DeltaRotator.Pitch = 0.0f;
					}
					if (!GetMoveControlParam().MoveControlSettings.RotationSetting.RootMotionRoll)
					{
						MoveControlResult.DeltaRotator.Roll = 0.0f;
					}
					if (!GetMoveControlParam().MoveControlSettings.RotationSetting.RootMotionYaw)
					{
						MoveControlResult.DeltaRotator.Yaw = 0.0f;
					}

					if (GetMoveControlParam().MoveControlSettings.RotationSetting.bTryLockTo)
					{
						FVector TargetDirection = InputState.ForwardVector;
						FVector DirectionLockTo = GetDirectionVectorFrom(GetMoveControlParam().MoveControlSettings.RotationSetting.RotationLockTo);
						TargetDirection = DirectionLockTo;

						if (FMath::Abs(UGBWPowerfulToolsFuncLib::GetAngle(
							MoveControlResult.TargetRotation.Vector(),
							TargetDirection,
							InputState.OwnerCharacter->GetActorUpVector())) <= 15.0f)
						{
							MoveControlResult.TargetRotation = FMath::QInterpTo(
							TaskState.LastActorTransform.Rotator().Quaternion(),
							TargetDirection.Rotation().Quaternion(),
							DeltaTime,
							GetMoveControlParam().MoveControlSettings.RotationSetting.RotationSmoothSpeed
							).Rotator();

							MoveControlResult.DeltaRotator = (MoveControlResult.TargetRotation.Quaternion() * InputState.OwnerCharacter->GetTransform().GetRotation().Inverse()).Rotator();
						}
					}
				}
				MoveControlResult.DeltaRotator.Yaw = MoveControlResult.DeltaRotator.Yaw + GetMoveControlParam().DeltaRotationYawOffset;
				
				//Make Result
				NewVelocity = Force;
			}
		}
		else
		{
			NewVelocity = FVector::ZeroVector;
		}

		//InertialVelocity
		NewVelocity = NewVelocity + GetMoveControlParam().InertialVelocity;

		//Scale By Actor Scale
		NewVelocity = NewVelocity * GetActorScale();
		
		//Adjust - VelocityTrend
		NewVelocity = bIsVelocityTrendOverride? VelocityTrend : NewVelocity + VelocityTrend;
		
		//Adjust - MaxSpeed
		if (TaskState.MoveControlParam.MoveControlSettings.Velocity.ControllableVelocity.SpeedAdjustSettings.MaxSpeed > 0.0f
			&& NewVelocity.Length() <= TaskState.MoveControlParam.MoveControlSettings.Velocity.ControllableVelocity.SpeedAdjustSettings.MaxSpeed)
		{
			FVector VelocityDirection = NewVelocity;
			VelocityDirection.Normalize();
			NewVelocity = VelocityDirection * TaskState.MoveControlParam.MoveControlSettings.Velocity.ControllableVelocity.SpeedAdjustSettings.MaxSpeed;
		}

		//Custom Adjust
		bool bIsGetAdjust = false;
		FVector Res_Velocity = FVector::Zero();
		FRotator Res_DeltaRotation = FRotator::ZeroRotator;
		if (GetMoveControlParam().MoveControlSettings.bTryGetMoveControlParamFromInterface && FlowMoveBrainValid())
		{
			FlowMoveBrain->FM_AdjustFlowMoveResult(
				GetMoveControlParam(),
				DeltaTime,
				NewVelocity,
				FRotator::ZeroRotator,
				bIsGetAdjust,
				Res_Velocity,
				Res_DeltaRotation);
		}
		if (bIsGetAdjust)
		{
			NewVelocity = Res_Velocity;
		}

		//Finally Apply
		MoveControlResult.Velocity = NewVelocity;

		//Moving Adjust
		{
			//MoveConstrain
			if (TaskState.MoveControlParam.MoveControlSettings.AdjustSettings.bIsConstrainMoveToTargetPlane
					&& TaskState.MoveControlParam.MoveControlSettings.TimeNow - TaskState.MoveControlParam.MoveControlSettings.StartTime <= TaskState.MoveControlParam.MoveControlSettings.AdjustSettings.ConstrainMoveToTargetPlaneTime
					&& CurrentActionTargetSceneInfo.bIsValid
					&& CurrentActionTargetSceneInfo.bHasTargetPoint)
			{
				FVector NormalVector = GetMoveControlParam().TargetPoint - CurrentActionTargetSceneInfo.BeginLocation;
				NormalVector.Z = 0.0f;
				NormalVector.Normalize();
				NormalVector = FVector(NormalVector.Y, -NormalVector.X, 0.0f);
					
				MoveControlResult.bConstrainToPlane = true;
				MoveControlResult.ConstrainPlaneNormal = NormalVector;
				MoveControlResult.ConstrainPlaneOrigin = CurrentActionTargetSceneInfo.BeginLocation;
					
			}
			else
			{
				MoveControlResult.bConstrainToPlane = false;
			}

			//Capsule And Mesh Adjust
			if (GetMoveControlParam().MoveControlSettings.AdjustSettings.bIsUseCapsuleAdjust)
			{
				float TargetHeight = UKismetMathLibrary::FInterpTo(
					InputState.OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*2,
					GetMoveControlParam().MoveControlSettings.AdjustSettings.CapsuleAdjust.AdjustTo,
					DeltaTime,
					GetMoveControlParam().MoveControlSettings.AdjustSettings.CapsuleAdjust.SmoothSpeed);

				float TargetRadius = GetMoveControlParam().MoveControlSettings.AdjustSettings.CapsuleAdjust.bRadiusAdjust? UKismetMathLibrary::FInterpTo(
					InputState.OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
					GetMoveControlParam().MoveControlSettings.AdjustSettings.CapsuleAdjust.RadiusAdjustTo,
					DeltaTime,
					GetMoveControlParam().MoveControlSettings.AdjustSettings.CapsuleAdjust.SmoothSpeed)
						: CapsuleRadiusCache;
				
				const FVector CapScale = InputState.OwnerCharacter->GetCapsuleComponent()->GetComponentScale();
				const float TargetHalfHeight = TargetHeight / 2.0f / CapScale.Z;
				TargetRadius = TargetRadius / CapScale.Z;

				FTransform TargetMeshTransform = MeshRelativeTransformCache;
				TargetMeshTransform.SetLocation(
						FVector(
						MeshRelativeTransformCache.GetLocation().X,
						MeshRelativeTransformCache.GetLocation().Y,
						MeshRelativeTransformCache.GetLocation().Z + (CapsuleHalfHeightCache * CapScale.Z - TargetHalfHeight)
						)
					);

				MoveControlResult.CapsuleRadius = TargetRadius;
				MoveControlResult.CapsuleHalfHeight = TargetHalfHeight;
				MoveControlResult.MeshRelativeTransform = TargetMeshTransform;
			}
			else
			{
				MoveControlResult.CapsuleHalfHeight = 
					FMath::FInterpTo(
						InputState.OwnerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(),
						CapsuleHalfHeightCache,
						DeltaTime,
						12.0f);
				MoveControlResult.CapsuleRadius =
					FMath::FInterpTo(
					InputState.OwnerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
					CapsuleRadiusCache,
					DeltaTime,
					12.0f);
				MoveControlResult.MeshRelativeTransform =
					UKismetMathLibrary::TInterpTo(
						InputState.OwnerCharacter->GetMesh()->GetRelativeTransform(),
						MeshRelativeTransformCache,
						DeltaTime,
						12.0f);
			}
		}
	}
}

void UGBWFlowMoveComponent::MovingAdjust()
{
	//MoveConstrain
	if (MoveControlResult.bConstrainToPlane)
	{
		SetMovementConstrain(MoveControlResult.ConstrainPlaneNormal, MoveControlResult.ConstrainPlaneOrigin);
	}
	else
	{
		InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintEnabled(bIsConstrainToPlaneCache);
		InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintNormal(PlaneConstraintNormalCache);
		InputState.OwnerCharacter->GetMovementComponent()->SetPlaneConstraintOrigin(PlaneConstraintOriginCache);
	}

	//Capsule And Mesh Adjust
	bHasSetCapsuleAndMesh = true;
	InputState.OwnerCharacter->GetCapsuleComponent()->SetCapsuleRadius(MoveControlResult.CapsuleRadius);
	InputState.OwnerCharacter->GetCapsuleComponent()->SetCapsuleHalfHeight(MoveControlResult.CapsuleHalfHeight);
	InputState.OwnerCharacter->GetMesh()->SetRelativeTransform(MoveControlResult.MeshRelativeTransform);

	//Rotation
	//GBWTODO
	//InputState.OwnerCharacter->SetActorRotation(MoveControlResult.TargetRotation);

	if (IsInServer())
	{
		if (TaskState.Timer >= 2.0f && GFrameCounter % 120 <= 2)
		{
			SetMovementReplication(true);
		}
		else
		{
			SetMovementReplication(false);
		}
	}
}

bool UGBWFlowMoveComponent::FlowMoveBrainValid()
{
	if (!FlowMoveBrain || FlowMoveBrain->GetClass() != FlowMoveBrainClass)
	{
		if (FlowMoveBrainClass)
		{
			FlowMoveBrain = NewObject<UGBWFlowMoveBrain_Base>(this,FlowMoveBrainClass);
		}
		if (!FlowMoveBrain)
		{
			return false;
		}
	}

	return FlowMoveBrain->CheckValid(this);
}

void UGBWFlowMoveComponent::FlowMoveTick(float DeltaTime)
{
	//Check Update Valid
	if (!CheckUpdateValid())
	{
		return;
	}

	//Check ViewMode
	CheckViewMode();
	CheckUpdateActorLockUI();
	
	//Update CharacterNetState
	UpdateCharacterState(DeltaTime);

	OnEvent(false,FFlowMoveEvent(EFlowMoveEventType::OnUpdate));

	//Update MoveControlParam
	UpdateMoveControlParam(DeltaTime);

	//Update RMS
	UpdateRMS(DeltaTime);
	
	if (MoveControlResult.bIsControlled && ActiveReady())
	{
		MovingAdjust();

		TaskState.Timer = TaskState.Timer + TaskState.FrameDeltaTime;
	}
	else
	{
		DeactivateReady();
	}
}

void UGBWFlowMoveComponent::CheckViewMode()
{
	if (IsLocalPlayer())
	{
		if (InputState.FocusActorComponent && InputState.ViewMode == TP_ActorLockMode)
		{
			FVector DirectionVector = InputState.FocusActorComponent->GetComponentLocation()-InputState.OwnerCharacter->GetActorLocation();
			const float Distance = DirectionVector.Length();
			//DirectionVector.Normalize();
			//const float Angle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(DirectionVector, GetControlVector()));
			/*if (Angle>ActorTargetFuzzyQueryAngleRestrictions || Distance > ActorTargetPrejudgeDistance)
			{
				InputState.FocusActorComponent = nullptr;
			}*/
			if (Distance > ActorTargetPrejudgeDistance)
			{
				InputState.FocusActorComponent = nullptr;
			}
		}
	}
	
	//if (!GetMoveVector().IsNearlyZero())
	{
		const FVector OldForwardVector = InputState.ForwardVector;
		
		if (InputState.ViewMode == EFMViewMode::TP_ActorLockMode
		&& InputState.FocusActorComponent)
		{
			FVector ForwardVector = InputState.FocusActorComponent->GetComponentLocation() - InputState.OwnerCharacter->GetActorLocation();
			ForwardVector.Z = 0.0f;
			ForwardVector.Normalize();
			
			InputState.ForwardVector = ForwardVector;
		}
		else if (InputState.ViewMode == EFMViewMode::TP_ForwardLockMode)
		{
			FVector ControlVector = GetControlVector();
			ControlVector.Z = 0.0f;
			ControlVector.Normalize();
			InputState.ForwardVector = ControlVector;
		}
		else if (InputState.ViewMode == EFMViewMode::TP_FreeMode)
		{
			if (!GetMoveVector().IsNearlyZero())
			{
				InputState.ForwardVector = GetMoveControlParam().MoveToDirection;
			}
			else
			{
				InputState.ForwardVector = InputState.OwnerCharacter->GetActorForwardVector();
			}
			InputState.ForwardVector.Z = 0.0f;
			InputState.ForwardVector.Normalize();
		}
		else
		{
			InputState.ForwardVector = GetMoveControlParam().MoveToDirection;
			InputState.ForwardVector.Z = 0.0f;
			InputState.ForwardVector.Normalize();
			if (InputState.ViewMode != DefaultViewMode)
			{
				SetFlowMoveViewMode(DefaultViewMode,false);
			}
		}

		InputState.ForwardVectorYawAngleSpeed = (InputState.ForwardVector - OldForwardVector).Rotation().Yaw / TaskState.FrameDeltaTime;
	}
	//else
	{
		//TaskState.ForwardVector = GetMoveControlParam().MoveToDirection;
	}
}

void UGBWFlowMoveComponent::CheckUpdateActorLockUI()
{
	if (IsLocalPlayer())
	{
		if (InputState.FocusActorComponent && InputState.ViewMode == TP_ActorLockMode)
		{
			if (!ActorTargetWidget && ActorTargetWidgetClass)
			{
				ActorTargetWidget = UWidgetBlueprintLibrary::Create(
					InputState.OwnerCharacter->GetWorld(),
					ActorTargetWidgetClass, InputState.OwnerCharacter->GetLocalViewingPlayerController());
			}

			if (ActorTargetWidget)
			{
				if (!ActorTargetWidget->IsInViewport())
				{
					ActorTargetWidget->AddToViewport();
				}

				FVector2D ViewportPosition = FVector2D::ZeroVector;
				if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
					InputState.OwnerCharacter->GetLocalViewingPlayerController(),
					InputState.FocusActorComponent->GetComponentLocation(), ViewportPosition, false))
				{
					ActorTargetWidget->SetRenderTranslation(ViewportPosition);
				}
				else
				{
					ActorTargetWidget->SetRenderTranslation(FVector2D(-10000.0f,-10000.0f));
				}
			}
		}
		else
		{
			if (ActorTargetWidget && ActorTargetWidget->IsInViewport())
			{
				ActorTargetWidget->RemoveFromParent();
			}
		}

		if (InputState.FocusActorComponent != LastFocusActorComponent)
		{
			if (InputState.FocusActorComponent)
			{
				UGameplayStatics::PlaySound2D(InputState.OwnerCharacter->GetWorld(),
					Sound_TargetGet,
					1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
			}
			else
			{
				UGameplayStatics::PlaySound2D(InputState.OwnerCharacter->GetWorld(),
					Sound_TargetLost,
					1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
			}
		}

		LastFocusActorComponent = InputState.FocusActorComponent;
	}

	if (InputState.FocusActorComponent && InputState.ViewMode == TP_ActorLockMode)
	{
		if (InputState.OwnerCharacter->GetController())
		{
			FRotator TargetRotation = InputState.OwnerCharacter->GetController()->GetControlRotation();
			const FRotator TheTargetRotation = UKismetMathLibrary::RInterpTo(
				TargetRotation,
				UKismetMathLibrary::FindLookAtRotation(InputState.OwnerCharacter->GetActorLocation(), InputState.FocusActorComponent->GetComponentLocation()),
				TaskState.FrameDeltaTime,
				2.0f
			);
			TargetRotation.Yaw = TheTargetRotation.Yaw;
			InputState.OwnerCharacter->GetController()->SetControlRotation(TargetRotation);
		}
	}
}

void UGBWFlowMoveComponent::SetFlowMoveViewMode(EFMViewMode NewViewMode, bool bAsDefaultViewMode)
{
	SetFlowMoveViewMode_Server(NewViewMode, bAsDefaultViewMode);
}
void UGBWFlowMoveComponent::SetFlowMoveViewMode_Server_Implementation(
	EFMViewMode NewViewMode, bool bAsDefaultViewMode)
{
	InputState.ViewMode = NewViewMode;
	if (bAsDefaultViewMode && DefaultViewMode != NewViewMode)
	{
		DefaultViewMode = NewViewMode;
	}
}
EFMViewMode UGBWFlowMoveComponent::GetFlowMoveViewMode() const
{
	return InputState.ViewMode;
}

void UGBWFlowMoveComponent::SetFlowMoveSpeedMode(EFMSpeedMode NewSpeedMode)
{
	SetFlowMoveSpeedMode_Server(NewSpeedMode);
}
void UGBWFlowMoveComponent::SetFlowMoveSpeedMode_Server_Implementation(EFMSpeedMode NewSpeedMode)
{
	InputState.SpeedMode = NewSpeedMode;
}
EFMSpeedMode UGBWFlowMoveComponent::GetFlowMoveSpeedMode() const
{
	return InputState.SpeedMode;
}

void UGBWFlowMoveComponent::SetFlowMoveBodyMode(EFMBodyMode NewBodyMode)
{
	SetFlowMoveBodyMode_Server(NewBodyMode);
}
void UGBWFlowMoveComponent::SetFlowMoveBodyMode_Server_Implementation(EFMBodyMode NewBodyMode)
{
	InputState.BodyMode = NewBodyMode;
}
EFMBodyMode UGBWFlowMoveComponent::GetFlowMoveBodyMode() const
{
	return InputState.BodyMode;
}

void UGBWFlowMoveComponent::SetFocusActor(USceneComponent* InComponent, bool bSetViewModeAsActorLock)
{
	if (IsLocalOwn() ||IsInServer())
	{
		SetFocusActor_Server(InComponent);
		if (InComponent && bSetViewModeAsActorLock)
		{
			SetFlowMoveViewMode(EFMViewMode::TP_ActorLockMode, false);
		}
		else
		{
			SetFlowMoveViewMode(DefaultViewMode, false);
		}
	}
}
void UGBWFlowMoveComponent::SetFocusActor_Server_Implementation(USceneComponent* InComponent)
{
	InputState.FocusActorComponent = InComponent;
	SetFocusActor_Multicast(InComponent);
}
void UGBWFlowMoveComponent::SetFocusActor_Multicast_Implementation(USceneComponent* InComponent)
{
	SetFocusActor_Imp(InComponent);
}
void UGBWFlowMoveComponent::SetFocusActor_Imp(USceneComponent* InComponent)
{
	OnEvent(false,FFlowMoveEvent(InComponent));
}

void UGBWFlowMoveComponent::GetFocusActor(bool& bIsGet, USceneComponent*& FocusedComponent)
{
	bIsGet = false;
	if (InputState.FocusActorComponent)
	{
		bIsGet = true;
	}

	FocusedComponent = InputState.FocusActorComponent;
}
