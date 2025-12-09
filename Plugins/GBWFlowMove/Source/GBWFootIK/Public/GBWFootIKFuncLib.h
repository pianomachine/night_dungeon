// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include <deque>

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/HitResult.h"
#include "BoneIndices.h"
#include "BonePose.h"
#include "GBWFootIKFuncLib.generated.h"

USTRUCT(BlueprintType)
struct FGBWFloatQueue
{
	GENERATED_BODY()

public:
	void Enqueue(const float Value)
	{
		MyDeque.push_back(Value);
		if (MaxSize > 0 && MyDeque.size() > MaxSize)
		{
			Dequeue();
		}
	}

	float Dequeue()
	{
		if (MyDeque.empty())
		{
			return 0.0f;
		}

		const float FrontElement = MyDeque.front();
		MyDeque.pop_front();
		return FrontElement;
	}

	TArray<float> GetAll() const
	{
		TArray<float> Res;

		for (auto Item : MyDeque)
		{
			Res.Add(Item);
		}
		
		return Res;
	}

	float GetMin() const
	{
		float Res = 9999999.0f;
		TArray<float> All = GetAll();
		for (auto V : All)
		{
			if (V <= Res)
			{
				Res = V;
			}
		}

		return Res;
	}
	
	FGBWFloatQueue(){};
	FGBWFloatQueue(const int InMaxSize) : MaxSize(InMaxSize){}
private:
	std::deque<float> MyDeque;
	int MaxSize = 0;
};

