// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "StateTree/GBWPS_STSchema.h"
#include "StateTreeConditionBase.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeTaskBase.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"


UGBWPS_STSchema::UGBWPS_STSchema()
	: ContextActorClass(AActor::StaticClass())
	, ContextActorDataDesc(FName("Actor"), AActor::StaticClass(), FGuid(0x1D971B01, 0x28884FDE, 0xB5436802, 0x36984FD5))
{
}

bool UGBWPS_STSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	return InScriptStruct->IsChildOf(FStateTreeConditionCommonBase::StaticStruct())
	|| InScriptStruct->IsChildOf(FStateTreeEvaluatorCommonBase::StaticStruct())
	|| InScriptStruct->IsChildOf(FStateTreeTaskCommonBase::StaticStruct());
}

bool UGBWPS_STSchema::IsClassAllowed(const UClass* InClass) const
{
	return IsChildOfBlueprintBase(InClass);
}

bool UGBWPS_STSchema::IsExternalItemAllowed(const UStruct& InStruct) const
{
	return InStruct.IsChildOf(AActor::StaticClass())
			|| InStruct.IsChildOf(UActorComponent::StaticClass())
			|| InStruct.IsChildOf(UWorldSubsystem::StaticClass());
}

TConstArrayView<FStateTreeExternalDataDesc> UGBWPS_STSchema::GetContextDataDescs() const
{
	return MakeArrayView(&ContextActorDataDesc, 1);
}

void UGBWPS_STSchema::PostLoad()
{
	Super::PostLoad();
	ContextActorDataDesc.Struct = ContextActorClass.Get();
}

#if WITH_EDITOR
void UGBWPS_STSchema::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	FProperty* Property = PropertyChangedEvent.Property;

	if (Property)
	{
		if (Property->GetOwnerClass() == UGBWPS_STSchema::StaticClass()
			&& Property->GetFName() == GET_MEMBER_NAME_CHECKED(UGBWPS_STSchema, ContextActorClass))
		{
			ContextActorDataDesc.Struct = ContextActorClass.Get();
		}
	}
}
#endif