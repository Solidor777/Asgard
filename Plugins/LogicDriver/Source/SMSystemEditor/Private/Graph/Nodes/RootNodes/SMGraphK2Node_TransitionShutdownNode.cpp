// Copyright Recursoft LLC 2019-2020. All Rights Reserved.
#include "SMGraphK2Node_TransitionShutdownNode.h"
#include "EdGraph/EdGraph.h"
#include "Graph/Schema/SMGraphK2Schema.h"
#include "Graph/SMTransitionGraph.h"
#include "Graph/SMConduitGraph.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "SMBlueprintEditorUtils.h"
#include "SMBlueprint.h"


#define LOCTEXT_NAMESPACE "SMTransitionShutdownNode"

USMGraphK2Node_TransitionShutdownNode::USMGraphK2Node_TransitionShutdownNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAllowMoreThanOneNode = true;
}

void USMGraphK2Node_TransitionShutdownNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, USMGraphK2Schema::PC_Exec, UEdGraphSchema_K2::PN_Then);
}

void USMGraphK2Node_TransitionShutdownNode::PostPlacedNewNode()
{
	RuntimeNodeGuid = GetRuntimeContainer()->GetRunTimeNodeChecked()->GetNodeGuid();
}

FText USMGraphK2Node_TransitionShutdownNode::GetMenuCategory() const
{
	return FText::FromString(STATE_MACHINE_HELPER_CATEGORY);
}

FText USMGraphK2Node_TransitionShutdownNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const bool IsConduit = Cast<USMConduitGraph>(FSMBlueprintEditorUtils::FindTopLevelOwningGraph(GetGraph())) != nullptr;

	if ((TitleType == ENodeTitleType::MenuTitle || TitleType == ENodeTitleType::ListView))
	{
		return IsConduit ? LOCTEXT("AddConduitShutdownEvent", "Add Event On Conduit Shutdown") : LOCTEXT("AddTransitionShutdownEvent", "Add Event On Transition Shutdown");
	}

	return FText::FromString(IsConduit ? TEXT("On Conduit Shutdown") : TEXT("On Transition Shutdown"));
}

FText USMGraphK2Node_TransitionShutdownNode::GetTooltipText() const
{
	return LOCTEXT("TransitionShutdownNodeTooltip", "Called once the state leading to this node has exited.");
}

void USMGraphK2Node_TransitionShutdownNode::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

bool USMGraphK2Node_TransitionShutdownNode::IsActionFilteredOut(FBlueprintActionFilter const& Filter)
{
	for (UBlueprint* Blueprint : Filter.Context.Blueprints)
	{
		if (!Cast<USMBlueprint>(Blueprint))
		{
			return true;
		}
	}

	for (UEdGraph* Graph : Filter.Context.Graphs)
	{
		// Only works on transition and conduit graphs.
		if (!Graph->IsA<USMTransitionGraph>() && !Graph->IsA<USMConduitGraph>())
		{
			return true;
		}
	}

	return false;
}

bool USMGraphK2Node_TransitionShutdownNode::IsCompatibleWithGraph(UEdGraph const* Graph) const
{
	return Graph->IsA<USMTransitionGraph>() || Graph->IsA<USMConduitGraph>();
}

#undef LOCTEXT_NAMESPACE
