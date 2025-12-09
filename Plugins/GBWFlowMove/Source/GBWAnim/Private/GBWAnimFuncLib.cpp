// Copyright GanBowen 2022-2024. All Rights Reserved.

#include "GBWAnimFuncLib.h"

#include "GBWPowerfulToolsFuncLib.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/Package.h"
#include "Animation/AnimClassInterface.h"
#include "AnimNodes/GBWAnimNode_MagicPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Launch/Resources/Version.h"

UGBWAnimComponent* UGBWAnimFuncLib::GetGBWAnimComponent(AActor* InActor)
{
	if (!InActor)
	{
		return nullptr;
	}
	UGBWAnimComponent* AComp = nullptr;
	if (UActorComponent* Component = InActor->GetComponentByClass(UGBWAnimComponent::StaticClass()))
	{
		AComp = Cast<UGBWAnimComponent>(Component);
	}

	return AComp && AComp->IsReadyToUse()? AComp : nullptr;
}

void UGBWAnimFuncLib::GBWPlayMontage(USkeletalMeshComponent* SKMComponent, UAnimMontage* MontageToPlay, float PlayRate,
                                     float StartingPosition, FName StartingSection)
{
	if (SKMComponent && SKMComponent->GetOwner())
	{
		if (UGBWAnimComponent* AComp = UGBWAnimFuncLib::GetGBWAnimComponent(SKMComponent->GetOwner()))
		{
			AComp->PlayMontage(SKMComponent,MontageToPlay,PlayRate,StartingPosition,StartingSection);
		}
	}
}

void UGBWAnimFuncLib::GBWStopMontage(USkeletalMeshComponent* SKMComponent)
{
	if (SKMComponent && SKMComponent->GetOwner())
	{
		if (UGBWAnimComponent* AComp = UGBWAnimFuncLib::GetGBWAnimComponent(SKMComponent->GetOwner()))
		{
			AComp->StopMontage(SKMComponent);
		}
	}
}

void UGBWAnimFuncLib::GBWPlayMagicAnim_(AActor* InActor, UAnimationAsset* AnimToPlay,
	const TArray<FMagicPlayerLinkedAnim> LinkedAnimSet, const FName Slot)
{
	if (UGBWAnimComponent* AnimComp = GetGBWAnimComponent(InActor))
	{
		AnimComp->PlayMagicAnim(AnimToPlay, LinkedAnimSet, Slot);
	}
}

void UGBWAnimFuncLib::GBWStopMagicAnim(AActor* InActor, const FName Slot)
{
	if (UGBWAnimComponent* AnimComp = GetGBWAnimComponent(InActor))
	{
		AnimComp->StopMagicAnim(Slot);
	}
}

float UGBWAnimFuncLib::GetMontageSectionLength(UAnimMontage* Montage, FName SectionName)
{
	if (!Montage)
	{
		return -1;
	}

	return Montage->GetSectionLength(Montage->GetSectionIndex(SectionName));
}

bool UGBWAnimFuncLib::GetAnimCurveValue(
	UAnimSequenceBase* Anim,
	FName CurveName,
	float TimeOffset,
	float PlayRate,
	float Position,
	float DeltaTime,
	float& Result,
	float& ResultDelta)
{
	if (Anim)
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
		if (Anim->HasCurveData(CurveName))
		{
			const float AnimLength = Anim->GetPlayLength();
			Result = Anim->EvaluateCurveData(CurveName, UKismetMathLibrary::FClamp(Position - TimeOffset,0.0f,AnimLength));
			ResultDelta = Anim->EvaluateCurveData(CurveName, UKismetMathLibrary::FClamp(Position + DeltaTime*PlayRate - TimeOffset,0.0f,AnimLength)) - Result;
			return true;
		}
		return false;
#else
		const USkeleton* Skeleton = Anim->GetSkeleton();
		if (!Skeleton)
		{
			return false;
		}
		const FSmartNameMapping* NameMapping = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
		if (!NameMapping)
		{
			return false;
		}
		const USkeleton::AnimCurveUID Uid = NameMapping->FindUID(CurveName);
		if (Uid == INDEX_NONE)
		{
			return false;
		}
		const float AnimLength = Anim->GetPlayLength();
		Result = Anim->EvaluateCurveData(Uid, UKismetMathLibrary::FClamp(Position - TimeOffset,0.0f,AnimLength));
		ResultDelta = Anim->EvaluateCurveData(Uid, UKismetMathLibrary::FClamp(Position + DeltaTime*PlayRate - TimeOffset,0.0f,AnimLength)) - Result;
		return true;
#endif
	}

	return false;
}

