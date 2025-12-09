// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "AnimNodes/GBWAnimNode_MagicPlayer.h"
#include "GBWAnimComponent.h"
#include "GBWAnimFuncLib.h"
#include "GBWPowerfulToolsFuncLib.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNotifyQueue.h"
#include "Animation/AnimSyncScope.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Engine/SkeletalMesh.h"
#include "Engine.h"

#define LOCTEXT_NAMESPACE "GBWAnimNode_MagicPlayer"

/////////////////////////////////////////////////////
// FGBWAnimMagicPlayerNode

void FGBWAnimNode_MagicPlayer::CheckAnimAsset()
{
	AnimationAssetNow = GetAnimAsset();
	GetPlayControlANS();
	if (AnimationAssetNow && AnimationAssetNow->GetClass())
	{
		if (AnimationAssetNow->GetClass()->IsChildOf(UAnimSequence::StaticClass()))
		{
			AnimSequenceNow = Cast<UAnimSequence>(AnimationAssetNow);
			AnimMontageNow = nullptr;
			BlendSpaceNow = nullptr;
		}
		else if (AnimationAssetNow->GetClass()->IsChildOf(UBlendSpace::StaticClass()))
		{
			AnimSequenceNow = nullptr;
			AnimMontageNow = nullptr;
			BlendSpaceNow = Cast<UBlendSpace>(AnimationAssetNow);
		}
		else if (AnimationAssetNow->GetClass()->IsChildOf(UAnimMontage::StaticClass()))
		{
			AnimSequenceNow = nullptr;
			AnimMontageNow = Cast<UAnimMontage>(AnimationAssetNow);
			BlendSpaceNow = nullptr;
		}
		else
		{
			AnimSequenceNow = nullptr;
			AnimMontageNow = nullptr;
			BlendSpaceNow = nullptr;
		}
	}
	else
	{
		AnimSequenceNow = nullptr;
		AnimMontageNow = nullptr;
		BlendSpaceNow = nullptr;
	}
}

bool FGBWAnimNode_MagicPlayer::IsAnimSequence() const
{
	return AnimSequenceNow != nullptr;
}

bool FGBWAnimNode_MagicPlayer::IsAnimMontage() const
{
	return AnimMontageNow != nullptr;
}

bool FGBWAnimNode_MagicPlayer::IsBlendSpace() const
{
	return BlendSpaceNow != nullptr;
}

bool FGBWAnimNode_MagicPlayer::EvaluateMagicPlayerBoneControl(FCSPose<FCompactPose>& CompactPose,
	const FBoneContainer& BoneContainer, const FTransform& ComponentTransform,
	const FMagicPlayerBoneControl& BoneControl, FBoneTransform& Res, FTransform& TargetTransform)
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
	bool bHasNewBoneTM = false;

	TargetTransform = BoneControl.TargetTransform;
	FTransform OldTargetTransform = BoneTargetTransformsCache.FindRef(BoneControl.BoneName);
	TargetTransform = UKismetMathLibrary::TInterpTo(
		OldTargetTransform,
		TargetTransform,
		DeltaTime,
		BoneControl.SmoothSpeed_Enter
	);
	
	if (BoneControl.BoneControlType == DirectAdjust)
	{
		TEnumAsByte<EBoneModificationMode> ScaleMode = BoneControl.ScaleMode;
		FVector Scale = TargetTransform.GetScale3D();
		TEnumAsByte<enum EBoneControlSpace> ScaleSpace = BoneControl.ScaleSpace;
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

		TEnumAsByte<EBoneModificationMode> RotationMode = BoneControl.RotationMode;
		FRotator Rotation = TargetTransform.GetRotation().Rotator();
		TEnumAsByte<enum EBoneControlSpace> RotationSpace = BoneControl.RotationSpace;
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

		TEnumAsByte<EBoneModificationMode> TranslationMode = BoneControl.TranslationMode;
		FVector Translation = TargetTransform.GetLocation();
		TEnumAsByte<enum EBoneControlSpace> TranslationSpace = BoneControl.TranslationSpace;
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

		bHasNewBoneTM = true;
	}
	else if (BoneControl.BoneControlType == RelativeToSpecifiedComponent)
	{
		if (TArray<USceneComponent*> TargetComponents;
					UGBWPowerfulToolsFuncLib::GetSceneComponentFromActorByTag(
					MeshComp->GetOwner(),
					TargetComponents,
					BoneControl.ComponentTag,
					BoneControl.bTryGetComponentFromChildOrAttachedActors,
					BoneControl.ChildOrAttachedActorTag
					))
		{
			FTransform TheOldTransform = MeshComp->GetSocketTransform(BoneControl.BoneName);
			FTransform TheTargetTransform = TargetComponents[0]->GetSocketTransform(
				BoneControl.SocketName,
				ERelativeTransformSpace::RTS_World);

			FTransform TheResTransform = UKismetMathLibrary::ComposeTransforms(TheTargetTransform,
				FTransform(
				TargetTransform.Rotator(),
				TargetTransform.GetTranslation()
				));
			TargetTransform = UKismetMathLibrary::TInterpTo(
				TheOldTransform,
				FTransform(TheResTransform.Rotator(),TheResTransform.GetTranslation(),TargetTransform.GetScale3D()),
				DeltaTime,
				BoneControl.SmoothSpeed_Enter
			);

			TEnumAsByte<EBoneModificationMode> ScaleMode = BoneControl.ScaleMode;
			FVector Scale = TargetTransform.GetScale3D();
			TEnumAsByte<enum EBoneControlSpace> ScaleSpace = EBoneControlSpace::BCS_WorldSpace;
			if (ScaleMode != BMM_Ignore)
			{
				// Convert to Bone Space.
				FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, ScaleSpace);

				NewBoneTM.SetScale3D(Scale);

				// Convert back to Component Space.
				FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, ScaleSpace);
			}

			TEnumAsByte<EBoneModificationMode> RotationMode = BoneControl.RotationMode;
			FRotator Rotation = TargetTransform.GetRotation().Rotator();
			TEnumAsByte<enum EBoneControlSpace> RotationSpace = EBoneControlSpace::BCS_WorldSpace;
			if (RotationMode != BMM_Ignore)
			{
				// Convert to Bone Space.
				FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, RotationSpace);

				const FQuat BoneQuat(Rotation);
				NewBoneTM.SetRotation(BoneQuat);

				// Convert back to Component Space.
				FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, RotationSpace);
			}

			TEnumAsByte<EBoneModificationMode> TranslationMode = BoneControl.TranslationMode;
			FVector Translation = TargetTransform.GetLocation();
			TEnumAsByte<enum EBoneControlSpace> TranslationSpace = EBoneControlSpace::BCS_WorldSpace;
			if (TranslationMode != BMM_Ignore)
			{
				// Convert to Bone Space.
				FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, TranslationSpace);

				NewBoneTM.SetTranslation(Translation);

				// Convert back to Component Space.
				FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, CompactPose, NewBoneTM, CompactPoseBoneToModify, TranslationSpace);
			}
			
			bHasNewBoneTM = true;
		}
	}

	if (bHasNewBoneTM)
	{
		Res = FBoneTransform(BoneToModify.CachedCompactPoseIndex, NewBoneTM);
		return true;
	}

	return false;
}

void FGBWAnimNode_MagicPlayer::EvaluateSkeletalControl(
		FCSPose<FCompactPose>& CompactPose,
		const FBoneContainer& BoneContainer,
		const FTransform& ComponentTransform)
{
	if (PlayStateNow != EMagicPlayerPlayStateType::Stopped
				|| PlayStateNow != EMagicPlayerPlayStateType::Stopping)
	{
		FMagicPlayerBoneControlParam BoneControlParam = GetBoneControlParam();

		TMap<FName, FTransform> TheBoneTargetTransformsCache;
		for (auto BoneControl : BoneControlParam.BoneControlQueue)
		{
			if (BoneControl.Target != EMagicPlayerBoneControlTargetType::LinkedActor
				&& GetPlayerType() == GetParamFromLinked)
			{
				continue;
			}
			if (BoneControl.Target == EMagicPlayerBoneControlTargetType::LinkedActor
				&& GetPlayerType() != GetParamFromLinked)
			{
				continue;
			}

			FTransform TargetTransform;
			if (FBoneTransform Res;
				EvaluateMagicPlayerBoneControl(
					CompactPose,
					BoneContainer,
					ComponentTransform,
					BoneControl,
					Res,
					TargetTransform))
			{
				TheBoneTargetTransformsCache.Add(BoneControl.BoneName, TargetTransform);
				CompactPose.LocalBlendCSBoneTransforms({Res}, BoneControl.Alpha);
			}
		}

		BoneTargetTransformsCache = TheBoneTargetTransformsCache;
	}
}

void FGBWAnimNode_MagicPlayer::UpdateAnimPlayer(const FAnimationUpdateContext& Context)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	if ((AnimationAssetNow != nullptr) && (Context.AnimInstanceProxy->GetSkeleton() == AnimationAssetNow->GetSkeleton()))
#else
	if ((AnimationAssetNow != nullptr) && (Context.AnimInstanceProxy->IsSkeletonCompatible(AnimationAssetNow->GetSkeleton())))
