// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Animation/PoseSnapshot.h"
#include "GBWAnimComponent.generated.h"

USTRUCT(BlueprintType)
struct FMagicPlayerSyncValueInfo
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	FName Name = NAME_None;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float Value = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float DeltaValue = 0.0f;
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category="Settings")
	float Time = 0.0f;
	
	FMagicPlayerSyncValueInfo(){}
	FMagicPlayerSyncValueInfo(const FName InName,
	                          const float InValue,
	                          const float InDeltaValue,
	                          const float InTime):
	Name(InName),
	Value(InValue),
	DeltaValue(InDeltaValue),
	Time(InTime)
	{}
};
UENUM(BlueprintType)
enum EMagicPlayerBoneControlType
{
	DirectAdjust,
	RelativeToSpecifiedComponent
};
UENUM(BlueprintType)
enum EMagicPlayerBoneControlTargetType
{
	MainActor,
	LinkedActor,
	Both
};
USTRUCT(BlueprintType)
struct FMagicPlayerBoneControl
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EMagicPlayerBoneControlTargetType> Target = EMagicPlayerBoneControlTargetType::MainActor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FName BoneName = FName();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float SmoothSpeed_Enter = 6.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FTransform TargetTransform = FTransform();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EBoneModificationMode> ScaleMode = BMM_Ignore;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EBoneModificationMode> RotationMode = BMM_Ignore;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EBoneModificationMode> TranslationMode = BMM_Ignore;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EMagicPlayerBoneControlType> BoneControlType = EMagicPlayerBoneControlType::DirectAdjust;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings,
		meta=(EditConditionHides, EditCondition="BoneControlType == EMagicPlayerBoneControlType::DirectAdjust"))
	TEnumAsByte<enum EBoneControlSpace> ScaleSpace = EBoneControlSpace::BCS_ComponentSpace;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings,
		meta=(EditConditionHides, EditCondition="BoneControlType == EMagicPlayerBoneControlType::DirectAdjust"))
	TEnumAsByte<enum EBoneControlSpace> RotationSpace = EBoneControlSpace::BCS_ComponentSpace;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings,
		meta=(EditConditionHides, EditCondition="BoneControlType == EMagicPlayerBoneControlType::DirectAdjust"))
	TEnumAsByte<enum EBoneControlSpace> TranslationSpace = EBoneControlSpace::BCS_ComponentSpace;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Settings,
		meta=(EditConditionHides, EditCondition="BoneControlType == EMagicPlayerBoneControlType::RelativeToSpecifiedComponent"))
	FName ComponentTag = NAME_None;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Settings,
		meta=(EditConditionHides, EditCondition="BoneControlType == EMagicPlayerBoneControlType::RelativeToSpecifiedComponent"))
	FName SocketName = NAME_None;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Settings,
		meta=(EditConditionHides, EditCondition="BoneControlType == EMagicPlayerBoneControlType::RelativeToSpecifiedComponent"))
	bool bTryGetComponentFromChildOrAttachedActors = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Settings,
		meta=(EditConditionHides, EditCondition="BoneControlType == EMagicPlayerBoneControlType::RelativeToSpecifiedComponent && bTryGetComponentFromChildOrAttachedActors"))
	FName ChildOrAttachedActorTag = NAME_None;

	UPROPERTY()
	float Alpha = 1.0f;
	
	FMagicPlayerBoneControl(){}
	FMagicPlayerBoneControl(
		const FName InBoneName,
		const FTransform& InTargetTransform,
		const EBoneModificationMode InScaleMode,
		const EBoneModificationMode InRotationMode,
		const EBoneModificationMode InTranslationMode,
		const EBoneControlSpace InScaleSpace,
		const EBoneControlSpace InRotationSpace,
		const EBoneControlSpace InTranslationSpace,
		const float InAlpha)
	:BoneName(InBoneName),
	TargetTransform(InTargetTransform),
	ScaleMode(InScaleMode),
	RotationMode(InRotationMode),
	TranslationMode(InTranslationMode),
	ScaleSpace(InScaleSpace),
	RotationSpace(InRotationSpace),
	TranslationSpace(InTranslationSpace),
	Alpha(InAlpha)
	{
		BoneControlType = EMagicPlayerBoneControlType::DirectAdjust;
		SmoothSpeed_Enter = 0.0f;
	}
};
USTRUCT(BlueprintType)
struct FMagicPlayerBoneControlParam
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TArray<FMagicPlayerBoneControl> BoneControlQueue;
	
	FMagicPlayerBoneControlParam(){}
};
UENUM(BlueprintType)
enum EMagicPlayerLinkType
{
	NoLink,
	Complete,
	OnlyAnimAsset
};
USTRUCT(BlueprintType)
struct FMagicPlayerLinkedAnim
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	UAnimationAsset* AnimAsset = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bAsMainState = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bAsRootMotionSource = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bTriggerMagicPlayerTickInterface = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EMagicPlayerLinkType> LinkType = EMagicPlayerLinkType::Complete;

	FORCEINLINE bool operator == (const FMagicPlayerLinkedAnim& Other) const;

	FMagicPlayerLinkedAnim(){}
};
USTRUCT(BlueprintType)
struct FMagicPlayerBlendInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	UAnimationAsset* AnimationAsset = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float BlendTimer = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float BlendTime = 0.2f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float BlendAlpha = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	int BlendFrames = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float InternalTimeAccumulator = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bIsLoopAnim = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TArray<FBlendSampleData> BlendSampleDataCache;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FPoseSnapshot PoseSnapshot;

	FMagicPlayerBlendInfo(){}
	FMagicPlayerBlendInfo(
		UAnimationAsset* InAnim,
		const float InBlendTime,
		const bool InIsLoopAnim,
		const float InInternalTimeAccumulator,
		const TArray<FBlendSampleData>& InBlendSampleDataCache,
		const FPoseSnapshot& InPoseSnapshot) :
	AnimationAsset(InAnim),
	BlendTime(InBlendTime),
	InternalTimeAccumulator(InInternalTimeAccumulator),
	bIsLoopAnim(InIsLoopAnim),
	BlendSampleDataCache(InBlendSampleDataCache),
	PoseSnapshot(InPoseSnapshot)
	{}

	bool CheckUpdate(const float DeltaTime)
	{
		if (!IsValid())
		{
			return false;
		}
		BlendFrames++;
		BlendTimer+=DeltaTime;
		InternalTimeAccumulator+=DeltaTime;
		BlendAlpha = BlendTime > BlendTimer? BlendTimer / BlendTime : 1.0f;
			/*FMath::FInterpTo(BlendAlpha,
			BlendTime > BlendTimer? BlendTimer / BlendTime : 1.0f,
			DeltaTime,30.0f);*/
		return BlendAlpha < 1.0f;
	}
	
	float GetBlendAlpha() const
	{
		return BlendAlpha;
	}

	bool IsValid() const
	{
		return AnimationAsset || PoseSnapshot.bIsValid;
	}
};
USTRUCT(BlueprintType)
struct FMagicPlayerParam
{
	GENERATED_BODY()
	
