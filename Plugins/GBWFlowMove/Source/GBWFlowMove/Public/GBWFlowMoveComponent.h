// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWFlowMoveObjects.h"
#include "AnimNodes/GBWMagicPlayerInterface.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "RMS/GBWRMSInterface.h"
#include "Blueprint/UserWidget.h"
#include "GBWFlowMoveComponent.generated.h"

UCLASS(Blueprintable, editinlinenew, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GBWFLOWMOVE_API UGBWFlowMoveComponent : public UActorComponent, public IGBWRMSInterface, public IGBWMagicPlayerInterface
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlowMoveDlg, FFlowMoveEvent, FlowMoveEvent);
	GENERATED_BODY()

public:	
	UGBWFlowMoveComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void BeginPlay() override;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//BEGIN Runtime
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GBW|FlowMove")
	FGBWFlowMoveInputState InputState = FGBWFlowMoveInputState();
	
	UPROPERTY(BlueprintReadOnly, Category = "GBW|FlowMove")
	FGBWFlowMoveState TaskState = FGBWFlowMoveState();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GBW|FlowMove")
	FGBWFlowMoveMoveControlInitInfo CurrentActionTargetSceneInfo = FGBWFlowMoveMoveControlInitInfo();
	UPROPERTY(BlueprintReadOnly, Category = "GBW|FlowMove")
	FGBWFlowMoveMoveControlResult MoveControlResult = FGBWFlowMoveMoveControlResult();

	bool bHasSetCapsuleAndMesh = false;
	float CapsuleHalfHeightCache = 90.0f;
	float CapsuleRadiusCache = 30.0f;
	FTransform MeshRelativeTransformCache = FTransform(
		FRotator(0.0f,0.0f,-90.0f),
		FVector(0.0f,0.0f,-90.0f),
		FVector(1.0f,1.0f,1.0f));
	
	float MovementCompMaxAccelerationCache = 0.0f;
	bool bUseControllerRotationYawCache = false;
	bool bIsConstrainToPlaneCache = false;
	FVector PlaneConstraintNormalCache = FVector::ZeroVector;
	FVector PlaneConstraintOriginCache = FVector::ZeroVector;
	
	bool bHasSetIgnoreClientMovementErrorChecksAndCorrection = false;
	bool bIgnoreClientMovementErrorChecksAndCorrectionCache = false;
	bool bServerAcceptClientAuthoritativePositionCache = false;
	
	UPROPERTY()
	UUserWidget* ActorTargetWidget = nullptr;
	UPROPERTY()
	USceneComponent* LastFocusActorComponent = nullptr;
	TSubclassOf<UUserWidget> ActorTargetWidgetClass = nullptr;
	UPROPERTY()
	USoundBase* Sound_TargetGet = nullptr;
	UPROPERTY()
	USoundBase* Sound_TargetLost = nullptr;
	float ActorTargetPrejudgeDistance = 0.0f;
	float ActorTargetFuzzyQueryAngleRestrictions = 180.0f;
	//END Runtime
	
	// BEGIN IGBWRMSInterface
	virtual bool GetRootMotionParamDirect(
		FTransform& Result,
		float SimulationTime,
		float MovementTickTime,
		const ACharacter& Character,
		const UCharacterMovementComponent& MoveComponent) override;
	// END IGBWRMSInterface

	// BEGIN IGBWMagicPlayerInterface
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
		const FName AnimSlot) override;
	// END IGBWMagicPlayerInterface

// BEGIN Dlg
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "GBW|FlowMove")
	FFlowMoveDlg OnFMEvent;
// END Dlg

	UFUNCTION()
	void OnEvent(bool bNetEvent, const FFlowMoveEvent& FlowMoveEvent);
	UFUNCTION(Reliable,Server,Category = "GBW|FlowMove|Action")
	void OnEvent_Server(const FFlowMoveEvent& FlowMoveEvent);
	UFUNCTION(Reliable, NetMulticast, Category = "GBW|FlowMove|Action")
	void OnEvent_Multicast(const FFlowMoveEvent& FlowMoveEvent);
	void OnEvent_Imp(const FFlowMoveEvent& FlowMoveEvent);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove")
	void SetBrian(TSubclassOf<UGBWFlowMoveBrain_Base> NewBrain);
	UFUNCTION(Reliable, Server, Category = "GBW|FlowMove")
	void SetBrian_Server(TSubclassOf<UGBWFlowMoveBrain_Base> NewBrain);

