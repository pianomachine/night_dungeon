// Copyright GanBowen 2022-2024. All Rights Reserved.

#include "ANS/GBWPPANS_Perceptron.h"
#include "GBWPowerfulPerceptionComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UGBWPPANS_Perceptron::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!(MeshComp->GetOwner() && MeshComp->GetOwner()->HasAuthority()))
	{
		return;
	}
	if (!bAllowMultipleInstances
		&& UGBWPowerfulPerceptionComponent::PP_HasPerception(MeshComp->GetOwner(),PerceptronTag,PerceptionClass))
	{
		return;
	}
	UGBWPowerfulPerceptionComponent::_ExecutePerception(MeshComp->GetOwner(),PerceptronTag,PerceptionClass,FGBWStruct());
}

void UGBWPPANS_Perceptron::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (MeshComp->GetOwner() && MeshComp->GetOwner()->HasAuthority())
	{
		if (!UGBWPowerfulPerceptionComponent::PP_HasPerception(MeshComp->GetOwner(),PerceptronTag,PerceptionClass))
		{
			UGBWPowerfulPerceptionComponent::_ExecutePerception(MeshComp->GetOwner(),PerceptronTag,PerceptionClass,FGBWStruct());
		}
	}
}

void UGBWPPANS_Perceptron::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (bRemovePerceptionWhenEnd)
	{
		if (MeshComp->GetOwner() && MeshComp->GetOwner()->HasAuthority())
		{
			UGBWPowerfulPerceptionComponent::PP_RemovePerception(MeshComp->GetOwner(), PerceptronTag);
		}
	}
}

FString UGBWPPANS_Perceptron::GetNotifyName_Implementation() const
{
	if(!Description.IsEmpty())
	{
		return Description;
	}

	return UAnimNotifyState::GetNotifyName_Implementation();
}