// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GBWAnimComponent.h"
#include "GBWAnimFuncLib.generated.h"

UCLASS()
class GBWANIM_API UGBWAnimFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static UGBWAnimComponent* GetGBWAnimComponent(AActor* InActor);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Anim")
	static void GBWPlayMontage(USkeletalMeshComponent* SKMComponent,
		UAnimMontage* MontageToPlay,
		float PlayRate=1.0f,
		float StartingPosition=0.0f,
		FName StartingSection=NAME_None);
	UFUNCTION(BlueprintCallable, Category = "GBW|Anim")
	static void GBWStopMontage(USkeletalMeshComponent* SKMComponent);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(DefaultToSelf="InActor"))
	static void GBWPlayMagicAnim_(
		AActor* InActor,
		UAnimationAsset* AnimToPlay,
		const TArray<FMagicPlayerLinkedAnim> LinkedAnimSet,
		FName Slot = NAME_None);
	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(DefaultToSelf="InActor"))
	static void GBWStopMagicAnim(AActor* InActor, FName Slot = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim")
	static float GetMontageSectionLength(UAnimMontage* Montage, FName SectionName);

	UFUNCTION(BlueprintCallable, Category="GBW|Anim")
	static bool GetAnimCurveValue(
		UAnimSequenceBase* Anim,
		FName CurveName,
		float TimeOffset,
		float PlayRate,
		float Position,
		float DeltaTime,
		float& Result,
		float& ResultDelta);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim")
	static bool IsPlayingSlotAnim(
		UAnimInstance* Instance,
		FName SlotName,
		UAnimSequenceBase*& AnimSequence,
		UAnimMontage*& OutMontage);

	//On the server, OutCondition is the same as InCondition;
	//On the client side, OutCondition is false.
	UFUNCTION(BlueprintPure, Category = "GBW|Anim|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe), DisplayName="OnlyServer")
	static void Anim_Pure_OnlyServer(
		bool InCondition,
		const UAnimInstance* AnimInstance,
		bool& OutCondition);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(DefaultToSelf="InActor"))
	static void RemoveMagicPlayer(AActor* InActor, const FName InSlot);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(DefaultToSelf="InActor", BlueprintThreadSafe))
	static bool GetMagicPlayerState(AActor* InActor, const FName InSlot, FMagicPlayerPlayState& ResState);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(BlueprintThreadSafe))
	static FTransform GetRootMotionDataFromAsset(
		UAnimationAsset* AnimAsset,
		const float CurrentAssetTime,
		const float DeltaTime,
		const float PlayRate,
		const bool bLoop,
		TArray<FBlendSampleData> BlendSampleDataCache,
		bool bInverse);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(BlueprintThreadSafe))
	static float GetNewPlayTimeByTargetDistanceAndRootMotionData(
		float TargetDistance,
		UAnimationAsset* AnimAsset,
		const float CurrentAssetTime,
		const float DeltaTime,
		const float PlayRate,
		const bool bLoop,
		TArray<FBlendSampleData> BlendSampleDataCache);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(BlueprintThreadSafe))
	static float GetNewPlayTimeByTargetRotationYawAndRootMotionData(
		float TargetRotationYaw,
		UAnimationAsset* AnimAsset,
		const float CurrentAssetTime,
		const float DeltaTime,
		const float PlayRate,
		const bool bLoop,
		TArray<FBlendSampleData> BlendSampleDataCache);

	static bool CheckBoneIndexValidForPose(const FCompactPoseBoneIndex BoneIndex, FCSPose<FCompactPose>& InPose);

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim", meta=(BlueprintThreadSafe))
	static float GetPlayTimeBySyncCurveValue(
		float CurveValue,
		float CurveDeltaValue,
		UAnimationAsset* AnimAsset,
		FName CurveName,
		const float DeltaTime,
		const float StartTime,
		const float EndTime,
		const FVector BSPosition);
};