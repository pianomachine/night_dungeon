// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWFlowMoveObjects.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GBWFlowMoveAN_FootStep.generated.h"

UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "FM_FootStep"))
class GBWFLOWMOVE_API UGBWFlowMoveAN_FootStep : public UAnimNotify
{
	GENERATED_BODY()

public:
	// Begin UAnimNotify interface
	virtual FString GetNotifyName_Implementation() const override;
	virtual FLinearColor GetEditorColor() override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	// End UAnimNotify interface
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName FootStepType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (AnimNotifyBoneName = "true"))
	FName FootSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FGBWFlowMoveTraceSetting FloorTraceSetting = FGBWFlowMoveTraceSetting();
};
