// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "GBWAnimComponent.h"
#include "GBWPowerfulToolsFuncLib.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"

bool FMagicPlayerLinkedAnim::operator==(const FMagicPlayerLinkedAnim& Other) const
{
	return AnimAsset == Other.AnimAsset
		&& bAsRootMotionSource == Other.bAsRootMotionSource;
}

bool FMagicPlayerParam::operator==(const FMagicPlayerParam& Other) const
{
	return AnimAsset==Other.AnimAsset
		&& LinkedAnimAsset==Other.LinkedAnimAsset
		&& bReplayNow==Other.bReplayNow
		&& ReplayCodeTime==Other.ReplayCodeTime
		&& Horizontal==Other.Horizontal
		&& Vertical==Other.Vertical
		&& PlayRate==Other.PlayRate
		&& StartPosition==Other.StartPosition
		&& bLoopAnimation==Other.bLoopAnimation
		&& BlendTime==Other.BlendTime;
}

FMagicPlayerParam::FMagicPlayerParam(const double InTime, const TObjectPtr<UAnimationAsset> InAnimAsset,
	const TArray<FMagicPlayerLinkedAnim>& InLinkedAnimAsset, const bool InReplayNow, const float InReplayCodeTime,
	const float InHorizontal, const float InVertical, const float InPlayRate, const float InStartPosition,
	const float InEndPosition, const bool InLoopAnimation, const float InBlendTime)
{
	Time = InTime;
	AnimAsset=InAnimAsset;
	LinkedAnimAsset = InLinkedAnimAsset;
	bReplayNow=InReplayNow;
	ReplayCodeTime=InReplayCodeTime;
	Horizontal=InHorizontal;
	Vertical=InVertical;
	PlayRate=InPlayRate;
	StartPosition=InStartPosition;
	EndPosition = InEndPosition;
	bLoopAnimation=InLoopAnimation;
	BlendTime=InBlendTime;
}

FMagicPlayerPlayState::FMagicPlayerPlayState(const uint32 InPlayStateHash,
	const EMagicPlayerPlayStateType InPlayStateType, const EGBWMagicPlayerPlayStateChangeType InPlayStateChangeType,
	const float InPlayDuration, const bool InIsStopped, const float InInternalTimeAccumulator,
	const float InAbsPlayProgress, const bool InIsLoopReply, const int InPlayFrames,
	const float InBlendAlpha, const TArray<FBlendSampleData>& InBlendSampleDataCache,
	const FMagicPlayerParam& InPlayerParamCache)
{
	PlayerParamCache = InPlayerParamCache;

	PlayState.PlayStateHash = InPlayStateHash;
	PlayState.PlayStateType = InPlayStateType;
	PlayState.PlayStateChangeType = InPlayStateChangeType;
	PlayState.PlayDuration = InPlayDuration;
	PlayState.InternalTimeAccumulator = InInternalTimeAccumulator;
	PlayState.AbsPlayProgress = InAbsPlayProgress;
	PlayState.BlendAlpha = InBlendAlpha;
	PlayState.bIsStopped = InIsStopped;
	PlayState.bIsLoopReply = InIsLoopReply;
	PlayState.PlayFrames = InPlayFrames;
	PlayState.LastBlendSampleDataCache = InBlendSampleDataCache;
}

int FMagicPlayerParamSet::GetParamIndex(const FName InSlot) const
{
	for (int i = 0; i<ParamSet.Num(); i++)
	{
		if (ParamSet[i].Slot == InSlot)
		{
			return i;
		}
	}

	return -1;
}

void FMagicPlayerParamSet::SetParam(const FName InSlot, UAnimationAsset* AnimAsset, const TArray<FMagicPlayerLinkedAnim>& LinkedAnimAsset,
                                    const bool bReplayNow,
                                    const float ReplayCodeTime, const float Time)
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		ParamSet[Index].AnimAsset = AnimAsset;
		ParamSet[Index].LinkedAnimAsset = LinkedAnimAsset;
		ParamSet[Index].bReplayNow = bReplayNow;
		ParamSet[Index].ReplayCodeTime = ReplayCodeTime;
		ParamSet[Index].Time = Time;
	}
	else
	{
		ParamSet.Add(FMagicPlayerParamState(InSlot,AnimAsset,LinkedAnimAsset,Time,bReplayNow,ReplayCodeTime));
	}
}

