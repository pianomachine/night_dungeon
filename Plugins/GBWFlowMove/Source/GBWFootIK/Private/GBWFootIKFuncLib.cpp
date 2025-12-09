// Copyright GanBowen 2022-2024. All Rights Reserved.

#include "GBWFootIKFuncLib.h"

#include "GBWFootIKInterface.h"
#include "GBWPowerfulToolsFuncLib.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/Package.h"
#include "Animation/AnimClassInterface.h"
#include "ANS/GBWANS_FootStep.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "DrawDebugHelpers.h"

bool UGBWFootIKFuncLib::MakeFootIKData(
		ACharacter* OwnerCharacter,
		float DeltaTime,
		FGBWFootIKData& FootIKData,
		FGBWFootIKSettings FootIKSettings,
		FCSPose<FCompactPose>& UnadjustedCompactPose,
		const FBoneContainer& BoneContainer)
{
	if (!OwnerCharacter)
	{
		return false;
	}
	const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	const USkeletalMeshComponent* OwnerSkMeshComponent = OwnerCharacter->GetMesh();

	FootIKData.bShouldDisableFootIK = !FootIKData.bActive || !MovementComponent->IsMovingOnGround();
	
	TArray<AActor*> IgnoreActors;
	UGBWPowerfulToolsFuncLib::GetAllRelevantActors(OwnerCharacter,IgnoreActors);

	//Check Init Bones
	{
		if (FootIKData.LeftFoot.TargetLimbBoneIndex == INDEX_NONE)
		{FootIKData.LeftFoot.TargetLimbBoneIndex = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneContainer.GetPoseBoneIndexForBoneName(FootIKSettings.LeftFoot.BoneName)));}
		if (FootIKData.LeftFoot.LowerLimbBoneIndex == INDEX_NONE && FootIKData.LeftFoot.TargetLimbBoneIndex != INDEX_NONE)
		{FootIKData.LeftFoot.LowerLimbBoneIndex = BoneContainer.GetParentBoneIndex(FootIKData.LeftFoot.TargetLimbBoneIndex);}
		if (FootIKData.LeftFoot.UpperLimbBoneIndex == INDEX_NONE && FootIKData.LeftFoot.LowerLimbBoneIndex != INDEX_NONE)
		{FootIKData.LeftFoot.UpperLimbBoneIndex = BoneContainer.GetParentBoneIndex(FootIKData.LeftFoot.LowerLimbBoneIndex);}

		if (FootIKData.RightFoot.TargetLimbBoneIndex == INDEX_NONE)
		{FootIKData.RightFoot.TargetLimbBoneIndex = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneContainer.GetPoseBoneIndexForBoneName(FootIKSettings.RightFoot.BoneName)));}
		if (FootIKData.RightFoot.LowerLimbBoneIndex == INDEX_NONE && FootIKData.RightFoot.TargetLimbBoneIndex != INDEX_NONE)
		{FootIKData.RightFoot.LowerLimbBoneIndex = BoneContainer.GetParentBoneIndex(FootIKData.RightFoot.TargetLimbBoneIndex);}
		if (FootIKData.RightFoot.UpperLimbBoneIndex == INDEX_NONE && FootIKData.RightFoot.LowerLimbBoneIndex != INDEX_NONE)
		{FootIKData.RightFoot.UpperLimbBoneIndex = BoneContainer.GetParentBoneIndex(FootIKData.RightFoot.LowerLimbBoneIndex);}

		if (FootIKData.RootBoneIndex == INDEX_NONE)
		{
			FCompactPoseBoneIndex RootIndex = FootIKData.LeftFoot.TargetLimbBoneIndex;
	        while (true)
	        {
		        if (FCompactPoseBoneIndex NewIndex = BoneContainer.GetParentBoneIndex(RootIndex); NewIndex != INDEX_NONE)
		        {FootIKData.RootBoneIndex = NewIndex; FootIKData.PelvisIndex = RootIndex; RootIndex = NewIndex;}
		        else {break;}
	        }
			
			if (FootIKData.RootBoneIndex != INDEX_NONE)
			{FootIKData.RootBoneName = BoneContainer.GetReferenceSkeleton().GetBoneName(FootIKData.RootBoneIndex.GetInt());}
			if (FootIKData.PelvisIndex != INDEX_NONE)
			{FootIKData.PelvisBoneName = BoneContainer.GetReferenceSkeleton().GetBoneName(FootIKData.PelvisIndex.GetInt());}
		}
	}

	if (!FootIKData.IsValidForPose(UnadjustedCompactPose.GetPose()))
	{return false;}
	
	FTransform Root = UnadjustedCompactPose.GetComponentSpaceTransform(FootIKData.RootBoneIndex);
	FTransform Pelvis = UnadjustedCompactPose.GetComponentSpaceTransform(FootIKData.PelvisIndex);
	FTransform LeftFoot = UnadjustedCompactPose.GetComponentSpaceTransform(FootIKData.LeftFoot.TargetLimbBoneIndex);
	FTransform RightFoot = UnadjustedCompactPose.GetComponentSpaceTransform(FootIKData.RightFoot.TargetLimbBoneIndex);
	FTransform LeftKnee = UnadjustedCompactPose.GetComponentSpaceTransform(FootIKData.LeftFoot.LowerLimbBoneIndex);
	FTransform RightKnee = UnadjustedCompactPose.GetComponentSpaceTransform(FootIKData.RightFoot.LowerLimbBoneIndex);
	FTransform LeftThigh = UnadjustedCompactPose.GetComponentSpaceTransform(FootIKData.LeftFoot.UpperLimbBoneIndex);
	FTransform RightThigh = UnadjustedCompactPose.GetComponentSpaceTransform(FootIKData.RightFoot.UpperLimbBoneIndex);

	FootIKData.UnadjustedRootBoneCSTransform = Root;
	FootIKData.UnadjustedPelvisBoneCSTransform = Pelvis;
	FootIKData.LeftFoot.HeightFromFloor = LeftFoot.GetLocation().Z - FootIKData.LeftFoot.MinHeightFromFloor;
	FootIKData.RightFoot.HeightFromFloor = RightFoot.GetLocation().Z - FootIKData.LeftFoot.MinHeightFromFloor;
	FootIKData.LeftFoot.FloorRotation = UKismetMathLibrary::TransformRotation(OwnerSkMeshComponent->GetComponentTransform(), LeftFoot.Rotator());
	FootIKData.RightFoot.FloorRotation = UKismetMathLibrary::TransformRotation(OwnerSkMeshComponent->GetComponentTransform(), RightFoot.Rotator());
	FootIKData.LeftFoot.UnadjustedFootBoneCSTransform = LeftFoot;
	FootIKData.RightFoot.UnadjustedFootBoneCSTransform = RightFoot;
	FootIKData.LeftFoot.UnadjustedKneeBoneCSTransform = LeftKnee;
	FootIKData.RightFoot.UnadjustedKneeBoneCSTransform = RightKnee;
	FootIKData.LeftFoot.UnadjustedThighBoneCSTransform = LeftThigh;
	FootIKData.RightFoot.UnadjustedThighBoneCSTransform = RightThigh;

	FTransform LeftFootReal = OwnerSkMeshComponent->GetSocketTransform(FootIKSettings.LeftFoot.BoneName, RTS_World);
	FTransform RightFootReal = OwnerSkMeshComponent->GetSocketTransform(FootIKSettings.RightFoot.BoneName, RTS_World);
	FootIKData.LeftFoot.RealFootBoneWorldTransform = LeftFootReal;
	FootIKData.RightFoot.RealFootBoneWorldTransform = RightFootReal;
	FootIKData.LeftFoot.FootBoneDiffBetweenRealAndTarget = (FootIKData.LeftFoot.RealFootBoneWorldTransform.GetLocation() - FootIKData.LeftFoot.TargetFootBoneWorldTransform.GetLocation()).Length();
	FootIKData.RightFoot.FootBoneDiffBetweenRealAndTarget = (FootIKData.RightFoot.RealFootBoneWorldTransform.GetLocation() - FootIKData.RightFoot.TargetFootBoneWorldTransform.GetLocation()).Length();

	if (!FootIKData.bShouldDisableFootIK)
	{
		float LHeightFromFloor = LeftFoot.GetLocation().Z - Root.GetLocation().Z;
		FootIKData.LeftFoot.HeightFromFloorQueue.Enqueue(LHeightFromFloor);
		FootIKData.LeftFoot.MinHeightFromFloor = FMath::FInterpTo(
			FootIKData.LeftFoot.MinHeightFromFloor,
			FootIKData.LeftFoot.HeightFromFloorQueue.GetMin(),
			DeltaTime, 12.0f);

		float RHeightFromFloor = RightFoot.GetLocation().Z - Root.GetLocation().Z;
		FootIKData.RightFoot.HeightFromFloorQueue.Enqueue(RHeightFromFloor);
		FootIKData.RightFoot.MinHeightFromFloor = FMath::FInterpTo(
			FootIKData.RightFoot.MinHeightFromFloor,
			FootIKData.RightFoot.HeightFromFloorQueue.GetMin(),
			DeltaTime, 12.0f);
	}

	auto RootAndPelvisOffset = [&FootIKData, DeltaTime, OwnerSkMeshComponent, FootIKSettings]()
	{
		if (!FootIKData.bShouldDisableFootIK)
		{FootIKData.PelvisAlpha = (FootIKData.LeftFoot.Ik_Rate + FootIKData.RightFoot.Ik_Rate) / 2.0f;}
		else
		{FootIKData.PelvisAlpha = FMath::FInterpTo(FootIKData.PelvisAlpha, 0.0f, DeltaTime, 15.0f);}
		
		if (FootIKData.PelvisAlpha > 0.0f)
		{
			const FVector PelvisTarget = FootIKData.LeftFoot.Target_LocationOffset.Z < FootIKData.RightFoot.Target_LocationOffset.Z ?
				FootIKData.LeftFoot.Target_LocationOffset : FootIKData.RightFoot.Target_LocationOffset;
			const float InterpSpeed = PelvisTarget.Z > FootIKData.PelvisOffset.Z ? 10.0f : 15.0f;
			FootIKData.PelvisOffset = FMath::VInterpTo(FootIKData.PelvisOffset, PelvisTarget, DeltaTime, InterpSpeed);
		}
		else
		{FootIKData.PelvisOffset = FMath::VInterpTo(FootIKData.PelvisOffset, FVector::ZeroVector, DeltaTime, 15.0f);}
		FootIKData.TargetPelvisBoneWorldTransform = FTransform(
			UKismetMathLibrary::TransformRotation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.UnadjustedPelvisBoneCSTransform.GetRotation().Rotator()),
			UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.UnadjustedPelvisBoneCSTransform.GetLocation() + FootIKData.PelvisOffset)
		);

		FootIKData.RootAlpha = FootIKData.PelvisAlpha;
		const FVector TargetRootBoneLocation = UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.UnadjustedRootBoneCSTransform.GetLocation());
		if (FootIKData.RootAlpha > 0.0f)
		{
			FootIKData.TargetRootBoneWorldLocation = UKismetMathLibrary::VInterpTo(
					FootIKData.TargetRootBoneWorldLocation,
					TargetRootBoneLocation,
					DeltaTime,
					FootIKSettings.RootHeightSmoothSpeed);
			FootIKData.RootOffset = FootIKData.TargetRootBoneWorldLocation - TargetRootBoneLocation;
			if (FootIKData.RootOffset.Z > FMath::Abs(FootIKSettings.RootMaxHeightDiff))
			{
				FootIKData.TargetRootBoneWorldLocation.Z -= (FootIKData.RootOffset.Z - FMath::Abs(FootIKSettings.RootMaxHeightDiff));
				FootIKData.RootOffset.Z = FMath::Abs(FootIKSettings.RootMaxHeightDiff);
			}
			else if (FootIKData.RootOffset.Z < -FMath::Abs(FootIKSettings.RootMaxHeightDiff))
			{
				FootIKData.TargetRootBoneWorldLocation.Z -= (FootIKData.RootOffset.Z + FMath::Abs(FootIKSettings.RootMaxHeightDiff));
				FootIKData.RootOffset.Z = -FMath::Abs(FootIKSettings.RootMaxHeightDiff);
			}
		}
		else
		{
			FootIKData.TargetRootBoneWorldLocation = TargetRootBoneLocation;
			FootIKData.RootOffset = FVector::ZeroVector;
		}
	};

	auto FootLock = [OwnerSkMeshComponent, FootIKSettings, FootIKData](FGBWFootIKFootData& FootData)
	{
		if (FootIKData.bShouldDisableFootIK)
		{FootData.Lock_Alpha = 0.0f;}
		else if (FootData.Lock_Rate * FootData.Ik_Rate >= 1.0f || FootData.Lock_Rate * FootData.Ik_Rate < FootData.Lock_Alpha)
		{FootData.Lock_Alpha = FootData.Lock_Rate * FootData.Ik_Rate;}

		FootData.bHasFootStepEvent = false;
		if (FootData.Lock_Alpha >= 0.8f
			&& !FootData.FloorLocation.IsZero()
			&& FootData.bReadyToFootStepEvent)
		{
			FootData.bHasFootStepEvent = true;
			FootData.bReadyToFootStepEvent = false;
		}
		if (FootData.Lock_Alpha < 0.5f)
		{
			FootData.bReadyToFootStepEvent = true;
			//FootData.bReadyToLockShot = true;
		}
		if (FootIKData.bShouldDisableFootIK)
		{
			FootData.Lock_Location_World = UKismetMathLibrary::TransformLocation(
				OwnerSkMeshComponent->GetComponentTransform(),
				FootData.UnadjustedFootBoneCSTransform.GetLocation());
			FootData.Lock_Rotation_World = UKismetMathLibrary::TransformRotation(
				OwnerSkMeshComponent->GetComponentTransform(),
				FootData.UnadjustedFootBoneCSTransform.Rotator());;
			FootData.LastLockShotTime = FootData.TimeNow;
		}
		else if (FootData.Lock_Alpha >= 1.0f
			&& !FootData.FloorLocation.IsZero()
			&& FootData.bReadyToLockShot)
		{
			FootData.Lock_Location_World = FootData.FloorLocation;
			FootData.Lock_Rotation_World = FootData.FloorRotation;
			FootData.LastLockShotTime = FootData.TimeNow;
			//FootData.bReadyToLockShot = false;
		}

		if (FootData.Lock_Alpha > 0.0f)
		{
			FootData.Lock_Location = UKismetMathLibrary::InverseTransformLocation(
					OwnerSkMeshComponent->GetComponentTransform(),
					FootData.Lock_Location_World + FootData.FloorNormal * FootIKSettings.FootHeight);
			FootData.Lock_Rotation = UKismetMathLibrary::InverseTransformRotation(OwnerSkMeshComponent->GetComponentTransform(), FootData.Lock_Rotation_World);
			FootData.Lock_Rotation.Normalize();
		}
		FootData.FootTraceWorldLocation = UKismetMathLibrary::VLerp(
		UKismetMathLibrary::TransformLocation(
			OwnerSkMeshComponent->GetComponentTransform(),
			FootData.UnadjustedFootBoneCSTransform.GetLocation()),
			FootData.Lock_Location_World,
			FootData.Lock_Location_World.IsZero()? 0.0f : FootData.Lock_Alpha);
		
		FootData.LockFootBoneWorldTransform.SetLocation(UKismetMathLibrary::TransformLocation(
			OwnerSkMeshComponent->GetComponentTransform(),
		UKismetMathLibrary::VLerp(
			FootData.UnadjustedFootBoneCSTransform.GetLocation(),
			FootData.Lock_Location,
			FootData.Lock_Alpha)));
		FootData.LockFootBoneWorldTransform.SetRotation(UKismetMathLibrary::TransformRotation(
			OwnerSkMeshComponent->GetComponentTransform(),
		UKismetMathLibrary::RLerp(
			FootData.UnadjustedFootBoneCSTransform.GetRotation().Rotator(),
			FootData.Lock_Rotation,
			FootData.Lock_Alpha,true)).Quaternion());
	};

	auto FootOffsets = [OwnerSkMeshComponent, OwnerCharacter, MovementComponent, IgnoreActors, FootIKSettings, FootIKData, DeltaTime](
		FGBWFootIKFootData& FootData)
	{
		const UWorld* World = OwnerCharacter->GetWorld();
		check(World);
		FVector FootTraceLocation = FootData.FootTraceWorldLocation;
		FootTraceLocation.Z = UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(), FootIKData.UnadjustedRootBoneCSTransform.GetLocation()).Z;
		FootData.FootTraceWorldLocation_Start = FootTraceLocation + FVector(0.0, 0.0, FootIKSettings.FootTraceRange);
		FootData.FootTraceWorldLocation_End = FootTraceLocation - FVector(0.0, 0.0, FootIKSettings.FootTraceRange);
		FCollisionQueryParams Params;
		Params.AddIgnoredActors(IgnoreActors);
		
		World->LineTraceSingleByChannel(FootData.FloorHitResult, FootData.FootTraceWorldLocation_Start,FootData.FootTraceWorldLocation_End,ECC_Visibility, Params);

		FRotator TargetRotOffset = FRotator::ZeroRotator;
		FVector TargetLocationOffset = FVector::ZeroVector;
		if (MovementComponent->IsWalkable(FootData.FloorHitResult) && !FootIKData.bShouldDisableFootIK && FootData.Ik_Rate > 0)
		{
			FootData.FloorLocation = FootData.FloorHitResult.ImpactPoint;
			FootData.FloorNormal = FootData.FloorHitResult.ImpactNormal;

			TargetLocationOffset = (FootData.FloorLocation + FootData.FloorNormal * FootIKSettings.FootHeight) - (FootTraceLocation + FVector(0, 0, FootIKSettings.FootHeight));
			TargetRotOffset.Pitch = -FMath::RadiansToDegrees(FMath::Atan2(FootData.FloorNormal.X, FootData.FloorNormal.Z));
			TargetRotOffset.Roll = FMath::RadiansToDegrees(FMath::Atan2(FootData.FloorNormal.Y, FootData.FloorNormal.Z));
		}
		else
		{
			FootData.FloorLocation = FVector::ZeroVector;
			FootData.FloorNormal = FVector::ZeroVector;
			FootData.FloorRotation = FRotator::ZeroRotator;
		}
		
		const float InterpSpeed = FootData.Target_LocationOffset.Z > TargetLocationOffset.Z ? 30.f : 15.0f;
		FootData.Target_LocationOffset = FMath::VInterpTo(FootData.Target_LocationOffset, TargetLocationOffset, DeltaTime, InterpSpeed);
		FootData.Target_RotationOffset = FMath::RInterpTo(FootData.Target_RotationOffset, TargetRotOffset, DeltaTime, 30.0f);

		FootData.TargetFootBoneWorldTransform.SetLocation(
			FootData.LockFootBoneWorldTransform.GetLocation() + FootData.Target_LocationOffset * FootData.Ik_Rate * (1.0f - FootData.Lock_Alpha));
		FootData.TargetFootBoneWorldTransform.SetRotation(
			UKismetMathLibrary::ComposeRotators(FootData.LockFootBoneWorldTransform.Rotator(),
				FootData.Target_RotationOffset * FootData.Ik_Rate).Quaternion());
		
		FootData.TargetKneeBoneWorldLocation =
			((UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(), FootData.UpperLimbCSTransform.GetLocation())
				+ UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(), FootData.UnadjustedKneeBoneCSTransform.GetLocation())
				- UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(), FootData.UnadjustedThighBoneCSTransform.GetLocation()))
			+ (FootData.TargetFootBoneWorldTransform.GetLocation()
				+ UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(), FootData.UnadjustedKneeBoneCSTransform.GetLocation())
				- UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(), FootData.UnadjustedFootBoneCSTransform.GetLocation())))
			/ 2.0f;
	};

	auto AutoFootLock = [DeltaTime, MovementComponent, FootIKData](FGBWFootIKFootData& FootData, const FGBWFootLockControlSettings& FootLockControlSettings)
	{
		FootData.MinDurationBetweenLock = FootLockControlSettings.MinDurationBetweenLock;
		float TargetLockRate = 0.0f;
		float ToTargetLockRateSpeed = 0.0f;

		if (FootIKData.bShouldDisableFootIK || MovementComponent->Velocity.Length() <= FootLockControlSettings.SpeedToUnlock)
		{
			TargetLockRate = MovementComponent->Velocity.Length() / FootLockControlSettings.SpeedToUnlock;
			ToTargetLockRateSpeed = 12.0f;
			FootData.LockStateType = EGBWFootIKLockStateType::Unlocked;
		}
		else
		{
			if (FootData.HeightFromFloor >= FootLockControlSettings.LockStartHeight)
			{
				TargetLockRate = 0.0f;
				ToTargetLockRateSpeed = 15.0f;
				if (FootData.LockStateType != EGBWFootIKLockStateType::Unlocked)
				{
					FootData.LockStateType = EGBWFootIKLockStateType::Unlocked;
					FootData.LastLockStateTime = FootData.TimeNow;
				}
			}
			else if (FootData.HeightFromFloor >= FootLockControlSettings.LockShotHeight)
			{
				TargetLockRate = (FootLockControlSettings.LockStartHeight - FootData.HeightFromFloor)
					/ (FootLockControlSettings.LockStartHeight - FootLockControlSettings.LockShotHeight);
				
				switch (FootData.LockStateType) {
				case LockIn:
					ToTargetLockRateSpeed = 15.0f;
					FootData.LockStateType = EGBWFootIKLockStateType::UnlockIn;
					FootData.LastLockStateTime = FootData.TimeNow;
					break;
				case Locked:
					ToTargetLockRateSpeed = 15.0f;
					FootData.LockStateType = EGBWFootIKLockStateType::UnlockIn;
					FootData.LastLockStateTime = FootData.TimeNow;
					break;
				case LockOut:
					ToTargetLockRateSpeed = 15.0f;
					FootData.LockStateType = EGBWFootIKLockStateType::UnlockIn;
					FootData.LastLockStateTime = FootData.TimeNow;
					break;
				case UnlockIn:
					ToTargetLockRateSpeed = 15.0f;
					FootData.LockStateType = EGBWFootIKLockStateType::UnlockIn;
					break;
				case Unlocked:
					ToTargetLockRateSpeed = 8.0f;
					FootData.LockStateType = EGBWFootIKLockStateType::UnlockOut;
					FootData.LastLockStateTime = FootData.TimeNow;
					break;
				case UnlockOut:
					ToTargetLockRateSpeed = 8.0f;
					FootData.LockStateType = EGBWFootIKLockStateType::UnlockOut;
					break;
				default: ;
				}
			}
			else if (FootData.HeightFromFloor >= FootLockControlSettings.LockFinishedHeight)
			{
				TargetLockRate = 1.0f;
				ToTargetLockRateSpeed = 30.0f;
				switch (FootData.LockStateType) {
				case LockIn:
					FootData.LockStateType = EGBWFootIKLockStateType::LockIn;
					break;
				case Locked:
					FootData.LockStateType = EGBWFootIKLockStateType::LockOut;
					FootData.LastLockStateTime = FootData.TimeNow;
					break;
				case LockOut:
					FootData.LockStateType = EGBWFootIKLockStateType::LockOut;
					break;
				case UnlockIn:
					FootData.LockStateType = EGBWFootIKLockStateType::LockIn;
					FootData.LastLockStateTime = FootData.TimeNow;
					break;
				case Unlocked:
					FootData.LockStateType = EGBWFootIKLockStateType::LockIn;
					FootData.LastLockStateTime = FootData.TimeNow;
					break;
				case UnlockOut:
					FootData.LockStateType = EGBWFootIKLockStateType::LockIn;
					FootData.LastLockStateTime = FootData.TimeNow;
					break;
				default: ;
				}
			}
			else
			{
				TargetLockRate = 1.0f;
				ToTargetLockRateSpeed = 0.0f;
				if (FootData.LockStateType != EGBWFootIKLockStateType::Locked)
				{
					FootData.LockStateType = EGBWFootIKLockStateType::Locked;
					FootData.LastLockStateTime = FootData.TimeNow;
				}
			}
		}

		FootData.Lock_Rate = FMath::FInterpTo(
					FootData.Lock_Rate,
					FMath::Clamp(TargetLockRate, 0.0f,1.0f),
					DeltaTime,ToTargetLockRateSpeed);
	};

	auto IKRate = [OwnerSkMeshComponent, MovementComponent, DeltaTime, OwnerCharacter](FGBWFootIKFootData& FootData, FGBWFootIKFootSettings FootSetting, const bool bIsLeftFoot)
	{
		bool bIsGetValue = false;
		for (auto IKControl : FootSetting.IKControlSettings)
		{
			if (IKControl.ControlType == EGBWFootIKControlType::ByAnimCurve)
			{
				if (float Value = 0.0f; OwnerSkMeshComponent->GetAnimInstance()->GetCurveValue(IKControl.IKRateCurveName,Value))
				{
					FootData.Ik_Rate = FMath::Clamp(Value,0.0f,1.0f);
					bIsGetValue = true; break;
				}
			}

			if (IKControl.ControlType == EGBWFootIKControlType::ByInterface)
			{
				UObject* FootIKInterfaceOb = nullptr;
				if (OwnerCharacter->GetClass()->ImplementsInterface(UGBWFootIKInterface::StaticClass()))
				{FootIKInterfaceOb = OwnerCharacter;}
				else
				{
					for (const auto Obj : OwnerCharacter->GetComponents())
					{
						if (Obj->GetClass()->ImplementsInterface(UGBWFootIKInterface::StaticClass()))
						{FootIKInterfaceOb = Obj; break;}
					}
				}
				if (FootIKInterfaceOb)
				{
					bool bIsGet = false; float IkRate = 0.0f;
					if (bIsLeftFoot) {IGBWFootIKInterface::Execute_GetIKRate_L(FootIKInterfaceOb,OwnerSkMeshComponent,OwnerCharacter,DeltaTime,bIsGet,IkRate);}
					else {IGBWFootIKInterface::Execute_GetIKRate_R(FootIKInterfaceOb,OwnerSkMeshComponent,OwnerCharacter,DeltaTime,bIsGet,IkRate);}
					if (bIsGet) {FootData.Ik_Rate = FMath::Clamp(IkRate, 0.0f,1.0f); bIsGetValue = true; break;}
				}
			}

			if (IKControl.ControlType == EGBWFootIKControlType::AutoControl)
			{
				if (MovementComponent->IsMovingOnGround()) {FootData.Ik_Rate = FMath::FInterpTo(FootData.Ik_Rate, 1.0f, DeltaTime, 15.0f);}
				else {FootData.Ik_Rate = FMath::FInterpTo(FootData.Ik_Rate, 0.0f, DeltaTime, 15.0f);}
				bIsGetValue = true; break;
			}
		}
		if (!bIsGetValue){FootData.Ik_Rate = 0.0f;}
	};

	auto FootLockRate = [OwnerSkMeshComponent, DeltaTime, OwnerCharacter, AutoFootLock](FGBWFootIKFootData& FootData, FGBWFootIKFootSettings FootSetting, const bool bIsLeftFoot)
	{
		FootData.ReadyToLockShotTimer -= DeltaTime;
		if (FootData.FootBoneDiffBetweenRealAndTarget >= 15.0f || FootData.ReadyToLockShotTimer > 0.0f)
		{
			FootData.Lock_Rate = FMath::FInterpTo(FootData.Lock_Rate, 0.0f, DeltaTime, 8.0f);
			if (FootData.ReadyToLockShotTimer <= 0.0f)
			{FootData.ReadyToLockShotTimer = 0.5f;}
			return;
		}
		
		bool bIsGetValue = false;
		
		for (auto LockControl : FootSetting.LockControlSettings)
		{
			if (LockControl.ControlType == EGBWFootIKControlType::ByAnimCurve)
			{
				if (float Value = 0.0f; OwnerSkMeshComponent->GetAnimInstance()->GetCurveValue(LockControl.LockRateCurveName,Value))
				{
					FootData.Lock_Rate = FMath::Clamp(Value,0.0f,1.0f);
					FootData.MinDurationBetweenLock = 0.0f;
					bIsGetValue = true; break;
				}
			}

			if (LockControl.ControlType == EGBWFootIKControlType::ByInterface)
			{
				UObject* FootIKInterfaceOb = nullptr;
				if (OwnerCharacter->GetClass()->ImplementsInterface(UGBWFootIKInterface::StaticClass()))
				{
					FootIKInterfaceOb = OwnerCharacter;
				}
				else
				{
					for (const auto Obj : OwnerCharacter->GetComponents())
					{
						if (Obj->GetClass()->ImplementsInterface(UGBWFootIKInterface::StaticClass()))
						{
							FootIKInterfaceOb = Obj;
							break;
						}
					}
				}
				if (FootIKInterfaceOb)
				{
					bool bIsGet = false; float LockRate = 0.0f;
					if (bIsLeftFoot) {IGBWFootIKInterface::Execute_GetFootLockRate_L(FootIKInterfaceOb,OwnerSkMeshComponent,OwnerCharacter,DeltaTime,bIsGet,LockRate);}
					else {IGBWFootIKInterface::Execute_GetFootLockRate_R(FootIKInterfaceOb,OwnerSkMeshComponent,OwnerCharacter,DeltaTime,bIsGet,LockRate);}
					if (bIsGet) {FootData.Lock_Rate = FMath::Clamp(LockRate, 0.0f,1.0f);  FootData.MinDurationBetweenLock = 0.0f; bIsGetValue = true; break;}
				}
			}

			if (LockControl.ControlType == EGBWFootIKControlType::AutoControl)
			{
				AutoFootLock(FootData,LockControl);
				bIsGetValue = true; break;
			}

			if (LockControl.ControlType == EGBWFootIKControlType::ByAnimNotify)
			{
				for (auto Notify : OwnerSkMeshComponent->GetAnimInstance()->ActiveAnimNotifyState)
				{
					if (Notify.NotifyStateClass
						&& Notify.NotifyStateClass->GetClass()->IsChildOf(UGBWANS_FootStep::StaticClass()))
					{
						const UGBWANS_FootStep* FootStep = Cast<UGBWANS_FootStep>(Notify.NotifyStateClass);
						if (FootStep->FootType == (bIsLeftFoot? EGBWFootType::LeftFoot : EGBWFootType::RightFoot))
						{
							FootData.FootUnlockInterpSpeed = FootStep->FootLockInterpSpeed_Out;
							if (FootStep->bStartLockRate){FootData.Lock_Rate = FootStep->StartLockRate;}
							
							FootData.Lock_Rate = FMath::FInterpTo(FootData.Lock_Rate,FootStep->TargetLockRate * 1.1f,DeltaTime,FootStep->FootLockInterpSpeed_In);
							FootData.Lock_Rate = FMath::Clamp(FootData.Lock_Rate, 0.0f,1.0f);
							FootData.MinDurationBetweenLock = 0.0f;
							bIsGetValue = true; break;
						}
					}
				}
				if (bIsGetValue){break;}
			}
		}

		if (!bIsGetValue){FootData.Lock_Rate = FMath::FInterpTo(FootData.Lock_Rate,0.0f,DeltaTime,FootData.FootUnlockInterpSpeed); FootData.MinDurationBetweenLock = 0.0f;}
	};

	IKRate(FootIKData.LeftFoot,FootIKSettings.LeftFoot,true);
	IKRate(FootIKData.RightFoot,FootIKSettings.RightFoot,false);
	FootLockRate(FootIKData.LeftFoot,FootIKSettings.LeftFoot,true);
	FootLockRate(FootIKData.RightFoot,FootIKSettings.RightFoot,false);

	FootIKData.LeftFoot.TimeNow = OwnerCharacter->GetWorld()->GetTimeSeconds();
	FootIKData.RightFoot.TimeNow = FootIKData.LeftFoot.TimeNow;
	
	FootLock(FootIKData.LeftFoot);
	FootLock(FootIKData.RightFoot);
	FootOffsets(FootIKData.LeftFoot);
	FootOffsets(FootIKData.RightFoot);
	RootAndPelvisOffset();

	if (FootIKSettings.bIsDebug)
	{
		DebugFootIKData(OwnerCharacter->GetWorld(),OwnerSkMeshComponent,FootIKData);
	}
	return true;
}

