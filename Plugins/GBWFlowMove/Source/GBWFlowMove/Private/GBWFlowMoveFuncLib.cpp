// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "GBWFlowMoveFuncLib.h"

#include "GBWFlowMoveComponent.h"
#include "GBWPowerfulToolsFuncLib.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "RMS/GBWRMSBPFuncLib.h"
#include "DrawDebugHelpers.h"

bool UGBWFlowMoveFuncLib::GetSKMeshByTag(
	USkeletalMeshComponent*& SKMeshComp,
	ACharacter* Character,
	FName MeshTag,
	TArray<FName> ExcludedMeshTag,
	UAnimationAsset* TargetAnimationAsset)
{
	SKMeshComp = nullptr;
	if (Character && TargetAnimationAsset)
	{
		auto SKMeshes = Character->GetComponentsByTag(USkeletalMeshComponent::StaticClass(),MeshTag);
		if (SKMeshes.IsEmpty())
		{
			Character->GetComponents(USkeletalMeshComponent::StaticClass(),SKMeshes);
		}

		for (const auto SKMeshItem:SKMeshes)
		{
			bool bIsExcluded = false;
			for (const auto ETag:ExcludedMeshTag)
			{
				if (SKMeshItem->ComponentHasTag(ETag))
				{
					bIsExcluded = true;
					break;
				}
			}

			if (!bIsExcluded)
			{
				USkeletalMeshComponent* SKMC = Cast<USkeletalMeshComponent>(SKMeshItem);
				USkeletalMesh* SKMS = nullptr;
				if (SKMC)
				{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
					SKMS = SKMC->GetSkeletalMeshAsset();
#else
					SKMS = SKMC->SkeletalMesh;
#endif
				}

				if (SKMS && SKMS->GetSkeleton())
				{
#if WITH_EDITOR
					if (!SKMS->GetSkeleton()->IsCompatibleForEditor(TargetAnimationAsset->GetSkeleton()))
					{
						continue;
					}
#else
					// Runtime: Use IsCompatibleMesh as an alternative check
					if (!TargetAnimationAsset->GetSkeleton()->IsCompatibleMesh(SKMS))
					{
						continue;
					}
#endif
					SKMeshComp = SKMC;
					return true;
				}
			}
		}
	}
	
	return false;
}

bool UGBWFlowMoveFuncLib::IsLocalOwn(AActor* Actor)
{
	bool bRes;

	if (Actor)
	{
		const ENetRole LocalRole = Actor->GetLocalRole();
		const ENetRole RemoteRole = Actor->GetRemoteRole();

		if (LocalRole == ENetRole::ROLE_Authority)
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 3
			if (RemoteRole != ENetRole::ROLE_AutonomousProxy)
			{
				bRes = true;
			}
			else
			{
				bRes = false;
			}
#else
			bRes = true;
#endif
		}
		else
		{
			if (LocalRole == ENetRole::ROLE_AutonomousProxy)
			{
				bRes = true;
			}
			else
			{
				bRes = false;
			}
		}
	}
	else
	{
		bRes = false;
	}
	
	return bRes;
}

bool UGBWFlowMoveFuncLib::IsAIPlayer(AActor* Actor)
{
	bool bRes;

	if (Actor)
	{
		const ENetRole LocalRole = Actor->GetLocalRole();
		const ENetRole RemoteRole = Actor->GetRemoteRole();
		
		bRes = LocalRole == ENetRole::ROLE_Authority
		&& RemoteRole != ENetRole::ROLE_AutonomousProxy
		&& Actor != UGameplayStatics::GetPlayerCharacter(Actor, 0);
	}
	else
	{
		bRes = false;
	}
	
	return bRes;
}

FHitResult UGBWFlowMoveFuncLib::SweepMulti(
	const UWorld* World,
	TArray<FHitResult>& OutHits,
	const FVector& Start,
	const FVector& End,
	const FQuat& Rot,
	FGBWFlowMoveTraceSetting TraceSetting,
	const FCollisionShape& CollisionShape,
	const FCollisionQueryParams& Params)
{
	FHitResult Res = FHitResult();
	if (!World)
	{
		return Res;
	}

	if (TraceSetting.TraceObjectTypes.IsEmpty() && TraceSetting.TraceChannels.IsEmpty())
	{
		return Res;
	}
	
	TArray<FHitResult> ObjectTypeOutHits;
	FCollisionObjectQueryParams ObjectQueryParameters;
	for (const auto ObjectType : TraceSetting.TraceObjectTypes)
	{
		ObjectQueryParameters.AddObjectTypesToQuery(UCollisionProfile::Get()->ConvertToCollisionChannel(false, ObjectType));
	}
	World->SweepMultiByObjectType(
		ObjectTypeOutHits,
		Start,
		End,
		Rot,
		ObjectQueryParameters,
		CollisionShape,
		Params);
	
	TArray<FHitResult> ChannelOutHits;
	for (const auto Channel:TraceSetting.TraceChannels)
	{
		TArray<FHitResult> TempChannelOutHits;
        	World->SweepMultiByChannel(
        	TempChannelOutHits,
        	Start,
        	End,
        	Rot,
        	UEngineTypes::ConvertToCollisionChannel(Channel),
        	CollisionShape,
        	Params
        	);

		ChannelOutHits.Append(TempChannelOutHits);
	}

	TArray<AActor*> ResultOutHits;
	OutHits.Empty();
	for (auto Item:ObjectTypeOutHits)
	{
   		if (!ResultOutHits.Contains(Item.GetActor()))
		{
			OutHits.Add(Item);
			ResultOutHits.Add(Item.GetActor());
		}
	}
	for (auto Item:ChannelOutHits)
	{
		if (Item.IsValidBlockingHit() && !ResultOutHits.Contains(Item.GetActor()))
		{
			OutHits.Add(Item);
			ResultOutHits.Add(Item.GetActor());
		}
	}

	if (TraceSetting.isDebug && TraceSetting.DebugTime>=0)
	{
		if (TraceSetting.DrawTraceShape)
		{
			DrawDebugShape(World,Start,FQuat::Identity,CollisionShape,
            			!OutHits.IsEmpty()? FColor::Red : FColor::White,
            			TraceSetting.DebugTime,1);
			DrawDebugShape(World,End,FQuat::Identity,CollisionShape,
						!OutHits.IsEmpty()? FColor::Red : FColor::White,
						TraceSetting.DebugTime,1);
		}
		
		if (!OutHits.IsEmpty())
		{
			DrawDebugSphere(World,OutHits[0].ImpactPoint,5,8,
				FColor::Green,
				false,TraceSetting.DebugTime,0,10);
		}

		if (TraceSetting.DrawTraceLine)
		{
			DrawDebugLine(World, Start, End,
			              !OutHits.IsEmpty()? FColor::Red : FColor::White,
			              false,TraceSetting.DebugTime,0,1);
		}
	}

	if (!OutHits.IsEmpty())
	{
		Res = OutHits[0];
	}
	else
	{
		Res.TraceStart = Start;
		Res.TraceEnd = End;
	}
	for (auto Hit:OutHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			Res = Hit;
			break;
		}
	}
	
	if (Res.GetActor() && !TraceSetting.IsMetScreeningConditions(Res.GetActor()))
	{
		Res = FHitResult();
	}
	
	return Res;
}

