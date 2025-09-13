// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory/NerveQuestObjectiveFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"

UNerveQuestObjectiveFactory::UNerveQuestObjectiveFactory(const FObjectInitializer& ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	ParentClass = UNerveQuestRuntimeObjectiveBase::StaticClass();
}