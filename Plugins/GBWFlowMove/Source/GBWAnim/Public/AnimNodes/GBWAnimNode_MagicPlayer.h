// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWAnimComponent.h"
#include "GBWMagicPlayerInterface.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Animation/InputScaleBias.h"
#include "AnimMetaData/GBWAnimMetaData_BSAxisControl.h"
#include "ANS/GBWANS_AnimPlayControl_Base.h"
#include "GBWAnimNode_MagicPlayer.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct GBWANIM_API FGBWAnimNode_MagicPlayer : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()

protected:
	friend class UGBWAnimGraphNode_MagicPlayer;

	// Corresponding state for PlayRateScaleBiasClampConstants
	UPROPERTY()
	FInputScaleBiasClampState PlayRateScaleBiasClampState;

	// Filter used to dampen coordinate changes
	FBlendFilter BlendFilter;
	// Cache of samples used to determine blend weights
	TArray<FBlendSampleData> BlendSampleDataCache;
	UPROPERTY(Transient)
	TObjectPtr<UBlendSpace> PreviousBlendSpace = nullptr;

	// The source input, passed thru to the output unless a montage or slot animation is currently playing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Links)
	FPoseLink Source;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinHiddenByDefault, FoldProperty))
	bool bDebugPlayStateToLog = false;
	
	//Whether we should continue to update the source pose regardless of whether it would be used.
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinShownByDefault, FoldProperty))
	bool bAlwaysUpdateSourcePose = false;

	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinShownByDefault, FoldProperty))
	FName SyncCurveName = FName("GBWSync");
	
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinShownByDefault, FoldProperty))
	TEnumAsByte<EMagicPlayerType> Type = EMagicPlayerType::AutoGetParam;
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinShownByDefault, FoldProperty))
	FName ParamSlot = FName("Default");
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinShownByDefault, FoldProperty))
	float PoseSmoothSpeed = 32.0f;
	
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinHiddenByDefault, FoldProperty))
	FMagicPlayerParam AnimParam = FMagicPlayerParam();
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinHiddenByDefault, FoldProperty))
	AActor* LinkedActor = nullptr;
	//When set to True and LinkedActor is not set,
	//the first Actor with GBWAnimComponent will be automatically obtained from the parent Actor or AttachTo's Actor as the LinkedActor.
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinHiddenByDefault, FoldProperty))
	bool AutoGetLinkedActor = true;
		
#endif

	void UpdateAnimPlayer(const FAnimationUpdateContext& Context);
	void UpdateAnimAsset(const FAnimationUpdateContext& Context);
	void GetPlayControlANS();
	
public:
	UPROPERTY()
	TArray<TScriptInterface<IGBWMagicPlayerInterface>> MagicPlayerInterfaceObjSet;
	UPROPERTY()
	UGBWAnimComponent* AnimComponent = nullptr;
	UPROPERTY()
	UGBWAnimComponent* LinkedAnimComponent = nullptr;
	UPROPERTY()
	USkeletalMeshComponent* MeshComp = nullptr;
	UPROPERTY()
	AActor* OwnerActor = nullptr;
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;
	UPROPERTY()
	UGBWANS_AnimPlayControl_Base* ANS_PlayControl = nullptr;
	float ANS_StartTime = 0.0f;
	float ANS_EndTime = 0.0f;
	float ANS_StartTimeRate = 0.0f;
	float ANS_EndTimeRate = 0.0f;

	TEnumAsByte<EMagicPlayerPlayStateType> PlayStateNow = EMagicPlayerPlayStateType::Stopped;
	TEnumAsByte<EGBWMagicPlayerPlayStateChangeType> PlayStateChangeType = EGBWMagicPlayerPlayStateChangeType::NoChange;
	
	UPROPERTY()
	UAnimSequence* AnimSequenceNow = nullptr;
	UPROPERTY()
	UAnimMontage* AnimMontageNow = nullptr;
	UPROPERTY()
	UBlendSpace* BlendSpaceNow = nullptr;
	UPROPERTY()
	UAnimationAsset* AnimationAssetNow = nullptr;
	float BlendTimeWhenStopping = 0.2f;
	
	UPROPERTY()
	UAnimationAsset* AnimationAssetLast = nullptr;
	bool bIsLoopAnim_Last = false;
	float InternalTimeAccumulator_Last = 0.0f;
	float BlendTime_Last = 0.0f;
	FVector LastBlendSpacePosition = FVector::ZeroVector;
	float LastAssetTime = 0.0f;
	FPoseSnapshot LastPoseSnapshot;
	FPoseSnapshot PrePoseSnapshot;
	
	TArray<FMagicPlayerBlendInfo> BlendInfoSet;
	FMagicPlayerSyncValueInfo SyncValueInfo;
	
	float DeltaTime = 0.0f;
	float BlendTimer = 0.0f;
	float AbsPlayProgress = 0.0f;
	bool bIsLoopReply = false;
	int LoopCount = 0;
	float BlendAlpha = 1.0f;
	bool bIsStopped = false;
	int PlayFrames = 0;
	
	TMap<FName, FTransform> BoneTargetTransformsCache;
		
	// FAnimNode_AssetPlayerBase interface
	virtual float GetCurrentAssetTime() const override;
	virtual float GetCurrentAssetTimePlayRateAdjusted() const override;
	virtual float GetCurrentAssetLength() const override;
	virtual UAnimationAsset* GetAnimAsset() const override;
	virtual float GetAccumulatedTime() const override;
	// End of FAnimNode_AssetPlayerBase interface

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	float GetCurrentAssetAbsLength() const;
	void SetAnimAsset(UAnimationAsset* NewAnimAsset) { AnimationAssetNow = NewAnimAsset; }
	FVector GetBlendSpacePosition() const;
	float GetBlendTimeWhenReplay() const;
	float GetBlendTimeWhenStopping();
	float GetBlendTime();
	bool GetAlwaysUpdateSourcePose() const;
	bool GetReplayNow() const;
	float GetReplayCodeTime() const;
	FMagicPlayerParam GetAnimParam(bool bForceUpdate) const;
	EMagicPlayerType GetPlayerType() const;
	AActor* GetLinkedActor() const;
	bool GetAutoGetLinkedActor() const;
	FName GetParamSlot() const;
	bool GetIsDebug() const;
	float GetPlayProgress() const;
	FMagicPlayerBoneControlParam GetBoneControlParam() const;
	UAnimationAsset* GetRootMotionSourceAnim() const;
	void GetAnimPlayControlANS(
		UGBWANS_AnimPlayControl_Base*& ANS,
		float& StartTime,
		float& EndTime,
		float& StartTimeRate,
		float& EndTimeRate) const;
	EMagicPlayerLinkType GetLinkType() const;
	UAnimationAsset* GetTickInterfaceAnim() const;
	UAnimationAsset* GetMainStateAnim() const;
	UGBWAnimComponent* GetRealAnimComponent() const;
	FName GetSyncCurveName() const;
	bool GetIsStartFromSync() const;
	UGBWAnimMetaData_BSAxisControl* GetBSAxisControl() const;
	FVector GetBSAxisValue() const;
	float GetPoseSmoothSpeed() const;

	bool ShouldReplay() const;
	bool IsPlaying() const;

	void CheckAnimAsset();
	
	bool IsAnimSequence() const;
	bool IsAnimMontage() const;
	bool IsBlendSpace() const;

	bool EvaluateMagicPlayerBoneControl(
		FCSPose<FCompactPose>& CompactPose,
		const FBoneContainer& BoneContainer,
		const FTransform& ComponentTransform,
		const FMagicPlayerBoneControl& BoneControl,
		FBoneTransform& Res,
		FTransform& TargetTransform);
	
	void EvaluateSkeletalControl(
		FCSPose<FCompactPose>& CompactPose,
		const FBoneContainer& BoneContainer,
		const FTransform& ComponentTransform);
	
