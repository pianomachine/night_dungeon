// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GBWPercetionDemo.generated.h"

UENUM(BlueprintType)
enum EGBWSurroundingsPointType
{
	TargetPoint,
	ActorPoint,
	LeftSidePoint,
	RightSidePoint,
	FloorPoint,
	RoofPoint,
	FloorForwardLedgePoint,
	FloorBackwardLedgePoint,
	NearestFloorLedgePoint,
	RoofForwardLedgePoint,
	RoofBackwardLedgePoint,
	NearestRoofLedgePoint,
	TargetPointFloorPoint
};

USTRUCT(BlueprintType)
struct FGBWCognition_TargetInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float Height = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float Right = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float Forward = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float PitchAngle = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float FaceDirection = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsHole = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float SpaceHeight = 0.0f;
	
	FGBWCognition_TargetInfo(){}
};

USTRUCT(BlueprintType)
struct FGBWCognition_ActorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector HeadLocation = FVector();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector FootLocation = FVector();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float ActorHeight = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float HeightFromGround = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float HeightFromRoof = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsHole = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float SpaceHeight = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsSide = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float CloserSideDistance = 999999.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsLeftSideCloser = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsRightSideCloser = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsForwardSideCloser = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float LeftSideDistance = 999999.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float RightSideDistance = 999999.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float ForwardSideDistance = 999999.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFloorLedge = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsRoofLedge = false;
	
	FGBWCognition_ActorInfo(){}
};

USTRUCT(BlueprintType)
struct FGBWCognition_FloorLedge
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFloorLedge = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector FloorLedgePoint = FVector();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector FloorLedge_Direction = FVector();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFloorLedge_Floor = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector FloorLedge_FloorPoint = FVector();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFloorLedge_FloorWalkable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFloorLedge_Roof = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector FloorLedge_RoofPoint = FVector();

	FGBWCognition_FloorLedge(){}
};
USTRUCT(BlueprintType)
struct FGBWCognition_Floor
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFloor = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Floor = FVector();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFloorWalkable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFootStuck = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TArray<FVector> FootStuckPoints;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWCognition_FloorLedge FloorLedge_F = FGBWCognition_FloorLedge();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWCognition_FloorLedge FloorLedge_B = FGBWCognition_FloorLedge();

	FGBWCognition_Floor(){}
};
USTRUCT(BlueprintType)
struct FGBWCognition_Roof
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsRoof = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Roof = FVector();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsRoofLedge_F = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector RoofLedge_F = FVector();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsRoofLedge_B = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector RoofLedge_B = FVector();

	FGBWCognition_Roof(){}
};

USTRUCT(BlueprintType)
struct FGBWCognition_Side
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsForward = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Forward = FVector();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Forward_Direction = FVector();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsLeft = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Left = FVector();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Left_Direction = FVector();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsRight = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Right = FVector();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Right_Direction = FVector();

	FGBWCognition_Side(){}
};

USTRUCT(BlueprintType)
struct FGBWCognition_ActorState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	AActor* SelfActor = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FTransform ActorTransform = FTransform();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TEnumAsByte<EMovementMode> MovementMode = EMovementMode::MOVE_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float Slope = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsSlopeWalkable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWCognition_Side Side = FGBWCognition_Side();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWCognition_Floor Floor = FGBWCognition_Floor();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWCognition_Roof Roof = FGBWCognition_Roof();

	FGBWCognition_ActorState(){}
};

USTRUCT(BlueprintType)
struct FGBWCognition_Target
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsBlocked = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFindTarget = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector TargetPoint = FVector();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	AActor* TargetActor = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	USceneComponent* TargetComponent = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsFloor = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Floor = FVector();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bIsRoof = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FVector Roof = FVector();
	
	FGBWCognition_Target(){}
};

USTRUCT(BlueprintType)
struct FGBWCognition_Surroundings
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsValid = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	FName Type = NAME_None;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWCognition_ActorState ActorState = FGBWCognition_ActorState();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWCognition_Target TargetState = FGBWCognition_Target();

	UPROPERTY()
	bool bHasInit = false;
	
	UPROPERTY()
	FGBWCognition_TargetInfo TargetInfo = FGBWCognition_TargetInfo();
	UPROPERTY()
	FGBWCognition_ActorInfo ActorInfo = FGBWCognition_ActorInfo();
	
	FGBWCognition_Surroundings(){}
};


UENUM(BlueprintType)
enum EInclusivityType
{
	Include_,
	NotInclude_
};
USTRUCT(BlueprintType)
struct FGBWFloatScope
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float Min = -1000000.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float Max = 1000000.0f;

	FGBWFloatScope(){}
	bool InScope(const float InValue) const;
};
USTRUCT(BlueprintType)
struct FSceneTypeCondition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TEnumAsByte<EInclusivityType> ConditionType = EInclusivityType::Include_;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TSet<FName> SceneTypeCondition;
};
USTRUCT(BlueprintType)
struct FSceneDirectionCondition
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings",
		meta=(EditConditionHides,EditCondition="ConditionType == EDirectionConditionType::ValueRange"))
	TArray<FGBWFloatScope> SceneDirectionRange;
	
	bool IsMet(const FGBWCognition_Surroundings& Scene) const;
};
USTRUCT(BlueprintType)
struct FMovementModeCondition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TEnumAsByte<EInclusivityType> ConditionType = EInclusivityType::Include_;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	TSet<TEnumAsByte<EMovementMode>> MovementMode;
};

