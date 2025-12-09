// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "ANS/GBWFlowMoveAN_FootStep.h"

#include "GBWFlowMoveComponent.h"
#include "GBWFlowMoveFuncLib.h"

FString UGBWFlowMoveAN_FootStep::GetNotifyName_Implementation() const
{
	return "FM_FootStep";
}

FLinearColor UGBWFlowMoveAN_FootStep::GetEditorColor()
{
	return FLinearColor::Yellow;
}

void UGBWFlowMoveAN_FootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UGBWFlowMoveComponent* Fmc = UGBWFlowMoveFuncLib::FM_GetFlowMoveComponent(MeshComp->GetOwner(), true))
		{
			Fmc->SendFootStepEvent(MeshComp,FootStepType,FootSocketName,FloorTraceSetting);
		}
	}
}