bool UGBWFlowMoveFuncLib::OverlapAnyTest(
	const UWorld* World,
	const FVector& Pos,
	const FQuat& Rot,
	FGBWFlowMoveTraceSetting TraceSetting,
	const FCollisionShape& CollisionShape,
	const FCollisionQueryParams& Params)
{
	if (!World)
	{
		return false;
	}

	if (TraceSetting.TraceObjectTypes.IsEmpty() && TraceSetting.TraceChannels.IsEmpty())
	{
		return false;
	}
	
	FCollisionObjectQueryParams ObjectQueryParameters;
	for (const auto ObjectType : TraceSetting.TraceObjectTypes)
	{
		ObjectQueryParameters.AddObjectTypesToQuery(UCollisionProfile::Get()->ConvertToCollisionChannel(false, ObjectType));
	}
	if (World->OverlapAnyTestByObjectType(
		Pos,
		Rot,
		ObjectQueryParameters,
		CollisionShape,
		Params))
	{
		return true;
	}

	for (auto Item:TraceSetting.TraceChannels)
	{
		if (World->OverlapAnyTestByChannel(
			Pos,
			Rot,
			UEngineTypes::ConvertToCollisionChannel(Item),
			CollisionShape,
			Params
		))
		{
			return true;
		}
	}

	return false;
}

FHitResult UGBWFlowMoveFuncLib::LineTrace(
		const UWorld* World,
		TArray<struct FHitResult>& OutHits,
		const FVector& Start,
		const FVector& End,
		FGBWFlowMoveTraceSetting TraceSetting,
		const FCollisionQueryParams& Params)
{
	FHitResult Res = FHitResult();
	if (!World)
	{
		return Res;
	}

	if (TraceSetting.TraceObjectTypes.IsEmpty() && TraceSetting.TraceChannels.IsEmpty())
	{
		return Res;
	}

	TArray<FHitResult> ObjectTypeOutHits;
	FCollisionObjectQueryParams ObjectQueryParameters;
	for (const auto ObjectType : TraceSetting.TraceObjectTypes)
	{
		ObjectQueryParameters.AddObjectTypesToQuery(UCollisionProfile::Get()->ConvertToCollisionChannel(false, ObjectType));
	}
	World->LineTraceMultiByObjectType(
		ObjectTypeOutHits,
		Start,
		End,
		ObjectQueryParameters,
		Params
	);
	
	TArray<FHitResult> ChannelOutHits;
	for (const auto Channel:TraceSetting.TraceChannels)
	{
		TArray<FHitResult> TempChannelOutHits;
		World->LineTraceMultiByChannel(
		ChannelOutHits,
		Start,
		End,
		UEngineTypes::ConvertToCollisionChannel(Channel),
		Params
		);

		ChannelOutHits.Append(TempChannelOutHits);
	}

	TArray<AActor*> ResultOutHits;
	OutHits.Empty();
	for (auto Item:ObjectTypeOutHits)
	{
		if (!ResultOutHits.Contains(Item.GetActor()))
		{
			OutHits.Add(Item);
			ResultOutHits.Add(Item.GetActor());
		}
	}
	for (auto Item:ChannelOutHits)
	{
		if (Item.IsValidBlockingHit() && !ResultOutHits.Contains(Item.GetActor()))
		{
			OutHits.Add(Item);
			ResultOutHits.Add(Item.GetActor());
		}
	}

	if (!OutHits.IsEmpty())
	{
		Res = OutHits[0];
	}
	else
	{
		Res.TraceStart = Start;
		Res.TraceEnd = End;
	}
	for (auto Hit:OutHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			Res = Hit;
			break;
		}
	}

	if (!TraceSetting.IsMetScreeningConditions(Res.GetActor()))
	{
		Res = FHitResult();
	}

	if (TraceSetting.isDebug && TraceSetting.DebugTime>=0)
	{
		DrawDebugLine(World,Start,End,FColor::Orange,false,TraceSetting.DebugTime,0,2);
		if (!OutHits.IsEmpty())
		{
			DrawDebugSphere(World,OutHits[0].ImpactPoint,5,8,FColor::Green,false,TraceSetting.DebugTime,0,10);
		}
	}
	
	return Res;
}

bool UGBWFlowMoveFuncLib::GetTheLastValidHitResult(TArray<FHitResult> HitResultArr, FHitResult& Result)
{
	for (int i=HitResultArr.Num()-1; i>=0; i--)
	{
		if (HitResultArr[i].IsValidBlockingHit())
		{
			Result = HitResultArr[i];
			return true;
		}
	}
	return false;
}

FVector UGBWFlowMoveFuncLib::RadianToDirectionXY(float Radian)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Radian);

	return {Cos, Sin, 0.0f};
}

FVector UGBWFlowMoveFuncLib::AngleToDirectionXY(float Angle)
{
	return RadianToDirectionXY(FMath::DegreesToRadians(Angle));
}

float UGBWFlowMoveFuncLib::GetVelocityYawAngle(FVector Velocity)
{
	return UE_REAL_TO_FLOAT(FMath::RadiansToDegrees(FMath::Atan2(Velocity.Y, Velocity.X)));
}

float UGBWFlowMoveFuncLib::GetInputYawAngle(FVector InputDirection)
{
	const FVector_NetQuantizeNormal TheInputDirection = InputDirection.GetSafeNormal();
	return UE_REAL_TO_FLOAT(FMath::RadiansToDegrees(FMath::Atan2(TheInputDirection.Y, TheInputDirection.X)));
}

float UGBWFlowMoveFuncLib::GetPointToLineDistance(const FVector LineStart, const FVector LineEnd, const FVector Point)
{
	const float A = (LineStart - LineEnd).Length();
	const float B = (LineStart - Point).Length();
	const float C = (LineEnd - Point).Length();

	return FMath::Sqrt((A+B+C)*(A+B-C)*(A+C-B)*(B+C-A))/2/A;
}

FVector UGBWFlowMoveFuncLib::GetPointToLineFoot(const FVector& LineStart, const FVector& LineEnd, const FVector& Point)
{
	FVector retVal;
	double dx = LineStart.X - LineEnd.X;
	double dy = LineStart.Y - LineEnd.Y;
	double dz = LineStart.Z - LineEnd.Z;
	if(abs(dx) < 0.00000001 && abs(dy) < 0.00000001 && abs(dz) < 0.00000001 )
		{ retVal = LineStart; return retVal; }
	double u = (Point.X - LineStart.X)*(LineStart.X - LineEnd.X)
			+ (Point.Y - LineStart.Y)*(LineStart.Y - LineEnd.Y)
			+ (Point.Z - LineStart.Z)*(LineStart.Z - LineEnd.Z);
	u = u/((dx*dx)+(dy*dy)+(dz*dz));
	retVal.X = LineStart.X + u*dx;
	retVal.Y = LineStart.Y + u*dy;
	retVal.Z = LineStart.Z + u*dz;
	return retVal;
}

