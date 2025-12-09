// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "Demo/GBWPercetionDemo.h"

#include "GBWPowerfulPerceptionComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

bool FGBWFloatScope::InScope(const float InValue) const
{
	return InValue >= Min && InValue <= Max;
}

bool FSceneDirectionCondition::IsMet(const FGBWCognition_Surroundings& Scene) const
{
	for (auto Item:SceneDirectionRange)
	{
		if (Item.InScope(Scene.TargetInfo.FaceDirection))
		{
			return true;
		}
	}
	return false;
}

void UGBWPerceptionDemoFuncLib::GetSurroundingsCognition_RealTime(AActor* InActor,FGameplayTag Key,
	FGBWCognition_Surroundings& SurroundingsCognition, bool& bIsGet)
{
	FGBWCognition_Surroundings Res = FGBWCognition_Surroundings();
	UGBWPowerfulPerceptionComponent::_GetCognitionAsStruct(InActor, Key, &Res, Res.StaticStruct());
	InitCognitionSurroundings(Res);
	bIsGet = Res.bIsValid;
	SurroundingsCognition = Res;
}

void UGBWPerceptionDemoFuncLib::GetSurroundingsCognition_Snapshot(AActor* InActor, FGameplayTag Key,
	FGBWCognition_Surroundings& SurroundingsCognition, bool& bIsGet)
{
	FGBWCognition_Surroundings Res = FGBWCognition_Surroundings();
	UGBWPowerfulPerceptionComponent::_GetCognitionSnapshotAsStruct(InActor, Key, &Res, Res.StaticStruct());
	InitCognitionSurroundings(Res);
	bIsGet = Res.bIsValid;
	SurroundingsCognition = Res;
}

void UGBWPerceptionDemoFuncLib::PPD_GetSurroundingsCognition_RealTime(AActor* InActor, FGameplayTag Key, bool& bIsGet,
	FGBWCognition_Surroundings& SurroundingsCognition, FGBWCognition_ActorInfo& ActorInfo,
	FGBWCognition_TargetInfo& TargetInfo)
{
	if (InActor)
	{
		GetSurroundingsCognition_RealTime(InActor, Key, SurroundingsCognition, bIsGet);
		ActorInfo = SurroundingsCognition.ActorInfo;
		TargetInfo = SurroundingsCognition.TargetInfo;
		return;
	}

	bIsGet = false;
}

void UGBWPerceptionDemoFuncLib::PPD_GetSurroundingsCognition_Snapshot(AActor* InActor, FGameplayTag Key, bool& bIsGet,
	FGBWCognition_Surroundings& SurroundingsCognition, FGBWCognition_ActorInfo& ActorInfo,
	FGBWCognition_TargetInfo& TargetInfo)
{
	if (InActor)
	{
		GetSurroundingsCognition_Snapshot(InActor, Key, SurroundingsCognition, bIsGet);
		ActorInfo = SurroundingsCognition.ActorInfo;
		TargetInfo = SurroundingsCognition.TargetInfo;
		return;
	}

	bIsGet = false;
}

void UGBWPerceptionDemoFuncLib::PPD_Anim_GetSurroundingsCognition_RealTime(const UAnimInstance* AnimInstance,
                                                                           FGameplayTag Key, bool& bIsGet, FGBWCognition_Surroundings& SurroundingsCognition, FGBWCognition_ActorInfo& ActorInfo,
                                                                           FGBWCognition_TargetInfo& TargetInfo)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		GetSurroundingsCognition_RealTime(AnimInstance->GetOwningActor(), Key, SurroundingsCognition, bIsGet);
		ActorInfo = SurroundingsCognition.ActorInfo;
		TargetInfo = SurroundingsCognition.TargetInfo;
		return;
	}

	bIsGet = false;
}

void UGBWPerceptionDemoFuncLib::PPD_Anim_GetSurroundingsCognition_Snapshot(const UAnimInstance* AnimInstance,
	FGameplayTag Key, bool& bIsGet, FGBWCognition_Surroundings& SurroundingsCognition,
	FGBWCognition_ActorInfo& ActorInfo, FGBWCognition_TargetInfo& TargetInfo)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		GetSurroundingsCognition_Snapshot(AnimInstance->GetOwningActor(), Key, SurroundingsCognition, bIsGet);
		ActorInfo = SurroundingsCognition.ActorInfo;
		TargetInfo = SurroundingsCognition.TargetInfo;
		return;
	}

	bIsGet = false;
}