USTRUCT(BlueprintType)
struct FGBWFootIKAutoFootLockSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MinDurationBetweenLock = 0.3f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float SpeedToUnlock = 100.0f;

	FGBWFootIKAutoFootLockSettings(){}
};
UENUM(BlueprintType)
enum EGBWFootIKControlType
{
	AutoControl,
	ByAnimCurve,
	ByInterface,
	ByAnimNotify
};
USTRUCT(BlueprintType)
struct FGBWFootLockControlSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EGBWFootIKControlType> ControlType = EGBWFootIKControlType::AutoControl;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ControlType == EGBWFootIKControlType::AutoControl"))
	float MinDurationBetweenLock = 0.3f;
	
	//When the character's speed is below this value, LockRate will be 0.0
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ControlType == EGBWFootIKControlType::AutoControl"))
	float SpeedToUnlock = 100.0f;

	//When the height of the foot from the ground is below this value and above LockShotHeight, the LockRate will be between 0.0 and 1.0.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ControlType == EGBWFootIKControlType::AutoControl"))
	float LockStartHeight = 5.0f;
	//When the height of the foot from the ground is below this value and above LockFinishedHeight, the LockRate will increase Smooth to 1.0.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ControlType == EGBWFootIKControlType::AutoControl"))
	float LockShotHeight = 3.0f;
	//When the height of the foot from the ground is below this value, LockRate will be directly set to 1.0.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ControlType == EGBWFootIKControlType::AutoControl"))
	float LockFinishedHeight = 2.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ControlType == EGBWFootIKControlType::ByAnimCurve"))
	FName LockRateCurveName = FName();

	FGBWFootLockControlSettings(){}
	FGBWFootLockControlSettings(
		const EGBWFootIKControlType InControlType,
		const float InMinDurationBetweenLock,
		const float InSpeedToUnlock,
		const FName InLockRateCurveName) :
	ControlType(InControlType),
	MinDurationBetweenLock(InMinDurationBetweenLock),
	SpeedToUnlock(InSpeedToUnlock),
	LockRateCurveName(InLockRateCurveName){}
};
USTRUCT(BlueprintType)
struct FGBWFootIKControlSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<EGBWFootIKControlType> ControlType = EGBWFootIKControlType::AutoControl;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(EditConditionHides,EditCondition="ControlType == EGBWFootIKControlType::ByAnimCurve"))
	FName IKRateCurveName = FName();

	FGBWFootIKControlSettings(){}
	FGBWFootIKControlSettings(
		const EGBWFootIKControlType InControlType,
		const FName InIKRateCurveName) :
	ControlType(InControlType),
	IKRateCurveName(InIKRateCurveName){}
};
USTRUCT(BlueprintType)
struct FGBWFootIKFootSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FName BoneName = FName();

	//This is the setting used to control IKRate.
	//The first currently valid control method will be selected in order from the configured control methods in the array.
	//ControlType = AutoControl means that when the character is on the ground, the IKRate will be 1.0 Otherwise, IKRate will be 0.0.
	//ControlType = ByAnimCurve means that the IKRate will be controlled by a AnimCurve.
	//ControlType = ByInterface means that the IKRate will be controlled by a Interface. The implementation of Interface can be in the character class or in the component class of the character.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(TitleProperty = "ControlType"))
	TArray<FGBWFootIKControlSettings> IKControlSettings;
	
	//This is the setting used to control FootLockRate.
	//The first currently valid control method will be selected in order from the configured control methods in the array.
	//ControlType = AutoControl means that when the character is on the ground, the LockRate is automatically set based on the distance between the character's feet and the ground.
	//ControlType = ByAnimCurve means that the LockRate will be controlled by a AnimCurve.
	//ControlType = ByInterface means that the LockRate will be controlled by a Interface. The implementation of Interface can be in the character class or in the component class of the character.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
		meta=(TitleProperty = "ControlType"))
	TArray<FGBWFootLockControlSettings> LockControlSettings;

	FGBWFootIKFootSettings(){}
	FGBWFootIKFootSettings(
		const FName InBoneName,
		const TArray<FGBWFootIKControlSettings>& InIKControlSettings,
		const TArray<FGBWFootLockControlSettings>& InLockControlSettings):
	BoneName(InBoneName),
	IKControlSettings(InIKControlSettings),
	LockControlSettings(InLockControlSettings){}
};
USTRUCT(BlueprintType)
struct FGBWFootIKSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bIsDebug = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float FootHeight = 8.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float FootTraceRange = 45.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float RootHeightSmoothSpeed = 16.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float RootMaxHeightDiff = 60.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFootIKFootSettings LeftFoot = FGBWFootIKFootSettings(
		FName(TEXT("foot_l")),
		{
			FGBWFootIKControlSettings(EGBWFootIKControlType::ByAnimCurve, FName(TEXT("IKRate_L"))),
			FGBWFootIKControlSettings(EGBWFootIKControlType::ByInterface, FName(TEXT("IKRate_L"))),
			FGBWFootIKControlSettings(EGBWFootIKControlType::AutoControl, FName(TEXT("IKRate_L"))),
			},
		{
			FGBWFootLockControlSettings(EGBWFootIKControlType::ByAnimNotify,0.3f,100.0f,FName(TEXT("FootLock_L")))/*,
			FGBWFootLockControlSettings(EGBWFootIKControlType::ByAnimCurve,0.3f,100.0f,FName(TEXT("FootLock_L"))),
			FGBWFootLockControlSettings(EGBWFootIKControlType::ByInterface,0.3f,100.0f,FName(TEXT("FootLock_L"))),
			FGBWFootLockControlSettings(EGBWFootIKControlType::AutoControl,0.3f,100.0f,FName(TEXT("FootLock_L")))*/
			});
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FGBWFootIKFootSettings RightFoot = FGBWFootIKFootSettings(
		FName(TEXT("foot_r")),
		{
			FGBWFootIKControlSettings(EGBWFootIKControlType::ByAnimCurve, FName(TEXT("IKRate_R"))),
			FGBWFootIKControlSettings(EGBWFootIKControlType::ByInterface, FName(TEXT("IKRate_R"))),
			FGBWFootIKControlSettings(EGBWFootIKControlType::AutoControl, FName(TEXT("IKRate_R"))),
			},
		{
			FGBWFootLockControlSettings(EGBWFootIKControlType::ByAnimNotify,0.3f,100.0f,FName(TEXT("FootLock_R")))/*,
			FGBWFootLockControlSettings(EGBWFootIKControlType::ByAnimCurve,0.3f,100.0f,FName(TEXT("FootLock_R"))),
			FGBWFootLockControlSettings(EGBWFootIKControlType::ByInterface,0.3f,100.0f,FName(TEXT("FootLock_R"))),
			FGBWFootLockControlSettings(EGBWFootIKControlType::AutoControl,0.3f,100.0f,FName(TEXT("FootLock_R")))*/
			});
};
UENUM()
enum EGBWFootIKLockStateType
{
	LockIn,
	Locked,
	LockOut,
	UnlockIn,
	Unlocked,
	UnlockOut
};
USTRUCT()
struct FGBWFootIKFootData
{
	GENERATED_BODY()
	
