// Copyright GanBowen 2022-2024. All Rights Reserved. I love DuoDuo!


#include "GBWPowerfulToolsFuncLib.h"

#include "GBWToolsComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void UGBWPowerfulToolsFuncLib::Boxplot_Float(const TArray<float>& InDataArr, TArray<float>& OutDataArr)
{
	if (InDataArr.Num() <= 0)
	{
		return;
	}
	TArray<float> DataArr = InDataArr;
	DataArr.Sort();
	const float Q1 = DataArr[DataArr.Num()*0.25f];
	//const float Q2 = DataArr[DataArr.Num()*0.50f];
	const float Q3 = DataArr[DataArr.Num()*0.75f];
	const float Iqr = Q3 - Q1;

	const float Outlier_Min = Q1 - 1.5 * Iqr;
	const float Outlier_Max = Q3 + 1.5 * Iqr;

	OutDataArr.Empty();
	for (auto Data : DataArr)
	{
		if (Outlier_Min <= Data && Data <= Outlier_Max)
		{
			OutDataArr.Add(Data);
		}
	}
}

float UGBWPowerfulToolsFuncLib::GetAngle(const FVector& From, const FVector& To, const FVector& PlaneNormal)
{
	FTransform T = FTransform();
	T.SetRotation(From.Rotation().Quaternion());
	
	FVector A = UKismetMathLibrary::ProjectVectorOnToPlane(
		To,
		PlaneNormal);
	A.Normalize();
	FVector B = UKismetMathLibrary::ProjectVectorOnToPlane(
		From,
		PlaneNormal);
	B.Normalize();
	const float DirectionAngle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(A,B));

	const FVector A2 = UKismetMathLibrary::InverseTransformDirection(T,A);
	const FVector B2 = UKismetMathLibrary::InverseTransformDirection(T,B);
	const bool bIsRight = UKismetMathLibrary::CrossProduct2D(
		FVector2D(A2.X, A2.Y),
		FVector2D(B2.X, B2.Y)
	)<=0;

	return DirectionAngle * (bIsRight? 1 : -1);
}

float UGBWPowerfulToolsFuncLib::GetAngleInHorizontalPlane(const FVector& From, const FVector& To)
{
	return UGBWPowerfulToolsFuncLib::GetAngle(From, To, FVector(0,0,1));
}

float UGBWPowerfulToolsFuncLib::GetAngleInVerticalPlane(const FVector& From, const FVector& To)
{
	FRotator ToR = To.Rotation();
	FRotator FromR = From.Rotation();
	ToR.Normalize();
	FromR.Normalize();
	return ToR.Pitch - FromR.Pitch;
}

float UGBWPowerfulToolsFuncLib::GetAlphaByBlendOption(
	EAlphaBlendOption BlendOption,
	const float InAlpha)
{
	float OutAlpha = InAlpha;
	switch(BlendOption)
	{
	case EAlphaBlendOption::Sinusoidal:		OutAlpha = FMath::Clamp<float>((FMath::Sin(InAlpha * UE_PI - UE_HALF_PI) + 1.f) / 2.f, 0.f, 1.f); break;
	case EAlphaBlendOption::Cubic:			OutAlpha = FMath::Clamp<float>(FMath::CubicInterp<float>(0.f, 0.f, 1.f, 0.f, InAlpha), 0.f, 1.f); break;
	case EAlphaBlendOption::QuadraticInOut: OutAlpha = FMath::Clamp<float>(FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, 2), 0.f, 1.f); break;
	case EAlphaBlendOption::CubicInOut:		OutAlpha = FMath::Clamp<float>(FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, 3), 0.f, 1.f); break;
	case EAlphaBlendOption::HermiteCubic:	OutAlpha = FMath::Clamp<float>(FMath::SmoothStep(0.0f, 1.0f, InAlpha), 0.0f, 1.0f); break;
	case EAlphaBlendOption::QuarticInOut:	OutAlpha = FMath::Clamp<float>(FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, 4), 0.f, 1.f); break;
	case EAlphaBlendOption::QuinticInOut:	OutAlpha = FMath::Clamp<float>(FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, 5), 0.f, 1.f); break;
	case EAlphaBlendOption::CircularIn:		OutAlpha = FMath::Clamp<float>(FMath::InterpCircularIn<float>(0.0f, 1.0f, InAlpha), 0.0f, 1.0f); break;
	case EAlphaBlendOption::CircularOut:	OutAlpha = FMath::Clamp<float>(FMath::InterpCircularOut<float>(0.0f, 1.0f, InAlpha), 0.0f, 1.0f); break;
	case EAlphaBlendOption::CircularInOut:	OutAlpha = FMath::Clamp<float>(FMath::InterpCircularInOut<float>(0.0f, 1.0f, InAlpha), 0.0f, 1.0f); break;
	case EAlphaBlendOption::ExpIn:			OutAlpha = FMath::Clamp<float>(FMath::InterpExpoIn<float>(0.0f, 1.0f, InAlpha), 0.0f, 1.0f); break;
	case EAlphaBlendOption::ExpOut:			OutAlpha = FMath::Clamp<float>(FMath::InterpExpoOut<float>(0.0f, 1.0f, InAlpha), 0.0f, 1.0f); break;
	case EAlphaBlendOption::ExpInOut:		OutAlpha = FMath::Clamp<float>(FMath::InterpExpoInOut<float>(0.0f, 1.0f, InAlpha), 0.0f, 1.0f); break;
	default: OutAlpha = FMath::Clamp<float>(InAlpha, 0.f, 1.f); break;
	}
	
	return OutAlpha;
}

