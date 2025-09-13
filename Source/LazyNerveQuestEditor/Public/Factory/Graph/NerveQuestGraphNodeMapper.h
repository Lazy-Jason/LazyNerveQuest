#pragma once

class UNerveQuestObjectiveNodeBase;
class UNerveQuestRuntimeObjectiveBase;

class LAZYNERVEQUESTEDITOR_API NerveQuestGraphNodeMapper
{
public:
	static TSubclassOf<UNerveQuestObjectiveNodeBase> CreateNewGraphNodeForRuntimeInstance
	(const UClass* NewObjectiveInstance);
};
