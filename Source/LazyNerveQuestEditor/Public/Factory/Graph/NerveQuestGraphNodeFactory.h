#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"

struct LAZYNERVEQUESTEDITOR_API FNerveQuestGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
	FNerveQuestGraphNodeFactory();
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};