void FMagicPlayerParamSet::SetPlayState(const FName InSlot, const FMagicPlayerPlayState& PlayState)
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		const uint32 OldPlayStateHash = ParamSet[Index].PlayState.PlayState.PlayStateHash;
		ParamSet[Index].PlayState = PlayState;
		ParamSet[Index].PlayState.LastUpdateFrame = GFrameCounter;

		if (ParamSet[Index].PlayState.PlayState.PlayStateHash == 0)
		{
			ParamSet[Index].PlayState.PlayState.PlayStateHash = OldPlayStateHash;
		}
	}
}

void FMagicPlayerParamSet::SetRootMotionState(const FName InSlot, const FTransform& InRootMotion)
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		ParamSet[Index].DeltaRootMotionNow = InRootMotion;
	}
}

void FMagicPlayerParamSet::RemoveParam(const FName InSlot)
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		ParamSet.RemoveAt(Index);
	}
}

bool FMagicPlayerParamSet::GetParam(const FName InSlot, UAnimationAsset*& AnimAsset, TArray<FMagicPlayerLinkedAnim>& LinkedAnimAsset, bool& bReplayNow) const
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		AnimAsset = ParamSet[Index].AnimAsset;
		LinkedAnimAsset = ParamSet[Index].LinkedAnimAsset;
		bReplayNow = ParamSet[Index].bReplayNow;
		return true;
	}

	return false;
}

bool FMagicPlayerParamSet::GetLinkedParam(const FName InSlot, USkeletalMesh* InSKMesh,
	FMagicPlayerParam& ResParam) const
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0 && InSKMesh && InSKMesh->GetSkeleton())
	{
		ResParam = ParamSet[Index].PlayState.PlayerParamCache;
		for (const auto Element : ParamSet[Index].LinkedAnimAsset)
		{
			if (Element.AnimAsset && Element.AnimAsset->GetSkeleton())
			{
#if WITH_EDITOR
				if (!Element.AnimAsset->GetSkeleton()->IsCompatibleForEditor(InSKMesh->GetSkeleton()))
				{
					continue;
				}
#else
				// Runtime: Use IsCompatibleMesh as an alternative check
				if (!Element.AnimAsset->GetSkeleton()->IsCompatibleMesh(InSKMesh))
				{
					continue;
				}
#endif
				ResParam.AnimAsset = Element.AnimAsset;
				return true;
			}
		}
	}

	return false;
}

bool FMagicPlayerParamSet::GetLinkedAnimSetting(const FName InSlot, USkeletalMesh* InSKMesh,
	FMagicPlayerLinkedAnim& Res) const
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0 && InSKMesh && InSKMesh->GetSkeleton())
	{
		for (const auto Element : ParamSet[Index].LinkedAnimAsset)
		{
			if (Element.AnimAsset && Element.AnimAsset->GetSkeleton())
			{
#if WITH_EDITOR
				if (!Element.AnimAsset->GetSkeleton()->IsCompatibleForEditor(InSKMesh->GetSkeleton()))
				{
					continue;
				}
#else
				// Runtime: Use IsCompatibleMesh as an alternative check
				if (!Element.AnimAsset->GetSkeleton()->IsCompatibleMesh(InSKMesh))
				{
					continue;
				}
#endif
				Res = Element;
				return true;
			}
		}
	}

	return false;
}

bool FMagicPlayerParamSet::GetPlayState(const FName InSlot, FMagicPlayerPlayState& PlayState) const
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		PlayState = ParamSet[Index].PlayState;
		return true;
	}

	return false;
}

