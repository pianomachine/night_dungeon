// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "RMS/GBWRMSBPFuncLib.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

int32 UGBWRMSBPFuncLib::GBWRMS_Common(
	FName& InstanceName,
	TScriptInterface<IGBWRMSInterface> RMSInterfaceObj,
	UCharacterMovementComponent* MovementComponent,
	bool ClearOtherRMS)
{
	InstanceName = GBWRMS_GetRMSCommonInstanceName();
	if (!MovementComponent)
	{
		return -1;
	}
	constexpr int32 NewPriority = 0;
	if (ClearOtherRMS)
	{
		MovementComponent->CurrentRootMotion.Clear();
	}
	else if (auto RMS = MovementComponent->GetRootMotionSource(InstanceName))
	{
		MovementComponent->RemoveRootMotionSource(InstanceName);
	}

	TSharedPtr<FGBWRMS_Common> RmsCommon = MakeShared<FGBWRMS_Common>();
	RmsCommon->Duration = 100000000.0f;
	RmsCommon->InstanceName = InstanceName;
	RmsCommon->AccumulateMode = ERootMotionAccumulateMode::Override;
	RmsCommon->Settings.SetFlag(ERootMotionSourceSettingsFlags::UseSensitiveLiftoffCheck);
	RmsCommon->Priority = NewPriority;
	
	RmsCommon->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::ClampVelocity;
	RmsCommon->FinishVelocityParams.SetVelocity = FVector::Zero();
	RmsCommon->FinishVelocityParams.ClampVelocity = 650.0f;
	
	RmsCommon->SetTime(0);
	RmsCommon->RMSInterfaceObj = RMSInterfaceObj;

	return MovementComponent->ApplyRootMotionSource(RmsCommon);
}

bool UGBWRMSBPFuncLib::GBWRMS_StopRMSCommon(UCharacterMovementComponent* MovementComponent)
{
	if (MovementComponent)
	{
		if (auto RMS = MovementComponent->GetRootMotionSource(GBWRMS_GetRMSCommonInstanceName()))
		{
			MovementComponent->RemoveRootMotionSource(GBWRMS_GetRMSCommonInstanceName());
		}
		return true;
	}
	return false;
}

FName UGBWRMSBPFuncLib::GBWRMS_GetRMSCommonInstanceName()
{
	return FName("GBWRMSCommon");
}

bool UGBWRMSBPFuncLib::GBWRMS_IsRMSCommonActive(UCharacterMovementComponent* MovementComponent)
{
	if (MovementComponent)
	{
		if (auto RMS = MovementComponent->GetRootMotionSource(GBWRMS_GetRMSCommonInstanceName()))
		{
			return true;
		}
	}
	return false;
}

bool UGBWRMSBPFuncLib::GBWRMS_GetCharacterViewInfo(
	const ACharacter* Character,
	FVector& ViewLocation,
	FRotator& ViewRotation,
	FVector& ForwardVector)
{
	if (Character)
	{
		if (UGameplayStatics::GetPlayerPawn(Character->GetWorld(),0) == Character)
		{
			const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(Character->GetWorld(),0);
			ViewLocation = CameraManager->GetCameraLocation();
			ViewRotation = CameraManager->GetCameraRotation();
			ForwardVector = CameraManager->GetActorForwardVector();
		}
		else
		{
			Character->GetActorEyesViewPoint(ViewLocation,ViewRotation);
			ForwardVector = Character->GetActorForwardVector();
		}
		
		return true;
	}

	return false;
}