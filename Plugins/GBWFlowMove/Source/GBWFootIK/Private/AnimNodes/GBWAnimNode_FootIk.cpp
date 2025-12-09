// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "AnimNodes/GBWAnimNode_FootIk.h"

#include "GBWFootIKFuncLib.h"
#include "GBWFootIKInterface.h"
#include "TwoBoneIK.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNotifyQueue.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#define LOCTEXT_NAMESPACE "GBWAnimNode_FootIK"

/////////////////////////////////////////////////////
// FGBWAnimMagicPlayerNode

bool FGBWAnimNode_FootIK::EvaluateBoneControl(FCSPose<FCompactPose>& CompactPose,
	const FBoneContainer& BoneContainer, const FTransform& ComponentTransform,
	const FGBWBoneControl& BoneControl, FBoneTransform& Res, FTransform& TargetTransform)
{
	FBoneReference BoneToModify = FBoneReference(BoneControl.BoneName);
	BoneToModify.BoneIndex = BoneContainer.GetPoseBoneIndexForBoneName(BoneToModify.BoneName);
	BoneToModify.CachedCompactPoseIndex = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneToModify.BoneIndex));
	if (BoneToModify.BoneIndex == INDEX_NONE)
	{
		return false;
	}

	const FCompactPoseBoneIndex CompactPoseBoneToModify = BoneToModify.GetCompactPoseIndex(BoneContainer);
	FTransform NewBoneTM = CompactPose.GetComponentSpaceTransform(CompactPoseBoneToModify);

	TargetTransform = BoneControl.TargetTransform;

	const TEnumAsByte<EBoneModificationMode> ScaleMode = BoneControl.ScaleMode;
	const FVector Scale = TargetTransform.GetScale3D();
	const TEnumAsByte<enum EBoneControlSpace> ScaleSpace = BoneControl.ScaleSpace;
	if (ScaleMode != BMM_Ignore)
	{
		// Convert to Bone Space.
		FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, ScaleSpace);

		if (ScaleMode == BMM_Additive)
		{
			NewBoneTM.SetScale3D(NewBoneTM.GetScale3D() * Scale);
		}
		else
		{
			NewBoneTM.SetScale3D(Scale);
		}

		// Convert back to Component Space.
		FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, ScaleSpace);
	}

	const TEnumAsByte<EBoneModificationMode> RotationMode = BoneControl.RotationMode;
	const FRotator Rotation = TargetTransform.GetRotation().Rotator();
	const TEnumAsByte<enum EBoneControlSpace> RotationSpace = BoneControl.RotationSpace;
	if (RotationMode != BMM_Ignore)
	{
		// Convert to Bone Space.
		FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, RotationSpace);

		const FQuat BoneQuat(Rotation);
		if (RotationMode == BMM_Additive)
		{	
			NewBoneTM.SetRotation(BoneQuat * NewBoneTM.GetRotation());
		}
		else
		{
			NewBoneTM.SetRotation(BoneQuat);
		}

		// Convert back to Component Space.
		FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, RotationSpace);
	}

	const TEnumAsByte<EBoneModificationMode> TranslationMode = BoneControl.TranslationMode;
	const FVector Translation = TargetTransform.GetLocation();
	const TEnumAsByte<enum EBoneControlSpace> TranslationSpace = BoneControl.TranslationSpace;
	if (TranslationMode != BMM_Ignore)
	{
		// Convert to Bone Space.
		FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, TranslationSpace);

		if (TranslationMode == BMM_Additive)
		{
			NewBoneTM.AddToTranslation(Translation);
		}
		else
		{
			NewBoneTM.SetTranslation(Translation);
		}

		// Convert back to Component Space.
		FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, TranslationSpace);
	}

	Res = FBoneTransform(BoneToModify.CachedCompactPoseIndex, NewBoneTM);
	return true;
}