UAnimationAsset* FMagicPlayerParamSet::GetRootMotionAnimAsset(const FName InSlot) const
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		UAnimationAsset* Res = ParamSet[Index].AnimAsset;
		for (const auto Element : ParamSet[Index].LinkedAnimAsset)
		{
			if (Element.AnimAsset
				&& Element.bAsRootMotionSource)
			{
				Res = Element.AnimAsset;
				break;
			}
		}
		return Res;
	}

	return nullptr;
}

UAnimationAsset* FMagicPlayerParamSet::GetMagicPlayerTickInterfaceAnimAsset(const FName InSlot) const
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		UAnimationAsset* Res = ParamSet[Index].AnimAsset;
		for (const auto Element : ParamSet[Index].LinkedAnimAsset)
		{
			if (Element.AnimAsset
				&& Element.bTriggerMagicPlayerTickInterface)
			{
				Res = Element.AnimAsset;
				break;
			}
		}
		return Res;
	}

	return nullptr;
}

UAnimationAsset* FMagicPlayerParamSet::GetMagicPlayerMainStateAnimAsset(const FName InSlot) const
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		UAnimationAsset* Res = ParamSet[Index].AnimAsset;
		for (const auto Element : ParamSet[Index].LinkedAnimAsset)
		{
			if (Element.AnimAsset
				&& Element.bAsMainState)
			{
				Res = Element.AnimAsset;
				break;
			}
		}
		return Res;
	}

	return nullptr;
}

FTransform FMagicPlayerParamSet::GetRootMotionNow(const FName InSlot) const
{
	if (const int Index = GetParamIndex(InSlot); Index >= 0)
	{
		return ParamSet[Index].DeltaRootMotionNow;
	}

	return FTransform();
}

// Sets default values for this component's properties
UGBWAnimComponent::UGBWAnimComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UGBWAnimComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;
	Parameters.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CharacterOwner, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MagicPlayerParamSet, Parameters)
}

// Called when the game starts
void UGBWAnimComponent::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterOwner();
}

// Called every frame
void UGBWAnimComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsReadyToUse())
	{
		TickCount++;
	}
	UpdateAnimPlayState(DeltaTime);
}

void UGBWAnimComponent::UpdateAnimPlayState(float DeltaTime)
{
	if (IsInServer())
	{
		const double CurrentTime = GetWorld()->GetTimeSeconds();
		for (int i = 0; i < MagicPlayerParamSet.ParamSet.Num(); i++)
		{
			if (MagicPlayerParamSet.ParamSet[i].bReplayNow
				&& CurrentTime-MagicPlayerParamSet.ParamSet[i].Time > 0.0f
				&& CurrentTime-MagicPlayerParamSet.ParamSet[i].Time >= MagicPlayerParamSet.ParamSet[i].ReplayCodeTime/5.0f)
			{
				MagicPlayerParamSet.ParamSet[i].bReplayNow = false;
			}
		}
	}
}

ACharacter* UGBWAnimComponent::GetCharacterOwner()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<ACharacter>(this->GetOwner());
	}

	return CharacterOwner;
}

bool UGBWAnimComponent::IsLocalOwn() const
{
	bool bRes = false;
	
	if (this->GetOwner())
	{
		const EGBWActorNetType ActorNetType = UGBWPowerfulToolsFuncLib::GetActorNetType(this->GetOwner());
		bRes = ActorNetType == EGBWActorNetType::InClient_LocalPlayer || ActorNetType == EGBWActorNetType::InServer_LocalPlayer;
	}
	
	return bRes;
}

bool UGBWAnimComponent::IsInServer()
{
	if (GetCharacterOwner() && GetCharacterOwner()->HasAuthority())
	{
		return true;
	}
	return false;
}

bool UGBWAnimComponent::IsReadyToUse()
{
	//return true;
	return TickCount>=10;
}

