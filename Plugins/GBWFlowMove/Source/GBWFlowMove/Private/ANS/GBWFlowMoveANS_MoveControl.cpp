// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "ANS/GBWFlowMoveANS_MoveControl.h"

#include "GBWAnimFuncLib.h"
#include "GBWFlowMoveComponent.h"
#include "GBWFlowMoveFuncLib.h"
#include "GBWPowerfulToolsFuncLib.h"
#include "GBWPSFuncLib.h"

FString UGBWFlowMoveANS_MoveControl::GetNotifyName_Implementation() const
{
	if(!Description.IsEmpty())
	{
		return Description;
	}

	return UAnimNotifyState::GetNotifyName_Implementation();
}

UGBWFlowMoveComponent* UGBWFlowMoveANS_MoveControl::GetFMComponent(AActor* InActor)
{
	return UGBWFlowMoveFuncLib::FM_GetFlowMoveComponent(InActor, true);
}

bool UGBWFlowMoveANS_MoveControl::GetMoveControlSettings(
		AActor* InActor,
		const FVector2D Position,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		bool& NoChange,
		FGBWFlowMoveControlSettingsPro& Res)
{
	const float UpdateDurationIntervalTime =
				SelectUpdateSettings.MaxUpdateIntervalTime>0.0f?
				ExecuteTime - static_cast<int>(ExecuteTime / SelectUpdateSettings.MaxUpdateIntervalTime) * SelectUpdateSettings.MaxUpdateIntervalTime
				: ExecuteTime;
	if (SettingType != SimpleSet
		&& SelectUpdateSettings.UpdateDuration > 0.0f
		&& UpdateDurationIntervalTime >= SelectUpdateSettings.UpdateDuration)
	{
		NoChange = true;
		return true;
	}

	NoChange = false;
	
	float MinDistance = -1.0f;
	UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(InActor);
	switch (SettingType)
	{
	case EFMMoveControlANSSettingType::SimpleSet:
		Res = MoveControlSetting;
		return true;
	case EFMMoveControlANSSettingType::StateSelect:
		for (const auto Element : StateSelectMoveControlSettings)
		{
			bool bIsGet = false;
			bool Result = false;
			float Duration = 0.0f;
			UGBWPSFuncLib::GetPSBoolValue(InActor, Element.Key, bIsGet, Result, Duration);
			if (Result)
			{
				Res = Element.Value;
				return true;
			}
		}
		break;
	case EFMMoveControlANSSettingType::BlendSpacePositionSelect:
		for (const auto Element : PositionSelectMoveControlSettings)
		{
			if (const float Dis = (Element.Position - Position).Length();
				MinDistance < 0.0 || MinDistance >= Dis)
			{
				MinDistance = Dis;
				Res = Element.MoveControlSetting;
			}
		}

		if (MinDistance >= 0.0f)
		{
			return true;
		}
		break;
	case EFMMoveControlANSSettingType::FMValueConditionSelect:
		if (FlowMoveComponent)
		{
			for (const auto Element : ValueConditionSelectMoveControlSettings)
			{
				if (Element.IsMeet(FlowMoveComponent))
				{
					Res = Element.MoveControlSetting;
					return true;
				}
			}
		}
		break;
	case EFMMoveControlANSSettingType::FMSpeedModeSelect:
		if (FlowMoveComponent)
		{
			for (const auto Element : SpeedModeSelectMoveControlSettings)
			{
				if (Element.Key == FlowMoveComponent->GetFlowMoveSpeedMode())
				{
					Res = Element.Value;
					return true;
				}
			}
		}
		break;
	default: ;
	}

	Res = MoveControlSetting;
	return true;
}

void UGBWFlowMoveANS_MoveControl::GetPlayTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, bool& bIsGet, float& StartTime, float& EndTime) const
{
	Super::GetPlayTime(MeshComp, Animation, OwnerActor, bIsGet, StartTime, EndTime);

	const UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor);
	if (!FlowMoveComponent)
	{
		bIsGet = false;
		return;
	}
	
	if (const FGBWFlowMoveControlSettingsPro TheMoveControlSettings = FlowMoveComponent->GetMoveControlParam().MoveControlSettings;
		TheMoveControlSettings.AnimPlaySetting.bIsSpecifyPlayDuration
		&& TheMoveControlSettings.AnimPlaySetting.PlayDuration.Y > TheMoveControlSettings.AnimPlaySetting.PlayDuration.X)
	{
		bIsGet = true;
		StartTime = TheMoveControlSettings.AnimPlaySetting.PlayDuration.X;
		EndTime = TheMoveControlSettings.AnimPlaySetting.PlayDuration.Y;
		return;
	}

	bIsGet = false;
}

