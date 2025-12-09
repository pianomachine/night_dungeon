// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GBWMagicPlayerInterface.generated.h"

UINTERFACE(MinimalAPI)
class UGBWMagicPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class GBWANIM_API IGBWMagicPlayerInterface
{
	GENERATED_BODY()

public:
	virtual void MagicPlayerTick(
		USkeletalMeshComponent* MeshComp,
		UAnimationAsset* Animation,
		AActor* OwnerActor,
		const float DeltaTime,
		const float ExecuteTime,
		const float StartTime,
		const float EndTime,
		const float Progress,
		const FVector BlendSpacePosition,
		const FTransform RootMotion,
		const bool bIsStartTick,
		const FName AnimSlot){}
};