UGBWToolsComponent* UGBWPowerfulToolsFuncLib::GetGBWtToolsComponent(AActor* InActor)
{
	if (!InActor)
	{
		return nullptr;
	}
	UGBWToolsComponent* AComp = nullptr;
	if (UActorComponent* Component = InActor->GetComponentByClass(UGBWToolsComponent::StaticClass()))
	{
		AComp = Cast<UGBWToolsComponent>(Component);
	}

	if (!AComp && InActor->HasAuthority())
	{
		AComp = Cast<UGBWToolsComponent>(
			InActor->AddComponentByClass(UGBWToolsComponent::StaticClass(),
				false,
				FTransform(),
				false));
		AComp->SetIsReplicated(true);
	}

	return AComp;
}

void UGBWPowerfulToolsFuncLib::execSetGBWProperty(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	P_GET_OBJECT(AActor,Z_Param_InActor);
	P_GET_PROPERTY(FStrProperty,Z_Param_Key);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const void* StructPtr = Stack.MostRecentPropertyAddress;
	FProperty* StructProperty = Stack.MostRecentProperty;
	
	P_FINISH;
	P_NATIVE_BEGIN;
	if (UGBWToolsComponent* TComp = UGBWPowerfulToolsFuncLib::GetGBWtToolsComponent(Z_Param_InActor))
	{
		const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
		{
			FGBWStruct GS = UserStructToGBWStruct(StructPtr, StructProp->Struct);
			TComp->AddProperty(Z_Param_Key,GS);
		}
	}
	P_NATIVE_END;
}

void UGBWPowerfulToolsFuncLib::execGetGBWPropertyAsStruct(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	P_GET_OBJECT(AActor,Z_Param_InActor);
	P_GET_PROPERTY(FStrProperty,Z_Param_Key);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	if (UGBWToolsComponent* TComp = UGBWPowerfulToolsFuncLib::GetGBWtToolsComponent(Z_Param_InActor))
	{
		FGBWStruct Value;
		if (TComp->GetProperty(Z_Param_Key, Value))
		{
			const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
			if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
			{
				*static_cast<bool*>(Z_Param__Result) = UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(Value, StructPtr, StructProp->Struct);
			}
			else
			{
				*static_cast<bool*>(Z_Param__Result) = false;
			}
		}
		else
		{
			*static_cast<bool*>(Z_Param__Result) = false;
		}
	}
	else
	{
		*static_cast<bool*>(Z_Param__Result) = false;
	}
}

void UGBWPowerfulToolsFuncLib::GetPlayerCamera(
	AActor* InActor,
	bool& bIsGet,
	UCameraComponent*& Camera,
	USpringArmComponent*& SpringArm)
{
	if (!InActor)
	{
		bIsGet = false;
		return;
	}
	TArray<UActorComponent*> OutComponents;
	InActor->GetComponents(UCameraComponent::StaticClass(),OutComponents);
	for (const auto Component : OutComponents)
	{
		if (Component->IsActive())
		{
			Camera = Cast<UCameraComponent>(Component);
			bIsGet = true;
			break;
		}
	}

	InActor->GetComponents(USpringArmComponent::StaticClass(),OutComponents);
	for (const auto Component : OutComponents)
	{
		if (Component->IsActive())
		{
			SpringArm = Cast<USpringArmComponent>(Component);
			bIsGet = true;
			break;
		}
	}
}