#endif
	{
		if (IsAnimSequence())
		{
			// HACK for 5.1.1 do allow us to fix UE-170739 without altering public API
			auto HACK_CreateTickRecordForNode = [this]( const FAnimationUpdateContext& Context, UAnimSequenceBase* Sequence, bool bLooping, float PlayRate)
			{
				// Create a tick record and push into the closest scope
				const float FinalBlendWeight = Context.GetFinalBlendWeight();

				UE::Anim::FAnimSyncGroupScope& SyncScope = Context.GetMessageChecked<UE::Anim::FAnimSyncGroupScope>();

				const EAnimGroupRole::Type SyncGroupRole = GetGroupRole();
				const FName SyncGroupName = GetGroupName();

				const FName GroupNameToUse = ((SyncGroupRole < EAnimGroupRole::TransitionLeader) || bHasBeenFullWeight) ? SyncGroupName : NAME_None;
				EAnimSyncMethod MethodToUse = GetGroupMethod();
				if(GroupNameToUse == NAME_None && MethodToUse == EAnimSyncMethod::SyncGroup)
				{
					MethodToUse = EAnimSyncMethod::DoNotSync;
				}

				const UE::Anim::FAnimSyncParams SyncParams(GroupNameToUse, SyncGroupRole, MethodToUse);
				FAnimTickRecord TickRecord(Sequence, bLooping, PlayRate, FinalBlendWeight, /*inout*/ InternalTimeAccumulator, MarkerTickRecord);
				TickRecord.GatherContextData(Context);

				TickRecord.RootMotionWeightModifier = Context.GetRootMotionWeightModifier();
				TickRecord.DeltaTimeRecord = &DeltaTimeRecord;
				
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
				
#else
				TickRecord.BlendSpace.bIsEvaluator = false;
#endif

				SyncScope.AddTickRecord(TickRecord, SyncParams, UE::Anim::FAnimSyncDebugInfo(Context));

				TRACE_ANIM_TICK_RECORD(Context, TickRecord);
			};
			
			const float CurrentPlayRate = GetPlayRate();
			const float CurrentPlayRateBasis = GetPlayRateBasis();
			InternalTimeAccumulator = FMath::Clamp(InternalTimeAccumulator, 0.f, AnimSequenceNow->GetPlayLength());
			const float AdjustedPlayRate = PlayRateScaleBiasClampState.ApplyTo(GetPlayRateScaleBiasClampConstants(), FMath::IsNearlyZero(CurrentPlayRateBasis) ? 0.f : (CurrentPlayRate / CurrentPlayRateBasis), Context.GetDeltaTime());
			HACK_CreateTickRecordForNode(Context, AnimSequenceNow, GetLoopAnimation(), AdjustedPlayRate);

#if WITH_EDITORONLY_DATA
			if (FAnimBlueprintDebugData* DebugData = Context.AnimInstanceProxy->GetAnimBlueprintDebugData())
			{
				DebugData->RecordSequencePlayer(Context.GetCurrentNodeId(), GetAccumulatedTime(), AnimSequenceNow != nullptr ? AnimSequenceNow->GetPlayLength() : 0.0f, AnimSequenceNow != nullptr ? AnimSequenceNow->GetNumberOfSampledKeys() : 0);
			}
#endif
		}
		else if (IsBlendSpace())
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
			if ((AnimationAssetNow != nullptr) && (Context.AnimInstanceProxy->GetSkeleton() == AnimationAssetNow->GetSkeleton()))
#else
			if ((AnimationAssetNow != nullptr) && (Context.AnimInstanceProxy->IsSkeletonCompatible(AnimationAssetNow->GetSkeleton())))
#endif
			{
				if (PreviousBlendSpace != BlendSpaceNow
					|| PlayStateChangeType == EGBWMagicPlayerPlayStateChangeType::NewPlay
					|| PlayStateChangeType == EGBWMagicPlayerPlayStateChangeType::StoppedToPlaying)
				{
					ReinitializeBlendSpace(true);
					DeltaTimeRecord.SetPrevious(InternalTimeAccumulator-DeltaTime);
					MarkerTickRecord.Reset();
				}

				const FVector Position = GetBlendSpacePosition();

				// Create a tick record and push into the closest scope
				UE::Anim::FAnimSyncGroupScope& SyncScope = Context.GetMessageChecked<UE::Anim::FAnimSyncGroupScope>();
				
				FAnimTickRecord TickRecord(
					BlendSpaceNow, Position, BlendSampleDataCache, BlendFilter, GetLoopAnimation(), GetPlayRate(), true, 
					true, Context.GetFinalBlendWeight(), /*inout*/ InternalTimeAccumulator, MarkerTickRecord);
				TickRecord.RootMotionWeightModifier = Context.GetRootMotionWeightModifier();
				TickRecord.DeltaTimeRecord = &DeltaTimeRecord;

				const UE::Anim::FAnimSyncParams SyncParams(GetGroupName(), GetGroupRole(), GetGroupMethod());
				TickRecord.GatherContextData(Context);

				SyncScope.AddTickRecord(TickRecord, SyncParams, UE::Anim::FAnimSyncDebugInfo(Context));

				TRACE_ANIM_TICK_RECORD(Context, TickRecord);

				PreviousBlendSpace = BlendSpaceNow;
			}
		}
	}
}

void FGBWAnimNode_MagicPlayer::UpdateAnimAsset(const FAnimationUpdateContext& Context)
{
	DeltaTime = Context.GetDeltaTime();
	CheckAnimAsset();
	
	if (!AnimComponent && GetPlayerType() == AutoGetParam)
	{
		if (Context.AnimInstanceProxy
			&& Context.AnimInstanceProxy->GetSkelMeshComponent()
			&& Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner())
		{
			AnimComponent = UGBWAnimFuncLib::GetGBWAnimComponent(Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());
		}
	}
	if (!LinkedAnimComponent && GetPlayerType() == GetParamFromLinked)
	{
		if (GetLinkedActor())
		{LinkedAnimComponent = UGBWAnimFuncLib::GetGBWAnimComponent(GetLinkedActor());}
		else if (GetAutoGetLinkedActor()
			&& Context.AnimInstanceProxy
			&& Context.AnimInstanceProxy->GetSkelMeshComponent()
			&& Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner())
		{
			AActor* TargetActor = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner();
			while (TargetActor && !LinkedAnimComponent)
			{
				TargetActor = TargetActor->GetParentActor();
				if (TargetActor)
				{
					LinkedAnimComponent = UGBWAnimFuncLib::GetGBWAnimComponent(TargetActor);
				}
			}
			TargetActor = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner();
			while (TargetActor && !LinkedAnimComponent)
			{
				TargetActor = TargetActor->GetAttachParentActor();
				if (TargetActor)
				{
					LinkedAnimComponent = UGBWAnimFuncLib::GetGBWAnimComponent(TargetActor);
				}
			}
		}
	}
	
	if (!MeshComp)
	{MeshComp = Context.AnimInstanceProxy->GetSkelMeshComponent();}
	if (!OwnerActor)
	{if (MeshComp) {OwnerActor = MeshComp->GetOwner();}}
	if (!OwnerCharacter)
	{if (OwnerActor) {OwnerCharacter = Cast<ACharacter>(OwnerActor);}}
	
	UGBWAnimComponent* RealAnimComponent = GetRealAnimComponent();
	uint32 PlayStateHashNow = 0;
	uint32 TargetPlayStateHash = GetAnimParam(false).PlayStateHash;

	if (FMagicPlayerPlayState ResState;
		GetLinkType() == Complete
		&& LinkedAnimComponent
		&& LinkedAnimComponent->GetMagicPlayerState(GetParamSlot(),ResState))
	{
		PlayStateChangeType = ResState.PlayState.PlayStateChangeType;
		PlayStateNow = ResState.PlayState.PlayStateType;
		BlendTimer = ResState.PlayState.PlayDuration;
		bIsStopped = ResState.PlayState.bIsStopped;
		InternalTimeAccumulator = ResState.PlayState.InternalTimeAccumulator;
		AbsPlayProgress = ResState.PlayState.AbsPlayProgress;
		bIsLoopReply = ResState.PlayState.bIsLoopReply;
		PlayFrames = ResState.PlayState.PlayFrames;
		BlendAlpha = ResState.PlayState.BlendAlpha;
	}
	else
	{
		PlayFrames ++;
		BlendTimer += DeltaTime;

		bool bReplay = GetReplayNow();
		
		PlayStateChangeType = NoChange;
		if (AnimationAssetNow && bIsStopped
			&& (AnimationAssetLast != AnimationAssetNow || bReplay))
		{
			PlayStateChangeType = StoppedToPlaying;
			bIsStopped = false;
			PlayStateNow = EMagicPlayerPlayStateType::Starting;
		}
		else if (!bIsStopped
			&&(!AnimationAssetNow
			|| (AnimationAssetNow
				&& AnimationAssetLast == AnimationAssetNow
				&& GetCurrentAssetTime() >= GetCurrentAssetLength()-0.01f
				&& !GetLoopAnimation())))
		{
			PlayStateChangeType = PlayingToStopped;
			bIsStopped = true;
			PlayStateNow = EMagicPlayerPlayStateType::Stopping;
		}
		else if (AnimationAssetNow
			&& (AnimationAssetLast != AnimationAssetNow
			|| (bReplay && BlendTimer >= GetReplayCodeTime())))
		{
			PlayStateChangeType = NewPlay;
			bIsStopped = false;
			PlayStateNow = EMagicPlayerPlayStateType::Starting;
		}
		else if (AnimationAssetNow
			&& AnimationAssetLast == AnimationAssetNow
			&& GetCurrentAssetTime() >= GetCurrentAssetLength()-0.01f
			&& GetLoopAnimation())
		{
			PlayStateChangeType = Replay;
			bIsStopped = false;
			PlayStateNow = EMagicPlayerPlayStateType::Starting;
		}

		if (FMagicPlayerPlayState ThePlayStateNow;
			!bIsStopped
			&& RealAnimComponent
			&& RealAnimComponent->GetMagicPlayerState(GetParamSlot(), ThePlayStateNow)
			&& ThePlayStateNow.LastUpdateFrame <= GFrameCounter - 2
			&& PlayStateChangeType != EGBWMagicPlayerPlayStateChangeType::StoppedToPlaying
			&& PlayStateChangeType != EGBWMagicPlayerPlayStateChangeType::NewPlay
			&& PlayStateChangeType != EGBWMagicPlayerPlayStateChangeType::Replay)
		{
			PlayStateChangeType = PlayingToStopped;
			bIsStopped = true;
			PlayStateNow = EMagicPlayerPlayStateType::Stopping;
		}
		
		switch (PlayStateChangeType) {
		case NoChange: break;
		case StoppedToPlaying:
			BlendTimer = 0.0f;
			bIsLoopReply = false;
			PlayFrames = 0;
			if (TargetPlayStateHash != 0)
			{PlayStateHashNow = TargetPlayStateHash;}
			else
			{PlayStateHashNow = UKismetMathLibrary::RandomInteger(100000);}
			break;
		
		case NewPlay:
			BlendTimer = 0.0f;
			bIsLoopReply = false;
			PlayFrames = 0;
			if (TargetPlayStateHash != 0)
			{PlayStateHashNow = TargetPlayStateHash;}
			else
			{PlayStateHashNow = UKismetMathLibrary::RandomInteger(100000);}
			break;

		case Replay:
			BlendTimer = 0.0f;
			bIsLoopReply = true;
			PlayFrames = 0;
			if (TargetPlayStateHash != 0)
			{PlayStateHashNow = TargetPlayStateHash;}
			else
			{PlayStateHashNow = UKismetMathLibrary::RandomInteger(100000);}
			break;
		
		case PlayingToStopped:
			BlendTimer = 0.0f;
			bIsLoopReply = false;
			PlayFrames = 0;
			if (TargetPlayStateHash != 0)
			{PlayStateHashNow = TargetPlayStateHash;}
			else
			{PlayStateHashNow = UKismetMathLibrary::RandomInteger(100000);}
			break;
		
		default: ;
		}

		//BlendAlpha = GetBlendTime() > 0.0f? BlendTimer / GetBlendTime() : 1.0f;
	
		if (PlayStateNow == EMagicPlayerPlayStateType::Starting && PlayFrames>0 && BlendAlpha >= 1.0f)
		{
			PlayStateNow = EMagicPlayerPlayStateType::Playing;
		}
		else if (PlayStateNow == EMagicPlayerPlayStateType::Stopping && PlayFrames>0 && BlendAlpha >= 1.0f)
		{
			PlayStateNow = EMagicPlayerPlayStateType::Stopped;
		}

		AbsPlayProgress = InternalTimeAccumulator/GetCurrentAssetAbsLength();
	}
	
	//RootMotionDate
	if (GetRootMotionSourceAnim() == AnimationAssetNow
		&& PlayStateNow != EMagicPlayerPlayStateType::Stopped
		&& PlayStateNow != EMagicPlayerPlayStateType::Stopping)
	{
		FTransform DeltaRootMotionNow = UGBWAnimFuncLib::GetRootMotionDataFromAsset(
			AnimationAssetNow,
			GetCurrentAssetTime(),
			DeltaTime,
			GetPlayRate(),
			GetLoopAnimation(),
			BlendSampleDataCache,
			false
		);
		
		if (GetPlayerType() == EMagicPlayerType::GetParamFromLinked)
		{
			if (LinkedAnimComponent)
			{
				LinkedAnimComponent->SetMagicPlayerRootMotionState_Imp(GetParamSlot(),DeltaRootMotionNow);
			}
		}
		else if (AnimComponent)
		{
			AnimComponent->SetMagicPlayerRootMotionState_Imp(GetParamSlot(),DeltaRootMotionNow);
		}
	}

	// Interface Tick
	if (GetTickInterfaceAnim() == AnimationAssetNow
		&& PlayStateNow != EMagicPlayerPlayStateType::Stopped
		&& PlayStateNow != EMagicPlayerPlayStateType::Stopping)
	{
		MagicPlayerInterfaceObjSet.Empty();
		if (MeshComp && MeshComp->GetOwner())
		{
			TArray<AActor*> RelevantActors;
			UGBWPowerfulToolsFuncLib::GetAllRelevantActors(MeshComp->GetOwner(),RelevantActors);

			for (const auto RelevantActor : RelevantActors)
			{
				TArray<UActorComponent*> InterfaceObj = RelevantActor->GetComponentsByInterface(UGBWMagicPlayerInterface::StaticClass());
				for (auto Obj : InterfaceObj)
				{
					MagicPlayerInterfaceObjSet.Add(Obj);
				}
			}
		}

		if (RealAnimComponent)
		{
			for (auto Element : MagicPlayerInterfaceObjSet)
			{
				if (Element && Element.GetInterface() != nullptr && Element.GetObject()->IsValidLowLevel())
				{
					Element->MagicPlayerTick(MeshComp,
						AnimationAssetNow,
						MeshComp? MeshComp->GetOwner() : nullptr,
						DeltaTime,
						GetCurrentAssetTime(),
						ANS_StartTime,
						ANS_EndTime,
						GetPlayProgress(),
						GetBlendSpacePosition(),
						RealAnimComponent->GetMagicPlayerRootMotion(GetParamSlot()),
						PlayStateChangeType == EGBWMagicPlayerPlayStateChangeType::NewPlay
						|| PlayStateChangeType == EGBWMagicPlayerPlayStateChangeType::StoppedToPlaying,
						GetParamSlot());
				}
			}
		}
	}

	switch (PlayStateChangeType) {
		case NoChange:
			UpdateSyncValue();
			break;
		case StoppedToPlaying:
			InternalTimeAccumulator = GetStartInternalTimeAccumulator();
			SnapshotPose();
			break;
		
		case NewPlay:
			InternalTimeAccumulator = GetStartInternalTimeAccumulator();
			SnapshotPose();
			break;

		case Replay:
			InternalTimeAccumulator = GetStartInternalTimeAccumulator();
			SnapshotPose();
			break;
		
		case PlayingToStopped:
			SnapshotPose();
			break;
		default: ;
	}

	if (PlayStateChangeType == EGBWMagicPlayerPlayStateChangeType::Replay)
	{LoopCount ++;}
	else if (PlayStateChangeType != EGBWMagicPlayerPlayStateChangeType::NoChange)
	{LoopCount = 0;}

	BlendTime_Last = GetBlendTime();
	bIsLoopAnim_Last = GetLoopAnimation();
	InternalTimeAccumulator_Last = InternalTimeAccumulator;
	AnimationAssetLast = AnimationAssetNow;
	LastBlendSpacePosition = GetBSAxisValue();
	//InternalTimeAccumulator = GetAccumulatedTime();
	LastAssetTime = GetCurrentAssetTime();

	BlendAlpha = 1.0f;
	for (int i=0; i < BlendInfoSet.Num(); i++)
	{
		if (!BlendInfoSet[i].CheckUpdate(DeltaTime))
		{
			BlendInfoSet.RemoveAt(i);
			i--;
		}
		else
		{
			if (BlendInfoSet[i].GetBlendAlpha() < BlendAlpha)
			{
				BlendAlpha = BlendInfoSet[i].GetBlendAlpha();
			}
		}
	}
	
	//Update Anim State
	if (RealAnimComponent && GetMainStateAnim() == AnimationAssetNow)
	{
		RealAnimComponent->SetMagicPlayerState_Imp(
			GetParamSlot()
			,FMagicPlayerPlayState(
				PlayStateHashNow,
				PlayStateNow,
				PlayStateChangeType,
				BlendTimer,
				bIsStopped,
				InternalTimeAccumulator,
				AbsPlayProgress,
				bIsLoopReply,
				PlayFrames,
				BlendAlpha,
				BlendSampleDataCache,
				GetAnimParam(false)));
	}
	
	UpdateAnimPlayer(Context);
}

