// // Copyright (C) 2024 Job Omondiale - All Rights Reserved


#include "Objects/Modifiers/TimeLimitModifier.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"

void UTimeLimitModifier::OnObjectiveStart(UNerveQuestRuntimeObjectiveBase* Objective)
{
	if(!IsValid(Objective) || !IsValid(GetWorld())) return;

	StartTime = GetWorld()->GetTimeSeconds();
}

bool UTimeLimitModifier::CheckCondition(UNerveQuestRuntimeObjectiveBase* Objective)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - StartTime) <= TimeLimit;
}