void UGBWFlowMoveANS_MoveControl::GetPlayRate(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet,
	float& PlayRate) const
{
	bIsGet = false;
	PlayRate = 1.0f;
	Super::GetPlayRate(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, NotifyLength, LoopCount, bIsGet, PlayRate);
	if (const UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor); FlowMoveComponent && Animation)
	{
		bIsGet = true;
		PlayRate = 1.0f;

		const FGBWFlowMoveControlSettingsPro TheMoveControlSettings = FlowMoveComponent->GetMoveControlParam().MoveControlSettings;
		if (TheMoveControlSettings.AnimPlaySetting.PlayTimeControl.PlayTimeControl == EGBWFlowMoveAnimPlayTimeControlType::DefaultControl)
		{
			if (FlowMoveComponent->TaskState.MoveControlParam.MoveControlSettings.FromAsset == Animation)
			{
				PlayRate = FlowMoveComponent->TaskState.MoveControlParam.AnimPlayRateNow;
			}
		}
		else
		{
			PlayRate = 0.0f;	
		}
	}
}

void UGBWFlowMoveANS_MoveControl::GetLoopAnimation(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet,
	bool& bIsLoop) const
{
	Super::GetLoopAnimation(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, NotifyLength, LoopCount, bIsGet, bIsLoop);

	const UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor);
	if (!FlowMoveComponent)
	{
		return;
	}
	const FGBWFlowMoveControlSettingsPro TheMoveControlSettings = FlowMoveComponent->GetMoveControlParam().MoveControlSettings;
	bIsGet = true;
	bIsLoop = TheMoveControlSettings.AnimPlaySetting.LoopAnimation;
}

void UGBWFlowMoveANS_MoveControl::GetBlendTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet,
	float& BlendTime, float& LoopBlendTime) const
{
	Super::GetBlendTime(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, NotifyLength, LoopCount, bIsGet, BlendTime,
	                    LoopBlendTime);
	
	const UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor);
	if (!FlowMoveComponent)
	{
		return;
	}
	const FGBWFlowMoveControlSettingsPro TheMoveControlSettings = FlowMoveComponent->GetMoveControlParam().MoveControlSettings;
	bIsGet = true;
	BlendTime = TheMoveControlSettings.AnimPlaySetting.BlendTime;
	LoopBlendTime = TheMoveControlSettings.AnimPlaySetting.LoopBlendTime;
}

void UGBWFlowMoveANS_MoveControl::GetStopBlendTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet,
	float& StopBlendTime) const
{
	Super::GetStopBlendTime(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, NotifyLength, LoopCount, bIsGet,
	                        StopBlendTime);
	const UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor);
	if (!FlowMoveComponent)
	{
		return;
	}
	const FGBWFlowMoveControlSettingsPro TheMoveControlSettings = FlowMoveComponent->GetMoveControlParam().MoveControlSettings;
	bIsGet = true;
	StopBlendTime = TheMoveControlSettings.AnimPlaySetting.StopBlendTime;
}

void UGBWFlowMoveANS_MoveControl::GetBoneControl(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
                                                 AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet,
                                                 FMagicPlayerBoneControlParam& BoneControlParam) const
{
	Super::GetBoneControl(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, NotifyLength, LoopCount, bIsGet,
	                      BoneControlParam);

	const UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor);
	if (!FlowMoveComponent)
	{
		return;
	}
	const FGBWFlowMoveControlSettingsPro TheMoveControlSettings = FlowMoveComponent->GetMoveControlParam().MoveControlSettings;
	bIsGet = TheMoveControlSettings.AdjustSettings.bIsUseBoneAdjust
		&& ExecuteTime >= TheMoveControlSettings.AdjustSettings.BoneAdjust.DelayTime;
	BoneControlParam = TheMoveControlSettings.AdjustSettings.BoneAdjust.BoneControlParam;

	if (TheMoveControlSettings.AdjustSettings.bIsUseTiltPostureAdjust)
	{
		bIsGet = true;
		FVector Velocity = FlowMoveComponent->MoveControlResult.Velocity;
		Velocity.Z = 0.0f;
		const float AngleSpeed = FlowMoveComponent->MoveControlResult.DeltaRotator.Yaw / FlowMoveComponent->TaskState.FrameDeltaTime;;
		const float Speed = Velocity.Length();

		const float AngleRate = FMath::Clamp((AngleSpeed / TheMoveControlSettings.AdjustSettings.TiltPostureAdjust.FinishedAngleSpeed),-1.0f,1.0f);
		const float SpeedRate = FMath::Clamp(
			((Speed >= TheMoveControlSettings.AdjustSettings.TiltPostureAdjust.StartSpeed? Speed - TheMoveControlSettings.AdjustSettings.TiltPostureAdjust.StartSpeed : 0.0f)
			     / (TheMoveControlSettings.AdjustSettings.TiltPostureAdjust.FinishedSpeed - TheMoveControlSettings.AdjustSettings.TiltPostureAdjust.StartSpeed)),0.0f,1.0f);

		const float Angle = TheMoveControlSettings.AdjustSettings.TiltPostureAdjust.MaxAdjustAngle * AngleRate * SpeedRate;
		
		FMagicPlayerBoneControl Posture = FMagicPlayerBoneControl(
			TheMoveControlSettings.AdjustSettings.TiltPostureAdjust.BoneName,
			FTransform(
				FRotator(Angle,0.0f,0.0f),
				FVector::ZeroVector
			),
			EBoneModificationMode::BMM_Ignore,
			EBoneModificationMode::BMM_Additive,
			EBoneModificationMode::BMM_Ignore,
			EBoneControlSpace::BCS_ComponentSpace,
			EBoneControlSpace::BCS_ComponentSpace,
			EBoneControlSpace::BCS_ComponentSpace,
			1.0);
		Posture.SmoothSpeed_Enter = 6.0f;
		BoneControlParam.BoneControlQueue.Add(Posture);
	}
}