	UPROPERTY()
	double Time = 0.0f;
	
	// The animation asset to play
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TObjectPtr<UAnimationAsset> AnimAsset = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TArray<FMagicPlayerLinkedAnim> LinkedAnimAsset;

	UPROPERTY()
	uint32 PlayStateHash = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bReplayNow = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float ReplayCodeTime = 0.1f;

	// The horizontal coordinate to sample in the blendspace
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Coordinates)
	float Horizontal = 0.0f;

	// The vertical coordinate to sample in the blendspace
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Coordinates)
	float Vertical = 0.0f;
	
	// The play rate multiplier. Can be negative, which will cause the animation to play in reverse.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float PlayRate = 1.0f;

	// The start position in [0, 1] to use when initializing. When looping, play will still jump back to the beginning when reaching the end.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float StartPosition = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float EndPosition = 0.0f;
	
	// Should the animation loop back to the start when it reaches the end?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bLoopAnimation = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float BlendTime = 0.2f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float LoopBlendTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FMagicPlayerBoneControlParam BoneControlParam = FMagicPlayerBoneControlParam();

	FORCEINLINE bool operator == (const FMagicPlayerParam& Other) const;

	FMagicPlayerParam(){}
	FMagicPlayerParam(
		const double InTime,
		const TObjectPtr<UAnimationAsset> InAnimAsset,
		const TArray<FMagicPlayerLinkedAnim>& InLinkedAnimAsset,
		const bool InReplayNow,
		const float InReplayCodeTime,
		const float InHorizontal,
		const float InVertical,
		const float InPlayRate,
		const float InStartPosition,
		const float InEndPosition,
		const bool InLoopAnimation,
		const float InBlendTime);
};

UENUM(BlueprintType)
enum EMagicPlayerType
{
	AutoGetParam,
	GetParamFromLinked,
	DirectSetParam
};
UENUM(BlueprintType)
enum EMagicPlayerPlayStateType
{
	Starting,
	Playing,
	Stopping,
	Stopped
};
UENUM()
enum EGBWMagicPlayerPlayStateChangeType
{
	NoChange,
	StoppedToPlaying,
	Replay,
	NewPlay,
	PlayingToStopped
};

USTRUCT(BlueprintType)
struct FMagicPlayerPlayStateValue
{
	GENERATED_BODY()
	