void UGBWPerceptionDemoFuncLib::GetSurroundingsAnalysisResults(FGBWCognition_Surroundings SurroundingsCognition,
                                                               FGBWCognition_ActorInfo& ActorInfo, FGBWCognition_TargetInfo& TargetInfo, bool& bIsGet)
{
	InitCognitionSurroundings(SurroundingsCognition);
	ActorInfo = SurroundingsCognition.ActorInfo;
	TargetInfo = SurroundingsCognition.TargetInfo;
	bIsGet = SurroundingsCognition.bIsValid;
}

FGBWCognition_Surroundings UGBWPerceptionDemoFuncLib::InitSurroundings(const FGBWCognition_Surroundings Cognition)
{
	FGBWCognition_Surroundings Res = Cognition;
	UGBWPerceptionDemoFuncLib::InitCognitionSurroundings(Res);
	return Res;
}

void UGBWPerceptionDemoFuncLib::InitCognitionSurroundings(FGBWCognition_Surroundings& Cognition)
{
	if (!Cognition.ActorState.SelfActor)
	{
		Cognition.bIsValid = false;
		Cognition.bHasInit = false;
		return;
	}

	if (Cognition.bHasInit)
	{
		return;
	}

	Cognition.bIsValid = true;
	Cognition.bHasInit = true;

	const ACharacter* AsCharacter = Cast<ACharacter>(Cognition.ActorState.SelfActor);
	
	const FVector ActorLocation = Cognition.ActorState.SelfActor->GetActorLocation();
	FTransform ActorFootTransform = Cognition.ActorState.SelfActor->GetTransform();
	if (AsCharacter)
	{
		ActorFootTransform.SetLocation(ActorFootTransform.GetLocation() - FVector(0,0, AsCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	}
	const FVector RefInPoint = UKismetMathLibrary::InverseTransformLocation(ActorFootTransform,Cognition.TargetState.TargetPoint);

	//-----------------------------------------
	Cognition.TargetInfo.Height = RefInPoint.Z;
	Cognition.TargetInfo.Right = RefInPoint.Y;
	Cognition.TargetInfo.Forward = RefInPoint.X;	

	//-----------------------------------------
	Cognition.TargetInfo.PitchAngle = (Cognition.TargetState.TargetPoint - Cognition.ActorState.SelfActor->GetActorLocation()).Rotation().Pitch;

	//-----------------------------------------
	FVector A = UKismetMathLibrary::ProjectVectorOnToPlane(
	Cognition.TargetState.TargetPoint-ActorLocation,Cognition.ActorState.SelfActor ->GetActorForwardVector());
	A.Normalize();
	FVector B = UKismetMathLibrary::ProjectVectorOnToPlane(Cognition.ActorState.SelfActor->GetActorUpVector(),Cognition.ActorState.SelfActor->GetActorForwardVector());
	B.Normalize();
	const float DirectionAngle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(A,B));

	const FTransform T = Cognition.ActorState.SelfActor->GetActorTransform();
	const FVector A2 = UKismetMathLibrary::InverseTransformDirection(T,A);
	const FVector B2 = UKismetMathLibrary::InverseTransformDirection(T,B);
	const bool bIsRight = UKismetMathLibrary::CrossProduct2D(
		FVector2D(A2.Y, A2.Z),
		FVector2D(B2.Y, B2.Z)
	)>=0;

	Cognition.TargetInfo.FaceDirection = DirectionAngle * (bIsRight? 1 : -1);

	//----------------------------------------------
	if (AsCharacter)
	{
		Cognition.ActorInfo.FootLocation = AsCharacter->GetCapsuleComponent()->GetComponentLocation() - FVector(0,0,AsCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		Cognition.ActorInfo.HeadLocation = AsCharacter->GetCapsuleComponent()->GetComponentLocation() + FVector(0,0,AsCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		Cognition.ActorInfo.ActorHeight = (Cognition.ActorInfo.FootLocation - Cognition.ActorInfo.HeadLocation).Length();
	}
	else
	{
		Cognition.ActorInfo.FootLocation = Cognition.ActorInfo.HeadLocation = Cognition.ActorState.SelfActor->GetActorLocation();
	}
	
	Cognition.ActorInfo.HeightFromGround = Cognition.ActorState.Floor.bIsFloor? (Cognition.ActorState.Floor.Floor-Cognition.ActorInfo.FootLocation).Length() : 999999.0f;
	Cognition.ActorInfo.HeightFromRoof = Cognition.ActorState.Roof.bIsRoof? (Cognition.ActorState.Roof.Roof-Cognition.ActorInfo.HeadLocation).Length() : 999999.0f;

	//----------------------------------------------
	if (Cognition.TargetState.bIsFloor && Cognition.TargetState.bIsRoof)
	{
		Cognition.TargetInfo.SpaceHeight = (Cognition.TargetState.Floor-Cognition.TargetState.Roof).Length();
		Cognition.TargetInfo.bIsHole = Cognition.TargetInfo.SpaceHeight < Cognition.ActorInfo.ActorHeight;
	}
	else
	{
		Cognition.TargetInfo.bIsHole = false;
	}

	//----------------------------------------------
	if (Cognition.ActorState.Floor.bIsFloor && Cognition.ActorState.Roof.bIsRoof)
	{
		Cognition.ActorInfo.SpaceHeight = (Cognition.ActorState.Floor.Floor-Cognition.ActorState.Roof.Roof).Length();
		Cognition.ActorInfo.bIsHole = Cognition.TargetInfo.SpaceHeight < Cognition.ActorInfo.ActorHeight;
	}
	else
	{
		Cognition.ActorInfo.bIsHole = false;
	}

	//---------------------------------------------
	Cognition.ActorInfo.bIsFloorLedge = Cognition.ActorState.Floor.FloorLedge_F.bIsFloorLedge || Cognition.ActorState.Floor.FloorLedge_B.bIsFloorLedge;
	Cognition.ActorInfo.bIsRoofLedge = Cognition.ActorState.Roof.bIsRoofLedge_F || Cognition.ActorState.Roof.bIsRoofLedge_B;

	//---------------------------------------------
	Cognition.ActorInfo.bIsSide = Cognition.ActorState.Side.bIsLeft || Cognition.ActorState.Side.bIsRight || Cognition.ActorState.Side.bIsForward;
	Cognition.ActorInfo.CloserSideDistance = 999999999.0f;
	if (Cognition.ActorState.Side.bIsLeft)
	{
		Cognition.ActorInfo.LeftSideDistance = (Cognition.ActorState.Side.Left - Cognition.ActorState.ActorTransform.GetLocation()).Length();
		if (Cognition.ActorInfo.CloserSideDistance >= Cognition.ActorInfo.LeftSideDistance)
		{
			Cognition.ActorInfo.bIsLeftSideCloser = true;
			Cognition.ActorInfo.bIsRightSideCloser = false;
			Cognition.ActorInfo.bIsForwardSideCloser = false;
			Cognition.ActorInfo.CloserSideDistance = Cognition.ActorInfo.LeftSideDistance;	
		}
	}
	if (Cognition.ActorState.Side.bIsRight)
	{
		Cognition.ActorInfo.RightSideDistance = (Cognition.ActorState.Side.Right - Cognition.ActorState.ActorTransform.GetLocation()).Length();
		if (Cognition.ActorInfo.CloserSideDistance >= Cognition.ActorInfo.RightSideDistance)
		{
			Cognition.ActorInfo.bIsLeftSideCloser = false;
			Cognition.ActorInfo.bIsRightSideCloser = true;
			Cognition.ActorInfo.bIsForwardSideCloser = false;
			Cognition.ActorInfo.CloserSideDistance = Cognition.ActorInfo.RightSideDistance;
		}
	}
	if (Cognition.ActorState.Side.bIsForward)
	{
		Cognition.ActorInfo.ForwardSideDistance = (Cognition.ActorState.Side.Forward - Cognition.ActorState.ActorTransform.GetLocation()).Length();
		if (Cognition.ActorInfo.CloserSideDistance >= Cognition.ActorInfo.ForwardSideDistance)
		{
			Cognition.ActorInfo.bIsLeftSideCloser = false;
			Cognition.ActorInfo.bIsRightSideCloser = false;
			Cognition.ActorInfo.bIsForwardSideCloser = true;
			Cognition.ActorInfo.CloserSideDistance = Cognition.ActorInfo.ForwardSideDistance;
		}
	}
}

bool UGBWPerceptionDemoFuncLib::GetPlaneConstraintSetting(FGBWCognition_Surroundings Cognition, FVector& PlaneNormal, FVector& PlaneOrigin)
{
	if (!Cognition.TargetState.bIsFindTarget)
	{
		return false;
	}
	FVector NormalVector = Cognition.TargetState.TargetPoint - Cognition.ActorState.ActorTransform.GetLocation();
	NormalVector.Z = 0.0f;
	NormalVector.Normalize();
	NormalVector = FVector(NormalVector.Y, -NormalVector.X, 0.0f);

	PlaneNormal = NormalVector;
	PlaneOrigin = Cognition.ActorState.ActorTransform.GetLocation();

	return true;
}

FVector UGBWPerceptionDemoFuncLib::GetSurroundingsPoint(const FGBWCognition_Surroundings Cognition,
                                                        const EGBWSurroundingsPointType PointType)
{
	float Dis_F;
	float Dis_B;
	if (!Cognition.bIsValid)
	{
		return FVector::ZeroVector;
	}
	switch (PointType) {
	case TargetPoint: return Cognition.TargetState.TargetPoint;
	case ActorPoint: return Cognition.ActorState.ActorTransform.GetLocation();
	case LeftSidePoint: return Cognition.ActorState.Side.Left;
	case RightSidePoint: return Cognition.ActorState.Side.Right;
	case FloorPoint: return Cognition.ActorState.Floor.Floor;
	case RoofPoint: return Cognition.ActorState.Roof.Roof;
	case FloorForwardLedgePoint: return Cognition.ActorState.Floor.FloorLedge_F.FloorLedgePoint;
	case FloorBackwardLedgePoint: return Cognition.ActorState.Floor.FloorLedge_B.FloorLedgePoint;
	case RoofForwardLedgePoint: return Cognition.ActorState.Roof.RoofLedge_F;
	case RoofBackwardLedgePoint: return Cognition.ActorState.Roof.RoofLedge_B;
	case NearestFloorLedgePoint:
		Dis_F = (Cognition.ActorState.Floor.FloorLedge_F.FloorLedgePoint
			- Cognition.ActorState.Floor.Floor).Length();
		Dis_B =  (Cognition.ActorState.Floor.FloorLedge_B.FloorLedgePoint
			- Cognition.ActorState.Floor.Floor).Length();
		if (Dis_F < Dis_B)
		{
			return Cognition.ActorState.Floor.FloorLedge_F.FloorLedgePoint;
		}
		else
		{
			return Cognition.ActorState.Floor.FloorLedge_B.FloorLedgePoint;
		}
	case NearestRoofLedgePoint: 
		Dis_F = (Cognition.ActorState.Roof.RoofLedge_F
			- Cognition.ActorState.Roof.Roof).Length();
		Dis_B =  (Cognition.ActorState.Roof.RoofLedge_B
			- Cognition.ActorState.Roof.Roof).Length();
		if (Dis_F < Dis_B)
		{
			return Cognition.ActorState.Roof.RoofLedge_F;
		}
		else
		{
			return Cognition.ActorState.Roof.RoofLedge_B;
		}
	case TargetPointFloorPoint:
		return Cognition.TargetState.bIsFloor? Cognition.TargetState.Floor : Cognition.TargetState.TargetPoint;
	default: ;
	}

	return FVector::ZeroVector;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsSceneType(
	AActor* InActor,
	FGameplayTag TargetScene,
	FSceneTypeCondition SceneTypeCondition)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		if (!SceneTypeCondition.SceneTypeCondition.IsEmpty())
		{
			if (const bool bHasSceneType = SceneTypeCondition.SceneTypeCondition.Contains(TheTargetScene.Type);
				(SceneTypeCondition.ConditionType == EInclusivityType::Include_ && !bHasSceneType)
				||(SceneTypeCondition.ConditionType == EInclusivityType::NotInclude_ && bHasSceneType))
			{
				return false;
			}
		}
		return true;
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsSceneType(
	const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	const FSceneTypeCondition& SceneTypeCondition)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsSceneType(
			AnimInstance->GetOwningActor(),
			TargetScene,
			SceneTypeCondition);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsSceneType_Simple(AActor* InActor, FGameplayTag TargetScene,
TEnumAsByte<EInclusivityType> ConditionType, FName SceneType)
{
	FSceneTypeCondition SceneTypeCondition = FSceneTypeCondition();
	SceneTypeCondition.ConditionType = ConditionType;
	SceneTypeCondition.SceneTypeCondition.Add(SceneType);
	return UGBWPerceptionDemoFuncLib::PPD_IsSceneType(
		InActor,
		TargetScene,
		SceneTypeCondition);
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsSceneType_Simple(const UAnimInstance* AnimInstance,
	FGameplayTag TargetScene,
	TEnumAsByte<EInclusivityType> ConditionType, FName SceneType)
{
	FSceneTypeCondition SceneTypeCondition = FSceneTypeCondition();
	SceneTypeCondition.ConditionType = ConditionType;
	SceneTypeCondition.SceneTypeCondition.Add(SceneType);
	return PPD_Anim_IsSceneType(AnimInstance, TargetScene, SceneTypeCondition);
}

void UGBWPerceptionDemoFuncLib::PPD_IsSide(AActor* InActor, FGameplayTag TargetScene, bool& IsLeft, bool& IsRight,
float& LeftDistance, float& RightDistance,bool& IsLeftCloser,
	bool& IsRightCloser)
{
	IsLeft = false;
	IsRight = false;
	LeftDistance = 0.0f;
	RightDistance = 0.0f;
	IsLeftCloser = false;
	IsRightCloser = false;
	
	FGBWCognition_Surroundings Scene;
	UGBWPowerfulPerceptionComponent::_GetCognitionAsStruct(
		InActor,
		TargetScene,
		&Scene, Scene.StaticStruct());
	UGBWPerceptionDemoFuncLib::InitCognitionSurroundings(Scene);
	if (Scene.bIsValid)
	{
		IsLeft = Scene.ActorState.Side.bIsLeft;
		IsRight = Scene.ActorState.Side.bIsRight;
		if (IsLeft)
		{
			LeftDistance = (Scene.ActorState.Side.Left - Scene.ActorState.ActorTransform.GetLocation()).Length();
			IsLeftCloser = true;
		}
		if (IsRight)
		{
			RightDistance = (Scene.ActorState.Side.Right - Scene.ActorState.ActorTransform.GetLocation()).Length();
			IsRightCloser = true;
		}

		if (IsLeft && IsRight)
		{
			if (LeftDistance > RightDistance)
			{
				IsLeftCloser = true;
				IsRightCloser = false;
			}
			else
			{
				IsLeftCloser = false;
				IsRightCloser = true;
			}
		}
	}
}

void UGBWPerceptionDemoFuncLib::PPD_Anim_IsSide(const UAnimInstance* AnimInstance, FGameplayTag TargetScene, bool& IsLeft,
bool& IsRight, float& LeftDistance, float& RightDistance,bool& IsLeftCloser,
	bool& IsRightCloser)
{
	IsLeft = false;
	IsRight = false;
	LeftDistance = 0.0f;
	RightDistance = 0.0f;
	IsLeftCloser = false;
	IsRightCloser = false;
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		UGBWPerceptionDemoFuncLib::PPD_IsSide(
			AnimInstance->GetOwningActor(),
			TargetScene,
			IsLeft,
			IsRight,
			LeftDistance,
			RightDistance,
			IsLeftCloser,
			IsRightCloser);
	}
}

bool UGBWPerceptionDemoFuncLib::PPD_IsSceneHeight(
	AActor* InActor,
	FGameplayTag TargetScene,
	FGBWFloatScope SceneHeightRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		return SceneHeightRange.InScope(TheTargetScene.TargetInfo.Height);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsSceneHeight(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope SceneHeightRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsSceneHeight(
			AnimInstance->GetOwningActor(),TargetScene,SceneHeightRange);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsSceneRight(AActor* InActor, FGameplayTag TargetScene,
                                          FGBWFloatScope SceneRightRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		return SceneRightRange.InScope(TheTargetScene.TargetInfo.Right);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsSceneRight(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope SceneRightRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsSceneRight(
			AnimInstance->GetOwningActor(),TargetScene,SceneRightRange);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsSceneForward(AActor* InActor, FGameplayTag TargetScene,
                                            FGBWFloatScope SceneForwardRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		return SceneForwardRange.InScope(TheTargetScene.TargetInfo.Forward);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsSceneForward(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope SceneForwardRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsSceneForward(
			AnimInstance->GetOwningActor(),TargetScene,SceneForwardRange);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsSceneAngle(AActor* InActor, FGameplayTag TargetScene,
                                          FGBWFloatScope SceneAngleRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		return SceneAngleRange.InScope(TheTargetScene.TargetInfo.PitchAngle);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsSceneAngle(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope SceneAngleRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsSceneAngle(
			AnimInstance->GetOwningActor(),TargetScene,SceneAngleRange);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsSceneDirection(AActor* InActor, FGameplayTag TargetScene,
                                              FSceneDirectionCondition SceneDirectionCondition)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		return SceneDirectionCondition.IsMet(TheTargetScene);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsSceneDirection(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
                                                   const FSceneDirectionCondition& SceneDirectionCondition)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsSceneDirection(
			AnimInstance->GetOwningActor(),TargetScene,SceneDirectionCondition);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsMovementMode(AActor* InActor,
                                            FMovementModeCondition MovementModeCondition)
{
	if (ACharacter* AsCharacter = Cast<ACharacter>(InActor))
	{
		if (!MovementModeCondition.MovementMode.IsEmpty())
		{
			if (const bool bHasMovementMode = MovementModeCondition.MovementMode.Contains(AsCharacter->GetCharacterMovement()->MovementMode);
				(MovementModeCondition.ConditionType == EInclusivityType::Include_ && !bHasMovementMode)
				||(MovementModeCondition.ConditionType == EInclusivityType::NotInclude_ && bHasMovementMode))
			{
				return false;
			}
		}
		return true;
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsMovementMode(const UAnimInstance* AnimInstance,
                                                 const FMovementModeCondition& MovementModeCondition)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsMovementMode(
			AnimInstance->GetOwningActor(),MovementModeCondition);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsHeightFromGround(AActor* InActor, FGameplayTag TargetScene,
                                                FGBWFloatScope HeightFromGroundRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		return HeightFromGroundRange.InScope(TheTargetScene.ActorInfo.HeightFromGround);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsHeightFromGround(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope HeightFromGroundRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsHeightFromGround(
			AnimInstance->GetOwningActor(),TargetScene,HeightFromGroundRange);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsHeightFromRoof(AActor* InActor, FGameplayTag TargetScene,
                                              FGBWFloatScope HeightFromRoofRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		return HeightFromRoofRange.InScope(TheTargetScene.ActorInfo.HeightFromRoof);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsHeightFromRoof(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope HeightFromRoofRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsHeightFromRoof(
			AnimInstance->GetOwningActor(),TargetScene,HeightFromRoofRange);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsFloor(AActor* InActor, FGameplayTag TargetScene,
	FGBWFloatScope HeightFromFloorRange, bool bIsAnyRequireLedge, FGBWFloatScope FloorLedgeRange,
	bool bIsRequireForwardLedge, FGBWFloatScope ForwardFloorLedgeRange, bool bIsRequireBackwardLedge,
	FGBWFloatScope BackwardFloorLedgeRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		if (!HeightFromFloorRange.InScope(TheTargetScene.ActorInfo.HeightFromGround))
		{
			return false;
		}
		if (bIsAnyRequireLedge
			&& (!FloorLedgeRange.InScope((TheTargetScene.ActorState.Floor.FloorLedge_F.FloorLedgePoint - TheTargetScene.ActorState.Floor.Floor).Length())
				&& !FloorLedgeRange.InScope((TheTargetScene.ActorState.Floor.FloorLedge_B.FloorLedgePoint - TheTargetScene.ActorState.Floor.Floor).Length())))
		{
			return false;
		}
		if (bIsRequireForwardLedge
			&& !ForwardFloorLedgeRange.InScope((TheTargetScene.ActorState.Floor.FloorLedge_F.FloorLedgePoint - TheTargetScene.ActorState.Floor.Floor).Length()))
		{
			return false;
		}
		if (bIsRequireBackwardLedge
			&& !BackwardFloorLedgeRange.InScope((TheTargetScene.ActorState.Floor.FloorLedge_B.FloorLedgePoint - TheTargetScene.ActorState.Floor.Floor).Length()))
		{
			return false;
		}
		return true;
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsFloor(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope HeightFromFloorRange, bool bIsAnyRequireLedge, FGBWFloatScope FloorLedgeRange,
	bool bIsRequireForwardLedge, FGBWFloatScope ForwardFloorLedgeRange, bool bIsRequireBackwardLedge,
	FGBWFloatScope BackwardFloorLedgeRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsFloor(AnimInstance->GetOwningActor(),
			TargetScene,
			HeightFromFloorRange,
			bIsAnyRequireLedge,
			FloorLedgeRange,
			bIsRequireForwardLedge,
			ForwardFloorLedgeRange,
			bIsRequireBackwardLedge,
			BackwardFloorLedgeRange);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsRoof(AActor* InActor, FGameplayTag TargetScene,
                                    FGBWFloatScope HeightFromRoofRange, bool bIsAnyRequireLedge, FGBWFloatScope RoofLedgeRange,
                                    bool bIsRequireForwardLedge, FGBWFloatScope ForwardRoofLedgeRange, bool bIsRequireBackwardLedge,
                                    FGBWFloatScope BackwardRoofLedgeRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		if (!HeightFromRoofRange.InScope(TheTargetScene.ActorInfo.HeightFromRoof))
		{
			return false;
		}
		if (bIsAnyRequireLedge
			&& (!RoofLedgeRange.InScope((TheTargetScene.ActorState.Roof.RoofLedge_F - TheTargetScene.ActorState.Roof.Roof).Length())
				&& !RoofLedgeRange.InScope((TheTargetScene.ActorState.Roof.RoofLedge_B - TheTargetScene.ActorState.Roof.Roof).Length())))
		{
			return false;
		}
		if (bIsRequireForwardLedge
			&& !ForwardRoofLedgeRange.InScope((TheTargetScene.ActorState.Roof.RoofLedge_F - TheTargetScene.ActorState.Roof.Roof).Length()))
		{
			return false;
		}
		if (bIsRequireBackwardLedge
			&& !BackwardRoofLedgeRange.InScope((TheTargetScene.ActorState.Roof.RoofLedge_B - TheTargetScene.ActorState.Roof.Roof).Length()))
		{
			return false;
		}
		return true;
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsRoof(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope HeightFromRoofRange, bool bIsAnyRequireLedge, FGBWFloatScope RoofLedgeRange,
	bool bIsRequireForwardLedge, FGBWFloatScope ForwardRoofLedgeRange, bool bIsRequireBackwardLedge,
	FGBWFloatScope BackwardRoofLedgeRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsRoof(AnimInstance->GetOwningActor(),
			TargetScene,
			HeightFromRoofRange,
			bIsAnyRequireLedge,
			RoofLedgeRange,
			bIsRequireForwardLedge,
			ForwardRoofLedgeRange,
			bIsRequireBackwardLedge,
			BackwardRoofLedgeRange);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_IsSlope(AActor* InActor, FGameplayTag TargetScene,
                                     FGBWFloatScope SlopeRange)
{
	bool bIsGet = false;
	FGBWCognition_Surroundings TheTargetScene;
	GetSurroundingsCognition_RealTime(InActor,TargetScene,TheTargetScene, bIsGet);
	if (bIsGet)
	{
		return SlopeRange.InScope(TheTargetScene.ActorState.Slope);
	}

	return false;
}

bool UGBWPerceptionDemoFuncLib::PPD_Anim_IsSlope(const UAnimInstance* AnimInstance, FGameplayTag TargetScene,
	FGBWFloatScope SlopeRange)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWPerceptionDemoFuncLib::PPD_IsSlope(
			AnimInstance->GetOwningActor(),TargetScene,SlopeRange);
	}

	return false;
}



