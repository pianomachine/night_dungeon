// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "AnimMetaData/GBWAnimMetaData_FM_BSAxisControl.h"

#include "GBWFlowMoveComponent.h"
#include "GBWFlowMoveFuncLib.h"
#include "Animation/BlendSpace.h"

UGBWFlowMoveComponent* UGBWAnimMetaData_FM_BSAxisControl::GetFMComponent(AActor* InActor)
{
	return UGBWFlowMoveFuncLib::FM_GetFlowMoveComponent(InActor, true);
}

void UGBWAnimMetaData_FM_BSAxisControl::GetBSAxisValue(USkeletalMeshComponent* MeshComp, UAnimationAsset* Animation,
                                                       AActor* OwnerActor, const float DeltaTime, const float ExecuteTime, const float OldHorizontal,
                                                       const float OldVertical, const int LoopCount, bool& bIsGet, float& Horizontal, float& Vertical) const
{
	bIsGet = false;
	Horizontal = 0.0f;
	Vertical = 0.0f;
	Super::GetBSAxisValue(MeshComp, Animation, OwnerActor, DeltaTime, ExecuteTime, OldHorizontal, OldVertical,
	                      LoopCount, bIsGet,
	                      Horizontal, Vertical);

	UGBWFlowMoveComponent* FlowMoveComponent = GetFMComponent(OwnerActor);
	if (!FlowMoveComponent){return;}
	
	if (const UBlendSpace* ForBS = Cast<UBlendSpace>(Animation))
	{
		bIsGet = true;
		const FBlendParameter BPX = ForBS->GetBlendParameter(0);
		const FBlendParameter BPY = ForBS->GetBlendParameter(1);
			
		Horizontal = HorizontalAxis.GetAxisValue(
			FlowMoveComponent,
			OldHorizontal,
			DeltaTime,
			ExecuteTime,
			FVector2D(BPX.Min,BPX.Max));
		Vertical = VerticalAxis.GetAxisValue(
			FlowMoveComponent,
			OldVertical,
			DeltaTime,
			ExecuteTime,
			FVector2D(BPY.Min,BPY.Max));
	}
}
