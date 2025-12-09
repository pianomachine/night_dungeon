// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "ANS/GBWANS_AnimPlayControl_Base.h"

void UGBWANS_AnimPlayControl_Base::GetPlayTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, bool& bIsGet, float& StartTime, float& EndTime) const
{
	Received_GetPlayTime(
		MeshComp,
		Animation,
		OwnerActor,
		bIsGet,
		StartTime,
		EndTime);
}

void UGBWANS_AnimPlayControl_Base::GetPlayRate(
	USkeletalMeshComponent* MeshComp,
	UAnimationAsset* Animation,
	AActor* OwnerActor,
	const float DeltaTime,
	const float ExecuteTime,
	const float NotifyLength,
		const int LoopCount,
	bool& bIsGet, float& PlayRate) const
{
	Received_GetPlayRate(
		MeshComp,
		Animation,
		OwnerActor,
		DeltaTime,
		ExecuteTime,
		NotifyLength,LoopCount,
		bIsGet,
		PlayRate);
}

void UGBWANS_AnimPlayControl_Base::GetLoopAnimation(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,
		const int LoopCount, bool& bIsGet,
	bool& bIsLoop) const
{
	Received_GetLoopAnimation(
		MeshComp,
		Animation,
		OwnerActor,
		DeltaTime,
		ExecuteTime,
		NotifyLength,LoopCount,
		bIsGet,
		bIsLoop);
}

void UGBWANS_AnimPlayControl_Base::GetBlendTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,
		const int LoopCount, bool& bIsGet,
	float& BlendTime, float& LoopBlendTime) const
{
	Received_GetBlendTime(
		MeshComp,
		Animation,
		OwnerActor,
		DeltaTime,
		ExecuteTime,
		NotifyLength,LoopCount,
		bIsGet,
		BlendTime,
		LoopBlendTime);
}

void UGBWANS_AnimPlayControl_Base::GetStopBlendTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,
		const int LoopCount, bool& bIsGet,
	float& StopBlendTime) const
{
	Received_GetStopBlendTime(
		MeshComp,
		Animation,
		OwnerActor,
		DeltaTime,
		ExecuteTime,
		NotifyLength,LoopCount,
		bIsGet, StopBlendTime);
}

void UGBWANS_AnimPlayControl_Base::GetBoneControl(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
                                                  AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,
		const int LoopCount, bool& bIsGet,
                                                  FMagicPlayerBoneControlParam& BoneControlParam) const
{
	Received_GetBoneControl(
		MeshComp,
		Animation,
		OwnerActor,
		DeltaTime,
		ExecuteTime,
		NotifyLength,LoopCount,
		bIsGet,
		BoneControlParam);
}

void UGBWANS_AnimPlayControl_Base::GetIsStartFromSync(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength, const int LoopCount,
	bool& bIsGet, bool& bIsStartFromSync) const
{
	Received_GetIsStartFromSync(
		MeshComp,
		Animation,
		OwnerActor,
		DeltaTime,
		ExecuteTime,
		NotifyLength,
		LoopCount,
		bIsGet,
		bIsStartFromSync);
}

void UGBWANS_AnimPlayControl_Base::GetReplayNow(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		bool& bReplayNow,
		uint32& PlayStateHash) const
{
	bIsGet = false;
}

void UGBWANS_AnimPlayControl_Base::GetInternalTimeAccumulator(
	USkeletalMeshComponent* MeshComp,
	UAnimationAsset* Animation, AActor* OwnerActor, const float DeltaTime, const float ExecuteTime,
	const float NotifyLength, const float InternalTimeAccumulatorNow,
		const int LoopCount, bool& bIsGet,
	float& InternalTimeAccumulator) const
{
	bIsGet = false;
}