float UGBWFlowMoveFuncLib::GetMoveDirection(const ACharacter* OwnerCharacter, const FVector& DirectionVector, bool bIsUseActorSpeedVector)
{
	if (!OwnerCharacter)
	{
		return 0.0f;
	}

	FTransform T = OwnerCharacter->GetActorTransform();
	FVector Velocity = OwnerCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Velocity.Normalize();
	if (bIsUseActorSpeedVector)
	{
		T.SetRotation(Velocity.Rotation().Quaternion());
	}
	
	FVector A = UKismetMathLibrary::ProjectVectorOnToPlane(
		DirectionVector,
		OwnerCharacter->GetActorUpVector());
	A.Normalize();
	FVector B = UKismetMathLibrary::ProjectVectorOnToPlane(
		bIsUseActorSpeedVector? Velocity:OwnerCharacter->GetCapsuleComponent()->GetForwardVector(),
		OwnerCharacter->GetActorUpVector());
	B.Normalize();
	const float DirectionAngle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(A,B));

	const FVector A2 = UKismetMathLibrary::InverseTransformDirection(T,A);
	const FVector B2 = UKismetMathLibrary::InverseTransformDirection(T,B);
	const bool bIsRight = UKismetMathLibrary::CrossProduct2D(
		FVector2D(A2.X, A2.Y),
		FVector2D(B2.X, B2.Y)
	)<=0;

	const float SceneDirection = DirectionAngle * (bIsRight? 1 : -1);

	return SceneDirection;
}

FVector UGBWFlowMoveFuncLib::GetMoveVectorFromCharacterInputVector(const ACharacter* OwnerCharacter)
{
	if (!OwnerCharacter)
    {
        return FVector::ZeroVector;
    }

	FTransform T = OwnerCharacter->GetActorTransform();
	FRotator CRotator = OwnerCharacter->GetControlRotation();
	CRotator.Roll = 0.0f;
	CRotator.Pitch = 0.0f;
    T.SetRotation(CRotator.Quaternion());

    FVector Res = UKismetMathLibrary::InverseTransformDirection(T, OwnerCharacter->GetLastMovementInputVector());
	//Res.Z = -Res.Z;
	Res.Normalize();
	return Res;
}

UGBWFlowMoveComponent* UGBWFlowMoveFuncLib::FM_GetFlowMoveComponent(AActor* InActor, bool bTryGetFromParentActor)
{
	if (!InActor)
	{
		return nullptr;
	}

	UActorComponent* AComp = InActor->GetComponentByClass(UGBWFlowMoveComponent::StaticClass());
	if (!AComp)
	{
		if (bTryGetFromParentActor)
		{
			AActor* PActor = InActor;
			while (PActor && !AComp)
			{
				PActor = PActor->GetParentActor();
				if (PActor)
				{
					AComp = PActor->GetComponentByClass(UGBWFlowMoveComponent::StaticClass());	
				}
			}

			PActor = InActor;
			while (PActor && !AComp)
			{
				PActor = PActor->GetAttachParentActor();
				if (PActor)
				{
					AComp = PActor->GetComponentByClass(UGBWFlowMoveComponent::StaticClass());
				}
			}
		}
	}

	if (UGBWFlowMoveComponent* FMComp = Cast<UGBWFlowMoveComponent>(AComp))
	{
		return FMComp;
	}

	return nullptr;
}

bool UGBWFlowMoveFuncLib::FM_IsFlowMoveControlled(AActor* InActor)
{
	if (const UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		return FMComp->IsFmControlled();
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_SetMoveVector(AActor* InActor, FVector NewMoveVector,
                                           float ZeroFaultToleranceDuration)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->SetMoveVector(NewMoveVector,ZeroFaultToleranceDuration);
		return true;
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_SetMoveVector_(
	AActor* InActor, float Right, float Forward, float Up,
	EFMMMoveVectorNormalizeType NormalizeType, float ZeroFaultToleranceDuration)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FVector NewMoveVector = FVector::ZeroVector;
		switch (NormalizeType) {
		case NoNormalize:
			NewMoveVector = FVector(Forward, Right, Up);
			break;
		case AllNormalize:
			NewMoveVector = FVector(Forward, Right, Up);
			NewMoveVector.Normalize();
			break;
		case HorizontalNormalize:
			NewMoveVector = FVector(Forward, Right, 0.0f);
			NewMoveVector.Normalize();
			NewMoveVector.Z = Up;
			break;
		}
		FMComp->SetMoveVector(NewMoveVector,ZeroFaultToleranceDuration);
		return true;
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_SetControlVector(AActor* InActor, FVector NewControlVector)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->SetControlVector(NewControlVector);
		return true;
	}

	return false;
}

void UGBWFlowMoveFuncLib::FM_UpdateInputFromCharacterInput(AActor* InActor)
{
	if (const EGBWActorNetType ActorNetType = UGBWPowerfulToolsFuncLib::GetActorNetType(InActor);
		ActorNetType == EGBWActorNetType::InClient_LocalPlayer
		|| ActorNetType == EGBWActorNetType::InServer_LocalPlayer)
	{
		ACharacter* AsCharacter = Cast<ACharacter>(InActor);
		if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(AsCharacter))
		{
			FMComp->SetControlVector(AsCharacter->GetControlRotation().Vector());
			FMComp->SetMoveVector(GetMoveVectorFromCharacterInputVector(AsCharacter));
		}
	}
}

bool UGBWFlowMoveFuncLib::FM_SetFocusActor(AActor* InActor, USceneComponent* FocusActorComponent, bool bSetViewModeAsActorLock)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->SetFocusActor(FocusActorComponent, bSetViewModeAsActorLock);
		return true;
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_SetViewMode(AActor* InActor, EFMViewMode NewViewMode, bool bAsDefaultViewMode)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->SetFlowMoveViewMode(NewViewMode, bAsDefaultViewMode);
		return true;
	}

	return false;
}

void UGBWFlowMoveFuncLib::FM_SetSpeedMode(AActor* InActor, EFMSpeedMode NewSpeedMode)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->SetFlowMoveSpeedMode(NewSpeedMode);
	}
}

EFMSpeedMode UGBWFlowMoveFuncLib::FM_GetSpeedMode(AActor* InActor)
{
	if (const UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		return FMComp->GetFlowMoveSpeedMode();
	}
	return EFMSpeedMode::FM_Medium;
}

void UGBWFlowMoveFuncLib::FM_SetBodyMode(AActor* InActor, EFMBodyMode NewBodyMode)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->SetFlowMoveBodyMode(NewBodyMode);
	}
}
EFMBodyMode UGBWFlowMoveFuncLib::FM_GetBodyMode(AActor* InActor)
{
	if (const UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		return FMComp->GetFlowMoveBodyMode();
	}
	return EFMBodyMode::FM_Stand;
}

