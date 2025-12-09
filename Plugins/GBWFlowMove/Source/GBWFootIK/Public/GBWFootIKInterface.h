// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GBWFootIKInterface.generated.h"

UINTERFACE(MinimalAPI,Blueprintable)
class UGBWFootIKInterface : public UInterface
{
	GENERATED_BODY()
};
class GBWFOOTIK_API IGBWFootIKInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GBWFootIK")
	void GetIKRate_L(
		const USkeletalMeshComponent* MeshComp,
		const AActor* OwnerActor,
		const float DeltaTime,
		bool& bIsGet,
		float& IKRate);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GBWFootIK")
	void GetIKRate_R(
		const USkeletalMeshComponent* MeshComp,
		const AActor* OwnerActor,
		const float DeltaTime,
		bool& bIsGet,
		float& IKRate);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GBWFootIK")
	void GetFootLockRate_L(
		const USkeletalMeshComponent* MeshComp,
		const AActor* OwnerActor,
		const float DeltaTime,
		bool& bIsGet,
		float& FootLockRate);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GBWFootIK")
	void GetFootLockRate_R(
		const USkeletalMeshComponent* MeshComp,
		const AActor* OwnerActor,
		const float DeltaTime,
		bool& bIsGet,
		float& FootLockRate);
};


UINTERFACE(MinimalAPI,Blueprintable)
class UGBWFootStepInterface : public UInterface
{
	GENERATED_BODY()
};
class GBWFOOTIK_API IGBWFootStepInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GBWFootIK")
	void OnFootStep(
		const USkeletalMeshComponent* MeshComp,
		const AActor* OwnerActor,
		const float DeltaTime,
		const FName FootBoneName,
		const FVector FloorLocation,
		const FRotator FootRotation,
		const FVector FloorNormal,
		const FHitResult FloorHitResult);
};