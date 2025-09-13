#include "Factory/Graph/NerveQuestGraphNodeMapper.h"
#include "Objects/Nodes/Objective/NerveSequenceRuntimeObjective.h"
#include "UnrealED/Node/NerveQuestSequenceObjective.h"

TSubclassOf<UNerveQuestObjectiveNodeBase> NerveQuestGraphNodeMapper::CreateNewGraphNodeForRuntimeInstance
(const UClass* NewObjectiveInstance)
{
	if (!IsValid(NewObjectiveInstance)) return nullptr;

	// Add more custom check for graph nodes
	if (NewObjectiveInstance == UNerveSequenceRuntimeObjective::StaticClass())
	{
		return UNerveQuestSequenceObjective::StaticClass();
	}
	
	return UNerveQuestObjectiveNodeBase::StaticClass();
}