void FGBWAnimNode_MagicPlayer::GetPlayControlANS()
{
	GetAnimPlayControlANS(
	ANS_PlayControl,
	ANS_StartTime,
	ANS_EndTime,
	ANS_StartTimeRate,
	ANS_EndTimeRate);
}

bool FGBWAnimNode_MagicPlayer::SnapshotPose()
{
	const float TheBlendTime = GetBlendTime();
	/*UE_LOG(LogTemp, Warning, TEXT("%d  SnapshotPose4444  %f  %f  %s  %s  %s"),
			static_cast<int>(GFrameCounter),
			TheBlendTime,
			BlendInfoSet.Num()>0? BlendInfoSet[BlendInfoSet.Num()-1].BlendTimer : -1.0f,
			*(StaticEnum<EGBWMagicPlayerPlayStateChangeType>()->GetNameStringByIndex( ( int32 ) PlayStateChangeType)),
			*(AnimationAssetLast ? AnimationAssetLast->GetName() : TEXT("NULL")),
			*(LastPoseSnapshot.bIsValid? FString("True"):FString("False"))
			);*/
	bool bSuccess = false;
	
	/*if (BlendInfoSet.Num()>0 &&  BlendInfoSet[BlendInfoSet.Num()-1].BlendTimer <= 0.05f)
	{
		bSuccess = true;
		BlendInfoSet[BlendInfoSet.Num()-1] = 
				FMagicPlayerBlendInfo(
					AnimationAssetLast,
					TheBlendTime,
					bIsLoopAnim_Last,
					InternalTimeAccumulator_Last,
					BlendSampleDataCache,
					PrePoseSnapshot);
		/*UE_LOG(LogTemp, Warning, TEXT("%d  SnapshotPose111  %f  %s  %s -> %s"),
			static_cast<int>(GFrameCounter),
			TheBlendTime,
			*(StaticEnum<EGBWMagicPlayerPlayStateChangeType>()->GetNameStringByIndex( ( int32 ) PlayStateChangeType)),
			*(BlendInfoSet[BlendInfoSet.Num()-1].AnimationAsset? BlendInfoSet[BlendInfoSet.Num()-1].AnimationAsset->GetName() : TEXT("NULL")),
			*(AnimationAssetNow ? AnimationAssetNow->GetName() : TEXT("NULL")));#1#
	}
	else*/ if (TheBlendTime > 0.0f
		&& (AnimationAssetLast || LastPoseSnapshot.bIsValid))
	{
		PrePoseSnapshot = LastPoseSnapshot;
		if (PlayStateChangeType == EGBWMagicPlayerPlayStateChangeType::StoppedToPlaying)
		{
			if (PlayFrames>0)
			{
				BlendInfoSet.Add(
				FMagicPlayerBlendInfo(
					nullptr,
					TheBlendTime,
					bIsLoopAnim_Last,
					InternalTimeAccumulator_Last,
					BlendSampleDataCache,
					PrePoseSnapshot));
				bSuccess = true;
				/*UE_LOG(LogTemp, Warning, TEXT("%d  SnapshotPose222  %f  %s  %s -> %s"),
				static_cast<int>(GFrameCounter),
				TheBlendTime,
				*(StaticEnum<EGBWMagicPlayerPlayStateChangeType>()->GetNameStringByIndex( ( int32 ) PlayStateChangeType)),
				*(BlendInfoSet[BlendInfoSet.Num()-1].AnimationAsset? BlendInfoSet[BlendInfoSet.Num()-1].AnimationAsset->GetName() : TEXT("NULL")),
				*(AnimationAssetNow ? AnimationAssetNow->GetName() : TEXT("NULL")));*/
			}
			else
			{
				if (!BlendInfoSet.IsEmpty() && BlendInfoSet.Last().BlendFrames <= 1)
				{
					BlendInfoSet[BlendInfoSet.Num()-1].BlendTime = TheBlendTime;
					bSuccess = true;
				}
			}
		}
		else
		{
			BlendInfoSet.Add(
				FMagicPlayerBlendInfo(
					AnimationAssetLast,
					TheBlendTime,
					bIsLoopAnim_Last,
					InternalTimeAccumulator_Last,
					BlendSampleDataCache,
					PrePoseSnapshot));
			bSuccess = true;
			/*UE_LOG(LogTemp, Warning, TEXT("%d  SnapshotPose222  %f  %s  %s -> %s"),
			static_cast<int>(GFrameCounter),
			TheBlendTime,
			*(StaticEnum<EGBWMagicPlayerPlayStateChangeType>()->GetNameStringByIndex( ( int32 ) PlayStateChangeType)),
			*(BlendInfoSet[BlendInfoSet.Num()-1].AnimationAsset? BlendInfoSet[BlendInfoSet.Num()-1].AnimationAsset->GetName() : TEXT("NULL")),
			*(AnimationAssetNow ? AnimationAssetNow->GetName() : TEXT("NULL")));*/
		}
		
		for (int i =0; i<BlendInfoSet.Num(); i++)
		{
			if (const float Delta = BlendInfoSet[i].BlendTime - BlendInfoSet[i].BlendTimer; Delta > TheBlendTime)
			{
				BlendInfoSet[i].BlendTime = TheBlendTime / (1- BlendInfoSet[i].BlendAlpha);
				BlendInfoSet[i].BlendTimer = BlendInfoSet[i].BlendTime * BlendInfoSet[i].BlendAlpha;
			}
		}
	}
	
	return bSuccess;
}