void UGBWFlowMoveFuncLib::FM_SwitchViewMode(AActor* InActor, TEnumAsByte<EFMViewMode>& ViewModeNow)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		switch (FMComp->GetFlowMoveViewMode()) {
		case TP_FreeMode:
			ViewModeNow = TP_ForwardLockMode;
			FMComp->SetFlowMoveViewMode(TP_ForwardLockMode,true);
			break;
		case TP_ForwardLockMode:
			ViewModeNow = TP_FreeMode;
			FMComp->SetFlowMoveViewMode(TP_FreeMode,true);
			break;
		case TP_ActorLockMode:
			FMComp->SetFocusActor(nullptr,false);
			ViewModeNow = FMComp->GetFlowMoveViewMode();
			break;
		case TP_Aim:
			ViewModeNow = TP_FreeMode;
			FMComp->SetFlowMoveViewMode(TP_FreeMode,true);
			break;
		}
	}
}

void UGBWFlowMoveFuncLib::FM_SwitchLockActor(AActor* InActor, bool& bIsLockActor,
	FGBWFlowMoveTraceSetting PerceptionSetting, TArray<FName> ComponentWithTags,float PrejudgeDistance, float FuzzyQueryAngleRestrictions,
	const TSubclassOf<UUserWidget> ActorTargetWidgetClass,
	USoundBase* Sound_TargetGet,
	USoundBase* Sound_TargetLost)
{
	bIsLockActor = false;
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->ActorTargetWidgetClass = ActorTargetWidgetClass;
		FMComp->Sound_TargetGet = Sound_TargetGet;
		FMComp->Sound_TargetLost = Sound_TargetLost;
		FMComp->ActorTargetPrejudgeDistance = PrejudgeDistance;
		FMComp->ActorTargetFuzzyQueryAngleRestrictions = FuzzyQueryAngleRestrictions;
		
		bool bIsGet = false;
		USceneComponent* FocusedComponent = nullptr;
		FMComp->GetFocusActor(bIsGet, FocusedComponent);
		if (bIsGet)
		{
			FMComp->SetFocusActor(nullptr,false);
			bIsLockActor = false;
		}
		else
		{
			FocusedComponent = FM_TryGetFocusActor(InActor, FMComp->GetMoveVector(),FMComp->GetControlVector(), PerceptionSetting, ComponentWithTags, PrejudgeDistance, FuzzyQueryAngleRestrictions);
			FMComp->SetFocusActor(FocusedComponent,true);
			if (FocusedComponent){bIsLockActor = true;}
		}
	}
}

void UGBWFlowMoveFuncLib::FM_GetFocusActor(AActor* InActor, bool& bIsGet, USceneComponent*& FocusedComponent)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->GetFocusActor(bIsGet, FocusedComponent);
	}
}

void UGBWFlowMoveFuncLib::FM_AddVelocityTrend(AActor* InActor, FGBWFlowMoveVelocityTrend VelocityTrend)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->AddVelocityTrend(VelocityTrend);
	}
}

void UGBWFlowMoveFuncLib::FM_SetMoveControlInitInfo_Simple(AActor* InActor, const bool bHasTargetPoint, const FVector& TargetPoint)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->SetMoveControlInitInfo_Simple(bHasTargetPoint, TargetPoint);
	}
}