	float Ik_Rate = 1.0f;
	float Lock_Rate = 0.0f;
	float FootUnlockInterpSpeed = 12.0f;

	float HeightFromFloor = -1.0f;
	FVector FloorLocation = FVector::ZeroVector;
	FVector FloorNormal = FVector::ZeroVector;
	FRotator FloorRotation = FRotator::ZeroRotator;
	FHitResult FloorHitResult = FHitResult();
	
	TEnumAsByte<EGBWFootIKLockStateType> LockStateType = EGBWFootIKLockStateType::Unlocked;
	float LastLockStateTime = 0.0f;
	float LastLockShotTime = 0.0f;
	float TimeNow = 0.0f;
	float MinDurationBetweenLock = 0.0f;
	bool bReadyToLockShot = true;
	float ReadyToLockShotTimer = 0.0f;
	
	float Lock_Alpha = 0.0f;
	
	FVector Lock_Location_World = FVector::ZeroVector;
	FRotator Lock_Rotation_World = FRotator::ZeroRotator;
	
	FVector Lock_Location = FVector::ZeroVector;
	FRotator TargetLock_Rotation = FRotator::ZeroRotator;
	FRotator Lock_Rotation = FRotator::ZeroRotator;
	FVector Target_LocationOffset = FVector::ZeroVector;
	FRotator Target_RotationOffset = FRotator::ZeroRotator;
	
	FCompactPoseBoneIndex UpperLimbBoneIndex = FCompactPoseBoneIndex(INDEX_NONE);
	FCompactPoseBoneIndex LowerLimbBoneIndex = FCompactPoseBoneIndex(INDEX_NONE);
	FCompactPoseBoneIndex TargetLimbBoneIndex = FCompactPoseBoneIndex(INDEX_NONE);
	
	FTransform UpperLimbCSTransform = FTransform();
	FTransform LowerLimbCSTransform = FTransform();
	FTransform TargetLimbCSTransform = FTransform();
	
	FTransform LockFootBoneWorldTransform = FTransform();
	FTransform TargetFootBoneWorldTransform = FTransform();
	FVector TargetKneeBoneWorldLocation = FVector::ZeroVector;

	FTransform RealFootBoneWorldTransform = FTransform();
	float FootBoneDiffBetweenRealAndTarget = 0.0f;
	
	FVector FootTraceWorldLocation = FVector::ZeroVector;
	FVector FootTraceWorldLocation_Start = FVector::ZeroVector;
	FVector FootTraceWorldLocation_End = FVector::ZeroVector;

	FTransform UnadjustedFootBoneCSTransform = FTransform();
	FTransform UnadjustedKneeBoneCSTransform = FTransform();
	FTransform UnadjustedThighBoneCSTransform = FTransform();

	float MinHeightFromFloor = 999.99f;
	FGBWFloatQueue HeightFromFloorQueue = FGBWFloatQueue(120);