float FGBWAnimNode_MagicPlayer::GetCurrentAssetTime() const
{
	if (IsAnimSequence())
	{
		return InternalTimeAccumulator;
	}
	if (IsBlendSpace() && PlayStateChangeType==NoChange)
	{
		if(const FBlendSampleData* HighestWeightedSample = GetHighestWeightedSample())
		{
			return HighestWeightedSample->Time;
		}
	}
	
	return InternalTimeAccumulator;
}

float FGBWAnimNode_MagicPlayer::GetCurrentAssetTimePlayRateAdjusted() const
{
	if (IsAnimSequence())
	{
		const UAnimSequence* CurrentSequence = AnimSequenceNow;
		const float SequencePlayRate = (CurrentSequence ? CurrentSequence->RateScale : 1.f);
		const float CurrentPlayRate = GetPlayRate();
		const float CurrentPlayRateBasis = GetPlayRateBasis();

		const float AdjustedPlayRate = PlayRateScaleBiasClampState.ApplyTo(GetPlayRateScaleBiasClampConstants(), FMath::IsNearlyZero(CurrentPlayRateBasis) ? 0.f : (CurrentPlayRate / CurrentPlayRateBasis));
		const float EffectivePlayRate = SequencePlayRate * AdjustedPlayRate;
		return (EffectivePlayRate < 0.0f) ? GetCurrentAssetLength() - InternalTimeAccumulator : InternalTimeAccumulator;
	}
	if (IsBlendSpace())
	{
		const float Length = GetCurrentAssetLength();
		return GetPlayRate() < 0.0f ? Length - InternalTimeAccumulator * Length : Length * InternalTimeAccumulator;
	}

	return 0.0f;
}

float FGBWAnimNode_MagicPlayer::GetCurrentAssetLength() const
{
	float Res = 0.0f;
	
	if (IsAnimSequence())
	{
		Res = AnimSequenceNow->GetPlayLength();

		if (GetAnimParam(false).EndPosition > GetAnimParam(false).StartPosition
			&& GetAnimParam(false).EndPosition > 0.0f)
		{
			Res = FMath::Min<float>(GetAnimParam(false).EndPosition, Res);
		}
	}
	else if (IsBlendSpace())
	{
		if(const FBlendSampleData* HighestWeightedSample = GetHighestWeightedSample())
		{
			if (BlendSpaceNow != nullptr
				&& BlendSpaceNow->GetBlendSamples().IsValidIndex(HighestWeightedSample->SampleDataIndex))
			{
				const FBlendSample& Sample = BlendSpaceNow->GetBlendSample(HighestWeightedSample->SampleDataIndex);
				Res = Sample.Animation->GetPlayLength();

				if (GetAnimParam(false).EndPosition > GetAnimParam(false).StartPosition
					&& GetAnimParam(false).EndPosition > 0.0f)
				{
					Res = Sample.Animation->GetPlayLength() * GetEndTimeRate();
				}
			}
		}
	}

	return Res;
}

UAnimationAsset* FGBWAnimNode_MagicPlayer::GetAnimAsset() const
{
	UAnimationAsset* AnimAsset = nullptr;
	
	switch (GetPlayerType()) {
	case AutoGetParam:
		if (AnimComponent)
		{
			TArray<FMagicPlayerLinkedAnim> LinkedAnimAsset;
			bool bReplayNow;
			AnimComponent->GetMagicPlayer(GetParamSlot(),AnimAsset,LinkedAnimAsset,bReplayNow);
		}
		break;
	case GetParamFromLinked:
		if (MeshComp && LinkedAnimComponent)
		{
			FMagicPlayerParam Res;
			LinkedAnimComponent->GetMagicPlayerLinked(GetParamSlot(), MeshComp->GetSkeletalMeshAsset(),Res);
			AnimAsset = Res.AnimAsset;
		}
		break;
	case DirectSetParam:
		FMagicPlayerParam Res;
		Res = GET_ANIM_NODE_DATA(FMagicPlayerParam, AnimParam);
		AnimAsset = Res.AnimAsset;
		break;
	}
	return AnimAsset;
}

float FGBWAnimNode_MagicPlayer::GetAccumulatedTime() const
{
	if (ANS_PlayControl)
	{
		bool bIsGet = false;
		float TheInternalTimeAccumulator = InternalTimeAccumulator;
		ANS_PlayControl->GetInternalTimeAccumulator(MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			ANS_EndTime - ANS_StartTime,
			InternalTimeAccumulator,
			LoopCount,
			bIsGet,
			TheInternalTimeAccumulator);

		if (bIsGet)
		{
			if (TheInternalTimeAccumulator > GetCurrentAssetAbsLength())
			{
				TheInternalTimeAccumulator = GetCurrentAssetAbsLength();
			}
			return TheInternalTimeAccumulator;
		}
	}
	return FAnimNode_AssetPlayerBase::GetAccumulatedTime();
}

void FGBWAnimNode_MagicPlayer::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread);

	FAnimNode_AssetPlayerBase::Initialize_AnyThread(Context);

	Source.Initialize(Context);
	
	GetEvaluateGraphExposedInputs().Execute(Context);

	BlendTimer = 0.0f;
	InternalTimeAccumulator = GetStartInternalTimeAccumulator();
	PlayRateScaleBiasClampState.Reinitialize();

	UpdateAnimAsset(Context.AnimInstanceProxy);
}

void FGBWAnimNode_MagicPlayer::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread);
	Source.CacheBones(Context);
}

void FGBWAnimNode_MagicPlayer::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(UpdateAssetPlayer);
	
	if (GetAlwaysUpdateSourcePose()
		|| PlayStateNow == EMagicPlayerPlayStateType::Stopped
		|| PlayStateNow == EMagicPlayerPlayStateType::Stopping)
	{Source.Update(Context.FractionalWeight(1.0f));}

	GetEvaluateGraphExposedInputs().Execute(Context);
	
	UpdateAnimAsset(Context);
}

void FGBWAnimNode_MagicPlayer::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread);
	
	bool bIsHasTargetPose = true;
	FPoseContext PoseTarget(Output.AnimInstanceProxy);
	FAnimationPoseData TargetAnimationPoseData(PoseTarget);
	bIsHasTargetPose = EvaluateTargetPose(PoseTarget, Output.AnimInstanceProxy);
	
	bool bIsHasPrePose = true;
	FPoseContext PoseSnapshot(Output.AnimInstanceProxy);
	FAnimationPoseData PreAnimationPoseData(PoseSnapshot);
	bIsHasPrePose = EvaluatePrePose(PoseSnapshot, Output.AnimInstanceProxy);
	
	FAnimationPoseData AnimationPoseData(Output);
	if (!bIsHasTargetPose)
	{
		if (bIsHasPrePose)
		{
			Output.Curve = PreAnimationPoseData.GetCurve();
			Output.Pose = PreAnimationPoseData.GetPose();
			Output.CustomAttributes = PreAnimationPoseData.GetAttributes();
		}
		else
		{
			Output.ResetToRefPose();
		}
	}
	else if (bIsHasPrePose)
	{
		FAnimationRuntime::BlendTwoPosesTogether(
			PreAnimationPoseData,
			TargetAnimationPoseData,
			FMath::Clamp<float>(1.0f-BlendAlpha,0.0f,1.0f), 
			/*out*/AnimationPoseData);
	}
	else
	{
		Output.Curve = TargetAnimationPoseData.GetCurve();
		Output.Pose = TargetAnimationPoseData.GetPose();
		Output.CustomAttributes = TargetAnimationPoseData.GetAttributes();
	}

	//Bone Control
	if (Output.Pose.IsValid())
	{
		//ConvertLocalToComponentSpace
		FCSPose<FCompactPose> CompactPose;
		CompactPose.InitPose(Output.Pose);

		//Bone Control
		EvaluateSkeletalControl(
				CompactPose,
				Output.Pose.GetBoneContainer(),
				Output.AnimInstanceProxy->GetComponentTransform());

		//ConvertComponentToLocalSpace
		FCSPose<FCompactPose>::ConvertComponentPosesToLocalPoses(MoveTemp(CompactPose), Output.Pose);
	}

	//Pose Smooth
	if (LastPoseSnapshot.bIsValid && GetPoseSmoothSpeed() > 0.0f)
	{
		FPoseSnapshot TargetPoseSnapshot;
		FCSPose<FCompactPose> CompactPoseTarget;
		CompactPoseTarget.InitPose(Output.Pose);
		MakeSnapshotPose(TargetPoseSnapshot, CompactPoseTarget, Output.Pose.GetBoneContainer());

		if (LastPoseSnapshot.LocalTransforms.Num() == TargetPoseSnapshot.LocalTransforms.Num())
		{
			for (int i=0; i<TargetPoseSnapshot.LocalTransforms.Num(); i++)
			{
				TargetPoseSnapshot.LocalTransforms[i] = UKismetMathLibrary::TInterpTo(
				LastPoseSnapshot.LocalTransforms[i],
					TargetPoseSnapshot.LocalTransforms[i],
					DeltaTime,
					GetPoseSmoothSpeed()
				);
			}
		}

		ApplyPose(TargetPoseSnapshot, Output.Pose);
	}

	FCSPose<FCompactPose> CompactPoseToSave;
	CompactPoseToSave.InitPose(Output.Pose);
	MakeSnapshotPose(LastPoseSnapshot, CompactPoseToSave, Output.Pose.GetBoneContainer());
	
	DebugStateToLog(bIsHasPrePose,bIsHasTargetPose);
}

