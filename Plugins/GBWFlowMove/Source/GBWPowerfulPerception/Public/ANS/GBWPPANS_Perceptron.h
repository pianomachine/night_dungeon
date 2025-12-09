// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GBWPerceptionBase.h"
#include "GBWPPANS_Perceptron.generated.h"

UCLASS(Blueprintable, meta = (DisplayName = "PP_Perceptron"))
class GBWPOWERFULPERCEPTION_API UGBWPPANS_Perceptron : public UAnimNotifyState
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Perceptron")
	FString Description = "PP_Perceptron";

	UPROPERTY(EditAnywhere, Category = "Perceptron")
	FGameplayTag PerceptronTag;
	UPROPERTY(EditAnywhere, Category = "Perceptron")
	TSubclassOf<AGBWPerceptionBase> PerceptionClass;
	UPROPERTY(EditAnywhere, Category="Perceptron")
	bool bAllowMultipleInstances = false;
	UPROPERTY(EditAnywhere, Category = "Perceptron")
	bool bRemovePerceptionWhenEnd = true;
	
	virtual void NotifyBegin(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference) override;
	// Overridden from UAnimNotifyState to provide custom notify name.
	FString GetNotifyName_Implementation() const override;
	virtual FLinearColor GetEditorColor() override {return FColor::Cyan;}
};