void UGBWFlowMoveANS_MoveControl::GetReplayNow(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount,
	bool& bIsGet, bool& bReplayNow, uint32& PlayStateHash) const
{
	bIsGet = false;
	Super::GetReplayNow(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, NotifyLength, LoopCount, bIsGet,
	                    bReplayNow,
	                    PlayStateHash);

	const UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor);
	if (!FlowMoveComponent)
	{
		return;
	}
	
	const FGBWFlowMoveControlSettingsPro TheMoveControlSettings = FlowMoveComponent->GetMoveControlParam().MoveControlSettings;

	if (!Cast<UBlendSpace>(Animation))
	{
		return;
	}
	
	if (FMagicPlayerPlayState AnimStateNow;
		UGBWAnimFuncLib::GetMagicPlayerState(OwnerActor, TheMoveControlSettings.AnimSlot,AnimStateNow))
	{
		const float H = AnimStateNow.PlayerParamCache.Horizontal;
		const float V = AnimStateNow.PlayerParamCache.Vertical;
		
		for (auto Element : TheMoveControlSettings.AnimPlaySetting.BS_ReplayZone)
		{
			if (H >= Element.HorizontalAxisRange.X && H <= Element.HorizontalAxisRange.Y
				&& V >= Element.VerticalAxisRange.X && V >= Element.VerticalAxisRange.Y)
			{
				PlayStateHash = TextKeyUtil::HashString(FString::Printf(TEXT("%s%s"),
					*Element.HorizontalAxisRange.ToString(),
					*Element.VerticalAxisRange.ToString()
				));

				if (PlayStateHash != AnimStateNow.PlayState.PlayStateHash)
				{
					bIsGet = true;
					bReplayNow = true;
					return;
				}
			}
		}
	}
}

void UGBWFlowMoveANS_MoveControl::GetInternalTimeAccumulator(USkeletalMeshComponent* MeshComp,
	UAnimationAsset* Animation, AActor* OwnerActor, const float DeltaTime, const float ExecuteTime,
	const float NotifyLength, const float InternalTimeAccumulatorNow,const int LoopCount, bool& bIsGet,
	float& InternalTimeAccumulator) const
{
	bIsGet = false;
	Super::GetInternalTimeAccumulator(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, NotifyLength,
	                                  InternalTimeAccumulatorNow,  LoopCount, bIsGet,
	                                  InternalTimeAccumulator);

	InternalTimeAccumulator = InternalTimeAccumulatorNow;
	bIsGet = true;
}

void UGBWFlowMoveANS_MoveControl::GetIsStartFromSync(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength, const int LoopCount,
	bool& bIsGet, bool& bIsStartFromSync) const
{
	bIsGet = false;
	Super::GetIsStartFromSync(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, NotifyLength, LoopCount, bIsGet,
	                          bIsStartFromSync);
	const UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor);
	if (!FlowMoveComponent)
	{
		return;
	}
	
	const FGBWFlowMoveControlSettingsPro TheMoveControlSettings = FlowMoveComponent->GetMoveControlParam().MoveControlSettings;
	bIsGet = true;
	bIsStartFromSync = TheMoveControlSettings.AnimPlaySetting.bIsStartFromSync;
}