void UGBWFootIKFuncLib::DebugFootIKData(
		const UWorld* InWorld,
		const USkeletalMeshComponent* OwnerSkMeshComponent,
		const FGBWFootIKData& FootIKData)
{
	if (InWorld && InWorld->IsGameWorld())
	{
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.TargetLimbCSTransform.GetLocation()),
				10.0f,1,FColor::Green,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.LowerLimbCSTransform.GetLocation()),
				10.0f,1,FColor::Green,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UpperLimbCSTransform.GetLocation()),
				10.0f,1,FColor::Green,0.0f,0.0f,0,1.0f);
		DrawDebugLine(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.TargetLimbCSTransform.GetLocation()),
			UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.LowerLimbCSTransform.GetLocation()),
			FColor::Green,0.0f,0.0f,SDPG_Foreground,2.0f);
		DrawDebugLine(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UpperLimbCSTransform.GetLocation()),
			UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.LowerLimbCSTransform.GetLocation()),
			FColor::Green,0.0f,0.0f,SDPG_Foreground,2.0f);

		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.TargetLimbCSTransform.GetLocation()),
				10.0f,1,FColor::Green,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.LowerLimbCSTransform.GetLocation()),
				10.0f,1,FColor::Green,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UpperLimbCSTransform.GetLocation()),
				10.0f,1,FColor::Green,0.0f,0.0f,0,1.0f);
		DrawDebugLine(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.TargetLimbCSTransform.GetLocation()),
			UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.LowerLimbCSTransform.GetLocation()),
			FColor::Green,0.0f,0.0f,SDPG_Foreground,2.0f);
		DrawDebugLine(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UpperLimbCSTransform.GetLocation()),
			UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.LowerLimbCSTransform.GetLocation()),
			FColor::Green,0.0f,0.0f,SDPG_Foreground,2.0f);

		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UnadjustedFootBoneCSTransform.GetLocation()),
				10.0f,1,FColor::Yellow,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UnadjustedKneeBoneCSTransform.GetLocation()),
				10.0f,1,FColor::Yellow,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UnadjustedThighBoneCSTransform.GetLocation()),
				10.0f,1,FColor::Yellow,0.0f,0.0f,0,1.0f);
		DrawDebugLine(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UnadjustedFootBoneCSTransform.GetLocation()),
		UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UnadjustedKneeBoneCSTransform.GetLocation()),
			FColor::Yellow,0.0f,0.0f,SDPG_Foreground,2.0f);
		DrawDebugLine(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UnadjustedThighBoneCSTransform.GetLocation()),
		UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.LeftFoot.UnadjustedKneeBoneCSTransform.GetLocation()),
			FColor::Yellow,0.0f,0.0f,SDPG_Foreground,2.0f);

		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UnadjustedFootBoneCSTransform.GetLocation()),
				10.0f,1,FColor::Yellow,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UnadjustedKneeBoneCSTransform.GetLocation()),
				10.0f,1,FColor::Yellow,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UnadjustedThighBoneCSTransform.GetLocation()),
				10.0f,1,FColor::Yellow,0.0f,0.0f,0,1.0f);
		DrawDebugLine(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UnadjustedFootBoneCSTransform.GetLocation()),
		UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UnadjustedKneeBoneCSTransform.GetLocation()),
			FColor::Yellow,0.0f,0.0f,SDPG_Foreground,2.0f);
		DrawDebugLine(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UnadjustedThighBoneCSTransform.GetLocation()),
		UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.RightFoot.UnadjustedKneeBoneCSTransform.GetLocation()),
			FColor::Yellow,0.0f,0.0f,SDPG_Foreground,2.0f);

		DrawDebugSphere(InWorld,FootIKData.LeftFoot.TargetKneeBoneWorldLocation,
				10.0f,1,FColor::Red,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,FootIKData.LeftFoot.TargetFootBoneWorldTransform.GetLocation(),
				10.0f,1,FColor::Red,0.0f,0.0f,0,1.0f);
		DrawDebugLine(InWorld,FootIKData.LeftFoot.TargetKneeBoneWorldLocation,
		FootIKData.LeftFoot.TargetFootBoneWorldTransform.GetLocation(),
			FColor::Red,0.0f,0.0f,SDPG_Foreground,2.0f);
		
		DrawDebugSphere(InWorld,FootIKData.RightFoot.TargetKneeBoneWorldLocation,
				10.0f,1,FColor::Red,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,FootIKData.RightFoot.TargetFootBoneWorldTransform.GetLocation(),
				10.0f,1,FColor::Red,0.0f,0.0f,0,1.0f);
		DrawDebugLine(InWorld,FootIKData.RightFoot.TargetKneeBoneWorldLocation,
		FootIKData.RightFoot.TargetFootBoneWorldTransform.GetLocation(),
			FColor::Red,0.0f,0.0f,SDPG_Foreground,2.0f);

		DrawDebugSphere(InWorld,UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.UnadjustedPelvisBoneCSTransform.GetLocation()),
				10.0f,1,FColor::Yellow,0.0f,0.0f,SDPG_Foreground,1.0f);
		DrawDebugSphere(InWorld,FootIKData.TargetPelvisBoneWorldTransform.GetLocation(),
				10.0f,1,FColor::Green,0.0f,0.0f,SDPG_Foreground,1.0f);

		DrawDebugString(InWorld, UKismetMathLibrary::TransformLocation(OwnerSkMeshComponent->GetComponentTransform(),FootIKData.UnadjustedRootBoneCSTransform.GetLocation()),
			FString::Printf(TEXT("L: H<%.2f %.2f> Lock<%.2f %.2f> IK<%.2f> Diff<%.2f>\nR: H<%.2f %.2f> Lock<%.2f %.2f> IK<%.2f> Diff<%.2f>"),
				FootIKData.LeftFoot.HeightFromFloor, FootIKData.LeftFoot.MinHeightFromFloor, FootIKData.LeftFoot.Lock_Rate, FootIKData.LeftFoot.Lock_Alpha,FootIKData.LeftFoot.Ik_Rate,
				FootIKData.LeftFoot.FootBoneDiffBetweenRealAndTarget,
				FootIKData.RightFoot.HeightFromFloor, FootIKData.RightFoot.MinHeightFromFloor, FootIKData.RightFoot.Lock_Rate, FootIKData.RightFoot.Lock_Alpha,FootIKData.RightFoot.Ik_Rate,
				FootIKData.RightFoot.FootBoneDiffBetweenRealAndTarget),
			nullptr, FColor::Red, 0.0f, false, 1.0f);

		if (!FootIKData.LeftFoot.FloorLocation.IsZero())
		{DrawDebugSphere(InWorld,FootIKData.LeftFoot.FloorLocation,10.0f,1,FColor::Red,0.0f,0.0f,0,1.0f);}
		DrawDebugSphere(InWorld,FootIKData.LeftFoot.FootTraceWorldLocation_Start,5.0f,1, FColor::Green,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,FootIKData.LeftFoot.FootTraceWorldLocation_End,5.0f,1, FColor::Blue,0.0f,0.0f,0,1.0f);
		DrawDebugLine(InWorld,FootIKData.LeftFoot.FootTraceWorldLocation_Start,FootIKData.LeftFoot.FootTraceWorldLocation_End,FColor::Red,false,0.0f,0,1.0f);

		if (!FootIKData.RightFoot.FloorLocation.IsZero())
		{DrawDebugSphere(InWorld,FootIKData.RightFoot.FloorLocation,10.0f,1,FColor::Red,0.0f,0.0f,0,1.0f);}
		DrawDebugSphere(InWorld,FootIKData.RightFoot.FootTraceWorldLocation_Start,5.0f,1, FColor::Green,0.0f,0.0f,0,1.0f);
		DrawDebugSphere(InWorld,FootIKData.RightFoot.FootTraceWorldLocation_End,5.0f,1, FColor::Blue,0.0f,0.0f,0,1.0f);
		DrawDebugLine(InWorld,FootIKData.RightFoot.FootTraceWorldLocation_Start,FootIKData.RightFoot.FootTraceWorldLocation_End,FColor::Red,false,0.0f,0,1.0f);
	}
}