bool UGBWFlowMoveFuncLib::FM_IsHasMoveVector(AActor* InActor)
{
	if (const UGBWFlowMoveComponent* FmComp = FM_GetFlowMoveComponent(InActor))
	{
		FVector MoveVector = FmComp->InputState.MoveVector;
		MoveVector.Z = 0.0f;
		return !MoveVector.IsNearlyZero();
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_Pure_IsHasMoveVector(const UAnimInstance* AnimInstance)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWFlowMoveFuncLib::FM_IsHasMoveVector(
			AnimInstance->GetOwningActor());
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_IsHasUpDownMoveVector(AActor* InActor, bool& HasUpMoveVector, bool& HasDownMoveVector)
{
	HasUpMoveVector = false;
	HasDownMoveVector = false;
	if (const UGBWFlowMoveComponent* FmComp = FM_GetFlowMoveComponent(InActor))
	{
		FVector MoveVector = FmComp->InputState.MoveVector;
		MoveVector.X = 0.0f;
		MoveVector.Y = 0.0f;
		HasUpMoveVector = MoveVector.Z > 0.0f;
		HasDownMoveVector = MoveVector.Z < 0.0f;
		return !MoveVector.IsNearlyZero();
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_IsFallingDistance(AActor* InActor, float MinValue, float MaxValue)
{
	if (const UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		if (FMComp->TaskState.FallingDistance < MinValue)
		{
			return false;
		}

		if (MaxValue > 0 && FMComp->TaskState.FallingDistance > MaxValue)
		{
			return false;
		}
		return true;
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_Pure_IsFallingDistance(const UAnimInstance* AnimInstance, float MinValue, float MaxValue)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWFlowMoveFuncLib::FM_IsFallingDistance(
			AnimInstance->GetOwningActor(),
			MinValue,
			MaxValue);
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_IsCharacterSpeed(
	AActor* InActor,
	const bool OnlyForHorizontalSpeed,
	const bool OnlyForMoveToDirectionSpeed,
	float MinValue,
	float MaxValue)
{
	if (const UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FVector Velocity = FMComp->InputState.OwnerCharacter->GetVelocity();
		if (OnlyForHorizontalSpeed)
		{
			Velocity.Z = 0.0f;
		}
		float Speed = Velocity.Length();
		if (OnlyForMoveToDirectionSpeed)
		{
			Speed = UKismetMathLibrary::Dot_VectorVector(Velocity,FMComp->TaskState.MoveControlParam.MoveToDirection);
		}
		return Speed <= MaxValue && Speed >= MinValue;
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_Pure_IsCharacterSpeed(
	const UAnimInstance* AnimInstance,
	bool OnlyForHorizontalSpeed,
	bool OnlyForMoveToDirectionSpeed,
	float MinValue,
	float MaxValue)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWFlowMoveFuncLib::FM_IsCharacterSpeed(
			AnimInstance->GetOwningActor(),
			OnlyForHorizontalSpeed,
			OnlyForMoveToDirectionSpeed,
			MinValue,
			MaxValue);
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_IsAngle(
	AActor* InActor,
	const EFlowMoveDirectionType From,
	const EFlowMoveDirectionType To,
	const float MinValue,
	const float MaxValue)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		const FVector FromVector = FMComp->GetDirectionVector(From,FVector::ZeroVector);
		const FVector ToVector = FMComp->GetDirectionVector(To,FVector::ZeroVector);
		const float SceneDirection = UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(FromVector,ToVector);
		return SceneDirection >= MinValue && SceneDirection <= MaxValue;
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_Pure_IsAngle(
	const UAnimInstance* AnimInstance,
	EFlowMoveDirectionType From,
	EFlowMoveDirectionType To,
	float MinValue,
	float MaxValue)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWFlowMoveFuncLib::FM_IsAngle(
			AnimInstance->GetOwningActor(),From,To,MinValue,MaxValue);
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_IsViewMode(AActor* InActor, TEnumAsByte<EInclusivityType> ConditionType,
                                        TSet<TEnumAsByte<EFMViewMode>> ViewModeSet)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		if (ConditionType == EInclusivityType::Include_)
		{
			return ViewModeSet.Contains(FMComp->InputState.ViewMode);
		}
		else if (ConditionType == EInclusivityType::NotInclude_)
		{
			return !ViewModeSet.Contains(FMComp->InputState.ViewMode);
		}

		return false;
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_Pure_IsViewMode(const UAnimInstance* AnimInstance,
	TEnumAsByte<EInclusivityType> ConditionType, TSet<TEnumAsByte<EFMViewMode>> ViewModeSet)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		return UGBWFlowMoveFuncLib::FM_IsViewMode(
			AnimInstance->GetOwningActor(),ConditionType,ViewModeSet);
	}

	return false;
}

bool UGBWFlowMoveFuncLib::FM_GetMoveControlParam(AActor* InActor, FGBWFlowMoveControlParam& MoveControlParam)
{
	if (const UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		MoveControlParam = FMComp->GetMoveControlParam();
		return true;
	}

	return false;
}

void UGBWFlowMoveFuncLib::GetCustomParam(FGBWFlowMoveControlParam MoveControlParam, const FName Key, bool& bIsGet)
{
	bIsGet = MoveControlParam.MoveControlSettings.CustomParams.Contains(Key);
}

void UGBWFlowMoveFuncLib::FM_GetMoveParamForAnimAsset(AActor* InActor, UAnimationAsset* ForAnimAsset,
                                                      UAnimationAsset*& AnimAsset, float& Direction, float& Speed, float& PlayRate, float& StartPosition,float& EndPosition,
                                                      EFlowMoveDirectionType DirectionType, 
		EFMBSAxisType BS_HorizontalAxisFrom,
		EFMBSAxisType BS_VerticalAxisFrom,
		float RefMaxSpeed, const bool bUseStep, const float Step)
{
	PlayRate = 1.0f;
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->GetMoveParamForAnimAsset(
			ForAnimAsset,AnimAsset,Direction,Speed,PlayRate,StartPosition,EndPosition,
			DirectionType, BS_HorizontalAxisFrom, BS_VerticalAxisFrom,
			RefMaxSpeed, bUseStep, Step);
	}
}

void UGBWFlowMoveFuncLib::FM_Pure_GetMoveParamForAnimAsset(
	const UAnimInstance* AnimInstance,
	UAnimationAsset* ForAnimAsset, UAnimationAsset*& AnimAsset,
	float& Direction, float& Speed, float& PlayRate,
	float& StartPosition,float& EndPosition,
	EFlowMoveDirectionType DirectionType,
	EFMBSAxisType BS_HorizontalAxisFrom,
	EFMBSAxisType BS_VerticalAxisFrom,
	float RefMaxSpeed, const bool bUseStep,
	const float Step)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		UGBWFlowMoveFuncLib::FM_GetMoveParamForAnimAsset(
			AnimInstance->GetOwningActor(),
			ForAnimAsset,
			AnimAsset, Direction, Speed, PlayRate, StartPosition,EndPosition,
			DirectionType, BS_HorizontalAxisFrom, BS_VerticalAxisFrom,
			RefMaxSpeed, bUseStep, Step);
	}
}

void UGBWFlowMoveFuncLib::FM_GetMagicPlayerParam(AActor* InActor, UAnimationAsset* ForAnimAsset,
	FMagicPlayerParam& MagicPlayerParam, const bool InReplayNow,
		const bool bAutoReplayCodeTime, const float InReplayCodeTime,
	const bool InLoopAnimation, const float InBlendTime,const float InLoopBlendTime,
	EFlowMoveDirectionType DirectionType,
	EFMBSAxisType BS_HorizontalAxisFrom,
	EFMBSAxisType BS_VerticalAxisFrom, float RefMaxSpeed,
	const bool bUseStep, const float Step)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->GetMagicPlayerParam(ForAnimAsset, MagicPlayerParam,
			InReplayNow,bAutoReplayCodeTime,
			InReplayCodeTime,
			InLoopAnimation,
			InBlendTime,
			InLoopBlendTime,
			DirectionType, BS_HorizontalAxisFrom, BS_VerticalAxisFrom,
			RefMaxSpeed, bUseStep, Step);
	}
}

void UGBWFlowMoveFuncLib::FM_Pure_GetMagicPlayerParam(const UAnimInstance* AnimInstance, UAnimationAsset* ForAnimAsset,
	FMagicPlayerParam& MagicPlayerParam, const bool InReplayNow,const bool bAutoReplayCodeTime, const float InReplayCodeTime,
	const bool InLoopAnimation, const float InBlendTime,const float InLoopBlendTime, EFlowMoveDirectionType DirectionType,
	EFMBSAxisType BS_HorizontalAxisFrom,
	EFMBSAxisType BS_VerticalAxisFrom, float RefMaxSpeed,
	const bool bUseStep, const float Step)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		UGBWFlowMoveFuncLib::FM_GetMagicPlayerParam(
			AnimInstance->GetOwningActor(),
			ForAnimAsset,
			MagicPlayerParam,
			InReplayNow,bAutoReplayCodeTime,
			InReplayCodeTime,
			InLoopAnimation,
			InBlendTime,
			InLoopBlendTime,
			DirectionType, BS_HorizontalAxisFrom, BS_VerticalAxisFrom,
			RefMaxSpeed, bUseStep, Step);
	}
}

void UGBWFlowMoveFuncLib::FM_GetMoveParam(AActor* InActor, UAnimationAsset* ForAnimAsset, UAnimSequence*& AnimSequence, float& PlayRate, float&StartPosition,float& EndPosition)
{
	PlayRate = 1.0f;
	StartPosition = 0.0f;
	if (const UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		FMComp->GetMoveParam(ForAnimAsset, AnimSequence,PlayRate,StartPosition,EndPosition);
	}
}

void UGBWFlowMoveFuncLib::FM_Pure_GetMoveParam(
		const UAnimInstance* AnimInstance,
		UAnimationAsset* ForAnimAsset,
		float& PlayRate,
		float& StartPosition,
		float& EndPosition,
		UAnimSequence*& AnimSequence)
{
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		UGBWFlowMoveFuncLib::FM_GetMoveParam(AnimInstance->GetOwningActor(), ForAnimAsset, AnimSequence,PlayRate, StartPosition,EndPosition);
	}
}

void UGBWFlowMoveFuncLib::FM_GetAnimExpectedPlayRate(AActor* InActor, float& PlayRate)
{
	PlayRate = 1.0f;
	if (const UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		PlayRate = FMComp->TaskState.MoveControlParam.AnimPlayRateNow;;
	}
}

void UGBWFlowMoveFuncLib::FM_Pure_GetAnimExpectedPlayRate(const UAnimInstance* AnimInstance, float& PlayRate)
{
	PlayRate = 1.0f;
	if (AnimInstance && AnimInstance->GetOwningActor())
	{
		UGBWFlowMoveFuncLib::FM_GetAnimExpectedPlayRate(
			AnimInstance->GetOwningActor(),
			PlayRate);
	}
}