UCLASS()
class GBWPOWERFULPERCEPTION_API UGBWPerceptionDemoFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings"
		, meta=(BlueprintThreadSafe, ExpandBoolAsExecs = "bIsGet", DefaultToSelf="InActor"))
	static void GetSurroundingsCognition_RealTime(AActor* InActor,
		FGameplayTag Key,
		FGBWCognition_Surroundings& SurroundingsCognition,
		bool& bIsGet);
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings"
		, meta=(BlueprintThreadSafe, ExpandBoolAsExecs = "bIsGet", DefaultToSelf="InActor"))
	static void GetSurroundingsCognition_Snapshot(AActor* InActor,
		FGameplayTag Key,
		FGBWCognition_Surroundings& SurroundingsCognition,
		bool& bIsGet);

	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings"
	, meta=(BlueprintThreadSafe, ExpandBoolAsExecs = "bIsGet", DefaultToSelf="InActor"))
	static void PPD_GetSurroundingsCognition_RealTime(AActor* InActor,
		FGameplayTag Key,
		bool& bIsGet,
		FGBWCognition_Surroundings& SurroundingsCognition,
		FGBWCognition_ActorInfo& ActorInfo, FGBWCognition_TargetInfo& TargetInfo);
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings"
	, meta=(BlueprintThreadSafe, ExpandBoolAsExecs = "bIsGet", DefaultToSelf="InActor"))
	static void PPD_GetSurroundingsCognition_Snapshot(AActor* InActor,
		FGameplayTag Key,
		bool& bIsGet,
		FGBWCognition_Surroundings& SurroundingsCognition,
		FGBWCognition_ActorInfo& ActorInfo, FGBWCognition_TargetInfo& TargetInfo);
	
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings"
		, meta=(BlueprintThreadSafe, DefaultToSelf="AnimInstance"))
	static void PPD_Anim_GetSurroundingsCognition_RealTime(const UAnimInstance* AnimInstance,
		FGameplayTag Key,
		bool& bIsGet,
		FGBWCognition_Surroundings& SurroundingsCognition,
		FGBWCognition_ActorInfo& ActorInfo, FGBWCognition_TargetInfo& TargetInfo);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings"
		, meta=(BlueprintThreadSafe, DefaultToSelf="AnimInstance"))
	static void PPD_Anim_GetSurroundingsCognition_Snapshot(const UAnimInstance* AnimInstance,
		FGameplayTag Key,
		bool& bIsGet,
		FGBWCognition_Surroundings& SurroundingsCognition,
		FGBWCognition_ActorInfo& ActorInfo, FGBWCognition_TargetInfo& TargetInfo);

	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings", meta=(BlueprintThreadSafe, ExpandBoolAsExecs = "bIsGet"))
	static void GetSurroundingsAnalysisResults(FGBWCognition_Surroundings SurroundingsCognition, FGBWCognition_ActorInfo& ActorInfo, FGBWCognition_TargetInfo& TargetInfo, bool& bIsGet);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings", meta=(BlueprintThreadSafe))
	static FGBWCognition_Surroundings InitSurroundings(FGBWCognition_Surroundings Cognition);
	
	UFUNCTION(meta=(BlueprintThreadSafe))
	static void InitCognitionSurroundings(FGBWCognition_Surroundings& Cognition);

	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings", meta=(BlueprintThreadSafe))
	static bool GetPlaneConstraintSetting(FGBWCognition_Surroundings Cognition, FVector& PlaneNormal, FVector& PlaneOrigin);

	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings", meta=(BlueprintThreadSafe))
	static FVector GetSurroundingsPoint(FGBWCognition_Surroundings Cognition, EGBWSurroundingsPointType PointType);
	
