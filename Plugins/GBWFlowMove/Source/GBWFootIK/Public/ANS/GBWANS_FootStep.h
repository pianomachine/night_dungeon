// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GBWANS_FootStep.generated.h"

UENUM(BlueprintType)
enum EGBWFootType
{
	LeftFoot,
	RightFoot
};
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "GBW_FootLock"))
class GBWFOOTIK_API UGBWANS_FootStep : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	// Begin UAnimNotify interface
	virtual FString GetNotifyName_Implementation() const override;
	virtual FLinearColor GetEditorColor() override;
	// End UAnimNotify interface

	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = "FootLock")
	TEnumAsByte<EGBWFootType> FootType = EGBWFootType::LeftFoot;
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = "FootLock",
		meta=(InlineEditConditionToggle))
	bool bStartLockRate = false;
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = "FootLock",
		meta=(EditCondition="bStartLockRate"))
	float StartLockRate = 0.0f;
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = "FootLock")
	float TargetLockRate = 1.0f;
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = "FootLock")
	float FootLockInterpSpeed_In = 36.0f;
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = "FootLock")
	float FootLockInterpSpeed_Out = 12.0f;
};