void UGBWFlowMoveFuncLib::DrawDebugShape(
	const UObject* WorldContext,
	const FVector Location,
	const FQuat Rotation,
	const FCollisionShape CollisionShape,
	const FColor Color,
	float Duration, 
	float Thickness)
{
	if (CollisionShape.IsBox())
	{
		DrawDebugBox(
			WorldContext->GetWorld(),
			Location,
			CollisionShape.GetExtent(),
			Color,
			false,
			Duration,
			0,
			Thickness
		);
	}

	if (CollisionShape.IsCapsule())
	{
		DrawDebugCapsule(
			WorldContext->GetWorld(),
			Location,
			CollisionShape.GetCapsuleHalfHeight(),
			CollisionShape.GetCapsuleRadius(),
			Rotation,
			Color,
			false,
			Duration,
			0,
			Thickness
		);
	}

	if (CollisionShape.IsSphere())
	{
		DrawDebugSphere(
			WorldContext->GetWorld(),
			Location,
			CollisionShape.GetSphereRadius(),
			8,
			Color,
			false,
			Duration,
			0,
			Thickness
		);
	}
}

void UGBWFlowMoveFuncLib::PrintDebugString(TArray<FString> DebugStrArr, const FName Key, const FColor Color, const float Duration)
{
	uint8 Index = 0;
	for (auto S:DebugStrArr)
	{
		Index++;
		uint64 InnerKey = -1;
		if (Key != NAME_None)
		{
			InnerKey = GetTypeHash(FName(Key.ToString().Append(FString::SanitizeFloat(Index))));
		}
		GEngine->AddOnScreenDebugMessage(
			InnerKey,Duration,Color,S
		);
	}
}

void UGBWFlowMoveFuncLib::PrintDebugStringNet(
	const AActor* WorldActor, TArray<FString> DebugStrArr, FName Key,
	const FColor ServerColor, const FColor ClientColor, float Duration)
{
	if (!WorldActor)
	{
		return;
	}
	const bool bIsServer = WorldActor->HasAuthority();

	PrintDebugString(DebugStrArr,
	FName(Key.IsValid()?
	(bIsServer? Key.ToString().Append("_Server") : Key.ToString().Append("_Client"))
	: (bIsServer? FString("_Server") : FString("_Client"))),
		bIsServer? ServerColor : ClientColor,
		Duration
		);
}

