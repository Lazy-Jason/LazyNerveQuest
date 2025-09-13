// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealED/Node/NerveQuestRootObjective.h"

void UNerveQuestRootObjective::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Launch"));
}

FText UNerveQuestRootObjective::GetTooltipText() const
{
	return FText::FromString(TEXT("All nodes are executed from this point on"));
}

bool UNerveQuestRootObjective::CanUserDeleteNode() const
{ return false; }

bool UNerveQuestRootObjective::CanDuplicateNode() const
{ return false; }

bool UNerveQuestRootObjective::CanPasteHere(const UEdGraph* TargetGraph) const
{ return false; }
