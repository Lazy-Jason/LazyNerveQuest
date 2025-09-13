// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Nodes/Objective/NerveEntryObjective.h"

FText UNerveEntryObjective::GetObjectiveName_Implementation()
{
	return FText::FromString(TEXT("Launch"));
}

FText UNerveEntryObjective::GetObjectiveDescription_Implementation()
{
	return FText::FromString(TEXT("Objects are evaluated from this point on."));
}

void UNerveEntryObjective::ExecuteObjective_Implementation(UNerveQuestAsset* NerveQuestAsset)
{
	CompleteObjective();
}