// BEGIN Get Input
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Input")
	void SetMoveVector(FVector NewMoveVector, float ZeroFaultToleranceDuration = 0.1f);
	UFUNCTION(Reliable, Server, Category = "GBW|FlowMove|Input")
	void SetMoveVector_Server(FVector_NetQuantizeNormal NewMoveVector);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Input")
	FVector GetMoveVector();

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Input")
	void SetControlVector(FVector NewControlVector);
	UFUNCTION(Unreliable, Server, Category = "GBW|FlowMove|Input")
	void SetControlVector_Server(FVector_NetQuantizeNormal ControlVector);
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Input")
	FVector GetControlVector();

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|AnimParam")
	void GetMoveParamForBlendSpace(
		UBlendSpace* ForBS,
		UBlendSpace*& BS,
		float& Horizontal,
		float& Vertical,
		float& PlayRate,
		float& StartPosition,
		float& EndPosition,
		EFlowMoveDirectionType DirectionType = EFlowMoveDirectionType::Velocity,
		EFMBSAxisType BS_HorizontalAxisFrom = EFMBSAxisType::HorizontalDirection,
		EFMBSAxisType BS_VerticalAxisFrom = EFMBSAxisType::Speed,
		float RefMaxSpeed = 900.0f,
		const bool bUseStep = false,
		const float Step = 8.0f);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|AnimParam")
	void GetMoveParam(UAnimationAsset* ForAnimAsset,
		UAnimSequence*& AnimSequence,
		float& PlayRate,
		float& StartPosition,
		float& EndPosition) const;
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|AnimParam")
	void GetMoveParamForAnimAsset(
		UAnimationAsset* ForAnimAsset,
		UAnimationAsset*& AnimAsset,
		float& Horizontal,
		float& Vertical,
		float& PlayRate,
		float& StartPosition,
		float& EndPosition,
		EFlowMoveDirectionType DirectionType = EFlowMoveDirectionType::Velocity,
		EFMBSAxisType BS_HorizontalAxisFrom = EFMBSAxisType::HorizontalDirection,
		EFMBSAxisType BS_VerticalAxisFrom = EFMBSAxisType::Speed,
		float RefMaxSpeed = 900.0f,
		const bool bUseStep = false,
		const float Step = 8.0f);

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|AnimParam")
	void GetMagicPlayerParam(
		UAnimationAsset* ForAnimAsset,
		FMagicPlayerParam& MagicPlayerParam,
		const bool InReplayNow = false,
		const bool bAutoReplayCodeTime = true,
		const float InReplayCodeTime = 0.1f,
		const bool InLoopAnimation = false,
		const float InBlendTime = 0.2f,
		const float InLoopBlendTime = 0.0f,
		const EFlowMoveDirectionType DirectionType = EFlowMoveDirectionType::CurrentMoveToDirection,
		const EFMBSAxisType BS_HorizontalAxisFrom = EFMBSAxisType::HorizontalDirection,
		const EFMBSAxisType BS_VerticalAxisFrom = EFMBSAxisType::Speed,
		float RefMaxSpeed = 900.0f,
		const bool bUseStep = false,
		const float Step = 8.0f);
// END Get Input

// BEGIN FlowMove Control
	bool ActiveReady();
	bool DeactivateReady();

	void SetMovementConstrain(const FVector& PlaneNormal, const FVector& PlaneOrigin) const;
// END FlowMove Control