void UGBWFlowMoveFuncLib::FM_Debug(AActor* InActor)
{
	if (UGBWFlowMoveComponent* FMComp = FM_GetFlowMoveComponent(InActor))
	{
		const FGBWFlowMoveState TaskState = FMComp->TaskState;
		const FVector StartPoint = FMComp->InputState.OwnerCharacter->GetActorLocation() + FVector(0,0,FMComp->InputState.OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		FString DebugStr = "";
		DebugStr = DebugStr + FString::Printf(
			TEXT(">>>>>>>FlowMoveState<<<<<<< "
			"\n Net : %s"	
			"\n OwnerCharacter : %s "
			"\n Timer : %f "
			"\n FrameDeltaTime : %f "
			"\n bIsActive : %s "
			"\n FrameDeltaTime : %.4f "
			"\n MovementMode : %s "
			"\n LastActorTransform : %s "
			"\n FallingDistance : %.2f"
			"\n ViewMode : %s "
			"\n SpeedMode : %s "
			"\n BodyMode : %s "
			"\n FocusActorComponent : %s"
			"\n MoveVector : %s "
			"\n LastMoveVectorTime : (%.2f) "
			"\n ControlVector : %s "
			"\n ForwardVector <%.4f> : %s "
			"\n >>>>>>>FlowMoveState<<<<<<<"),
			*(StaticEnum<EGBWActorNetType>()->GetNameStringByIndex( static_cast<int32>(UGBWPowerfulToolsFuncLib::GetActorNetType(InActor)))),
			*(FMComp->InputState.OwnerCharacter? FMComp->InputState.OwnerCharacter->GetName() : "None"),
			TaskState.Timer,
			TaskState.FrameDeltaTime,
			*(TaskState.bIsActive? FString("True") : FString("False")),
			TaskState.FrameDeltaTime,
			*(StaticEnum<EFMViewMode>()->GetNameStringByIndex( ( int32 ) FMComp->InputState.ViewMode)),
			*(TaskState.LastActorTransform.ToString()),
			TaskState.FallingDistance,
			*(StaticEnum<EFMViewMode>()->GetNameStringByIndex( ( int32 ) FMComp->InputState.ViewMode)),
			*(StaticEnum<EFMSpeedMode>()->GetNameStringByIndex( ( int32 ) FMComp->InputState.SpeedMode)),
			*(StaticEnum<EFMBodyMode>()->GetNameStringByIndex( ( int32 ) FMComp->InputState.BodyMode)),
			*(FMComp->InputState.FocusActorComponent? FMComp->InputState.FocusActorComponent->GetName() : "None"),
			*FMComp->GetMoveVector().ToString(),
			FMComp->InputState.LastMoveVectorTime,
			*FMComp->InputState.ControlVector.ToString(),
			FMComp->InputState.ForwardVectorYawAngleSpeed,
			*FMComp->InputState.ForwardVector.ToString()
		);
		DebugStr = DebugStr + "\n\n" + FString::Printf(
			TEXT(">>>>>>>FlowMoveControlParam<<<<<<< "
			"\n bIsActive : %s "
			"\n FromAsset : %s "
			"\n bIsNew : %s "
			"\n ActionDuration : %f "
			"\n AnimComponent : %s "
			"\n AnimPlayRateNow : %.4f "
			"\n TargetPoint : %s "
			"\n ToTargetPointRate : %.4f "
			"\n ToTargetPointSpeed : %f "
			"\n Gravity : %.2f"
			"\n MoveSpeed : %.2f "
			"\n DeltaPathOffset : %s"
			"\n MoveToDirection : %s "
			"\n RootMotionData <%f> : %s "
			"\n BSPosition : <%s>"
			"\n >>>>>>>FlowMoveControlParam<<<<<<<"),
			*(TaskState.MoveControlParam.bIsActive? FString("True"):FString("False")),
			*(TaskState.MoveControlParam.MoveControlSettings.FromAsset? TaskState.MoveControlParam.MoveControlSettings.FromAsset->GetName():"None"),
			*(TaskState.MoveControlParam.MoveControlSettings.bIsNew? FString("True"):FString("False")),
			TaskState.MoveControlParam.ActionDuration,
			*(TaskState.MoveControlParam.AnimComponent? TaskState.MoveControlParam.AnimComponent->GetName():"None"),
			TaskState.MoveControlParam.AnimPlayRateNow,
			*TaskState.MoveControlParam.TargetPoint.ToString(),
			TaskState.MoveControlParam.ToTargetPointRate,
			TaskState.MoveControlParam.ToTargetPointSpeed,
			TaskState.MoveControlParam.Gravity,
			TaskState.MoveControlParam.MoveSpeed,
			*TaskState.MoveControlParam.DeltaPathOffset.ToString(),
			*TaskState.MoveControlParam.MoveToDirection.ToString(),
			TaskState.MoveControlParam.RootMotionNow.GetLocation().Length(),
			*TaskState.MoveControlParam.RootMotionNow.ToString(),
			*TaskState.MoveControlParam.BlendSpacePosition.ToString()
		);
		DebugStr = DebugStr + "\n\n" + FString::Printf(
			TEXT(">>>>>>>FlowMoveControlResult<<<<<<< "
			"\n TargetRotation : %s "
			"\n RMSVelocity : %s "
			"\n RMSRotation : %s "
			"\n bConstrainToPlane : %s "
			"\n ConstrainPlaneNormal : %s "
			"\n ConstrainPlaneOrigin : %s "
			"\n CapsuleRadius : %.4f "
			"\n CapsuleHalfHeight : %.4f "
			"\n MeshRelativeTransform : %s "
			"\n >>>>>>>FlowMoveControlResult<<<<<<<"),
			*(FMComp->MoveControlResult.TargetRotation.ToString()),
			*(FMComp->MoveControlResult.Velocity.ToString()),
			*(FMComp->MoveControlResult.DeltaRotator.ToString()),
			*(FMComp->MoveControlResult.bConstrainToPlane?FString("True"):FString("False")),
			*(FMComp->MoveControlResult.ConstrainPlaneNormal.ToString()),
			*FMComp->MoveControlResult.ConstrainPlaneOrigin.ToString(),
			FMComp->MoveControlResult.CapsuleRadius,
			FMComp->MoveControlResult.CapsuleHalfHeight,
			*FMComp->MoveControlResult.MeshRelativeTransform.ToString()
		);

		PrintDebugStringNet(InActor, {DebugStr}, FName("FMDebug"),
			FColor::Orange, FColor::Green, 0);

		const FMatrix RotationMatrix = FRotationMatrix::MakeFromZ(FMComp->InputState.OwnerCharacter->GetActorUpVector());
		const FVector ForwardVector = RotationMatrix.GetScaledAxis(EAxis::X);
		const FVector RightVector = RotationMatrix.GetScaledAxis(EAxis::Y);
		DrawDebugCircle(FMComp->InputState.OwnerCharacter->GetWorld(),
			StartPoint,
			50.0f,
			48,
			FColor::White,
			false,
			0.0f,
			0,
			1.0f,
			ForwardVector,
			RightVector,
			true);
		DrawDebugCircle(FMComp->InputState.OwnerCharacter->GetWorld(),
			StartPoint + FVector(0.0f,0.0f,-40.0f),
			50.0f,
			48,
			FColor::White,
			false,
			0.0f,
			0,
			1.0f,
			ForwardVector,
			RightVector,
			true);

		FM_DrawDebugDirectionArrow(FMComp,EFlowMoveDirectionType::ActorForward,false,StartPoint,0.0f,-40.0f,
			100.0f,
			FColor::White,"");
		FM_DrawDebugDirectionArrow(FMComp,EFlowMoveDirectionType::FlowMoveForward,false,StartPoint,-5.0f,-40.0f,
			100.0f,
			FColor::Orange,"");
		
		FM_DrawDebugDirectionArrow(FMComp,EFlowMoveDirectionType::Velocity,false,StartPoint,-10.0f,-40.0f,
			(50.0f + 100.0f * FMComp->InputState.OwnerCharacter->GetVelocity().Length() / 1000.0f),
			FColor::Cyan,FString::Printf(TEXT("<%.4f cm/s>"),FMComp->InputState.OwnerCharacter->GetVelocity().Length()));
		
		FM_DrawDebugDirectionArrow(FMComp,EFlowMoveDirectionType::Controller,true,StartPoint,-15.0f,-40.0f,
			100.0f,
			FColor::Red,"");
		
		FM_DrawDebugDirectionArrow(FMComp,EFlowMoveDirectionType::CurrentMoveToDirection,false,StartPoint,-20.0f,-40.0f,
			(50.0f + 100.0f * FMComp->TaskState.MoveControlParam.MoveSpeed / 1000.0f),
			FColor::Magenta,FString::Printf(TEXT("<%.4f cm/s>"),FMComp->TaskState.MoveControlParam.MoveSpeed));
		
		FM_DrawDebugDirectionArrow(FMComp,EFlowMoveDirectionType::TargetMoveToDirection,false,StartPoint,-25.0f,-40.0f,
			(50.0f + 100.0f * FMComp->GetMoveVector().Length()),
			FColor::Yellow,FString::Printf(TEXT("<%.4f><%.4f>")
				,FMComp->GetMoveVector().Length()
				,FVector(FMComp->GetMoveVector().X,FMComp->GetMoveVector().Y,0.0f).Length()));
		
		FM_DrawDebugDirectionArrow(FMComp,EFlowMoveDirectionType::StartMoveToDirection,false,StartPoint,-30.0f,-40.0f,
			100.0f,
			FColor(255,100,255),"");
		
		FM_DrawDebugDirectionArrow(FMComp,EFlowMoveDirectionType::StartTargetMoveToDirection,false,StartPoint,-35.0f,-40.0f,
			100.0f,
			FColor(255,255,100),"");
		
		DrawDebugCapsule(
			FMComp->InputState.OwnerCharacter->GetWorld(),
			FMComp->InputState.OwnerCharacter->GetCapsuleComponent()->GetComponentLocation(),
			FMComp->InputState.OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
			FMComp->InputState.OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
			FMComp->InputState.OwnerCharacter->GetCapsuleComponent()->GetComponentRotation().Quaternion(),
			FMComp->InputState.OwnerCharacter->HasAuthority()? FColor::Black : FColor::White,
			false,
			0.0f,
			0,
			1.0f);

		if (FMComp->InputState.OwnerCharacter->GetMovementComponent()->bConstrainToPlane)
		{
			DrawDebugSolidPlane(
				FMComp->InputState.OwnerCharacter->GetWorld(),
				FPlane(FMComp->InputState.OwnerCharacter->GetMovementComponent()->GetPlaneConstraintOrigin(),FMComp->InputState.OwnerCharacter->GetMovementComponent()->GetPlaneConstraintNormal()),
				FMComp->InputState.OwnerCharacter->GetMovementComponent()->GetPlaneConstraintOrigin(),
				200.0f,
				FColor::Purple,
				false,
				-1,
				0
			);
		}
	}
}

void UGBWFlowMoveFuncLib::FM_DrawDebugDirectionArrow(
		UGBWFlowMoveComponent* FlowMoveComponent,
		const EFlowMoveDirectionType DirectionType,
		const bool bOnlyXY,
		const FVector StartPoint,
		const float ZOffset,
		const float ZOffset_String,
		const float Length,
		const FColor Color,
		const FString AppendStr)
{
	if (!FlowMoveComponent)
	{
		return;
	}

	FVector Vector = FlowMoveComponent->GetDirectionVector(DirectionType,FVector::ZeroVector);
	if (bOnlyXY)
	{
		Vector.Z = 0.0f;
		Vector.Normalize();
	}
	DrawDebugDirectionalArrow(
		FlowMoveComponent->GetWorld(),
		StartPoint + FVector(0.0f,0.0f,ZOffset),
		StartPoint+ FVector(0.0f,0.0f,ZOffset) + Vector*Length,
		50.0f,
		Color,
		false,
		0.0f,
		0,
		2.0f);

	DrawDebugString(
		FlowMoveComponent->GetWorld(),
		StartPoint + FVector(0.0f,0.0f,ZOffset+ZOffset_String),
		StaticEnum<EFlowMoveDirectionType>()->GetNameStringByIndex( static_cast<int32>(DirectionType)) + AppendStr,
		nullptr,
		Color,
		0.0f,
		true,
		0.9f);
}

bool UGBWFlowMoveFuncLib::FM_TestVisibility(const AActor* StartActor, const AActor* EndActor, const FGBWFlowMoveTraceSetting& TraceSetting,
                                            FVector& TouchPoint)
{
	if (!StartActor || !EndActor)
	{
		return false;
	}

	FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionQueryParams.bTraceComplex = false;
	CollisionQueryParams.AddIgnoredActor(StartActor);
	
	TArray<FHitResult> OutHits;
	FHitResult HitRes = UGBWFlowMoveFuncLib::LineTrace(
		StartActor->GetWorld(),
		OutHits,
		StartActor->GetActorLocation(),
		EndActor->GetActorLocation(),
		TraceSetting,
		CollisionQueryParams);

	if (HitRes.IsValidBlockingHit())
	{
		if (HitRes.GetActor() == EndActor)
		{
			TouchPoint = HitRes.ImpactPoint;
			return true;
		}
	}

	return false;
}

USceneComponent* UGBWFlowMoveFuncLib::FM_TryGetFocusActor(const AActor* InActor, const FVector MoveDirection,
                                                       const FVector ControllerDirection, FGBWFlowMoveTraceSetting PerceptionSetting, TArray<FName> ComponentWithTags,
                                                       float PrejudgeDistance, float FuzzyQueryAngleRestrictions)
{
	USceneComponent* Res = nullptr;
	const ACharacter* InCharacter = Cast<ACharacter>(InActor);
	if (!InCharacter)
	{
		return Res;
	}
	
	//Reusable parameters - start
	FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionQueryParams.bTraceComplex = false;
	CollisionQueryParams.AddIgnoredActor(InCharacter);
	//Reusable parameters - end

	FVector ForwardVector = ControllerDirection;
	FVector ViewForwardVector;
	FVector ViewLocation;
	FRotator ViewRotation;
	UGBWRMSBPFuncLib::GBWRMS_GetCharacterViewInfo(InCharacter,ViewLocation, ViewRotation, ViewForwardVector);
	ForwardVector = ViewForwardVector;
	
	ForwardVector.Z = 0.0f;
	if (!MoveDirection.IsNearlyZero())
	{
		ForwardVector = ForwardVector.Rotation().RotateVector(MoveDirection);
	}
	
	const auto ForwardTraceDirection{ControllerDirection};

	const auto* Capsule{InCharacter->GetCapsuleComponent()};

	const auto CapsuleScale{Capsule->GetComponentScale().Z};
	const auto CapsuleRadius{Capsule->GetScaledCapsuleRadius()};
	const auto TraceCapsuleRadius{CapsuleRadius - 1.0f};
	
	// Get Target Point
	static const FName ForwardTraceTag{FString::Format(TEXT("{0} (Forward Trace)"), {ANSI_TO_TCHAR(__FUNCTION__)})};
	
	auto ForwardTraceStart{ViewLocation+ForwardTraceDirection*CapsuleRadius*2.0f};
	auto ForwardTraceEnd{ForwardTraceStart + ForwardTraceDirection * (CapsuleRadius + (PrejudgeDistance + 1.0f) * CapsuleScale)};

	FHitResult ForwardTraceHit;
	CollisionQueryParams.TraceTag = ForwardTraceTag;
	TArray<struct FHitResult> ForwardTraceHitArr;
	ForwardTraceHit = UGBWFlowMoveFuncLib::SweepMulti(
		InCharacter->GetWorld(),
		ForwardTraceHitArr,
		ForwardTraceStart,
		ForwardTraceEnd,
		FQuat::Identity,
		PerceptionSetting,
		FCollisionShape::MakeSphere(TraceCapsuleRadius),
		CollisionQueryParams
	);

	if (ForwardTraceHit.IsValidBlockingHit())
	{
		if (FVector TargetPoint = FVector::ZeroVector;
				FM_TestVisibility(InCharacter,
					ForwardTraceHit.GetActor(),PerceptionSetting,TargetPoint))
		{
			AActor* TargetActor = ForwardTraceHit.GetActor();
			if (ComponentWithTags.IsEmpty())
			{
				return TargetActor->GetRootComponent();
			}
			
			for (auto Tag:ComponentWithTags)
			{
				TArray<UActorComponent*> ACs = TargetActor->GetComponentsByTag(USceneComponent::StaticClass(), Tag);
				if (!ACs.IsEmpty())
				{
					Res = Cast<USceneComponent>(ACs[0]);
				}
				if (Res)
				{
					return Res;
				}
			}
		}
	}

	ForwardTraceHit = UGBWFlowMoveFuncLib::SweepMulti(
			InCharacter->GetWorld(),
			ForwardTraceHitArr,
			InCharacter->GetActorLocation(),
			InCharacter->GetActorLocation(),
			FQuat::Identity,
			PerceptionSetting,
			FCollisionShape::MakeSphere(PrejudgeDistance),
			CollisionQueryParams
		);
	
	float AngleCache = 180.0f;
	AActor* TargetActor = nullptr;
	FVector TouchPoint = FVector::ZeroVector;
	for (auto Hit:ForwardTraceHitArr)
	{
		if (FM_TestVisibility(InCharacter,
			Hit.GetActor(),PerceptionSetting, TouchPoint))
		{
			FVector DirectionVector = TouchPoint-ViewLocation;
			DirectionVector.Normalize();
			float Angle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(DirectionVector, ViewForwardVector));
			if (Angle<=FuzzyQueryAngleRestrictions && Angle <= AngleCache)
			{
				AngleCache = Angle;
				TargetActor = Hit.GetActor();
			}
		}
	}

	if (TargetActor)
	{
		if (ComponentWithTags.IsEmpty())
		{
			return TargetActor->GetRootComponent();
		}
			
		for (auto Tag:ComponentWithTags)
		{
			TArray<UActorComponent*> ACs = TargetActor->GetComponentsByTag(USceneComponent::StaticClass(), Tag);
			if (!ACs.IsEmpty())
			{
				Res = Cast<USceneComponent>(ACs[0]);
			}
			if (Res)
			{
				return Res;
			}
		}
	}

	return nullptr;
}
