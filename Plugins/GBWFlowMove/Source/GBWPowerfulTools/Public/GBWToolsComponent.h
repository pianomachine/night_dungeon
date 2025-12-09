// Copyright GanBowen 2022-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GBWToolsComponent.generated.h"

UENUM(BlueprintType)
enum class EGBWActorNetType : uint8
{
	None,
	InClient_NotPlayer,
	InClient_RemotePlayer,
	InClient_LocalPlayer,
	InServer_NotPlayer,
	InServer_RemotePlayer,
	InServer_LocalPlayer
};

USTRUCT(BlueprintType, DisplayName="GBWStruct")
struct FGBWStruct
{
	GENERATED_BODY()

	UPROPERTY()
	FString StructType = FString("");
	UPROPERTY()
	FString StructStr = FString("");

	FGBWStruct(){}
	FGBWStruct(FString InStructTypeStr, FString InStructStr)
	{
		StructType = InStructTypeStr;
		StructStr = InStructStr;
	}

	bool IsValidStruct() const
	{
		return !StructType.IsEmpty() || !StructStr.IsEmpty();
	}

	bool Equals(const FGBWStruct& other) const
	{
		return StructType == other.StructType && StructStr == other.StructStr;
	}
};

USTRUCT(BlueprintType, DisplayName="SpawnRules")
struct FGBWStructItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FString Key = FString("");
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	FGBWStruct Struct = FGBWStruct();

	FGBWStructItem(){}
	FGBWStructItem(FString InKey, const FGBWStruct& InStruct)
	{
		Key = InKey;
		Struct = InStruct;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GBWPOWERFULTOOLS_API UGBWToolsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGBWToolsComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(Replicated)
	TArray<FGBWStructItem> StructItems;
	
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void AddProperty(
		const FString& Key,
		const FGBWStruct& Value);
	UFUNCTION(Reliable, Server, Category = "GBW|Effect")
	void AddProperty_Server(
		const FString& Key,
		const FGBWStruct& Value);
	void AddProperty_Imp(
		const FString& Key,
		const FGBWStruct& Value);

	bool GetProperty(const FString& Key, FGBWStruct& Value);
};
