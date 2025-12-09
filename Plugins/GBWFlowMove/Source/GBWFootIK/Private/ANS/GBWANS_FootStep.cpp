// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "ANS/GBWANS_FootStep.h"

FString UGBWANS_FootStep::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("%.2f - %s      _")
		,TargetLockRate
		,*(StaticEnum<EGBWFootType>()->GetNameStringByIndex( static_cast<int32>(FootType))));
}

FLinearColor UGBWANS_FootStep::GetEditorColor()
{
	return FootType == EGBWFootType::LeftFoot? FColor(255,50,50) : FColor(50,255,50);
}