void FGBWAnimNode_MagicPlayer::GatherDebugData(FNodeDebugData& DebugData)
{
	if (GetAlwaysUpdateSourcePose())
	{
		Source.GatherDebugData(DebugData.BranchFlow(1.f));
	}
	
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += FString::Printf(TEXT("('%s' Play Time: %.3f)"), GetAnimAsset() ? *GetAnimAsset()->GetName() : TEXT("NULL"), InternalTimeAccumulator);
	DebugData.AddDebugItem(DebugLine, true);
}

float FGBWAnimNode_MagicPlayer::GetCurrentAssetAbsLength() const
{
	float Res = 0.0f;
	
	if (IsAnimSequence())
	{
		Res = AnimSequenceNow->GetPlayLength();
	}
	else if (IsBlendSpace())
	{
		Res = 1.0f;
	}

	return Res;
}

FVector FGBWAnimNode_MagicPlayer::GetBlendSpacePosition() const
{
	return LastBlendSpacePosition;
}

float FGBWAnimNode_MagicPlayer::GetBlendTimeWhenReplay() const
{return bIsLoopReply? GetAnimParam(true).LoopBlendTime : GetAnimParam(true).BlendTime;}

float FGBWAnimNode_MagicPlayer::GetBlendTimeWhenStopping()
{
	UGBWANS_AnimPlayControl_Base* PlayControl = nullptr;
	float StartTime = 0.0f;
	float EndTime = 0.0f;
	float StartTimeRate = 0.0f;
	float EndTimeRate = 0.0f;

	GetAnimPlayControlANS(
		PlayControl,
		StartTime,
		EndTime,
		StartTimeRate,
		EndTimeRate);

	if (PlayControl)
	{
		bool bIsGet = false;
		float StopBlendTime = 0.0f;
		PlayControl->GetStopBlendTime(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			EndTime - StartTime,
			LoopCount,
			bIsGet,
			StopBlendTime
		);

		if (bIsGet)
		{
			BlendTimeWhenStopping = StopBlendTime;
		}
	}
	
	return BlendTimeWhenStopping;
}

float FGBWAnimNode_MagicPlayer::GetBlendTime()
{
	float BlendTime;
	if (PlayStateNow == EMagicPlayerPlayStateType::Stopping)
	{
		BlendTime = GetBlendTimeWhenStopping();
	}
	/*else if (PlayStateNow == EMagicPlayerPlayStateType::Stopped)
	{
		BlendTime = 0.0f;
	}*/
	else
	{
		BlendTime = GetBlendTimeWhenReplay();	
	}

	return BlendTime;
}

bool FGBWAnimNode_MagicPlayer::GetAlwaysUpdateSourcePose() const
{return GET_ANIM_NODE_DATA(bool, bAlwaysUpdateSourcePose);}

bool FGBWAnimNode_MagicPlayer::GetReplayNow() const
{return GetAnimParam(false).bReplayNow;}

float FGBWAnimNode_MagicPlayer::GetReplayCodeTime() const
{return GetAnimParam(false).ReplayCodeTime;}

FMagicPlayerParam FGBWAnimNode_MagicPlayer::GetAnimParam(bool bForceUpdate) const
{
	if (GetPlayerType() == DirectSetParam)
	{
		return GET_ANIM_NODE_DATA(FMagicPlayerParam, AnimParam);
	}
	
	FMagicPlayerParam Res;
	Res.AnimAsset = GetAnimAsset();

	UGBWANS_AnimPlayControl_Base* PlayControl = nullptr;
	float StartTime = 0.0f;
	float EndTime = 0.0f;
	float StartTimeRate = 0.0f;
	float EndTimeRate = 0.0f;

	const UGBWAnimComponent* RealAnimComponent = GetRealAnimComponent();
	FMagicPlayerPlayState ThePlayStateNow = FMagicPlayerPlayState();
	bool bHasPlayStateNow = false;
	if (RealAnimComponent)
	{
		UAnimationAsset* AnimAsset;
		TArray<FMagicPlayerLinkedAnim> LinkedAnimAsset;
		bool bReplayNow;
		RealAnimComponent->GetMagicPlayer(GetParamSlot(),AnimAsset,LinkedAnimAsset,bReplayNow);
		Res.LinkedAnimAsset = LinkedAnimAsset;
		Res.bReplayNow = bReplayNow;

		bHasPlayStateNow = RealAnimComponent->GetMagicPlayerState(GetParamSlot(), ThePlayStateNow);
	}

	if (ANS_PlayControl && !bForceUpdate)
	{
		PlayControl = ANS_PlayControl;
		StartTime = ANS_StartTime;
		EndTime = ANS_EndTime;
		StartTimeRate = ANS_StartTimeRate;
		EndTimeRate = ANS_EndTimeRate;
	}
	else
	{
		GetAnimPlayControlANS(
		PlayControl,
		StartTime,
		EndTime,
		StartTimeRate,
		EndTimeRate);
	}

	bool bIsGetStartAndEndTime = false;
	
	bool bIsGetAxis = false;
	float Horizontal = 0.0f;
	float Vertical = 0.0f;

	bool bIsGetPlayRate = false;
	float PlayRate = 1.0f;

	bool bIsGetLoopAnimation = false;
	bool LoopAnimation = false;

	bool bIsGetBlendTime= false;
	float BlendTime = 0.0f;
	float LoopBlendTime = 0.0f;

	bool bIsGetBoneControl= false;
	FMagicPlayerBoneControlParam BoneControlParam = FMagicPlayerBoneControlParam();

	bool bIsGetReplay = false;
	bool bReplayNow = false;
	uint32 ThePlayStateHash = 0;

	if (UGBWAnimMetaData_BSAxisControl* BSAxisControl = GetBSAxisControl())
	{
		BSAxisControl->GetBSAxisValue(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			GetBlendSpacePosition().X,
			GetBlendSpacePosition().Y,
			LoopCount,
			bIsGetAxis,
			Horizontal,
			Vertical);
	}
	
	if (PlayControl)
	{
		bIsGetStartAndEndTime = true;
		
		PlayControl->GetPlayRate(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			EndTime - StartTime,
			LoopCount,
			bIsGetPlayRate,
			PlayRate
		);
		
		PlayControl->GetLoopAnimation(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			EndTime - StartTime,
			LoopCount,
			bIsGetLoopAnimation,
			LoopAnimation
		);
		
		PlayControl->GetBlendTime(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			EndTime - StartTime,
			LoopCount,
			bIsGetBlendTime,
			BlendTime,
			LoopBlendTime
		);
		
		PlayControl->GetBoneControl(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			EndTime - StartTime,
			LoopCount,
			bIsGetBoneControl,
			BoneControlParam
		);

		PlayControl->GetReplayNow(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			EndTime - StartTime,
			LoopCount,
			bIsGetReplay,
			bReplayNow,
			ThePlayStateHash);
	}

	if (bIsGetStartAndEndTime)
	{
		Res.StartPosition = StartTime;
		Res.EndPosition = EndTime;
	}
	else if (bHasPlayStateNow)
	{
		Res.StartPosition = ThePlayStateNow.PlayerParamCache.StartPosition;
		Res.EndPosition = ThePlayStateNow.PlayerParamCache.EndPosition;
	}
	
	if (bIsGetAxis)
	{
		Res.Horizontal = Horizontal;
		Res.Vertical = Vertical;
	}
	else if (bHasPlayStateNow)
	{
		Res.Horizontal = ThePlayStateNow.PlayerParamCache.Horizontal;
		Res.Vertical = ThePlayStateNow.PlayerParamCache.Vertical;
	}
	
	if (bIsGetPlayRate)
	{
		Res.PlayRate = PlayRate;
	}
	else if (bHasPlayStateNow)
	{
		Res.PlayRate = ThePlayStateNow.PlayerParamCache.PlayRate;
	}
	
	if (bIsGetLoopAnimation)
	{
		Res.bLoopAnimation = LoopAnimation;
	}
	else if (bHasPlayStateNow)
	{
		Res.bLoopAnimation = ThePlayStateNow.PlayerParamCache.bLoopAnimation;
	}
	
	if (bIsGetBlendTime)
	{
		Res.BlendTime = BlendTime;
		Res.LoopBlendTime = LoopBlendTime;
	}
	else if (bHasPlayStateNow)
	{
		Res.BlendTime = ThePlayStateNow.PlayerParamCache.BlendTime;
		Res.LoopBlendTime = ThePlayStateNow.PlayerParamCache.LoopBlendTime;
	}
	
	if (bIsGetBoneControl)
	{
		Res.BoneControlParam = BoneControlParam;
	}
	else if (bHasPlayStateNow)
	{
		if (GetMainStateAnim() == AnimationAssetNow)
		{
			Res.BoneControlParam = FMagicPlayerBoneControlParam();
		}
		else
		{
			Res.BoneControlParam = ThePlayStateNow.PlayerParamCache.BoneControlParam;
		}
	}

	if (bIsGetReplay)
	{
		Res.bReplayNow = bReplayNow;
		Res.PlayStateHash = ThePlayStateHash;
	}
	else if (bHasPlayStateNow)
	{
		Res.PlayStateHash = 0;
	}
	
	return Res;
}

EMagicPlayerType FGBWAnimNode_MagicPlayer::GetPlayerType() const
{return GET_ANIM_NODE_DATA(TEnumAsByte<EMagicPlayerType>, Type);}

AActor* FGBWAnimNode_MagicPlayer::GetLinkedActor() const
{return GET_ANIM_NODE_DATA(AActor*, LinkedActor);}

bool FGBWAnimNode_MagicPlayer::GetAutoGetLinkedActor() const
{return GET_ANIM_NODE_DATA(bool, AutoGetLinkedActor);}

FName FGBWAnimNode_MagicPlayer::GetParamSlot() const
{return GET_ANIM_NODE_DATA(FName, ParamSlot);}

bool FGBWAnimNode_MagicPlayer::GetIsDebug() const
{
	if (!GET_ANIM_NODE_DATA(bool, bDebugPlayStateToLog))
	{
		return false;
	}
	
	return true;
}