void UGBWAnimComponent::PlayMontage(USkeletalMeshComponent* SKMComponent, UAnimMontage* MontageToPlay, float PlayRate,
                                    float StartingPosition, FName StartingSection)
{
	if (IsLocalOwn() || IsInServer())
	{
		PlayMontage_Server(SKMComponent,MontageToPlay,PlayRate,StartingPosition,StartingSection);
	}
}
void UGBWAnimComponent::PlayMontage_Server_Implementation(USkeletalMeshComponent* SKMComponent,
                                                          UAnimMontage* MontageToPlay, float PlayRate, float StartingPosition, FName StartingSection)
{
	PlayMontage_Multicast(SKMComponent,MontageToPlay,PlayRate,StartingPosition,StartingSection);
}
void UGBWAnimComponent::PlayMontage_Multicast_Implementation(USkeletalMeshComponent* SKMComponent,
                                                             UAnimMontage* MontageToPlay, float PlayRate, float StartingPosition, FName StartingSection)
{
	PlayMontage_Imp(SKMComponent,MontageToPlay,PlayRate,StartingPosition,StartingSection);
}
void UGBWAnimComponent::PlayMontage_Imp(const USkeletalMeshComponent* SKMComponent, UAnimMontage* MontageToPlay,
                                        float PlayRate, float StartingPosition, FName StartingSection)
{
	if (SKMComponent && SKMComponent->GetAnimInstance())
	{
		SKMComponent->GetAnimInstance()->Montage_Play(MontageToPlay,
			PlayRate,
			EMontagePlayReturnType::MontageLength,
			StartingPosition,
			true);
		SKMComponent->GetAnimInstance()->Montage_JumpToSection(StartingSection,MontageToPlay);
	}
}

void UGBWAnimComponent::StopMontage(USkeletalMeshComponent* SKMComponent)
{
	if (IsLocalOwn() || IsInServer())
	{
		StopMontage_Server(SKMComponent);
	}
}
void UGBWAnimComponent::StopMontage_Server_Implementation(USkeletalMeshComponent* SKMComponent)
{
	StopMontage_Multicast(SKMComponent);
}
void UGBWAnimComponent::StopMontage_Multicast_Implementation(USkeletalMeshComponent* SKMComponent)
{
	StopMontage_Imp(SKMComponent);
}
void UGBWAnimComponent::StopMontage_Imp(const USkeletalMeshComponent* SKMComponent)
{
	if (SKMComponent && SKMComponent->GetAnimInstance())
	{
		SKMComponent->GetAnimInstance()->Montage_Stop(0.1f);
	}
}

void UGBWAnimComponent::PlayMagicAnim(
	UAnimationAsset* AnimToPlay,
	const TArray<FMagicPlayerLinkedAnim> LinkedAnimSet,
	const FName Slot)
{
	SetMagicPlayer(Slot,AnimToPlay,LinkedAnimSet,true,0.3f);
}

void UGBWAnimComponent::StopMagicAnim(const FName Slot)
{
	RemoveMagicPlayer(Slot);
}

void UGBWAnimComponent::SetMagicPlayer(const FName InSlot,
		UAnimationAsset* InAnimAsset,
		const TArray<FMagicPlayerLinkedAnim>& InLinkedAnimAsset,
		const bool InReplayNow,
		const float ReplayCodeTime)
{
	UAnimationAsset* OldAnimAsset; 
	TArray<FMagicPlayerLinkedAnim> OldLinkedAnimAsset;
	bool OldReplayNow = false;
	if (GetMagicPlayer(InSlot,OldAnimAsset,OldLinkedAnimAsset,OldReplayNow)
		&& OldAnimAsset==InAnimAsset
		&& OldLinkedAnimAsset==InLinkedAnimAsset
		&& OldReplayNow == InReplayNow)
	{
		return;
	}
	SetMagicPlayer_Server(InSlot, InAnimAsset, InLinkedAnimAsset, InReplayNow,ReplayCodeTime);
}

void UGBWAnimComponent::SetMagicPlayer_Server_Implementation(const FName InSlot,
		UAnimationAsset* InAnimAsset,
		const TArray<FMagicPlayerLinkedAnim>& InLinkedAnimAsset,
		const bool InReplayNow,
		const float ReplayCodeTime)
{
	MagicPlayerParamSet.SetParam(InSlot,InAnimAsset,InLinkedAnimAsset,InReplayNow,ReplayCodeTime,GetWorld()->GetTimeSeconds());
}

