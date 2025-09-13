#include "Factory/Graph/NerveQuestGraphNodeFactory.h"
#include "UnrealED/Node/NerveQuestObjectiveNodeBase.h"
#include "UnrealED/Slate/NerveQuestObjectiveBase.h"

FNerveQuestGraphNodeFactory::FNerveQuestGraphNodeFactory()
{}

TSharedPtr<SGraphNode> FNerveQuestGraphNodeFactory::CreateNode(UEdGraphNode* Node) const
{
	if (UNerveQuestObjectiveNodeBase* ObjectiveNode = Cast<UNerveQuestObjectiveNodeBase>(Node))
	{
		return SNew(SNerveQuestObjectiveBase, ObjectiveNode);
	}
    
	// Fall back to default node widget for other node types
	return FGraphPanelNodeFactory::CreateNode(Node);
}
