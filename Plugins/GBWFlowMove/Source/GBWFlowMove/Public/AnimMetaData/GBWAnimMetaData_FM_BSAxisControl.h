// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWFlowMoveObjects.h"
#include "AnimMetaData/GBWAnimMetaData_BSAxisControl.h"
#include "GBWAnimMetaData_FM_BSAxisControl.generated.h"

UCLASS(Blueprintable, meta = (DisplayName = "FM_BSAxisControl"))
class GBWFLOWMOVE_API UGBWAnimMetaData_FM_BSAxisControl : public UGBWAnimMetaData_BSAxisControl
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWFlowMoveControlSettings_BlendSpaceAxisControlSetting HorizontalAxis = FGBWFlowMoveControlSettings_BlendSpaceAxisControlSetting();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWFlowMoveControlSettings_BlendSpaceAxisControlSetting VerticalAxis = FGBWFlowMoveControlSettings_BlendSpaceAxisControlSetting();

	static UGBWFlowMoveComponent* GetFMComponent(AActor* InActor);
	virtual void GetBSAxisValue(
		USkeletalMeshComponent* MeshComp,
		UAnimationAsset* Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float OldHorizontal,
		const float OldVertical,
		const int LoopCount,
		bool& bIsGet,
		float& Horizontal,
		float& Vertical) const override;
};
