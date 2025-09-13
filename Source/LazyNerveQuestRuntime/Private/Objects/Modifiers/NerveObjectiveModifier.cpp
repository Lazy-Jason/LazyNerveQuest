// // Copyright (C) 2024 Job Omondiale - All Rights Reserved


#include "Objects/Modifiers/NerveObjectiveModifier.h"

UWorld* UNerveObjectiveModifier::GetWorld() const
{
	if (!IsValid(WorldContextObject)) return nullptr;

	// In all other cases...
	return WorldContextObject->GetWorld();
}

void UNerveObjectiveModifier::OnObjectiveStart(UNerveQuestRuntimeObjectiveBase* Objective)
{}

void UNerveObjectiveModifier::OnObjectiveProgress(UNerveQuestRuntimeObjectiveBase* Objective)
{}
