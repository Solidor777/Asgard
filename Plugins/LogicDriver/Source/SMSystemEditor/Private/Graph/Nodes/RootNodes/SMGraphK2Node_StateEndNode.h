// Copyright Recursoft LLC 2019-2020. All Rights Reserved.
#pragma once

#include "SMGraphK2Node_RuntimeNodeContainer.h"
#include "SMGraphK2Node_StateEndNode.generated.h"


UCLASS(MinimalAPI)
class USMGraphK2Node_StateEndNode : public USMGraphK2Node_RuntimeNodeReference
{
	GENERATED_UCLASS_BODY()

	//~ Begin UEdGraphNode Interface
	void AllocateDefaultPins() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
	bool IsCompatibleWithGraph(UEdGraph const* Graph) const override;
	//~ End UEdGraphNode Interface
};