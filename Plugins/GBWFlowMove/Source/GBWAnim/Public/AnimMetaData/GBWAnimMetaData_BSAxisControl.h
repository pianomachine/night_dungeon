// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMetaData.h"
#include "GBWAnimMetaData_BSAxisControl.generated.h"

UCLASS(HideDropdown)
class GBWANIM_API UGBWAnimMetaData_BSAxisControl : public UAnimMetaData
{
	GENERATED_BODY()
	
public:
	virtual void GetBSAxisValue(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float OldHorizontal,
		const float OldVertical,
		const int LoopCount,
		bool& bIsGet,
		float& Horizontal,
		float& Vertical) const;
	UFUNCTION(BlueprintImplementableEvent)
	void Received_GetBSAxisValue(
		USkeletalMeshComponent * MeshComp,
		UAnimationAsset * Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float OldHorizontal,
		const float OldVertical,
		const int LoopCount,
		bool& bIsGet,
		float& Horizontal,
		float& Vertical) const;
};