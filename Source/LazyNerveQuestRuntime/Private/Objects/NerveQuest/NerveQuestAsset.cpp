// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/NerveQuest/NerveQuestAsset.h"
#include "UObject/ObjectSaveContext.h"

TArray<UNerveQuestRuntimeObjectiveBase*> UNerveQuestAsset::GetQuestObjectives() const
{
	if(!IsValid(RuntimeGraph)) return {}; 
		
	// Convert smart pointers to raw pointers for Blueprint compatibility
	TArray<UNerveQuestRuntimeObjectiveBase*> Objectives;
	for (const TObjectPtr<UNerveQuestRuntimeObjectiveBase>& Node : RuntimeGraph->GraphNodes)
	{
		if (UNerveQuestRuntimeObjectiveBase* ObjectiveBase = Node.Get())
		{
			Objectives.Add(ObjectiveBase);
		}
	}
	return Objectives;
}

#if WITH_EDITOR
void UNerveQuestAsset::PreEditChange(FProperty* PropertyAboutToChange)
{
	if(PreEditListener) PreEditListener();
	UObject::PreEditChange(PropertyAboutToChange);
}

void UNerveQuestAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if(PostEditListener) PostEditListener();
	UObject::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
void UNerveQuestAsset::PreSave(FObjectPreSaveContext SaveContext)
{
	if(PreSaveListener) PreSaveListener();
}