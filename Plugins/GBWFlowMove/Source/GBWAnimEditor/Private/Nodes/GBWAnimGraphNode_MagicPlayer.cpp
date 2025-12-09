// Copyright GanBowen 2022-2024. All Rights Reserved.


#include "Nodes/GBWAnimGraphNode_MagicPlayer.h"

#include "AnimGraphCommands.h"
#include "EditorCategoryUtils.h"
#include "IAnimBlueprintNodeOverrideAssetsContext.h"
#include "Animation/AnimComposite.h"
//#include "Animation/AnimPoseSearchProvider.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimRootMotionProvider.h"
#include "UObject/UE5MainStreamObjectVersion.h"

#define LOCTEXT_NAMESPACE "UGBWAnimGraphNode_MagicPlayer"

/////////////////////////////////////////////////////
// UGBWAnimGraphNode_MagicPlayer

UGBWAnimGraphNode_MagicPlayer::UGBWAnimGraphNode_MagicPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), Node()
{
}

void UGBWAnimGraphNode_MagicPlayer::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FUE5MainStreamObjectVersion::GUID);

	if(Ar.IsLoading() && Ar.CustomVer(FUE5MainStreamObjectVersion::GUID) < FUE5MainStreamObjectVersion::AnimNodeConstantDataRefactorPhase0)
	{
		/*Node.PlayRateScaleBiasClampConstants.CopyFromLegacy(Node.PlayRateScaleBiasClamp_DEPRECATED);*/
	}
}

void UGBWAnimGraphNode_MagicPlayer::PreloadRequiredAssets()
{
	PreloadObject(Node.GetAnimAsset());

	Super::PreloadRequiredAssets();
}

FLinearColor UGBWAnimGraphNode_MagicPlayer::GetNodeTitleColor() const
{
	return FColor::Black;
}

FSlateIcon UGBWAnimGraphNode_MagicPlayer::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon("EditorStyle", "ClassIcon.AnimSequence");
}

FText UGBWAnimGraphNode_MagicPlayer::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("GBW Magic Player");
}

FText UGBWAnimGraphNode_MagicPlayer::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Animation);
}

void UGBWAnimGraphNode_MagicPlayer::GetMenuActions(FBlueprintActionDatabaseRegistrar& InActionRegistrar) const
{
	GetMenuActionsHelper(
		InActionRegistrar,
		GetClass(),
		{ UAnimSequence::StaticClass(), UBlendSpace::StaticClass(), UAnimMontage::StaticClass(), UAnimComposite::StaticClass() },
		{ },
		[](const FAssetData& InAssetData, UClass* InClass)
		{
			if(InAssetData.IsValid())
			{
				const FString TagValue = InAssetData.AssetName.ToString();
				if(const bool bKnownToBeAdditive = (!TagValue.IsEmpty() && !TagValue.Equals(TEXT("AAT_None"))))
				{
					return FText::Format(LOCTEXT("MenuDescFormat_PlayAdditive", "Play '{0}' (additive)"), FText::FromName(InAssetData.AssetName));
				}
				else
				{
					return FText::Format(LOCTEXT("MenuDescFormat_Play1", "Play '{0}'"), FText::FromName(InAssetData.AssetName));
				}
			}
			else
			{
				return LOCTEXT("PlayerDesc", "GBW Magic Player");
			}
		},
		[](const FAssetData& InAssetData, UClass* InClass)
		{
			if(InAssetData.IsValid())
			{
				const FString TagValue = InAssetData.AssetName.ToString();
				if(const bool bKnownToBeAdditive = (!TagValue.IsEmpty() && !TagValue.Equals(TEXT("AAT_None"))))
				{
					return FText::Format(LOCTEXT("MenuDescTooltipFormat_PlayAdded", "Play (additive)\n'{0}'"), FText::FromName(InAssetData.GetSoftObjectPath().GetAssetFName()));
				}
				else
				{
					return FText::Format(LOCTEXT("MenuDescTooltipFormat_Play", "Play\n'{0}'"), FText::FromName(InAssetData.GetSoftObjectPath().GetAssetFName()));
				}
			}
			else
			{
				return LOCTEXT("PlayerDescTooltip", "GBW Magic Player");
			}
		},
		[](UEdGraphNode* InNewNode, bool bInIsTemplateNode, const FAssetData& InAssetData)
		{
			UAnimGraphNode_AssetPlayerBase::SetupNewNode(InNewNode, bInIsTemplateNode, InAssetData);
		});	
}

EAnimAssetHandlerType UGBWAnimGraphNode_MagicPlayer::SupportsAssetClass(const UClass* AssetClass) const
{
	if (AssetClass->IsChildOf(UAnimationAsset::StaticClass()))
	{
		return EAnimAssetHandlerType::PrimaryHandler;
	}
	else
	{
		return EAnimAssetHandlerType::NotSupported;
	}
}

