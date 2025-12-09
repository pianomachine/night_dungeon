// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "AnimMetaData/GBWAnimMetaData_BSAxisControl.h"

void UGBWAnimMetaData_BSAxisControl::GetBSAxisValue(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
	AActor* OwnerActor, const float DeltaTime, const float ExecuteTime,
	const float OldHorizontal, const float OldVertical, const int LoopCount, bool& bIsGet, float& Horizontal,
	float& Vertical) const
{
	Received_GetBSAxisValue(
		MeshComp,
		Animation,
		OwnerActor,
		DeltaTime,
		ExecuteTime,
		OldHorizontal,
		OldVertical,
		LoopCount,
		bIsGet,
		Horizontal,
		Vertical);
}