public:
	// Set the animation to continue looping when it reaches the end
	virtual bool SetLoopAnimation(bool bInLoopAnimation) { return false; }
	
	// The Basis in which the PlayRate is expressed in. This is used to rescale PlayRate inputs.
	// For example a Basis of 100 means that the PlayRate input will be divided by 100.
	virtual float GetPlayRateBasis() const;

	// The play rate multiplier. Can be negative, which will cause the animation to play in reverse.
	virtual float GetPlayRate() const;

	// Additional scaling, offsetting and clamping of PlayRate input.
	// Performed after PlayRateBasis.
	virtual const FInputScaleBiasClampConstants& GetPlayRateScaleBiasClampConstants() const { static FInputScaleBiasClampConstants Dummy; return Dummy; }

	// The start position [range: 0 - sequence length] to use when initializing. When looping, play will still jump back to the beginning when reaching the end.
	virtual float GetStartPosition() const;

	// Should the animation loop back to the start when it reaches the end?
	virtual bool GetLoopAnimation() const;

	// Use pose matching to choose the start position. Requires experimental PoseSearch plugin.
	virtual bool GetStartFromMatchingPose() const { return false; }

	// Set the start time of this node.
	// @return true if the value was set (it is dynamic), or false if it could not (it is not dynamic or pin exposed)
	virtual bool SetStartPosition(float InStartPosition) { return false; }

	// Set the play rate of this node.
	// @return true if the value was set (it is dynamic), or false if it could not (it is not dynamic or pin exposed)	
	virtual bool SetPlayRate(float InPlayRate) { return false; }

private:
	void UpdateSyncValue();
	const FBlendSampleData* GetHighestWeightedSample() const;
	void ReinitializeBlendSpace(bool bResetTime = true);
	bool SnapshotPose();
	static void ApplyPose(const FPoseSnapshot& PoseSnapshot, FCompactPose& OutPose);

	float GetStartTimeRate() const;
	float GetEndTimeRate() const;
	float GetStartInternalTimeAccumulator() const;

	void DebugStateToLog(bool bIsHasPrePose,bool bIsHasTargetPose) const;
	bool MakeSnapshotPose(FPoseSnapshot& Snapshot, FCSPose<FCompactPose>& InPose,const FBoneContainer& BoneContainer) const;
	static bool EvaluatePoseFromAnimAsset(
		FPoseContext& Res,
		UAnimationAsset* AnimationAsset,
		FAnimInstanceProxy* AnimInstanceProxy,
		float TheInternalTimeAccumulator,
		bool bIsLoopAnimation,
		FDeltaTimeRecord TheDeltaTimeRecord,
		TArray<FBlendSampleData> TheBlendSampleDataCache);
	bool EvaluatePrePose(FPoseContext& Res, FAnimInstanceProxy* AnimInstanceProxy);
	bool EvaluateTargetPose(FPoseContext& Res, FAnimInstanceProxy* AnimInstanceProxy);
};