float FGBWAnimNode_MagicPlayer::GetPlayProgress() const
{
	const float Length = GetCurrentAssetAbsLength();
	const float Start = Length * GetStartTimeRate();
	const float End = Length * GetEndTimeRate();
	float TimeNow = InternalTimeAccumulator;
	switch (PlayStateChangeType) {
	case NoChange:
		break;
	case StoppedToPlaying:
		TimeNow = GetStartInternalTimeAccumulator();
		break;
		
	case NewPlay:
		TimeNow = GetStartInternalTimeAccumulator();
		break;

	case Replay:
		TimeNow = GetStartInternalTimeAccumulator();
		break;
		
	case PlayingToStopped:
		break;
	default: ;
	}
	const float Res = (End - Start) > 0.0f ? (TimeNow - Start) / (End - Start) : 0.0f;
	return Res;
}

FMagicPlayerBoneControlParam FGBWAnimNode_MagicPlayer::GetBoneControlParam() const
{  return GetAnimParam(false).BoneControlParam; }

UAnimationAsset* FGBWAnimNode_MagicPlayer::GetRootMotionSourceAnim() const
{
	if (GetPlayerType() == GetParamFromLinked)
	{
		if (LinkedAnimComponent)
		{
			return LinkedAnimComponent->GetMagicPlayerRootMotionSourceAnimAsset(GetParamSlot());
		}
	}
	else if (GetPlayerType() == AutoGetParam)
	{
		if (AnimComponent)
		{
			return AnimComponent->GetMagicPlayerRootMotionSourceAnimAsset(GetParamSlot());
		}
	}
	else
	{
		return AnimationAssetNow;
	}
	
	return nullptr;
}

void FGBWAnimNode_MagicPlayer::GetAnimPlayControlANS(
	UGBWANS_AnimPlayControl_Base*& ANS,
	float& StartTime,
	float& EndTime,
	float& StartTimeRate,
	float& EndTimeRate) const
{
	ANS = nullptr;
	StartTime = 0.0f;
	EndTime = 0.0f;
	StartTimeRate = 0.0f;
	EndTimeRate = 0.0f;
	
	if (!AnimationAssetNow)
	{
		return;
	}
	
	//Get Anim NotifyState
	if (IsAnimMontage() || IsAnimSequence())
	{
		if (UAnimSequenceBase* A = Cast<UAnimSequenceBase>(AnimationAssetNow))
		{
			for (const auto Notify : A->Notifies)
			{
				if (Notify.NotifyStateClass
					&& Notify.NotifyStateClass->GetClass()->IsChildOf(UGBWANS_AnimPlayControl_Base::StaticClass()))
				{
					ANS = Cast<UGBWANS_AnimPlayControl_Base>(Notify.NotifyStateClass);

					float TheStartTime = 0.0f;
					float TheEndTime = 0.0f;
					bool bIsGetPlayTime = false;
					ANS->GetPlayTime(MeshComp,AnimationAssetNow,OwnerActor,bIsGetPlayTime,TheStartTime,TheEndTime);
					if (bIsGetPlayTime)
					{
						StartTime = TheStartTime;
						EndTime = TheEndTime;
					}
					else
					{
						StartTime = Notify.GetTriggerTime();
						EndTime = Notify.GetEndTriggerTime();
					}
					StartTimeRate = StartTime / AnimationAssetNow->GetPlayLength();
					EndTimeRate = EndTime / AnimationAssetNow->GetPlayLength();
					
					break;
				}
			}
		}
	}
	else if (IsBlendSpace())
	{
		if (const UBlendSpace* B = Cast<UBlendSpace>(AnimationAssetNow))
		{
			float MinDis = -1.0f;
			for (const auto BSSample : B->GetBlendSamples())
			{
				for (const auto Notify : BSSample.Animation->Notifies)
				{
					if (Notify.NotifyStateClass
					&& Notify.NotifyStateClass->GetClass()->IsChildOf(UGBWANS_AnimPlayControl_Base::StaticClass()))
					{
						if (const float Dis = (GetBlendSpacePosition() - BSSample.SampleValue).Length();
							MinDis < 0.0f || MinDis >= Dis)
						{
							MinDis = Dis;
							
							ANS = Cast<UGBWANS_AnimPlayControl_Base>(Notify.NotifyStateClass);

							float TheStartTime = 0.0f;
							float TheEndTime = 0.0f;
							bool bIsGetPlayTime = false;
							ANS->GetPlayTime(MeshComp,AnimationAssetNow,OwnerActor,bIsGetPlayTime,TheStartTime,TheEndTime);
							if (bIsGetPlayTime)
							{
								StartTime = TheStartTime;
								EndTime = TheEndTime;
							}
							else
							{
								StartTime = Notify.GetTriggerTime();
								EndTime = Notify.GetEndTriggerTime();
							}
							StartTimeRate = StartTime / BSSample.Animation->GetPlayLength();
							EndTimeRate = EndTime / BSSample.Animation->GetPlayLength();
						}
					}
				}
			}
		}
	}
}

EMagicPlayerLinkType FGBWAnimNode_MagicPlayer::GetLinkType() const
{
	FMagicPlayerLinkedAnim Res = FMagicPlayerLinkedAnim();
	if (GetPlayerType() == EMagicPlayerType::GetParamFromLinked
		&& MeshComp
		&& LinkedAnimComponent
		&& LinkedAnimComponent->GetMagicPlayerLinkedAnimSetting(GetParamSlot(), MeshComp->GetSkeletalMeshAsset(),Res))
	{
		return Res.LinkType;
	}

	return EMagicPlayerLinkType::NoLink;
}

UAnimationAsset* FGBWAnimNode_MagicPlayer::GetTickInterfaceAnim() const
{
	if (GetPlayerType() == GetParamFromLinked)
	{
		if (LinkedAnimComponent)
		{
			return LinkedAnimComponent->GetMagicPlayerTickInterfaceAnimAsset(GetParamSlot());
		}
	}
	else if (GetPlayerType() == AutoGetParam)
	{
		if (AnimComponent)
		{
			return AnimComponent->GetMagicPlayerTickInterfaceAnimAsset(GetParamSlot());
		}
	}
	else
	{
		return AnimationAssetNow;
	}
	
	return nullptr;
}

UAnimationAsset* FGBWAnimNode_MagicPlayer::GetMainStateAnim() const
{
	if (GetPlayerType() == GetParamFromLinked)
	{
		if (LinkedAnimComponent)
		{
			return LinkedAnimComponent->GetMagicPlayerMainStateAnimAsset(GetParamSlot());
		}
	}
	else if (GetPlayerType() == AutoGetParam)
	{
		if (AnimComponent)
		{
			return AnimComponent->GetMagicPlayerMainStateAnimAsset(GetParamSlot());
		}
	}
	else
	{
		return AnimationAssetNow;
	}
	
	return nullptr;
}

UGBWAnimComponent* FGBWAnimNode_MagicPlayer::GetRealAnimComponent() const
{
	UGBWAnimComponent* RealAnimComponent = nullptr;
	if (GetPlayerType() == GetParamFromLinked)
	{
		RealAnimComponent = LinkedAnimComponent;
	}
	else
	{
		RealAnimComponent = AnimComponent;
	}
	return RealAnimComponent;
}

FName FGBWAnimNode_MagicPlayer::GetSyncCurveName() const
{return GET_ANIM_NODE_DATA(FName, SyncCurveName);}

bool FGBWAnimNode_MagicPlayer::GetIsStartFromSync() const
{
	UGBWANS_AnimPlayControl_Base* PlayControl = nullptr;
	float StartTime = 0.0f;
	float EndTime = 0.0f;
	float StartTimeRate = 0.0f;
	float EndTimeRate = 0.0f;
	if (ANS_PlayControl)
	{
		PlayControl = ANS_PlayControl;
		StartTime = ANS_StartTime;
		EndTime = ANS_EndTime;
		StartTimeRate = ANS_StartTimeRate;
		EndTimeRate = ANS_EndTimeRate;
	}
	else
	{
		GetAnimPlayControlANS(
		PlayControl,
		StartTime,
		EndTime,
		StartTimeRate,
		EndTimeRate);
	}

	if (PlayControl)
	{
		bool bIsGet = false;
		bool bIsStartFromSync = false;
		PlayControl->GetIsStartFromSync(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			ANS_EndTime - ANS_StartTime,
			LoopCount,
			bIsGet,
			bIsStartFromSync);
		if (bIsGet)
		{
			return bIsStartFromSync;
		};
	}

	return false;
}

UGBWAnimMetaData_BSAxisControl* FGBWAnimNode_MagicPlayer::GetBSAxisControl() const
{
	if (AnimationAssetNow)
	{
		return Cast<UGBWAnimMetaData_BSAxisControl>(AnimationAssetNow->FindMetaDataByClass(UGBWAnimMetaData_BSAxisControl::StaticClass()));
	}
	return nullptr;
}

FVector FGBWAnimNode_MagicPlayer::GetBSAxisValue() const
{
	float Horizontal = 0.0f;
	float Vertical = 0.0f;

	if (const UGBWAnimMetaData_BSAxisControl* BSAxisControl = GetBSAxisControl())
	{
		bool bIsGetAxis = false;
		BSAxisControl->GetBSAxisValue(
			MeshComp,
			AnimationAssetNow,
			MeshComp? MeshComp->GetOwner() : nullptr,
			DeltaTime,
			BlendTimer,
			GetBlendSpacePosition().X,
			GetBlendSpacePosition().Y,
			LoopCount,
			bIsGetAxis,
			Horizontal,
			Vertical);
	}

	return FVector(Horizontal, Vertical, 0.0f);
}

float FGBWAnimNode_MagicPlayer::GetPoseSmoothSpeed() const
{return GET_ANIM_NODE_DATA(float, PoseSmoothSpeed);}

bool FGBWAnimNode_MagicPlayer::ShouldReplay() const
{
	if (!GetAnimAsset())
	{
		return false;
	}
	
	return
	(GetReplayNow() && BlendTimer >= GetReplayCodeTime())
	|| (GetCurrentAssetTime() >= GetCurrentAssetLength()-0.02f && GetLoopAnimation());
}

bool FGBWAnimNode_MagicPlayer::IsPlaying() const
{
	if (!GetAnimAsset())
	{
		return false;
	}

	if (GetCurrentAssetTime() >= GetCurrentAssetLength()-0.02f && !GetLoopAnimation())
	{
		return false;
	}
	return true;
}

