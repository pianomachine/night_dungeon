// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWFootIKFuncLib.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "GameFramework/Character.h"
#include "GBWAnimNode_FootIK.generated.h"


USTRUCT(BlueprintType)
struct FGBWBoneControl
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FName BoneName = FName();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FTransform TargetTransform = FTransform();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EBoneModificationMode> ScaleMode = BMM_Ignore;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EBoneModificationMode> RotationMode = BMM_Ignore;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EBoneModificationMode> TranslationMode = BMM_Ignore;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<enum EBoneControlSpace> ScaleSpace = EBoneControlSpace::BCS_ComponentSpace;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<enum EBoneControlSpace> RotationSpace = EBoneControlSpace::BCS_ComponentSpace;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<enum EBoneControlSpace> TranslationSpace = EBoneControlSpace::BCS_ComponentSpace;

	UPROPERTY()
	float Alpha = 1.0f;
	
	FGBWBoneControl(){}
	FGBWBoneControl(
		const FName InBoneName,
		const FTransform& InTargetTransform,
		const EBoneModificationMode InScaleMode,
		const EBoneModificationMode InRotationMode,
		const EBoneModificationMode InTranslationMode,
		const EBoneControlSpace InScaleSpace,
		const EBoneControlSpace InRotationSpace,
		const EBoneControlSpace InTranslationSpace,
		const float InAlpha)
	:BoneName(InBoneName),
	TargetTransform(InTargetTransform),
	ScaleMode(InScaleMode),
	RotationMode(InRotationMode),
	TranslationMode(InTranslationMode),
	ScaleSpace(InScaleSpace),
	RotationSpace(InRotationSpace),
	TranslationSpace(InTranslationSpace),
	Alpha(InAlpha){}
};

USTRUCT(BlueprintInternalUseOnly)
struct GBWFOOTIK_API FGBWAnimNode_FootIK : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()

protected:
	friend class UGBWAnimGraphNode_FootIK;
	
	// The source input, passed thru to the output unless a montage or slot animation is currently playing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Links)
	FPoseLink Source;
	
#if WITH_EDITORONLY_DATA
	
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PinHiddenByDefault, FoldProperty))
	FGBWFootIKSettings FootIKSettings = FGBWFootIKSettings();
		
#endif
	
public:
	float DeltaTime = 0.0f;
	UPROPERTY()
	USkeletalMeshComponent* MeshComp = nullptr;
	UPROPERTY()
	AActor* OwnerActor = nullptr;
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	FGBWFootIKData FootIKData = FGBWFootIKData();

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface
	
	FGBWFootIKSettings GetFootIKSettings() const;

	bool EvaluateBoneControl(
		FCSPose<FCompactPose>& CompactPose,
		const FBoneContainer& BoneContainer,
		const FTransform& ComponentTransform,
		const FGBWBoneControl& BoneControl,
		FBoneTransform& Res,
		FTransform& TargetTransform);
	
	void EvaluateFootIKSkeletalControl(
		FCSPose<FCompactPose>& CompactPose,
		const FBoneContainer& BoneContainer,
		const FTransform& ComponentTransform);

	void EvaluateFootBone(
		FCSPose<FCompactPose>& TheCompactPose,
		FGBWFootIKFootData& FootData) const;
};