void FGBWAnimNode_FootIK::EvaluateFootIKSkeletalControl(
	FCSPose<FCompactPose>& CompactPose,
	const FBoneContainer& BoneContainer,
	const FTransform& ComponentTransform)
{
	if (!MeshComp || !OwnerActor)
	{
		return;
	}

	if (UGBWFootIKFuncLib::MakeFootIKData(OwnerCharacter,DeltaTime,FootIKData,GetFootIKSettings(),CompactPose,BoneContainer))
		//&& !FootIKData.bShouldDisableFootIK)
	{
		FTransform TargetTransform;
		//PelvisBone Offset
		if (FBoneTransform Res;
			EvaluateBoneControl(
				CompactPose,
				BoneContainer,
				ComponentTransform,
				FGBWBoneControl(FootIKData.PelvisBoneName,
				FTransform(FootIKData.PelvisOffset + FVector(0.0f,0.0f,FootIKData.RootOffset.Z * FootIKData.RootAlpha)),
					EBoneModificationMode::BMM_Ignore,
					EBoneModificationMode::BMM_Ignore,
					EBoneModificationMode::BMM_Additive,
					EBoneControlSpace::BCS_ComponentSpace,
					EBoneControlSpace::BCS_ComponentSpace,
					EBoneControlSpace::BCS_WorldSpace,
					FootIKData.PelvisAlpha),
				Res,
				TargetTransform))
		{
			CompactPose.LocalBlendCSBoneTransforms({Res}, FootIKData.PelvisAlpha);
		}
	
		EvaluateFootBone(CompactPose,FootIKData.LeftFoot);
		EvaluateFootBone(CompactPose,FootIKData.RightFoot);

		
		TArray<UObject*> FootStepInterfaceObjSet;
		if (OwnerCharacter->GetClass()->ImplementsInterface(UGBWFootStepInterface::StaticClass()))
		{
			FootStepInterfaceObjSet.Add(OwnerCharacter);
		}
		for (const auto Obj : OwnerCharacter->GetComponents())
		{
			if (Obj->GetClass()->ImplementsInterface(UGBWFootStepInterface::StaticClass()))
			{
				FootStepInterfaceObjSet.Add(Obj);
			}
		}
		for (const auto StepInterfaceObjSet : FootStepInterfaceObjSet)
		{
			if (FootIKData.LeftFoot.bHasFootStepEvent)
			{
				IGBWFootStepInterface::Execute_OnFootStep(StepInterfaceObjSet,
								MeshComp,OwnerCharacter,DeltaTime,
								GetFootIKSettings().LeftFoot.BoneName,
								FootIKData.LeftFoot.FloorLocation,
								FootIKData.LeftFoot.FloorRotation,
								FootIKData.LeftFoot.FloorNormal,
								FootIKData.LeftFoot.FloorHitResult);
			}
			if (FootIKData.RightFoot.bHasFootStepEvent)
			{
				IGBWFootStepInterface::Execute_OnFootStep(StepInterfaceObjSet,
								MeshComp,OwnerCharacter,DeltaTime,
								GetFootIKSettings().RightFoot.BoneName,
								FootIKData.RightFoot.FloorLocation,
								FootIKData.RightFoot.FloorRotation,
								FootIKData.RightFoot.FloorNormal,
								FootIKData.RightFoot.FloorHitResult);
			}
		}
	}
}

