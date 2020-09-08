// Copyright Recursoft LLC 2019-2020. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "Nodes/SMNode_Base.h"
#include "SMGraph.generated.h"


class USMGraphK2Node_StateMachineNode;
class USMGraphNode_StateMachineEntryNode;
class USMGraphNode_StateMachineStateNode;

UCLASS()
class SMSYSTEMEDITOR_API USMGraph : public UEdGraph
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UEdGraph Interface
	bool Modify(bool bAlwaysMarkDirty = true) override;
	void PostEditUndo() override;
	void NotifyGraphChanged() override;
	void NotifyGraphChanged(const FEdGraphEditAction& Action) override;
	//~ End UEdGraph Interface

	USMGraphNode_StateMachineEntryNode* GetEntryNode() const;

	/** When referencing from top level or state graph. */
	USMGraphK2Node_StateMachineNode* GetOwningStateMachineK2Node() const;

	/** When referencing from a nested definition. */
	USMGraphNode_StateMachineStateNode* GetOwningStateMachineNodeWhenNested() const;

	/** Checks the graph node owning this graph and returns the runtime state. */
	virtual FSMNode_Base* GetRuntimeNode() const;

	/** Checks if the entry node is connected to any state. */
	bool HasAnyLogicConnections() const;

	const FGuid& GetOrGenerateTemporaryContainerGuid();

	// Set by compiler -- stored on consolidated graph.
	UPROPERTY(Transient)
	class USMGraphK2Node_StateMachineEntryNode* GeneratedContainerNode;
	
	// Entry node within the state machine
	UPROPERTY()
	class USMGraphNode_StateMachineEntryNode* EntryNode;

protected:
	/** Generated during compile to help with entry point creation. */
	UPROPERTY(Transient)
	FGuid TemporaryContainerGuid;

};