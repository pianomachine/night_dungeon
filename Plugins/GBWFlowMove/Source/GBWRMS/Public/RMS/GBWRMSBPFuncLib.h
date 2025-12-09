// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RMS/GBWRMS.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GBWRMSBPFuncLib.generated.h"

/**
 * 
 */
UCLASS()
class GBWRMS_API UGBWRMSBPFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="GBW|RMS")
	static int32 GBWRMS_Common(
		FName& InstanceName,
		TScriptInterface<IGBWRMSInterface> RMSInterfaceObj,
		UCharacterMovementComponent* MovementComponent,
		bool ClearOtherRMS = true);
	
	UFUNCTION(BlueprintCallable, Category="GBW|RMS")
	static bool GBWRMS_StopRMSCommon(UCharacterMovementComponent* MovementComponent);

	UFUNCTION(BlueprintCallable, Category="GBW|RMS")
	static FName GBWRMS_GetRMSCommonInstanceName();
	
	UFUNCTION(BlueprintCallable, Category="GBW|RMS")
	static bool GBWRMS_IsRMSCommonActive(UCharacterMovementComponent* MovementComponent);
	
	UFUNCTION(BlueprintCallable, Category="GBW|RMS")
	static bool GBWRMS_GetCharacterViewInfo(
		const ACharacter* Character,
		FVector& ViewLocation,
		FRotator& ViewRotation,
		FVector& ForwardVector);
};