	UPROPERTY()
	uint32 PlayStateHash = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EMagicPlayerPlayStateType> PlayStateType = EMagicPlayerPlayStateType::Stopped;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TEnumAsByte<EGBWMagicPlayerPlayStateChangeType> PlayStateChangeType = EGBWMagicPlayerPlayStateChangeType::NoChange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float PlayDuration = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float InternalTimeAccumulator = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float AbsPlayProgress = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float BlendAlpha = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bIsStopped = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bIsLoopReply = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	int PlayFrames = 0;

	TArray<FBlendSampleData> LastBlendSampleDataCache;

	FMagicPlayerPlayStateValue(){}
};
USTRUCT(BlueprintType)
struct FMagicPlayerPlayState
{
	GENERATED_BODY()
	uint64 LastUpdateFrame = 0;
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = Settings)
	FMagicPlayerPlayStateValue PlayState = FMagicPlayerPlayStateValue();
	
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = Settings)
	FMagicPlayerParam PlayerParamCache = FMagicPlayerParam();

	FMagicPlayerPlayState(){}

	FMagicPlayerPlayState(uint32 InPlayStateHash, EMagicPlayerPlayStateType InPlayStateType,
	                      EGBWMagicPlayerPlayStateChangeType InPlayStateChangeType, float InPlayDuration,
	                      bool InIsStopped,
	                      float InInternalTimeAccumulator, float InAbsPlayProgress, bool InIsLoopReply,
	                      int InPlayFrames,
	                      float InBlendAlpha, const TArray<FBlendSampleData>& InBlendSampleDataCache, const FMagicPlayerParam& InPlayerParamCache);
};
USTRUCT(BlueprintType)
struct FMagicPlayerParamState
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FName Slot = FName("Default");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	UAnimationAsset* AnimAsset = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TArray<FMagicPlayerLinkedAnim> LinkedAnimAsset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float Time = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float ReplayCodeTime = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bReplayNow = false;

	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = Settings)
	FMagicPlayerPlayState PlayState = FMagicPlayerPlayState();
	UPROPERTY(NotReplicated, BlueprintReadWrite, EditAnywhere, Category = Settings)
	FTransform DeltaRootMotionNow = FTransform();

	FMagicPlayerParamState(){}
	FMagicPlayerParamState(const FName InSlot,
		UAnimationAsset* InAnimAsset,
		const TArray<FMagicPlayerLinkedAnim>& InLinkedAnimAsset,
		const float InTime,
		const bool InReplayNow,
		const float InReplayCodeTime)
	{
		Slot = InSlot;
		AnimAsset = InAnimAsset;
		LinkedAnimAsset = InLinkedAnimAsset;
		Time = InTime;
		bReplayNow = InReplayNow;
		ReplayCodeTime = InReplayCodeTime;
	}
};

USTRUCT(BlueprintType)
struct FMagicPlayerParamSet
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	TArray<FMagicPlayerParamState> ParamSet;
	
	FMagicPlayerParamSet(){}

	int GetParamIndex(const FName InSlot) const;
	void SetParam(const FName InSlot,
		UAnimationAsset* AnimAsset,
		const TArray<FMagicPlayerLinkedAnim>& LinkedAnimAsset,
		bool bReplayNow,
		float ReplayCodeTime,
		float Time);
	void SetPlayState(const FName InSlot, const FMagicPlayerPlayState& PlayState);
	void SetRootMotionState(const FName InSlot, const FTransform& InRootMotion);
	void RemoveParam(const FName InSlot);
	bool GetParam(const FName InSlot, UAnimationAsset*& AnimAsset, TArray<FMagicPlayerLinkedAnim>& LinkedAnimAsset, bool& bReplayNow) const;
	bool GetLinkedParam(const FName InSlot, USkeletalMesh* InSKMesh, FMagicPlayerParam& ResParam) const;
	bool GetLinkedAnimSetting(const FName InSlot, USkeletalMesh* InSKMesh, FMagicPlayerLinkedAnim& Res) const;
	bool GetPlayState(const FName InSlot, FMagicPlayerPlayState& PlayState) const;
	UAnimationAsset* GetRootMotionAnimAsset(const FName InSlot) const;
	UAnimationAsset* GetMagicPlayerTickInterfaceAnimAsset(const FName InSlot) const;
	UAnimationAsset* GetMagicPlayerMainStateAnimAsset(const FName InSlot) const;
	FTransform GetRootMotionNow(const FName InSlot) const;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GBWANIM_API UGBWAnimComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGBWAnimComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	int TickCount = 0;
	