//Cognition_Surroundings Analysis Start

	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsSceneType(AActor* InActor,
		FGameplayTag TargetScene,
		FSceneTypeCondition SceneTypeCondition);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsSceneType(const UAnimInstance* AnimInstance,
									FGameplayTag TargetScene,
									const FSceneTypeCondition& SceneTypeCondition);
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsSceneType_Simple(AActor* InActor,
		FGameplayTag TargetScene,
		TEnumAsByte<EInclusivityType> ConditionType = EInclusivityType::Include_,
		FName SceneType = NAME_None);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsSceneType_Simple(const UAnimInstance* AnimInstance,
		FGameplayTag TargetScene,
		TEnumAsByte<EInclusivityType> ConditionType = EInclusivityType::Include_,
		FName SceneType = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static void PPD_IsSide(AActor* InActor,
		FGameplayTag TargetScene,
		bool& IsLeft,
		bool& IsRight,
		float& LeftDistance,
		float& RightDistance,
		bool& IsLeftCloser,
		bool& IsRightCloser);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static void PPD_Anim_IsSide(const UAnimInstance* AnimInstance, 
		FGameplayTag TargetScene,
		bool& IsLeft,
		bool& IsRight,
		float& LeftDistance,
		float& RightDistance,
		bool& IsLeftCloser,
		bool& IsRightCloser);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsSceneHeight(AActor* InActor,
		FGameplayTag TargetScene,
		FGBWFloatScope SceneHeightRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsSceneHeight(const UAnimInstance* AnimInstance, 
		FGameplayTag TargetScene,
		FGBWFloatScope SceneHeightRange);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsSceneRight(AActor* InActor,
		FGameplayTag TargetScene,
		FGBWFloatScope SceneRightRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsSceneRight(const UAnimInstance* AnimInstance, 
		FGameplayTag TargetScene,
		FGBWFloatScope SceneRightRange);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsSceneForward(AActor* InActor,
		FGameplayTag TargetScene,
		FGBWFloatScope SceneForwardRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsSceneForward(const UAnimInstance* AnimInstance, 
		FGameplayTag TargetScene,
		FGBWFloatScope SceneForwardRange);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsSceneAngle(AActor* InActor,
		FGameplayTag TargetScene,
		FGBWFloatScope SceneAngleRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsSceneAngle(const UAnimInstance* AnimInstance, 
		FGameplayTag TargetScene,
		FGBWFloatScope SceneAngleRange);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsSceneDirection(AActor* InActor,
		FGameplayTag TargetScene,
		FSceneDirectionCondition SceneDirectionCondition);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsSceneDirection(const UAnimInstance* AnimInstance, 
		FGameplayTag TargetScene,
		const FSceneDirectionCondition& SceneDirectionCondition);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsMovementMode(AActor* InActor, FMovementModeCondition MovementModeCondition);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsMovementMode(const UAnimInstance* AnimInstance, const FMovementModeCondition& MovementModeCondition);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsHeightFromGround(AActor* InActor,
		FGameplayTag TargetScene,
		FGBWFloatScope HeightFromGroundRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsHeightFromGround(const UAnimInstance* AnimInstance,
		FGameplayTag TargetScene,
		FGBWFloatScope HeightFromGroundRange);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsHeightFromRoof(AActor* InActor,
		FGameplayTag TargetScene,
		FGBWFloatScope HeightFromRoofRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsHeightFromRoof(const UAnimInstance* AnimInstance,
		FGameplayTag TargetScene,
		FGBWFloatScope HeightFromRoofRange);

	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsFloor(AActor* InActor,
		FGameplayTag TargetScene,
		FGBWFloatScope HeightFromFloorRange,
		bool bIsAnyRequireLedge,
		FGBWFloatScope FloorLedgeRange,
		bool bIsRequireForwardLedge,
		FGBWFloatScope ForwardFloorLedgeRange,
		bool bIsRequireBackwardLedge,
		FGBWFloatScope BackwardFloorLedgeRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsFloor(const UAnimInstance* AnimInstance,
		FGameplayTag TargetScene,
		FGBWFloatScope HeightFromFloorRange,
		bool bIsAnyRequireLedge,
		FGBWFloatScope FloorLedgeRange,
		bool bIsRequireForwardLedge,
		FGBWFloatScope ForwardFloorLedgeRange,
		bool bIsRequireBackwardLedge,
		FGBWFloatScope BackwardFloorLedgeRange);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsRoof(AActor* InActor,
		FGameplayTag TargetScene,
		FGBWFloatScope HeightFromRoofRange,
		bool bIsAnyRequireLedge,
		FGBWFloatScope RoofLedgeRange,
		bool bIsRequireForwardLedge,
		FGBWFloatScope ForwardRoofLedgeRange,
		bool bIsRequireBackwardLedge,
		FGBWFloatScope BackwardRoofLedgeRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsRoof(const UAnimInstance* AnimInstance,
		FGameplayTag TargetScene,
		FGBWFloatScope HeightFromRoofRange,
		bool bIsAnyRequireLedge,
		FGBWFloatScope RoofLedgeRange,
		bool bIsRequireForwardLedge,
		FGBWFloatScope ForwardRoofLedgeRange,
		bool bIsRequireBackwardLedge,
		FGBWFloatScope BackwardRoofLedgeRange);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Cognition|Demo|Surroundings|Condition", meta=(DefaultToSelf="InActor"))
	static bool PPD_IsSlope(AActor* InActor, FGameplayTag TargetScene, FGBWFloatScope SlopeRange);
	UFUNCTION(BlueprintPure, Category = "GBW|Cognition|Demo|Surroundings|Pure|Condition", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static bool PPD_Anim_IsSlope(const UAnimInstance* AnimInstance, FGameplayTag TargetScene, FGBWFloatScope SlopeRange);
	//Cognition_Surroundings Analysis End
};