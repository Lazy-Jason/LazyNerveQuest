#include "Helper/LazyNerveValidator.h"

bool LazyNerveValidator::ValidateEditorGraphPin(const UEdGraphPin* GraphPin)
{
	if (GraphPin == nullptr) return false;

	if (GraphPin->PinName.ToString().ToLower() == FString(TEXT("Failed")).ToLower()
		&& GraphPin->Direction == EGPD_Output) return false;
	
	return true;
}

bool LazyNerveValidator::ValidateEditorOptionalGraphPin(const UEdGraphPin* GraphPin,
const UNerveQuestObjectiveNodeBase* GraphNode)
{
	if (GraphPin == nullptr) return false;
	if (!IsValid(GraphNode)) return false;
	if (!UNerveQuestObjectiveNodeBase::IsOptionalPin(GraphPin)) return false;

	return !GraphNode->GetOptionalPins().IsEmpty();
}