float FGBWAnimNode_MagicPlayer::GetPlayRateBasis() const
{ return 1.0f; }

float FGBWAnimNode_MagicPlayer::GetPlayRate() const
{
	if (IsBlendSpace())
	{
		float PlayRateAdjustRate = 0.0f;
		for (auto Element : BlendSampleDataCache)
		{
			if (BlendSpaceNow->GetBlendSamples().IsValidIndex(Element.SampleDataIndex))
			{
				const FBlendSample SourceSample = BlendSpaceNow->GetBlendSample(Element.SampleDataIndex);
				PlayRateAdjustRate += (SourceSample.RateScale * Element.GetClampedWeight());
			}
		}
		if (PlayRateAdjustRate > 0.0f)
		{
			return GetAnimParam(false).PlayRate * PlayRateAdjustRate;
		}
	}
	return GetAnimParam(false).PlayRate;
}

float FGBWAnimNode_MagicPlayer::GetStartPosition() const
{ return GetAnimParam(false).StartPosition; }

bool FGBWAnimNode_MagicPlayer::GetLoopAnimation() const
{ return GetAnimParam(false).bLoopAnimation; }

void FGBWAnimNode_MagicPlayer::UpdateSyncValue()
{
	if (float SyncCurveValue = 0.0f;
		MeshComp->GetAnimInstance()->GetCurveValue(GetSyncCurveName(),SyncCurveValue))
	{
		const float TimeNow = MeshComp->GetWorld()->GetTimeSeconds();
		SyncValueInfo = FMagicPlayerSyncValueInfo(
			GetSyncCurveName(),
			SyncCurveValue,
			TimeNow - SyncValueInfo.Time <= DeltaTime * 4.0f? SyncCurveValue - SyncValueInfo.Value : 0.0f,
			TimeNow);
	}
}

const FBlendSampleData* FGBWAnimNode_MagicPlayer::GetHighestWeightedSample() const
{
	if(BlendSampleDataCache.Num() == 0)
	{
		return nullptr;
	}

	const FBlendSampleData* HighestSample = &BlendSampleDataCache[0];

	for(int32 Idx = 1; Idx < BlendSampleDataCache.Num(); ++Idx)
	{
		if(BlendSampleDataCache[Idx].TotalWeight > HighestSample->TotalWeight)
		{
			HighestSample = &BlendSampleDataCache[Idx];
		}
	}

	return HighestSample;
}

void FGBWAnimNode_MagicPlayer::ReinitializeBlendSpace(bool bResetTime)
{
	if (!IsBlendSpace())
	{
		return;
	}
	BlendSampleDataCache.Empty();
	
	int32 InOutCachedTriangulationIndex = 0;
	BlendSpaceNow->GetSamplesFromBlendInput(GetBlendSpacePosition(), BlendSampleDataCache, InOutCachedTriangulationIndex, true);
	
	if(bResetTime)
	{
		const float CurrentStartPosition = GetStartInternalTimeAccumulator();

		InternalTimeAccumulator = FMath::Clamp(CurrentStartPosition, 0.f, 1.0f);
		if (CurrentStartPosition == 0.f && GetPlayRate() < 0.0f)
		{
			// Blend spaces run between 0 and 1
			InternalTimeAccumulator = 1.0f;
		}
	}
	
	BlendSpaceNow->InitializeFilter(&BlendFilter);
}

void FGBWAnimNode_MagicPlayer::ApplyPose(const FPoseSnapshot& PoseSnapshot, FCompactPose& OutPose)
{
	const TArray<FTransform>& LocalTMs = PoseSnapshot.LocalTransforms;
	const FBoneContainer& RequiredBones = OutPose.GetBoneContainer();
	const FString PoseSnapshotSKMName = PoseSnapshot.SkeletalMeshName.ToString();
	
	for (FCompactPoseBoneIndex PoseBoneIndex : OutPose.ForEachBoneIndex())
	{
		const FMeshPoseBoneIndex MeshBoneIndex = RequiredBones.MakeMeshPoseIndex(PoseBoneIndex);
		const int32 Index = MeshBoneIndex.GetInt();

		if (LocalTMs.IsValidIndex(Index))
		{
			OutPose[PoseBoneIndex] = LocalTMs[Index];
		}
	}
}

float FGBWAnimNode_MagicPlayer::GetStartTimeRate() const
{
	UGBWANS_AnimPlayControl_Base* PlayControl = nullptr;
	float StartTime = 0.0f;
	float EndTime = 0.0f;
	float StartTimeRate = 0.0f;
	float EndTimeRate = 0.0f;
	if (ANS_PlayControl)
	{
		PlayControl = ANS_PlayControl;
		StartTime = ANS_StartTime;
		EndTime = ANS_EndTime;
		StartTimeRate = ANS_StartTimeRate;
		EndTimeRate = ANS_EndTimeRate;
	}
	else
	{
		GetAnimPlayControlANS(
		PlayControl,
		StartTime,
		EndTime,
		StartTimeRate,
		EndTimeRate);
	}
	if (PlayControl)
	{
		return StartTimeRate;
	}

	if (AnimationAssetNow)
	{
		return GetAnimParam(false).StartPosition / AnimationAssetNow->GetPlayLength();
	}
	return 0.0f;
}

float FGBWAnimNode_MagicPlayer::GetEndTimeRate() const
{
	UGBWANS_AnimPlayControl_Base* PlayControl = nullptr;
	float StartTime = 0.0f;
	float EndTime = 0.0f;
	float StartTimeRate = 0.0f;
	float EndTimeRate = 0.0f;
	if (ANS_PlayControl)
	{
		PlayControl = ANS_PlayControl;
		StartTime = ANS_StartTime;
		EndTime = ANS_EndTime;
		StartTimeRate = ANS_StartTimeRate;
		EndTimeRate = ANS_EndTimeRate;
	}
	else
	{
		GetAnimPlayControlANS(
		PlayControl,
		StartTime,
		EndTime,
		StartTimeRate,
		EndTimeRate);
	}
	if (PlayControl)
	{
		return EndTimeRate;
	}

	if (AnimationAssetNow)
	{
		return GetAnimParam(false).EndPosition / AnimationAssetNow->GetPlayLength();
	}
	return 0.0f;
}

float FGBWAnimNode_MagicPlayer::GetStartInternalTimeAccumulator() const
{
	float Res = GetStartPosition();
	if (MeshComp
		&& MeshComp->GetWorld()->GetTimeSeconds() - SyncValueInfo.Time <= DeltaTime * 4.0f
		&& GetIsStartFromSync()
		&& (PlayStateChangeType == EGBWMagicPlayerPlayStateChangeType::StoppedToPlaying || PlayStateChangeType == EGBWMagicPlayerPlayStateChangeType::NewPlay))
	{
		const FVector BSPosition = GetBSAxisValue();
		Res = UGBWAnimFuncLib::GetPlayTimeBySyncCurveValue(
				SyncValueInfo.Value,
				SyncValueInfo.DeltaValue,
				AnimationAssetNow,
				SyncValueInfo.Name,
				DeltaTime,
				GetStartTimeRate() * GetCurrentAssetAbsLength(),
				GetEndTimeRate() * GetCurrentAssetAbsLength(),
				BSPosition);
	}
	
	return Res;
}

void FGBWAnimNode_MagicPlayer::DebugStateToLog(const bool bIsHasPrePose, const bool bIsHasTargetPose) const
{
	if (!GetIsDebug() || !MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
	
	const FString Msg = FString::Printf(
		TEXT("%s<%d>  "
			"<%s <%.2f> %s> "
		   "PRate<%.2f> "
			"Progress<%.2f=%.2f/%.2f> <%.2f=%.2f/%.2f> "
		   "Range<%.2f-%.2f><%.2f-%.2f> "
		   "BS<%.2f %.2f> "
		   "SyncV<%.2f %.2f> "
		   "<%s %s> "
		   "<%s><%d> "
		   )
		, *(MeshComp && MeshComp->GetOwner() && MeshComp->GetOwner()->HasAuthority()? FString("S"):FString("C"))
		, static_cast<int>(GFrameCounter)
		, *(bIsHasPrePose? FString("P"):FString("_")) 
		, BlendAlpha
		, *(bIsHasTargetPose? FString("T"):FString("_")) 
		, GetPlayRate()
		, GetCurrentAssetTime()/GetCurrentAssetLength()
		, GetCurrentAssetTime()
		, GetCurrentAssetLength()
		, InternalTimeAccumulator/GetCurrentAssetAbsLength()
		, InternalTimeAccumulator
		, GetCurrentAssetAbsLength()
		, GetAnimParam(false).StartPosition
		, GetAnimParam(false).EndPosition
		, GetStartTimeRate()
		, GetEndTimeRate()

		, LastBlendSpacePosition.X, LastBlendSpacePosition.Y
		, SyncValueInfo.Value, SyncValueInfo.DeltaValue
		
		, *(StaticEnum<EMagicPlayerPlayStateType>()->GetNameStringByIndex( ( int32 ) PlayStateNow))
		, *(StaticEnum<EGBWMagicPlayerPlayStateChangeType>()->GetNameStringByIndex( ( int32 ) PlayStateChangeType))
		, *(AnimationAssetNow?AnimationAssetNow->GetName():"None")
		, PlayFrames
		);
	switch (PlayStateNow)
	{
	case Starting:
		UE_LOG(LogTemp,Warning,TEXT("%s"), *Msg);
		break;
	case Playing:
		UE_LOG(LogTemp,Log,TEXT("%s"), *Msg);
		break;
	case Stopping:
		UE_LOG(LogTemp,Error,TEXT("%s"), *Msg);
		break;
	case Stopped:
		UE_LOG(LogTemp,Log,TEXT("%s"), *Msg);
		break;
	default:
		UE_LOG(LogTemp,Log,TEXT("%s"), *Msg);
	}

	GEngine->AddOnScreenDebugMessage(
		MeshComp && MeshComp->GetOwner() && MeshComp->GetOwner()->HasAuthority()? 628 : 1220,
		0.0f,
		MeshComp && MeshComp->GetOwner() && MeshComp->GetOwner()->HasAuthority()? FColor::Orange : FColor::Green,
		FString::Printf(TEXT("%s"),*Msg));
}

bool FGBWAnimNode_MagicPlayer::MakeSnapshotPose(FPoseSnapshot& Snapshot, FCSPose<FCompactPose>& InPose, const FBoneContainer& BoneContainer) const
{
	if (!MeshComp || !MeshComp->GetSkeletalMeshAsset() || !InPose.GetPose().IsValid())
	{
		return false;
	}
	if (USkeletalMesh* SkelMesh = MeshComp->GetSkeletalMeshAsset())
	{
		const FReferenceSkeleton& RefSkeleton = SkelMesh->GetRefSkeleton();
		
		TArray<FTransform> ComponentSpaceTMs;
		int i = 0;
		while (RefSkeleton.IsValidIndex(i))
		{
			if (const FCompactPoseBoneIndex BoneIndex = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(i));
				UGBWAnimFuncLib::CheckBoneIndexValidForPose(BoneIndex, InPose))
			{ComponentSpaceTMs.Add(InPose.GetComponentSpaceTransform(BoneIndex));}
			else {return false;}
			i++;
		}
		
		const TArray<FTransform>& RefPoseSpaceBaseTMs = RefSkeleton.GetRefBonePose();

		Snapshot.SkeletalMeshName = SkelMesh->GetFName();

		const int32 NumSpaceBases = ComponentSpaceTMs.Num();
		Snapshot.LocalTransforms.Reset(NumSpaceBases);
		Snapshot.LocalTransforms.AddUninitialized(NumSpaceBases);
		Snapshot.BoneNames.Reset(NumSpaceBases);
		Snapshot.BoneNames.AddUninitialized(NumSpaceBases);

		//Set root bone which is always evaluated.
		Snapshot.LocalTransforms[0] = ComponentSpaceTMs[0];
		Snapshot.BoneNames[0] = RefSkeleton.GetBoneName(0);

		int32 CurrentRequiredBone = 1;
		for (int32 ComponentSpaceIdx = 1; ComponentSpaceIdx < NumSpaceBases; ++ComponentSpaceIdx)
		{
			Snapshot.BoneNames[ComponentSpaceIdx] = RefSkeleton.GetBoneName(ComponentSpaceIdx);

			const bool bBoneHasEvaluated =
				MeshComp->FillComponentSpaceTransformsRequiredBones.IsValidIndex(CurrentRequiredBone)
				&& ComponentSpaceIdx == MeshComp->FillComponentSpaceTransformsRequiredBones[CurrentRequiredBone];
			const int32 ParentIndex = RefSkeleton.GetParentIndex(ComponentSpaceIdx);

			const FTransform& ParentTransform = ComponentSpaceTMs[ParentIndex];
			const FTransform& ChildTransform = ComponentSpaceTMs[ComponentSpaceIdx];
			Snapshot.LocalTransforms[ComponentSpaceIdx] = bBoneHasEvaluated ? ChildTransform.GetRelativeTransform(ParentTransform) : RefPoseSpaceBaseTMs[ComponentSpaceIdx];

			if (bBoneHasEvaluated)
			{
				CurrentRequiredBone++;
			}
		}

		Snapshot.bIsValid = true;
		return true;
	}
	
	Snapshot.bIsValid = false;
	return false;
}

