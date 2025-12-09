// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GBWPowerfulToolsFuncLib.h"
#include "GameFramework/Actor.h"
#include "GBWPerceptionBase.generated.h"

class UGBWPowerfulPerceptionComponent;

UCLASS(HideDropdown)
class GBWPOWERFULPERCEPTION_API AGBWPerceptionBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGBWPerceptionBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|Perception")
	FGuid PerceptionGuid = FGuid();
	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|Perception")
	UGBWPowerfulPerceptionComponent* PPComponent = nullptr;
	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|Perception")
	int PerceptionKeyIndex = -1;
	UPROPERTY(BlueprintReadOnly, Category="GBW|Perception")
	EGBWActorNetType OwnerActorNetType = EGBWActorNetType::None;
	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|Perception")
	AActor* OwnerActor = nullptr;
	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|Perception")
	ACharacter* OwnerCharacter = nullptr;
	UPROPERTY(BlueprintReadOnly, Replicated, Category="GBW|Perception")
	FGBWStruct PerceptionParamsStr = FGBWStruct();

	bool bIsStarted = false;

	EGBWActorNetType GetOwnerActorNetType();

	UFUNCTION(BlueprintCallable,Category = "GBW|Perception")
	FGameplayTag GetPerceptionKey() const;
	UFUNCTION(BlueprintCallable,Category = "GBW|Perception")
	void EndPerception();
	UFUNCTION(BlueprintPure,Category = "GBW|Perception")
	AActor* GetOwnerActor() const;

	void DestroyPerception();

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|Perception")
	void OnPerceptionStart(EGBWActorNetType OwnerNetType);
	virtual void OnPerceptionStart_Implementation(EGBWActorNetType OwnerNetType){}
	void PerceptionStart(EGBWActorNetType OwnerNetType)
	{
		if (!bIsStarted)
		{
			OnPerceptionStart(OwnerNetType);
			bIsStarted = true;
		}
	}

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|Perception")
	void OnPerceptionUpdate(EGBWActorNetType OwnerNetType, float DeltaTime);
	virtual void OnPerceptionUpdate_Implementation(EGBWActorNetType OwnerNetType, float DeltaTime){}
	void PerceptionUpdate(EGBWActorNetType OwnerNetType, float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|Perception")
	void OnPerceptionEnd(EGBWActorNetType OwnerNetType);
	virtual void OnPerceptionEnd_Implementation(EGBWActorNetType OwnerNetType){}
	void PerceptionEnd();
	UFUNCTION(Reliable, Server, Category = "GBW|Perception")
	void PerceptionEnd_Server();
	UFUNCTION(Reliable, NetMulticast, Category = "GBW|Perception")
	void PerceptionEnd_Multicast();
	void PerceptionEnd_Imp();

	UFUNCTION(BlueprintNativeEvent, Category = "GBW|Perception")
	void OnPerceptionEvent(FName Event);
	virtual void OnPerceptionEvent_Implementation(FName Event){}
	void PerceptionEvent(FName Event);
	UFUNCTION(Reliable, Server, Category = "GBW|Perception")
	void PerceptionEvent_Server(FName Event);
	UFUNCTION(Reliable, NetMulticast, Category = "GBW|Perception")
	void PerceptionEvent_Multicast(FName Event);
	void PerceptionEvent_Imp(FName Event);

	UFUNCTION(BlueprintCallable,
		CustomThunk,
		meta = (CustomStructureParam = "PerceptionParameters", ExpandBoolAsExecs = "bIsSuccess"),
		Category = "GBW|Perception")
	void InitPerceptionParameters(bool& bIsSuccess, const int32& PerceptionParameters) {}
	DECLARE_FUNCTION(execInitPerceptionParameters);

	UFUNCTION(BlueprintCallable,
		meta = (ExpandBoolAsExecs = "bIsSuccess"),
		Category = "GBW|Perception")
	void EnableInputPerception(bool& bIsSuccess);

	UFUNCTION(BlueprintCallable,
		CustomThunk,
		meta = (CustomStructureParam = "CognitionStruct"),
		Category = "GBW|Perception")
	void UpdateCognition(float EffectiveTime, bool bIsReplicated, bool bIsEffectivePerception, const int32& CognitionStruct) {}
	DECLARE_FUNCTION(execUpdateCognition);

	UFUNCTION(BlueprintCallable,
		CustomThunk,
		meta = (CustomStructureParam = "CognitionStruct"),
		Category = "GBW|Perception")
	void EffectivePerception(bool bIsReplicated, const int32& CognitionStruct) {}
	DECLARE_FUNCTION(execEffectivePerception);

	UFUNCTION(BlueprintCallable, Category="GBW|Perception")
	void ClearCognition();

	UFUNCTION(BlueprintCallable, Category="GBW|Perception", meta=(ExpandBoolAsExecs = "bIsSuccess"))
	void GetOwnerCharacter(bool& bIsSuccess, ACharacter*& Character);

	UFUNCTION(BlueprintCallable, Category="GBW|Perception", meta=(ExpandBoolAsExecs = "bIsSuccess"))
	void GetOwnerPlayerController(bool& bIsSuccess, APlayerController*& PlayerController);
};