bool UGBWAnimFuncLib::IsPlayingSlotAnim(
	UAnimInstance* Instance,
	FName SlotName,
	UAnimSequenceBase*& AnimSequence,
	UAnimMontage*& OutMontage)
{
	for (int32 InstanceIndex = 0; InstanceIndex < Instance->MontageInstances.Num(); InstanceIndex++)
	{
		// check if this is playing
		const FAnimMontageInstance* MontageInstance = Instance->MontageInstances[InstanceIndex];
		// make sure what is active right now is transient that we created by request
		if (MontageInstance && MontageInstance->IsActive() && MontageInstance->IsPlaying())
		{
			UAnimMontage* CurMontage = MontageInstance->Montage;
			if (CurMontage && CurMontage->GetOuter() == GetTransientPackage())
			{
				const FAnimTrack* AnimTrack = CurMontage->GetAnimationData(SlotName);
				if (AnimTrack && AnimTrack->AnimSegments.Num() == 1)
				{
					OutMontage = CurMontage;
					AnimSequence = AnimTrack->AnimSegments[0].GetAnimReference();
					if (AnimSequence)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

void UGBWAnimFuncLib::Anim_Pure_OnlyServer(bool InCondition, const UAnimInstance* AnimInstance, bool& OutCondition)
{
	if (AnimInstance && AnimInstance->GetOwningActor() && AnimInstance->GetOwningActor()->HasAuthority())
	{
		OutCondition = InCondition;
		return;
	}

	OutCondition = false;
}

void UGBWAnimFuncLib::RemoveMagicPlayer(AActor* InActor, const FName InSlot)
{
	if (UGBWAnimComponent* AComp = UGBWAnimFuncLib::GetGBWAnimComponent(InActor))
	{
		AComp->RemoveMagicPlayer(InSlot);
	}
}

bool UGBWAnimFuncLib::GetMagicPlayerState(AActor* InActor, const FName InSlot, FMagicPlayerPlayState& ResState)
{
	if (const UGBWAnimComponent* AComp = UGBWAnimFuncLib::GetGBWAnimComponent(InActor))
	{
		return AComp->GetMagicPlayerState(InSlot, ResState);
	}
	
	return false;
}

FTransform UGBWAnimFuncLib::GetRootMotionDataFromAsset(
	UAnimationAsset* AnimAsset,
	const float CurrentAssetTime,
	const float DeltaTime,
	const float PlayRate,
	const bool bLoop,
	TArray<FBlendSampleData> BlendSampleDataCache,
	bool bInverse)
{
	FTransform DeltaRootMotionNow = FTransform();
	
	if (const UBlendSpace* BlendSpaceNow = Cast<UBlendSpace>(AnimAsset))
	{
		FRootMotionMovementParams RootMotionMovementParams;
		RootMotionMovementParams.bHasRootMotion = false;
		
		for (int32 I = 0; I < BlendSampleDataCache.Num(); ++I)
		{
			FBlendSampleData SampleEntry = BlendSampleDataCache[I];
			const int32 SampleDataIndex = SampleEntry.SampleDataIndex;

			// Skip SamplesPoints that has no relevant weight
			if (BlendSpaceNow->GetBlendSamples().IsValidIndex(SampleDataIndex) && (SampleEntry.TotalWeight > ZERO_ANIMWEIGHT_THRESH))
			{
				const FBlendSample& Sample = BlendSpaceNow->GetBlendSamples()[SampleDataIndex];
				if (IsValid(Sample.Animation))
				{
					const float CurrentSampleDataTime =
						(SampleEntry.Time == Sample.Animation->GetPlayLength())?
						CurrentAssetTime : SampleEntry.Time;

					const float MultipliedSampleRateScale = Sample.Animation->RateScale * Sample.RateScale;
					const float SampleMoveDelta = DeltaTime * MultipliedSampleRateScale * PlayRate;

					const float PrevSampleDataTime = FMath::Max(0.0f, CurrentSampleDataTime - SampleMoveDelta);

					if (UGBWPowerfulToolsFuncLib::CanAnimRootMotion(Sample.Animation,
							CurrentSampleDataTime,
							SampleMoveDelta,
							bLoop))
					{
						float StartTime = CurrentSampleDataTime;
						if (bInverse)
						{
							StartTime = CurrentSampleDataTime - SampleMoveDelta;
							while (StartTime < 0.0f)
							{
								StartTime += SampleEntry.Animation->GetPlayLength(); 
							}
						}
						RootMotionMovementParams.AccumulateWithBlend(
						Sample.Animation->ExtractRootMotion(
							StartTime,
							SampleMoveDelta,
							bLoop),
						SampleEntry.GetClampedWeight());
					}
				}
			}
		}
		
		DeltaRootMotionNow = FTransform(
		RootMotionMovementParams.GetRootMotionTransform().Rotator(),
		RootMotionMovementParams.GetRootMotionTransform().GetTranslation()
		);
	}
	else if (const UAnimSequence* AnimSequenceNow = Cast<UAnimSequence>(AnimAsset))
	{
		float StartTime = CurrentAssetTime;
		if (bInverse)
		{
			StartTime = CurrentAssetTime - DeltaTime * PlayRate * AnimSequenceNow->RateScale;
			while (StartTime < 0.0f)
			{
				StartTime += AnimSequenceNow->GetPlayLength(); 
			}
		}
		DeltaRootMotionNow = AnimSequenceNow->ExtractRootMotion(
				StartTime,
				DeltaTime * PlayRate * AnimSequenceNow->RateScale,
				bLoop);
	}
	else if (const UAnimMontage* AnimMontageNow = Cast<UAnimMontage>(AnimAsset))
	{
		float StartTime = CurrentAssetTime;
		float EndTime = FMath::Min<float>(CurrentAssetTime + DeltaTime * PlayRate,AnimMontageNow->GetPlayLength());
		if (bInverse)
		{
			StartTime = CurrentAssetTime - DeltaTime * PlayRate;
			while (StartTime < 0.0f)
			{
				StartTime += AnimMontageNow->GetPlayLength(); 
			}
			EndTime = CurrentAssetTime;
		}
		DeltaRootMotionNow = AnimMontageNow->ExtractRootMotionFromTrackRange(
				StartTime,
				EndTime);
	}

	return DeltaRootMotionNow;
}

float UGBWAnimFuncLib::GetNewPlayTimeByTargetDistanceAndRootMotionData(
	float TargetDistance,
	UAnimationAsset* AnimAsset,
	const float CurrentAssetTime,
	const float DeltaTime,
	const float PlayRate,
	const bool bLoop,
	TArray<FBlendSampleData> BlendSampleDataCache)
{
	FTransform RmData = FTransform();

	int MaxCount = 20;
	float DtTime = DeltaTime;
	float MoveDtTime = DeltaTime;
	bool bLastUp = true;
	while (FMath::Abs(RmData.GetLocation().Length() - TargetDistance) >= 0.1f
		&& MaxCount > 0)
	{
		MaxCount--;
		if (RmData.GetLocation().Length() > TargetDistance)
		{
			if (!bLastUp)
			{
				MoveDtTime /= 2.0f;
			}
			bLastUp = true;
			DtTime -= MoveDtTime;
		}
		else
		{
			if (bLastUp)
			{
				MoveDtTime /= 2.0f;
			}
			bLastUp = false;
			DtTime += MoveDtTime;
		}

		DtTime = FMath::Max(DtTime, DeltaTime / 10.0f);
		RmData = GetRootMotionDataFromAsset(
			AnimAsset,
			CurrentAssetTime,
			DtTime,
			PlayRate,
			bLoop,
			BlendSampleDataCache,
			false);
	}

	const float Res = CurrentAssetTime + DtTime;
	
	return Res;
}

float UGBWAnimFuncLib::GetNewPlayTimeByTargetRotationYawAndRootMotionData(float TargetRotationYaw,
	UAnimationAsset* AnimAsset, const float CurrentAssetTime, const float DeltaTime, const float PlayRate,
	const bool bLoop, TArray<FBlendSampleData> BlendSampleDataCache)
{
	FTransform RmData = RmData = GetRootMotionDataFromAsset(
			AnimAsset,
			CurrentAssetTime,
			DeltaTime,
			PlayRate,
			bLoop,
			BlendSampleDataCache,
			false);

	bool bInverse = true;
	if (RmData.Rotator().Yaw * TargetRotationYaw > 0.0f)
	{
		bInverse = false;
	}

	int MaxCount = 1000;
	float DtTime = DeltaTime;
	float MoveDtTime = DeltaTime;
	bool bLastUp = true;
	
	while (FMath::Abs(FMath::Abs(RmData.Rotator().Yaw) - FMath::Abs(TargetRotationYaw)) >= 0.01f
		&& MaxCount > 0)
	{
		MaxCount--;
		if (FMath::Abs(RmData.Rotator().Yaw) > FMath::Abs(TargetRotationYaw))
		{
			if (!bLastUp)
			{
				MoveDtTime /= 2.0f;
			}
			bLastUp = true;
			DtTime -= MoveDtTime;
		}
		else
		{
			if (bLastUp)
			{
				MoveDtTime /= 2.0f;
			}
			bLastUp = false;
			DtTime += MoveDtTime;
		}

		//DtTime = FMath::Max(DtTime, DeltaTime / 10.0f);

		RmData = GetRootMotionDataFromAsset(
			AnimAsset,
			CurrentAssetTime,
			DtTime,
			PlayRate,
			bLoop,
			BlendSampleDataCache,
			bInverse);
	}

	const float Res = CurrentAssetTime + FMath::Min(DtTime, 0.1f);
	
	return Res;
}

bool UGBWAnimFuncLib::CheckBoneIndexValidForPose(const FCompactPoseBoneIndex BoneIndex, FCSPose<FCompactPose>& InPose)
{
	bool bIsValid = true;
	if (BoneIndex == INDEX_NONE)
	{
		bIsValid = false;
	}
	else if (!InPose.GetPose().IsValidIndex(BoneIndex))
	{
		bIsValid = false;
	}
	else if (InPose.GetPose()[BoneIndex].ContainsNaN())
	{
		bIsValid = false;
	}
	if (bIsValid)
	{
		if (InPose.GetComponentSpaceFlags()[BoneIndex] == 0)
		{
			const FCompactPoseBoneIndex ParentIndex = InPose.GetPose().GetParentBoneIndex(BoneIndex);
			if (ParentIndex != INDEX_NONE)
			{bIsValid = CheckBoneIndexValidForPose(ParentIndex, InPose);}

			if (bIsValid)
			{
				InPose.CalculateComponentSpaceTransform(BoneIndex);
				if (InPose.GetPose()[BoneIndex].ContainsNaN())
				{bIsValid = false;}
			}
		}
	}
	
	return bIsValid;
}

float UGBWAnimFuncLib::GetPlayTimeBySyncCurveValue(
	float CurveValue,
	float CurveDeltaValue,
	UAnimationAsset* AnimAsset,
	FName CurveName,
	const float DeltaTime,
	const float StartTime,
	const float EndTime,
	const FVector BSPosition)
{
	float Res = StartTime;
	
	if (const UAnimSequence* AnimSequenceNow = Cast<UAnimSequence>(AnimAsset))
	{
		for (auto Element : AnimSequenceNow->GetCurveData().FloatCurves)
		{
			if (Element.GetName() == CurveName)
			{
				float TimeNow = StartTime;
				float ValueNow = Element.Evaluate(TimeNow);
				float MinDis = -1.0f;
				while (TimeNow >= StartTime && TimeNow <= EndTime)
				{
					TimeNow+= DeltaTime;
					float V = Element.Evaluate(TimeNow);
					if ((V - ValueNow) * CurveDeltaValue >= 0.0f)
					{
						if (FMath::Abs(V - CurveValue) <= CurveDeltaValue)
						{
							Res =  TimeNow;
							break;
						}

						if (MinDis < 0.0f || MinDis > FMath::Abs(V - CurveValue))
						{
							Res = TimeNow;
							MinDis = FMath::Abs(V - CurveValue);
						}
					}
					ValueNow = V;
				}
			}
			break;
		}
	}
	else if (const UBlendSpace* BlendSpaceNow = Cast<UBlendSpace>(AnimAsset))
	{
		TArray<FBlendSample> BlendSampleData = BlendSpaceNow->GetBlendSamples();
		float MinDis = -1.0f;
		UAnimSequence* AnimSequence = nullptr;
		for (auto SampleData : BlendSampleData)
		{
			if (MinDis < 0.0f
				|| MinDis > (SampleData.SampleValue - BSPosition).Length())
			{
				if (SampleData.Animation)
				{
					AnimSequence = SampleData.Animation;
					MinDis = (SampleData.SampleValue - BSPosition).Length();
				}
			}
		}

		if (AnimSequence)
		{
			float Length = AnimSequence->GetPlayLength();
			for (auto Element : AnimSequence->GetCurveData().FloatCurves)
			{
				if (Element.GetName() == CurveName)
				{
					float TimeNow = StartTime * Length;
					float ValueNow = Element.Evaluate(TimeNow);
					MinDis = -1.0f;
					while (TimeNow >= StartTime * Length && TimeNow <= EndTime * Length)
					{
						TimeNow+= DeltaTime;
						float V = Element.Evaluate(TimeNow);
						if ((V - ValueNow) * CurveDeltaValue >= 0.0f)
						{
							if (FMath::Abs(V - CurveValue) <= CurveDeltaValue)
							{
								Res =  TimeNow / Length;
								break;
							}

							if (MinDis < 0.0f || MinDis > FMath::Abs(V - CurveValue))
							{
								Res = TimeNow / Length;
								MinDis = FMath::Abs(V - CurveValue);
							}
						}
						ValueNow = V;
					}
					break;
				}
			}
		}
	}

	Res = (Res >= StartTime && Res < EndTime)? Res : StartTime;
	return Res;
}