bool FGBWAnimNode_MagicPlayer::EvaluatePoseFromAnimAsset(
	FPoseContext& Res,
	UAnimationAsset* AnimationAsset,
	FAnimInstanceProxy* AnimInstanceProxy,
	float TheInternalTimeAccumulator,
	bool bIsLoopAnimation,
	FDeltaTimeRecord TheDeltaTimeRecord,
	TArray<FBlendSampleData> TheBlendSampleDataCache)
{
	bool bIsGetPose = true;
	FPoseContext PoseTarget(AnimInstanceProxy);
	PoseTarget.ResetToRefPose();
	FAnimationPoseData PoseDataRes(PoseTarget);
	
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	if ((AnimationAsset != nullptr) && (AnimInstanceProxy->GetSkeleton() == AnimationAsset->GetSkeleton()))
#else
	if ((AnimationAsset != nullptr)
		&& (AnimInstanceProxy->IsSkeletonCompatible(AnimationAsset->GetSkeleton())))
#endif
	{
		if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(AnimationAsset))
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
			AnimSequence->GetAnimationPose(
				PoseDataRes,
				FAnimExtractContext(static_cast<double>(TheInternalTimeAccumulator), AnimInstanceProxy->ShouldExtractRootMotion(), TheDeltaTimeRecord, bIsLoopAnimation)
			);
#else
			AnimSequence->GetAnimationPose(
				PoseDataRes,
				FAnimExtractContext(TheInternalTimeAccumulator, AnimInstanceProxy->ShouldExtractRootMotion(), TheDeltaTimeRecord, bIsLoopAnimation)
			);
#endif
		}
		else if (UBlendSpace* BlendSpace = Cast<UBlendSpace>(AnimationAsset))
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
			BlendSpace->GetAnimationPose(
				TheBlendSampleDataCache,
				FAnimExtractContext(static_cast<double>(TheInternalTimeAccumulator), AnimInstanceProxy->ShouldExtractRootMotion(), TheDeltaTimeRecord, bIsLoopAnimation),
				PoseDataRes);
#else
			BlendSpace->GetAnimationPose(
				TheBlendSampleDataCache,
				FAnimExtractContext(TheInternalTimeAccumulator, AnimInstanceProxy->ShouldExtractRootMotion(), TheDeltaTimeRecord, bIsLoopAnimation),
				PoseDataRes);
#endif
		}
		else
		{
			bIsGetPose = false;
		}
	}
	else
	{
		bIsGetPose = false;
	}

	Res.Pose = PoseDataRes.GetPose();
	Res.Curve = PoseDataRes.GetCurve();
	Res.CustomAttributes = PoseDataRes.GetAttributes();

	return bIsGetPose;
}

bool FGBWAnimNode_MagicPlayer::EvaluatePrePose(FPoseContext& Res, FAnimInstanceProxy* AnimInstanceProxy)
{
	bool bIsGetPose = false;
	
	FPoseContext PoseTarget(AnimInstanceProxy);
	if (PrePoseSnapshot.bIsValid)
	{
		ApplyPose(PrePoseSnapshot, PoseTarget.Pose);
	}
	else
	{
		PoseTarget.Pose.ResetToRefPose();
	}
	FAnimationPoseData TargetAnimationPoseData(PoseTarget);

	FDeltaTimeRecord TheDeltaTimeRecord;
	TheDeltaTimeRecord.Delta = DeltaTime;
	for (int i=BlendInfoSet.Num()-1 ; i >= 0; i--)
	{
		bool bIsGetThisPose = false;
		
		FPoseContext PoseSnapshot(AnimInstanceProxy);
		PoseSnapshot.Pose.ResetToRefPose();
			
		if (EvaluatePoseFromAnimAsset(
			PoseSnapshot,
			BlendInfoSet[i].AnimationAsset,
			AnimInstanceProxy,
			BlendInfoSet[i].InternalTimeAccumulator,
			BlendInfoSet[i].bIsLoopAnim,
			TheDeltaTimeRecord,
			BlendInfoSet[i].BlendSampleDataCache))
		{
			bIsGetThisPose = true;
		}
		else if (BlendInfoSet[i].PoseSnapshot.bIsValid)
		{
			bIsGetThisPose = true;
			ApplyPose(BlendInfoSet[i].PoseSnapshot, PoseSnapshot.Pose);
		}

		if (bIsGetThisPose)
		{
			FPoseContext ThisPoseTarget(AnimInstanceProxy);
			ThisPoseTarget.Pose.ResetToRefPose();
			FAnimationPoseData ThisAnimationPoseData(ThisPoseTarget);
				
			bIsGetPose = true;
			const float ThisBlendAlpha = BlendInfoSet[i].GetBlendAlpha();

			/*UE_LOG(LogTemp,Error,TEXT("%d   %.4f   %.4f/%.4f  %.4f  %.4f   %s   %s"),
				static_cast<int>(GFrameCounter),
				BlendInfoSet[i].InternalTimeAccumulator,
				BlendInfoSet[i].BlendTimer,
				BlendInfoSet[i].BlendTime,
				ThisBlendAlpha,
				TheDeltaTimeRecord.Delta,
				*(PoseSnapshot.Pose.GetBoneContainer().IsValid()? FString("True") : FString("False")),
				*(BlendInfoSet[i].AnimationAsset? BlendInfoSet[i].AnimationAsset->GetName() : FString("None"))
			);*/
			
			FAnimationPoseData PreAnimationPoseData(PoseSnapshot);
			FAnimationRuntime::BlendTwoPosesTogether(
			PreAnimationPoseData,
			TargetAnimationPoseData,
			FMath::Clamp<float>(1.0f-ThisBlendAlpha,0.0f,1.0f), 
			/*out*/ThisAnimationPoseData);
				
			PoseTarget.Pose = ThisAnimationPoseData.GetPose();
			PoseTarget.Curve = ThisAnimationPoseData.GetCurve();
			PoseTarget.CustomAttributes = ThisAnimationPoseData.GetAttributes();
		}
	}

	Res.Pose = TargetAnimationPoseData.GetPose();
	Res.Curve = TargetAnimationPoseData.GetCurve();
	Res.CustomAttributes = TargetAnimationPoseData.GetAttributes();

	return bIsGetPose;
}

bool FGBWAnimNode_MagicPlayer::EvaluateTargetPose(FPoseContext& Res, FAnimInstanceProxy* AnimInstanceProxy)
{
	bool bIsHasTargetPose = true;
	FPoseContext PoseTarget(AnimInstanceProxy);
	FAnimationPoseData TargetAnimationPoseData(PoseTarget);

	if (bIsStopped && Source.GetLinkNode())
	{
		Source.Evaluate(PoseTarget);
		if (PlayFrames <= 1)
		{
			bIsHasTargetPose = false;
		}
	}
	else
	{
		bIsHasTargetPose = EvaluatePoseFromAnimAsset(
			PoseTarget,
			AnimationAssetNow,
			AnimInstanceProxy,
			InternalTimeAccumulator,
			GetLoopAnimation(),
			DeltaTimeRecord,
			BlendSampleDataCache);
	}

	Res.Pose = TargetAnimationPoseData.GetPose();
	Res.Curve = TargetAnimationPoseData.GetCurve();
	Res.CustomAttributes = TargetAnimationPoseData.GetAttributes();

	return bIsHasTargetPose;
}
#undef LOCTEXT_NAMESPACE