void UGBWPowerfulToolsFuncLib::GetPlayerController(AActor* InActor, bool& bIsGet, bool& bIsLocalController,
	APlayerController*& PlayerController)
{
	bIsGet = false;
	bIsLocalController = false;
	PlayerController = nullptr;
	
	if (!InActor)
	{
		return;
	}

	if (const APawn* Pawn = Cast<APawn>(InActor))
	{
		PlayerController = Cast<APlayerController>(Pawn->GetController());
	}
	
	if (!PlayerController)
	{
		if (const APlayerState* PlayerState = Cast<APlayerState>(InActor))
		{
			PlayerController = PlayerState->GetPlayerController();
		}
	}

	if (!PlayerController)
	{
		PlayerController = Cast<APlayerController>(InActor);
	}

	if (PlayerController)
	{
		bIsGet = true;
		bIsLocalController = PlayerController->IsLocalController();
	}
}

EGBWActorNetType UGBWPowerfulToolsFuncLib::GetActorNetType(AActor* InActor)
{
	if (!InActor)
	{
		return EGBWActorNetType::None;
	}

	const ENetRole LocalRole = InActor->GetLocalRole();
	const ENetRole RemoteRole = InActor->GetRemoteRole();

	bool bIsPlayer = false;
	bool bIsLocalPlayer = false;
	if (const APawn* PlayerActor = Cast<APawn>(InActor))
	{
		if (PlayerActor->IsPlayerControlled())
		{
			bIsPlayer = true;
		}

		if (PlayerActor->GetController() && PlayerActor->GetController()->IsLocalPlayerController())
		{
			bIsLocalPlayer = true;
		}
	}
	
	if (LocalRole == ENetRole::ROLE_Authority)
	{
		if (bIsLocalPlayer)
		{
			return EGBWActorNetType::InServer_LocalPlayer;
		}

		if (bIsPlayer)
		{
			return EGBWActorNetType::InServer_RemotePlayer;
		}
		
		return EGBWActorNetType::InServer_NotPlayer;
	}

	if (LocalRole != ENetRole::ROLE_None)
	{
		if (bIsLocalPlayer)
		{
			return EGBWActorNetType::InClient_LocalPlayer;
		}

		if (bIsPlayer)
		{
			return EGBWActorNetType::InClient_RemotePlayer;
		}

		return EGBWActorNetType::InClient_NotPlayer;
	}

	return EGBWActorNetType::None;
}

void UGBWPowerfulToolsFuncLib::GetAllPropertiesStr(UObject* InObject, TArray<FString>& PropertyNames, TArray<FString>& PropertyValues)
{
	PropertyNames = {};
	PropertyValues = {};
	if (!InObject)
	{
		return;
	}
	for(const FProperty* Property = InObject->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		FString ValueStr = "";
		Property->ExportTextItem_Direct(
			ValueStr,
			Property->ContainerPtrToValuePtr<void>(InObject),
			nullptr,
			InObject,
			PPF_None,
			nullptr);
		PropertyNames.Add(Property->GetName());
		PropertyValues.Add(ValueStr);
	}
}

void UGBWPowerfulToolsFuncLib::execToUserStruct(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	P_GET_UBOOL_REF(Z_Param_Out_bIsSuccess);
	P_GET_STRUCT(FGBWStruct,Z_Param_GBWStruct);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	
	const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
	{
		Z_Param_Out_bIsSuccess = GBWStructToUserStruct(Z_Param_GBWStruct, StructPtr, StructProp->Struct);
	}
	else
	{
		Z_Param_Out_bIsSuccess = false;
	}
}

void UGBWPowerfulToolsFuncLib::execToGBWStruct(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const void* StructPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (StructPtr != nullptr)))
	{
		*static_cast<FGBWStruct*>(Z_Param__Result) = UserStructToGBWStruct(StructPtr, StructProp->Struct);
	}
	else
	{
		*static_cast<FGBWStruct*>(Z_Param__Result) = FGBWStruct();
	}
}

bool UGBWPowerfulToolsFuncLib::GBWStructToUserStruct(const FGBWStruct& InGBWStruct, void* StructPtr, UScriptStruct* ScriptStruct)
{
	if (StructPtr != nullptr && ScriptStruct != nullptr)
	{
		const FString SourceName = InGBWStruct.StructType;
		const FString SourceText = InGBWStruct.StructStr;
		FString TargetName;
		ScriptStruct->GetName(TargetName);
			
		if (TargetName != SourceName)
		{
			return false;
		}
		if (nullptr != ScriptStruct->ImportText(*SourceText, StructPtr, nullptr, PPF_None, nullptr, FString("")))
		{
			return true;
		}
	}
	return false;
}

