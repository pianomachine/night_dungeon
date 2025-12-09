// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GBWToolsComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/AnimSequence.h"
#include "GBWPowerfulToolsFuncLib.generated.h"

UCLASS()
class GBWPOWERFULTOOLS_API UGBWPowerfulToolsFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	//Math - Start
	UFUNCTION(BlueprintPure, Category = "GBW|Tools|Math")
	static void Boxplot_Float(
		const TArray<float>& InDataArr,
		TArray<float>& OutDataArr);

	UFUNCTION(BlueprintPure, Category = "GBW|Tools|Math")
	static float GetAngle(const FVector& From, const FVector& To, const FVector& PlaneNormal);
	UFUNCTION(BlueprintPure, Category = "GBW|Tools|Math")
	static float GetAngleInHorizontalPlane(const FVector& From, const FVector& To);
	UFUNCTION(BlueprintPure, Category = "GBW|Tools|Math")
	static float GetAngleInVerticalPlane(const FVector& From, const FVector& To);

	UFUNCTION(BlueprintPure, Category = "GBW|Tools|Math")
	static float GetAlphaByBlendOption(EAlphaBlendOption BlendOption, const float InAlpha);
	//Math - End
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Tools", meta=(DefaultToSelf="InActor"))
	static UGBWToolsComponent* GetGBWtToolsComponent(AActor* InActor);
	
	UFUNCTION(BlueprintCallable, Category = "GBW|Tools",
			CustomThunk,
			meta = (DefaultToSelf="InActor", CustomStructureParam = "StructProperty"))
	static void SetGBWProperty(
		AActor* InActor,
		FString Key,
		const int32& StructProperty){}
	DECLARE_FUNCTION(execSetGBWProperty);

	UFUNCTION(BlueprintCallable, 
		CustomThunk,
		meta = (DefaultToSelf="InActor", CustomStructureParam = "PropertyStruct"),
		Category = "GBW|Tools")
	static bool GetGBWPropertyAsStruct(AActor* InActor, FString Key, const int32& PropertyStruct) { return false; }
	DECLARE_FUNCTION(execGetGBWPropertyAsStruct);


	UFUNCTION(BlueprintCallable, Category = "GBW|Tools", meta=(DefaultToSelf="InActor"))
	static void GetPlayerCamera(
		AActor* InActor,
		bool& bIsGet,
		UCameraComponent*& Camera,
		USpringArmComponent*& SpringArm);

	UFUNCTION(BlueprintCallable, Category = "GBW|Tools", meta=(DefaultToSelf="InActor"))
	static void GetPlayerController(
		AActor* InActor,
		bool& bIsGet,
		bool& bIsLocalController,
		APlayerController*& PlayerController);

	UFUNCTION(BlueprintCallable, Category = "GBW|Tools", meta=(DefaultToSelf="InActor"))
	static EGBWActorNetType GetActorNetType(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "GBW|Tools", meta=(DefaultToSelf="InObject"))
	static void GetAllPropertiesStr(UObject* InObject, TArray<FString>& PropertyNames, TArray<FString>& PropertyValues);

	UFUNCTION(BlueprintCallable, 
		CustomThunk,
		meta = (CustomStructureParam = "InStruct", ExpandBoolAsExecs = "bIsSuccess"),
		Category = "GBW|Tools")
	static void ToUserStruct(bool& bIsSuccess, FGBWStruct GBWStruct, const int32& InStruct) {}
	DECLARE_FUNCTION(execToUserStruct);

	UFUNCTION(BlueprintPure,
		CustomThunk,
		meta = (CustomStructureParam = "InStruct"),
		Category = "GBW|Tools")
	static FGBWStruct ToGBWStruct(const int32& InStruct) { return FGBWStruct(); }
	DECLARE_FUNCTION(execToGBWStruct);
	
	static bool GBWStructToUserStruct(const FGBWStruct& InGBWStruct, void* StructPtr, UScriptStruct* ScriptStruct);
	static FGBWStruct UserStructToGBWStruct(const void* StructPtr, const UScriptStruct* ScriptStruct);

	UFUNCTION(BlueprintCallable, 
		meta = (ExpandBoolAsExecs = "bIsServer"),
		Category = "GBW|Tools")
	static void NetTypeInServer(EGBWActorNetType NetType, bool& bIsServer);
	UFUNCTION(BlueprintCallable, 
		meta = (ExpandBoolAsExecs = "bIsLocalPlayer"),
		Category = "GBW|Tools")
	static void NetTypeLocalPlayer(EGBWActorNetType NetType, bool& bIsLocalPlayer);

	UFUNCTION(BlueprintPure,
		Category = "GBW|Tools")
	static FVector GetBoxHalfSizeFromCharacter(const ACharacter* InCharacter);

	UFUNCTION(BlueprintCallable,
		meta = (ExpandBoolAsExecs = "bIsSuccess"),
		Category = "GBW|Tools")
	static void GetHeadAndFootFromCharacter(const ACharacter* InCharacter, FVector& HeadPoint, FVector& FootPoint, bool& bIsSuccess);

	UFUNCTION(BlueprintCallable, Category = "GBW|Tools", meta=(DefaultToSelf="InActor"))
	static bool IsActorDisplayingOnScreen(AActor* InActor, FVector2D& ScreenPosition, FVector2D& ScreenCenter, float& DistanceToScreenCenter);

	UFUNCTION(BlueprintCallable, Category = "GBW|Tools", meta=(DefaultToSelf="InActor"))
	static bool GetSceneComponentFromActorByTag(AActor* InActor, TArray<USceneComponent*>& Res, FName ComponentTag, bool bTryGetFromChildOrAttachedActor, FName ChildActorTag);

	UFUNCTION(BlueprintPure, Category = "GBW|FlowMove|Pure|Input", meta=(DefaultToSelf="AnimInstance", BlueprintThreadSafe))
	static void GetSkMeshCompFromSelfOrParent(const UAnimInstance* AnimInstance, USkeletalMeshComponent*& Res);

	UFUNCTION(BlueprintCallable, Category = "GBW|Tools", meta=(DefaultToSelf="InActor"))
	static void GetAllRelevantActors(AActor* InActor,
		TArray<AActor*>& Res,
		bool bIncludeChildActors = true,
		bool bIncludeAttachedChildActors = true,
		bool bIncludeParentActors = true,
		bool bIncludeAttachedParentActors = true
		);

	static bool CanAnimRootMotion(const UAnimSequence* InAnim, float StartTime, float DeltaTime, bool bAllowLooping);
};