void UGBWAnimGraphNode_MagicPlayer::GetOutputLinkAttributes(FNodeAttributeArray& OutAttributes) const
{
	Super::GetOutputLinkAttributes(OutAttributes);

	if (UE::Anim::IAnimRootMotionProvider::Get())
	{
		OutAttributes.Add(UE::Anim::IAnimRootMotionProvider::AttributeName);
	}
}

void UGBWAnimGraphNode_MagicPlayer::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);

	/*ValidateAnimNodeDuringCompilationHelper(ForSkeleton, MessageLog, Node.GetAnimAsset(),
		UAnimationAsset::StaticClass(),
		FindPin(GET_MEMBER_NAME_STRING_CHECKED(FGBWAnimNode_MagicPlayer, AnimAsset)),
		GET_MEMBER_NAME_CHECKED(FGBWAnimNode_MagicPlayer, AnimAsset));*/
}

void UGBWAnimGraphNode_MagicPlayer::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (!Context->bIsDebugging)
	{
		// add an option to convert to single frame
		{
			FToolMenuSection& Section = Menu->AddSection("AnimGraphNodeMagicPlayer", LOCTEXT("MagicPlayerHeading", "GBW Magic Player"));
			Section.AddMenuEntry(FAnimGraphCommands::Get().OpenRelatedAsset);
			Section.AddMenuEntry(FAnimGraphCommands::Get().ConvertToSeqEvaluator);
		}
	}
}

void UGBWAnimGraphNode_MagicPlayer::SetAnimationAsset(UAnimationAsset* Asset)
{
	Node.SetAnimAsset(Asset);
}

void UGBWAnimGraphNode_MagicPlayer::OnOverrideAssets(IAnimBlueprintNodeOverrideAssetsContext& InContext) const
{
	if(InContext.GetAssets().Num() > 0)
	{
		if (UAnimationAsset* Asset = Cast<UAnimationAsset>(InContext.GetAssets()[0]))
		{
			FGBWAnimNode_MagicPlayer& AnimNode = InContext.GetAnimNode<FGBWAnimNode_MagicPlayer>();
			AnimNode.SetAnimAsset(Asset);
		}
	}
}

void UGBWAnimGraphNode_MagicPlayer::BakeDataDuringCompilation(class FCompilerResultsLog& MessageLog)
{
	UAnimBlueprint* AnimBlueprint = GetAnimBlueprint();
	AnimBlueprint->FindOrAddGroup(Node.GetGroupName());
}

void UGBWAnimGraphNode_MagicPlayer::GetAllAnimationSequencesReferred(TArray<UAnimationAsset*>& AnimationAssets) const
{
	if(Node.GetAnimAsset())
	{
		HandleAnimReferenceCollection(Node.GetAnimParam(false).AnimAsset, AnimationAssets);
	}
}

void UGBWAnimGraphNode_MagicPlayer::ReplaceReferredAnimations(const TMap<UAnimationAsset*, UAnimationAsset*>& AnimAssetReplacementMap)
{
	HandleAnimReferenceReplacement(Node.AnimationAssetNow, AnimAssetReplacementMap);
}

bool UGBWAnimGraphNode_MagicPlayer::DoesSupportTimeForTransitionGetter() const
{
	return true;
}

UAnimationAsset* UGBWAnimGraphNode_MagicPlayer::GetAnimationAsset() const 
{
	return Node.GetAnimAsset();
}

const TCHAR* UGBWAnimGraphNode_MagicPlayer::GetTimePropertyName() const 
{
	return TEXT("InternalTimeAccumulator");
}

UScriptStruct* UGBWAnimGraphNode_MagicPlayer::GetTimePropertyStruct() const 
{
	return FGBWAnimNode_MagicPlayer::StaticStruct();
}

void UGBWAnimGraphNode_MagicPlayer::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	Super::CustomizeDetails(DetailBuilder);

	/*if (!UE::Anim::IPoseSearchProvider::IsAvailable())
	{
		DetailBuilder.HideCategory(TEXT("PoseMatching"));
	}*/
}

void UGBWAnimGraphNode_MagicPlayer::CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const
{
	Super::CustomizePinData(Pin, SourcePropertyName, ArrayIndex);
}

void UGBWAnimGraphNode_MagicPlayer::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	/*const FName PropertyName = (PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None);

	// Reconstruct node to show updates to PinFriendlyNames.
	if ((PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FGBWAnimNode_MagicPlayer, PlayRateBasis))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, bMapRange))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputRange, Min))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputRange, Max))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, Scale))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, Bias))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, bClampResult))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, ClampMin))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, ClampMax))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, bInterpResult))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, InterpSpeedIncreasing))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClampConstants, InterpSpeedDecreasing)))
	{
		ReconstructNode();
	}*/

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#undef LOCTEXT_NAMESPACE