public:
	UPROPERTY(Replicated)
	ACharacter* CharacterOwner = nullptr;
	UPROPERTY(BlueprintReadOnly, Replicated, Category="AnimState")
	FMagicPlayerParamSet MagicPlayerParamSet = FMagicPlayerParamSet();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateAnimPlayState(float DeltaTime);

	ACharacter* GetCharacterOwner();
	bool IsLocalOwn() const;
	bool IsInServer();

	UFUNCTION(BlueprintCallable, Category = "GBW|Anim")
	bool IsReadyToUse();

	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	void PlayMontage(USkeletalMeshComponent* SKMComponent,
		UAnimMontage* MontageToPlay,
		float PlayRate=1.0f,
		float StartingPosition=0.0f,
		FName StartingSection=NAME_None);
	UFUNCTION(Reliable, Server, Category = "GBW|Anim")
	void PlayMontage_Server(USkeletalMeshComponent* SKMComponent,
		UAnimMontage* MontageToPlay,
		float PlayRate=1.0f,
		float StartingPosition=0.0f,
		FName StartingSection=NAME_None);
	UFUNCTION(Reliable, NetMulticast, Category = "GBW|Anim")
	void PlayMontage_Multicast(USkeletalMeshComponent* SKMComponent,
		UAnimMontage* MontageToPlay,
		float PlayRate=1.0f,
		float StartingPosition=0.0f,
		FName StartingSection=NAME_None);
	void PlayMontage_Imp(const USkeletalMeshComponent* SKMComponent,
	                            UAnimMontage* MontageToPlay,
	                            float PlayRate=1.0f,
	                            float StartingPosition=0.0f,
	                            FName StartingSection=NAME_None);

	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	void StopMontage(USkeletalMeshComponent* SKMComponent);
	UFUNCTION(Reliable, Server, Category = "GBW|Anim")
	void StopMontage_Server(USkeletalMeshComponent* SKMComponent);
	UFUNCTION(Reliable, NetMulticast, Category = "GBW|Anim")
	void StopMontage_Multicast(USkeletalMeshComponent* SKMComponent);
	static void StopMontage_Imp(const USkeletalMeshComponent* SKMComponent);

	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	void PlayMagicAnim(
		UAnimationAsset* AnimToPlay,
		const TArray<FMagicPlayerLinkedAnim> LinkedAnimSet,
		const FName Slot = NAME_None);
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	void StopMagicAnim(FName Slot = NAME_None);

	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	void SetMagicPlayer(const FName InSlot,
		UAnimationAsset* InAnimAsset,
		const TArray<FMagicPlayerLinkedAnim>& InLinkedAnimAsset,
		const bool InReplayNow,
		const float ReplayCodeTime);
	UFUNCTION(Reliable, Server, Category = "GBW|Anim")
	void SetMagicPlayer_Server(const FName InSlot,
		UAnimationAsset* InAnimAsset,
		const TArray<FMagicPlayerLinkedAnim>& InLinkedAnimAsset,
		const bool InReplayNow,
		const float ReplayCodeTime);

	void SetMagicPlayerState(const FName InSlot, const FMagicPlayerPlayState& InState);
	UFUNCTION(Reliable, Server, Category = "GBW|Anim")
	void SetMagicPlayerState_Server(const FName InSlot, const FMagicPlayerPlayState& InState);
	void SetMagicPlayerState_Imp(const FName InSlot, const FMagicPlayerPlayState& InState);

	void SetMagicPlayerRootMotionState(const FName InSlot, const FTransform& InRootMotion);
	UFUNCTION(Reliable, Server, Category = "GBW|Anim")
	void SetMagicPlayerRootMotionState_Server(const FName InSlot, const FTransform& InRootMotion);
	void SetMagicPlayerRootMotionState_Imp(const FName InSlot, const FTransform& InRootMotion);

	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	bool GetMagicPlayer(const FName InSlot, UAnimationAsset*& AnimAsset, TArray<FMagicPlayerLinkedAnim>& LinkedAnimAsset, bool& bReplayNow) const;
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	bool GetMagicPlayerLinked(const FName InSlot, USkeletalMesh* InSKMesh, FMagicPlayerParam& ResParam) const;
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	bool GetMagicPlayerLinkedAnimSetting(const FName InSlot, USkeletalMesh* InSKMesh, FMagicPlayerLinkedAnim& Res) const;
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	bool GetMagicPlayerState(const FName InSlot, FMagicPlayerPlayState& ResState) const;
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	UAnimationAsset* GetMagicPlayerRootMotionSourceAnimAsset(const FName InSlot) const;
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	UAnimationAsset* GetMagicPlayerTickInterfaceAnimAsset(const FName InSlot) const;
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	UAnimationAsset* GetMagicPlayerMainStateAnimAsset(const FName InSlot) const;
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	FTransform GetMagicPlayerRootMotion(const FName InSlot) const;
	
	UFUNCTION(BlueprintCallable,Category = "GBW|Anim")
	void RemoveMagicPlayer(const FName InSlot);
	UFUNCTION(Reliable, Server, Category = "GBW|Anim")
	void RemoveMagicPlayer_Server(const FName InSlot);
};