void FGBWAnimNode_FootIK::EvaluateFootBone(
	FCSPose<FCompactPose>& TheCompactPose,
	FGBWFootIKFootData& FootData) const
{
	if (!MeshComp){return;}

	if (FootData.TargetLimbBoneIndex == INDEX_NONE
		|| FootData.LowerLimbBoneIndex == INDEX_NONE
		|| FootData.UpperLimbBoneIndex == INDEX_NONE){return;}

	FTransform LowerLimbCSTransform = TheCompactPose.GetComponentSpaceTransform(FootData.LowerLimbBoneIndex);
	FTransform UpperLimbCSTransform = TheCompactPose.GetComponentSpaceTransform(FootData.UpperLimbBoneIndex);
	FTransform TargetLimbCSTransform = TheCompactPose.GetComponentSpaceTransform(FootData.TargetLimbBoneIndex);

	const FTransform EffectorTransform = FTransform(
		UKismetMathLibrary::InverseTransformRotation(MeshComp->GetComponentTransform(),FootData.TargetFootBoneWorldTransform.Rotator()),
		UKismetMathLibrary::InverseTransformLocation(MeshComp->GetComponentTransform(),FootData.TargetFootBoneWorldTransform.GetLocation()));
	const FTransform JointTargetTransform = FTransform(
		UKismetMathLibrary::InverseTransformLocation(MeshComp->GetComponentTransform(),FootData.TargetKneeBoneWorldLocation));
	
	AnimationCore::SolveTwoBoneIK(UpperLimbCSTransform,LowerLimbCSTransform,TargetLimbCSTransform,
		JointTargetTransform.GetTranslation(),EffectorTransform.GetTranslation(),
		true,1.0f,1.0f);
	TargetLimbCSTransform.SetRotation(EffectorTransform.GetRotation());
	
	FootData.UpperLimbCSTransform = UKismetMathLibrary::TInterpTo(FootData.UpperLimbCSTransform,UpperLimbCSTransform,DeltaTime,60.0f);
	FootData.LowerLimbCSTransform = UKismetMathLibrary::TInterpTo(FootData.LowerLimbCSTransform,LowerLimbCSTransform,DeltaTime,50.0f);
	FootData.TargetLimbCSTransform = UKismetMathLibrary::TInterpTo(FootData.TargetLimbCSTransform,TargetLimbCSTransform,DeltaTime,90.0f);
	
	TArray<FBoneTransform> BoneTransformsRes = {};
	BoneTransformsRes.Add(FBoneTransform(FootData.UpperLimbBoneIndex,FootData.UpperLimbCSTransform));
	BoneTransformsRes.Add(FBoneTransform(FootData.LowerLimbBoneIndex,FootData.LowerLimbCSTransform));
	BoneTransformsRes.Add(FBoneTransform(FootData.TargetLimbBoneIndex,FootData.TargetLimbCSTransform));

	TheCompactPose.LocalBlendCSBoneTransforms(BoneTransformsRes,FootData.Ik_Rate);
}

void FGBWAnimNode_FootIK::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread);

	FAnimNode_AssetPlayerBase::Initialize_AnyThread(Context);

	Source.Initialize(Context);
	
	GetEvaluateGraphExposedInputs().Execute(Context);
}

void FGBWAnimNode_FootIK::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread);
	Source.CacheBones(Context);
}

void FGBWAnimNode_FootIK::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(UpdateAssetPlayer);
	Source.Update(Context.FractionalWeight(1.0f));
	GetEvaluateGraphExposedInputs().Execute(Context);
	DeltaTime = Context.GetDeltaTime();
}

void FGBWAnimNode_FootIK::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread);

	if (!MeshComp)
	{
		MeshComp = Output.AnimInstanceProxy->GetSkelMeshComponent();
	}
	if (!OwnerActor)
	{
		if (MeshComp)
		{
			OwnerActor = MeshComp->GetOwner();
		}
	}
	if (!OwnerCharacter)
	{
		if (OwnerActor)
		{
			OwnerCharacter = Cast<ACharacter>(OwnerActor);
		}
	}
	
	FPoseContext PoseTarget(Output);
	FAnimationPoseData TargetAnimationPoseData(PoseTarget);

	if (Source.GetLinkNode())
	{
		Source.Evaluate(PoseTarget);
		Output.Curve = TargetAnimationPoseData.GetCurve();
		Output.Pose = TargetAnimationPoseData.GetPose();
		Output.CustomAttributes = TargetAnimationPoseData.GetAttributes();
	}
	else
	{
		Output.ResetToRefPose();
	}

	//Bone Control
	if (Output.Pose.IsValid())
	{
		//ConvertLocalToComponentSpace
		FCSPose<FCompactPose> CompactPose;
		CompactPose.InitPose(Output.Pose);
		
		//FootIk
		EvaluateFootIKSkeletalControl(
			CompactPose,
			Output.Pose.GetBoneContainer(),
			MeshComp? MeshComp->GetComponentTransform():FTransform());

		//ConvertComponentToLocalSpace
		FCSPose<FCompactPose>::ConvertComponentPosesToLocalPoses(MoveTemp(CompactPose), Output.Pose);
	}
}

void FGBWAnimNode_FootIK::GatherDebugData(FNodeDebugData& DebugData)
{
	Source.GatherDebugData(DebugData.BranchFlow(1.f));
	
	FString DebugLine = DebugData.GetNodeName(this);
}

FGBWFootIKSettings FGBWAnimNode_FootIK::GetFootIKSettings() const
{return GET_ANIM_NODE_DATA(FGBWFootIKSettings, FootIKSettings);}

#undef LOCTEXT_NAMESPACE