	bool bHasFootStepEvent = false;
	bool bReadyToFootStepEvent = true;
};
USTRUCT()
struct FGBWFootIKData
{
	GENERATED_BODY()
	
	bool bActive = true;
	bool bShouldDisableFootIK = false;

	FCompactPoseBoneIndex RootBoneIndex = FCompactPoseBoneIndex(INDEX_NONE);
	FName RootBoneName = NAME_None;
	FCompactPoseBoneIndex PelvisIndex = FCompactPoseBoneIndex(INDEX_NONE);
	FName PelvisBoneName = NAME_None;
	
	FGBWFootIKFootData LeftFoot = FGBWFootIKFootData();
	FGBWFootIKFootData RightFoot = FGBWFootIKFootData();
	
	FVector RootOffset = FVector::ZeroVector;
	float RootAlpha = 0.0f;
	
	FVector PelvisOffset = FVector::ZeroVector;
	float PelvisAlpha = 0.0f;
	
	FVector TargetRootBoneWorldLocation = FVector::ZeroVector;
	
	FTransform UnadjustedRootBoneCSTransform = FTransform();
	FTransform UnadjustedPelvisBoneCSTransform = FTransform();
	FTransform TargetPelvisBoneWorldTransform = FTransform();

	bool IsValid() const
	{
		return RootBoneIndex != INDEX_NONE
			&& PelvisIndex != INDEX_NONE
			&& PelvisBoneName != NAME_None
			&& LeftFoot.UpperLimbBoneIndex != INDEX_NONE
			&& LeftFoot.LowerLimbBoneIndex != INDEX_NONE
			&& LeftFoot.TargetLimbBoneIndex != INDEX_NONE
			&& RightFoot.UpperLimbBoneIndex != INDEX_NONE
			&& RightFoot.LowerLimbBoneIndex != INDEX_NONE
			&& RightFoot.TargetLimbBoneIndex != INDEX_NONE;
	}
	bool IsValidForPose(const FCompactPose& InPose) const
	{
		return InPose.IsValidIndex(RootBoneIndex) && !InPose[RootBoneIndex].ContainsNaN()
			&& InPose.IsValidIndex(PelvisIndex) && !InPose[PelvisIndex].ContainsNaN()
			&& RootBoneName != NAME_None
			&& PelvisBoneName != NAME_None
			&& InPose.IsValidIndex(LeftFoot.UpperLimbBoneIndex) && !InPose[LeftFoot.UpperLimbBoneIndex].ContainsNaN()
			&& InPose.IsValidIndex(LeftFoot.LowerLimbBoneIndex) && !InPose[LeftFoot.LowerLimbBoneIndex].ContainsNaN()
			&& InPose.IsValidIndex(LeftFoot.TargetLimbBoneIndex) && !InPose[LeftFoot.TargetLimbBoneIndex].ContainsNaN()
			&& InPose.IsValidIndex(RightFoot.UpperLimbBoneIndex) && !InPose[RightFoot.UpperLimbBoneIndex].ContainsNaN()
			&& InPose.IsValidIndex(RightFoot.LowerLimbBoneIndex) && !InPose[RightFoot.LowerLimbBoneIndex].ContainsNaN()
			&& InPose.IsValidIndex(RightFoot.TargetLimbBoneIndex) && !InPose[RightFoot.TargetLimbBoneIndex].ContainsNaN();
	}
};

UCLASS()
class GBWFOOTIK_API UGBWFootIKFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	//Ik
	static bool MakeFootIKData(
		ACharacter* OwnerCharacter,
		float DeltaTime,
		FGBWFootIKData& FootIKData,
		FGBWFootIKSettings FootIKSettings,
		FCSPose<FCompactPose>& UnadjustedCompactPose,
		const FBoneContainer& BoneContainer);
	static void DebugFootIKData(
		const UWorld* InWorld,
		const USkeletalMeshComponent* OwnerSkMeshComponent,
		const FGBWFootIKData& FootIKData);
};