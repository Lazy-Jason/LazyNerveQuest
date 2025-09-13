#pragma once
#include "UnrealED/Node/NerveQuestObjectiveNodeBase.h"

class LAZYNERVEQUESTEDITOR_API LazyNerveValidator
{
public:
	/** */
	static bool ValidateEditorGraphPin(const UEdGraphPin* GraphPin);
	static bool ValidateEditorOptionalGraphPin(const UEdGraphPin* GraphPin, const UNerveQuestObjectiveNodeBase* GraphNode);
};
