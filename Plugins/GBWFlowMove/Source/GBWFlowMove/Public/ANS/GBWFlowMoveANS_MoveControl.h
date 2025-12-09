// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWFlowMoveObjects.h"
#include "ANS/GBWANS_AnimPlayControl_Base.h"
#include "GBWFlowMoveANS_MoveControl.generated.h"

UCLASS(Blueprintable, meta = (DisplayName = "FM_MoveControl"))
class GBWFLOWMOVE_API UGBWFlowMoveANS_MoveControl : public UGBWANS_AnimPlayControl_Base
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "FlowMove")
	FString Description = "FM_MoveControl";

	UPROPERTY(EditAnywhere, Category = "FlowMove")
	TEnumAsByte<EFMMoveControlANSSettingType> SettingType = EFMMoveControlANSSettingType::SimpleSet;

	UPROPERTY(EditAnywhere, Category = "FlowMove",
		meta=(EditConditionHides,EditCondition="SettingType != EFMMoveControlANSSettingType::SimpleSet"))
	FGBWFlowMoveControlSettings_SelectUpdateSettings SelectUpdateSettings = FGBWFlowMoveControlSettings_SelectUpdateSettings();
	
	UPROPERTY(EditAnywhere, Category = "FlowMove")
	FGBWFlowMoveControlSettingsPro MoveControlSetting;
	
	UPROPERTY(EditAnywhere, Category = "FlowMove",
		meta=(ForceInlineRow,EditConditionHides,EditCondition="SettingType == EFMMoveControlANSSettingType::StateSelect"))
	TMap<FName, FGBWFlowMoveControlSettingsPro> StateSelectMoveControlSettings;
	UPROPERTY(EditAnywhere, Category = "FlowMove",
		meta=(EditConditionHides,EditCondition="SettingType == EFMMoveControlANSSettingType::BlendSpacePositionSelect"))
	TArray<FGBWFlowMoveMoveControlSetting_PositionSelectItem> PositionSelectMoveControlSettings;
	UPROPERTY(EditAnywhere, Category = "FlowMove",
		meta=(EditConditionHides,EditCondition="SettingType == EFMMoveControlANSSettingType::FMValueConditionSelect"))
	TArray<FGBWFlowMoveMoveControlSetting_ValueConditionSelectItem> ValueConditionSelectMoveControlSettings;
	UPROPERTY(EditAnywhere, Category = "FlowMove",
		meta=(ForceInlineRow,EditConditionHides,EditCondition="SettingType == EFMMoveControlANSSettingType::FMSpeedModeSelect"))
	TMap<TEnumAsByte<EFMSpeedMode>, FGBWFlowMoveControlSettingsPro> SpeedModeSelectMoveControlSettings;
	
	// Overridden from UAnimNotifyState to provide custom notify name.
	virtual FString GetNotifyName_Implementation() const override;
	virtual FLinearColor GetEditorColor() override {return FColor::Yellow;}

	static UGBWFlowMoveComponent* GetFMComponent(AActor* InActor);
	bool GetMoveControlSettings(
		AActor* InActor,
		const FVector2D Position,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		bool& NoChange,
		FGBWFlowMoveControlSettingsPro& Res);
	//GBWAnim Interfaces
	virtual void GetPlayTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation, AActor* OwnerActor, bool& bIsGet, float& StartTime, float& EndTime) const override;
	virtual void GetPlayRate(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation, AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet, float& PlayRate) const override;
	virtual void GetLoopAnimation(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation, AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet, bool& bIsLoop) const override;
	virtual void GetBlendTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation, AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet, float& BlendTime, float& LoopBlendTime) const override;
	virtual void GetStopBlendTime(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation, AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet, float& StopBlendTime) const override;
	virtual void GetBoneControl(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation, AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength,const int LoopCount, bool& bIsGet, FMagicPlayerBoneControlParam& BoneControlParam) const override;
	virtual void GetReplayNow(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,const int LoopCount,
		bool& bIsGet,
		bool& bReplayNow,
		uint32& PlayStateHash) const override;
	virtual void GetInternalTimeAccumulator(
		USkeletalMeshComponent* MeshComp,
		UAnimationAsset* Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float NotifyLength,
		const float InternalTimeAccumulatorNow,const int LoopCount,
		bool& bIsGet,
		float& InternalTimeAccumulator) const override;
	virtual void GetIsStartFromSync(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation, AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float NotifyLength, const int LoopCount, bool& bIsGet, bool& bIsStartFromSync) const override;
	//GBWAnim Interfaces
};