// BEGIN FlowMove State
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State")
	void SetFlowMoveViewMode(EFMViewMode NewViewMode, bool bAsDefaultViewMode);
	UFUNCTION(Reliable, Server, Category = "GBW|FlowMove|State")
	void SetFlowMoveViewMode_Server(EFMViewMode NewViewMode, bool bAsDefaultViewMode);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|State")
	EFMViewMode GetFlowMoveViewMode() const;

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State")
	void SetFlowMoveSpeedMode(EFMSpeedMode NewSpeedMode);
	UFUNCTION(Reliable, Server, Category = "GBW|FlowMove|State")
	void SetFlowMoveSpeedMode_Server(EFMSpeedMode NewSpeedMode);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|State")
	EFMSpeedMode GetFlowMoveSpeedMode() const;

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State")
	void SetFlowMoveBodyMode(EFMBodyMode NewBodyMode);
	UFUNCTION(Reliable, Server, Category = "GBW|FlowMove|State")
	void SetFlowMoveBodyMode_Server(EFMBodyMode NewBodyMode);
	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|State")
	EFMBodyMode GetFlowMoveBodyMode() const;
	
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State")
	void SetFocusActor(USceneComponent* InComponent, bool bSetViewModeAsActorLock);
	UFUNCTION(Reliable, Server, Category = "GBW|FlowMove|State")
	void SetFocusActor_Server(USceneComponent* InComponent);
	UFUNCTION(Reliable, NetMulticast, Category = "GBW|FlowMove|State")
	void SetFocusActor_Multicast(USceneComponent* InComponent);
	void SetFocusActor_Imp(USceneComponent* InComponent);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State")
	void GetFocusActor(bool& bIsGet, USceneComponent*& FocusedComponent);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Movement")
	void SetMoveControlInitInfo_Simple(bool bHasTargetPoint, const FVector& TargetPoint);
	UFUNCTION(Reliable, Server, Category = "GBW|FlowMove|Movement")
	void SetMoveControlInitInfo_Simple_Server(bool bHasTargetPoint, const FVector_NetQuantize100& TargetPoint);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Movement")
	FGBWFlowMoveControlParam GetMoveControlParam() const;

	FVector GetMoveToVector();

	UFUNCTION(BlueprintCallable, Category="GBW|FlowMove|State")
	FVector GetDirectionVector(
		const EFlowMoveDirectionType Type,
		const FVector WorldAbsoluteVector);
	UFUNCTION(BlueprintCallable, Category="GBW|FlowMove|State")
	FVector GetDirectionVectorFrom(const FGBWFlowMoveDirection Direction);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|Movement")
	void AddVelocityTrend(FGBWFlowMoveVelocityTrend VelocityTrend);
	UFUNCTION(Reliable,Server,Category = "GBW|FlowMove|Movement")
	void AddVelocityTrend_Server(FGBWFlowMoveVelocityTrend VelocityTrend);
	
	UFUNCTION(BlueprintCallable, Category="GBW|FlowMove|State")
	void SendFootStepEvent(USkeletalMeshComponent* InSKMesh,
		FName InFootStepType,
		FName InFootStepSocket,
		FGBWFlowMoveTraceSetting FloorTraceSetting);

	UFUNCTION(BlueprintCallable, Category = "GBW|FlowMove|State")
	void SetMovementReplication(bool bReplication);

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|State")
	bool IsFmControlled() const;
// END FlowMove State

// BEGIN FlowMove Tools
	UFUNCTION(BlueprintPure,Category = "GBW|FlowMove|State")
	bool IsLocalPlayer(bool bIsReset = true);
	UFUNCTION(BlueprintPure,Category = "GBW|FlowMove|State")
	bool IsLocalOwn(bool bIsReset = true);
	UFUNCTION(BlueprintPure,Category = "GBW|FlowMove|State")
	bool IsInServer(bool bIsReset = true);
	
	UFUNCTION(BlueprintCallable,Category = "GBW|FlowMove|Tools")
	float GetActorScale() const;

	UFUNCTION(BlueprintCallable,Category = "GBW|FlowMove|Tools")
	FTransform ConvertLocalRootMotionToWorld(const FTransform& InTransform);
	UFUNCTION(BlueprintCallable,Category = "GBW|FlowMove|Tools")
	FTransform ConvertLocalRootMotionToActor(const FTransform& InTransform);
	// END FlowMove Tools

// BEGIN Config
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "FlowMoveSettings|ViewMode")
	TEnumAsByte<EFMViewMode> DefaultViewMode = EFMViewMode::TP_FreeMode;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FlowMoveSettings|Input")
	int MaxMoveVectorUpdateCountPerSecond = 30;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FlowMoveSettings|Input")
	int MaxControlVectorUpdateCountPerSecond = 30;

	UPROPERTY(Replicated)
	TSubclassOf<UGBWFlowMoveBrain_Base> FlowMoveBrainClass;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FlowMoveSettings")
	UGBWFlowMoveBrain_Base* FlowMoveBrain = nullptr;

	UFUNCTION(BlueprintCallable,Category="GBW|FlowMove")
	bool FlowMoveBrainValid();
// END Config
	
private:

	bool bIsLocalPlayer = false;
	bool bIsLocalPlayerInit = false;
	
	bool bIsLocalOwn = false;
	bool bIsLocalOwnInit = false;
	bool bIsInServer = false;
	bool bIsInServerInit = false;
	bool bIsFlowMoveActive= false;
	bool bIsFlowMoveActiveInit = false;

	int FrameNumber = INDEX_NONE;
	
//BEGIN Runtime
	//FlowMove Tick
	void FlowMoveTick(float DeltaTime);
	//Check ViewMode
	void CheckViewMode();
	void CheckUpdateActorLockUI();
	//Check Valid
	bool CheckUpdateValid();
	void EnsureRMSActivation();
	//Update CharacterNetState
	void UpdateCharacterState(float DeltaTime);
	//Update MoveControlParam And RMS
	void UpdateMoveControlParam(float DeltaTime);
	//Update RMS
	void UpdateRMS(float DeltaTime);
	//Move Adjust
	void MovingAdjust();
//END Runtime
};
