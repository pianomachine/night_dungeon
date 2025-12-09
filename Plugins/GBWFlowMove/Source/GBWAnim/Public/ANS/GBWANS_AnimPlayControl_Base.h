// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWAnimComponent.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GBWANS_AnimPlayControl_Base.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "PA_AnimPlayControl_Base"))
class GBWANIM_API UGBWANS_AnimPlayControl_Base : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void GetPlayTime(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		bool& bIsGet,
		float& StartTime,
		float& EndTime) const;
	UFUNCTION(BlueprintImplementableEvent)
	void Received_GetPlayTime(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		bool& bIsGet,
		float& StartTime,
		float& EndTime) const;

	virtual void GetPlayRate(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		float& PlayRate) const;
	UFUNCTION(BlueprintImplementableEvent)
	void Received_GetPlayRate(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		float& PlayRate) const;

	virtual void GetLoopAnimation(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		bool& bIsLoop) const;
	UFUNCTION(BlueprintImplementableEvent)
	void Received_GetLoopAnimation(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		bool& bIsLoop) const;

	virtual void GetBlendTime(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		float& BlendTime,
		float& LoopBlendTime) const;
	UFUNCTION(BlueprintImplementableEvent)
	void Received_GetBlendTime(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		float& BlendTime,
		float& LoopBlendTime) const;

	virtual void GetStopBlendTime(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet, float& StopBlendTime) const;
	UFUNCTION(BlueprintImplementableEvent)
	void Received_GetStopBlendTime(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet, float& StopBlendTime) const;
	
	virtual void GetBoneControl(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		FMagicPlayerBoneControlParam& BoneControlParam) const;
	UFUNCTION(BlueprintImplementableEvent)
	void Received_GetBoneControl(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		FMagicPlayerBoneControlParam& BoneControlParam) const;

	virtual void GetIsStartFromSync(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		bool& bIsStartFromSync) const;
	UFUNCTION(BlueprintImplementableEvent)
	void Received_GetIsStartFromSync(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		bool& bIsStartFromSync) const;

	virtual void GetReplayNow(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const int LoopCount,
		bool& bIsGet,
		bool& bReplayNow,
		uint32& PlayStateHash) const;


	virtual void GetInternalTimeAccumulator(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const float InternalTimeAccumulatorNow,
		const int LoopCount,
		bool& bIsGet,
		float& InternalTimeAccumulator) const;
};