void UGBWAnimComponent::SetMagicPlayerState(const FName InSlot, const FMagicPlayerPlayState& InState)
{
	SetMagicPlayerState_Server(InSlot, InState);
}

void UGBWAnimComponent::SetMagicPlayerState_Server_Implementation(const FName InSlot,
	const FMagicPlayerPlayState& InState)
{
	SetMagicPlayerState_Imp(InSlot, InState);
}

void UGBWAnimComponent::SetMagicPlayerState_Imp(const FName InSlot, const FMagicPlayerPlayState& InState)
{
	MagicPlayerParamSet.SetPlayState(InSlot,InState);
}

void UGBWAnimComponent::SetMagicPlayerRootMotionState(const FName InSlot, const FTransform& InRootMotion)
{
	SetMagicPlayerRootMotionState_Server(InSlot,InRootMotion);
}
void UGBWAnimComponent::SetMagicPlayerRootMotionState_Server_Implementation(const FName InSlot, const FTransform& InRootMotion)
{
	SetMagicPlayerRootMotionState_Imp(InSlot,InRootMotion);
}
void UGBWAnimComponent::SetMagicPlayerRootMotionState_Imp(const FName InSlot, const FTransform& InRootMotion)
{
	MagicPlayerParamSet.SetRootMotionState(InSlot,InRootMotion);
}

bool UGBWAnimComponent::GetMagicPlayer(const FName InSlot, UAnimationAsset*& AnimAsset, TArray<FMagicPlayerLinkedAnim>& LinkedAnimAsset, bool& bReplayNow) const
{
	return MagicPlayerParamSet.GetParam(InSlot,AnimAsset,LinkedAnimAsset,bReplayNow);
}

bool UGBWAnimComponent::GetMagicPlayerLinked(const FName InSlot, USkeletalMesh* InSKMesh,
	FMagicPlayerParam& ResParam) const
{
	return MagicPlayerParamSet.GetLinkedParam(InSlot,InSKMesh,ResParam);
}

bool UGBWAnimComponent::GetMagicPlayerLinkedAnimSetting(const FName InSlot, USkeletalMesh* InSKMesh,
	FMagicPlayerLinkedAnim& Res) const
{
	return MagicPlayerParamSet.GetLinkedAnimSetting(InSlot,InSKMesh,Res);
}

bool UGBWAnimComponent::GetMagicPlayerState(const FName InSlot, FMagicPlayerPlayState& ResState) const
{
	return MagicPlayerParamSet.GetPlayState(InSlot,ResState);
}

UAnimationAsset* UGBWAnimComponent::GetMagicPlayerRootMotionSourceAnimAsset(const FName InSlot) const
{
	return MagicPlayerParamSet.GetRootMotionAnimAsset(InSlot);
}

UAnimationAsset* UGBWAnimComponent::GetMagicPlayerTickInterfaceAnimAsset(const FName InSlot) const
{
	return MagicPlayerParamSet.GetMagicPlayerTickInterfaceAnimAsset(InSlot);
}

UAnimationAsset* UGBWAnimComponent::GetMagicPlayerMainStateAnimAsset(const FName InSlot) const
{
	return MagicPlayerParamSet.GetMagicPlayerMainStateAnimAsset(InSlot);
}

FTransform UGBWAnimComponent::GetMagicPlayerRootMotion(const FName InSlot) const
{
	return MagicPlayerParamSet.GetRootMotionNow(InSlot);
}

void UGBWAnimComponent::RemoveMagicPlayer(const FName InSlot)
{
	UAnimationAsset* AnimAsset;
	TArray<FMagicPlayerLinkedAnim> LinkedAnimAsset;
	bool bReplayNow;
	if (!GetMagicPlayer(InSlot,AnimAsset,LinkedAnimAsset,bReplayNow))
	{
		return;
	}
	RemoveMagicPlayer_Server(InSlot);
}
void UGBWAnimComponent::RemoveMagicPlayer_Server_Implementation(const FName InSlot)
{
	MagicPlayerParamSet.RemoveParam(InSlot);
}