FGBWStruct UGBWPowerfulToolsFuncLib::UserStructToGBWStruct(const void* StructPtr, const UScriptStruct* ScriptStruct)
{
	FString Name;
	FString STStr;
	if (StructPtr != nullptr && ScriptStruct != nullptr)
	{
		ScriptStruct->GetName(Name);
		ScriptStruct->ExportText(STStr,StructPtr,nullptr,nullptr,PPF_None,nullptr);
	}

	return FGBWStruct(Name,STStr);
}

void UGBWPowerfulToolsFuncLib::NetTypeInServer(EGBWActorNetType NetType, bool& bIsServer)
{
	bIsServer = NetType ==
		EGBWActorNetType::InServer_LocalPlayer
		|| NetType == EGBWActorNetType::InServer_NotPlayer
		|| NetType == EGBWActorNetType::InServer_RemotePlayer;
}

void UGBWPowerfulToolsFuncLib::NetTypeLocalPlayer(EGBWActorNetType NetType, bool& bIsLocalPlayer)
{
	bIsLocalPlayer = NetType ==
		EGBWActorNetType::InServer_LocalPlayer
		|| NetType == EGBWActorNetType::InClient_LocalPlayer;
}

FVector UGBWPowerfulToolsFuncLib::GetBoxHalfSizeFromCharacter(const ACharacter* InCharacter)
{
	FVector Res = FVector();

	if (InCharacter)
	{
		Res.X = InCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		Res.Y = Res.X;
		Res.Z = InCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	return Res;
}

void UGBWPowerfulToolsFuncLib::GetHeadAndFootFromCharacter(const ACharacter* InCharacter,
	FVector& HeadPoint,
	FVector& FootPoint,
	bool& bIsSuccess)
{
	if (!InCharacter)
	{
		bIsSuccess = false;
		return;
	}

	FootPoint = InCharacter->GetCapsuleComponent()->GetComponentLocation();
	FootPoint.Z = FootPoint.Z - InCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	HeadPoint = InCharacter->GetCapsuleComponent()->GetComponentLocation();
	HeadPoint.Z = HeadPoint.Z + InCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	bIsSuccess = true;
}

bool UGBWPowerfulToolsFuncLib::IsActorDisplayingOnScreen(AActor* InActor, FVector2D& ScreenPosition, FVector2D& ScreenCenter, float& DistanceToScreenCenter)
{
	if (!InActor)
	{
		return false;
	}

	const APlayerController* Player = UGameplayStatics::GetPlayerController(InActor, 0);
	const ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
	if (LP && LP->ViewportClient)
	{
		if (FSceneViewProjectionData ProjectionData; LP->GetProjectionData(LP->ViewportClient->Viewport, ProjectionData))
		{
			ScreenCenter = FVector2D((ProjectionData.GetViewRect().Max.X + ProjectionData.GetViewRect().Min.X)/2.0f, (ProjectionData.GetViewRect().Max.Y + ProjectionData.GetViewRect().Min.Y)/2.0f);
			FMatrix const ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
			const bool bResult = FSceneView::ProjectWorldToScreen(InActor->GetActorLocation(), ProjectionData.GetConstrainedViewRect(), ViewProjectionMatrix, ScreenPosition);
			if (bResult
				&& ScreenPosition.X > ProjectionData.GetViewRect().Min.X
				&& ScreenPosition.X < ProjectionData.GetViewRect().Max.X
				&& ScreenPosition.Y > ProjectionData.GetViewRect().Min.Y
				&& ScreenPosition.Y < ProjectionData.GetViewRect().Max.Y)
			{
				DistanceToScreenCenter = (ScreenPosition - FVector2D((ProjectionData.GetViewRect().Max.X + ProjectionData.GetViewRect().Min.X)/2.0f, (ProjectionData.GetViewRect().Max.Y + ProjectionData.GetViewRect().Min.Y)/2.0f)).Length();
				return true;
			}
		}
	}

	return false;
}

bool UGBWPowerfulToolsFuncLib::GetSceneComponentFromActorByTag(
	AActor* InActor,
	TArray<USceneComponent*>& Res,
	const FName ComponentTag,
	const bool bTryGetFromChildOrAttachedActor,
	const FName ChildActorTag)
{
	if (!InActor)
	{
		return false;
	}

	TArray<UActorComponent*> TheRes;
	TheRes.Append(InActor->GetComponentsByTag(USceneComponent::StaticClass(),ComponentTag));

	if (bTryGetFromChildOrAttachedActor)
	{
		TArray<AActor*> ChildActors;
		InActor->GetAllChildActors(ChildActors,true);
		for (const auto A : ChildActors)
		{
			if (A->Tags.Contains(ChildActorTag))
			{
				TheRes.Append(A->GetComponentsByTag(USceneComponent::StaticClass(),ComponentTag));	
			}
		}

		ChildActors.Empty();
		InActor->GetAttachedActors(ChildActors,true);
		for (const auto A : ChildActors)
		{
			if (A->Tags.Contains(ChildActorTag))
			{
				TheRes.Append(A->GetComponentsByTag(USceneComponent::StaticClass(),ComponentTag));	
			}
		}
	}
	
	for (const auto R : TheRes)
	{
		Res.Add(Cast<USceneComponent>(R));
	}

	return !Res.IsEmpty();
}

void UGBWPowerfulToolsFuncLib::GetSkMeshCompFromSelfOrParent(const UAnimInstance* AnimInstance, USkeletalMeshComponent*& Res)
{
	Res = nullptr;
	if (AnimInstance)
	{
		TArray<USceneComponent*> Parents;
		AnimInstance->GetSkelMeshComponent()->GetParentComponents(Parents);

		for (const auto Parent : Parents)
		{
			Res = Cast<USkeletalMeshComponent>(Parent);
			if (Res)
			{
				return;
			}
		}
		
		AActor* TargetActor = AnimInstance->GetOwningActor();
		UActorComponent* ResComp = nullptr;
		while (TargetActor && !ResComp)
		{
			TargetActor = TargetActor->GetParentActor();
			if (TargetActor)
			{
				ResComp = TargetActor->GetComponentByClass(USkeletalMeshComponent::StaticClass());
			}
		}

		TargetActor = AnimInstance->GetOwningActor();
		while (TargetActor && !ResComp)
		{
			TargetActor = TargetActor->GetAttachParentActor();
			if (TargetActor)
			{
				ResComp = TargetActor->GetComponentByClass(USkeletalMeshComponent::StaticClass());
			}
		}

		if (ResComp)
		{
			Res = Cast<USkeletalMeshComponent>(ResComp);
		}
	}
}

void UGBWPowerfulToolsFuncLib::GetAllRelevantActors(
	AActor* InActor,
	TArray<AActor*>& Res,
	const bool bIncludeChildActors,
	const bool bIncludeAttachedChildActors,
	const bool bIncludeParentActors,
	const bool bIncludeAttachedParentActors)
{
	Res.Empty();
	if (InActor)
	{
		Res.Add(InActor);

		if (bIncludeChildActors)
		{
			TArray<AActor*> ChildActors;
			InActor->GetAllChildActors(ChildActors, true);
			Res.Append(ChildActors);
		}

		if (bIncludeAttachedChildActors)
		{
			TArray<AActor*> ChildActors;
			InActor->GetAttachedActors(ChildActors, true, true);
			Res.Append(ChildActors);
		}

		if (bIncludeParentActors)
		{
			AActor* ParentActor = InActor;
			while (ParentActor)
			{
				ParentActor = ParentActor->GetParentActor();
				if (ParentActor)
				{
					Res.Add(ParentActor);
				}
			}
		}

		if (bIncludeAttachedParentActors)
		{
			AActor* ParentActor = InActor;
			while (ParentActor)
			{
				ParentActor = ParentActor->GetAttachParentActor();
				if (ParentActor)
				{
					Res.Add(ParentActor);
				}
			}
		}
	}
}

bool UGBWPowerfulToolsFuncLib::CanAnimRootMotion(const UAnimSequence* InAnim, const float StartTime, const float DeltaTime,
                                                 const bool bAllowLooping)
{
	if (InAnim && DeltaTime != 0.f)
	{
		bool const bPlayingBackwards = (DeltaTime < 0.f);

		float PreviousPosition = StartTime;
		float CurrentPosition = StartTime;
		float DesiredDeltaMove = DeltaTime;

		do
		{
			// Disable looping here. Advance to desired position, or beginning / end of animation 
			const ETypeAdvanceAnim AdvanceType = FAnimationRuntime::AdvanceTime(false, DesiredDeltaMove, CurrentPosition, InAnim->GetPlayLength());

			// Verify position assumptions
			if (!(bPlayingBackwards ? (CurrentPosition <= PreviousPosition) : (CurrentPosition >= PreviousPosition)))
			{
				return false;
			}
			// If we've hit the end of the animation, and we're allowed to loop, keep going.
			if ((AdvanceType == ETAA_Finished) && bAllowLooping)
			{
				const float ActualDeltaMove = (CurrentPosition - PreviousPosition);
				DesiredDeltaMove -= ActualDeltaMove;

				PreviousPosition = bPlayingBackwards ? InAnim->GetPlayLength() : 0.f;
				CurrentPosition = PreviousPosition;
			}
			else
			{
				break;
			}
		} while (true);
	}
	else
	{
		return false;
	}

	return true;
}
