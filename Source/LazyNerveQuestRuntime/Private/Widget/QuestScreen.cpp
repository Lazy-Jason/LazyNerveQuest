// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/QuestScreen.h"
#include "Components/PanelWidget.h"
#include "Subsystem/NerveQuestSubsystem.h"
#include "Widget/GameQuestObjectiveContainer.h"

void UQuestScreen::InitQuestObjective_Implementation(const UNerveQuestRuntimeData* QuestToTrack, const TArray<UNerveObjectiveRuntimeData*>& DisplayableObjectives)
{
    if (!IsValid(GetQuestContainer()) || !IsValid(GetQuestObjectiveClassContainer()) || !IsValid(QuestToTrack)) return;

    // Check if we already have a container for this quest
    UGameQuestObjectiveContainer* QuestContainer;
    if (ActiveQuestContainers.Contains(QuestToTrack))
    {
        QuestContainer = ActiveQuestContainers[QuestToTrack];
    }
    else
    {
        // Create new container for this quest
        QuestContainer = CreateWidget<UGameQuestObjectiveContainer>(GetOwningPlayer(), GetQuestObjectiveClassContainer());
        if (!IsValid(QuestContainer)) return;
        
        ActiveQuestContainers.Add(QuestToTrack, QuestContainer);
        GetQuestContainer()->AddChild(QuestContainer);
    }
    
    // Initialize the container with all displayable objectives
    QuestContainer->InitQuestObjective(QuestToTrack, DisplayableObjectives);
    CleanupInvalidEntries();
}

void UQuestScreen::InitQuestObjectiveSingle_Implementation(const UNerveQuestRuntimeData* QuestToTrack)
{
    if (!IsValid(QuestToTrack) || !IsValid(QuestToTrack->CurrentObjective)) return;
    
    TArray<UNerveObjectiveRuntimeData*> SingleObjective;
    SingleObjective.Add(QuestToTrack->CurrentObjective);
    InitQuestObjective(QuestToTrack, SingleObjective);
}

void UQuestScreen::UnInitQuestObjective_Implementation(const UNerveQuestRuntimeData* QuestToTrack)
{
    if (!IsValid(GetQuestContainer()) || !IsValid(QuestToTrack)) return;
    
    // Find and remove the specific quest container
    if (ActiveQuestContainers.Contains(QuestToTrack))
    {
        UGameQuestObjectiveContainer* QuestContainer = ActiveQuestContainers[QuestToTrack];
        if (IsValid(QuestContainer))
        {
            QuestContainer->UnInitQuestObjective(QuestToTrack);
            GetQuestContainer()->RemoveChild(QuestContainer);
        }
        ActiveQuestContainers.Remove(QuestToTrack);
    }

    CleanupInvalidEntries();
}

void UQuestScreen::CleanupInvalidEntries()
{
    TArray<const UNerveQuestRuntimeData*> KeysToRemove;
    for (const auto& Pair : ActiveQuestContainers)
    {
        if (!IsValid(Pair.Key->QuestAsset))
        {
            KeysToRemove.Add(Pair.Key);
        }
    }
    for (const auto& Key : KeysToRemove)
    {
        if (UGameQuestObjectiveContainer* Container = ActiveQuestContainers[Key])
        {
            if (IsValid(Container))
            {
                Container->RemoveFromParent();
            }
            ActiveQuestContainers.Remove(Key);
        }
    }
}

void UQuestScreen::UpdateObjective_Implementation(const UNerveObjectiveRuntimeData* ObjectiveToUpdate)
{
    if (!IsValid(ObjectiveToUpdate)) return;

    // Find the quest container that contains this objective
    for (const auto& QuestContainerPair : ActiveQuestContainers)
    {
        if (IsValid(QuestContainerPair.Value))
        {
            QuestContainerPair.Value->UpdateObjectiveItem(ObjectiveToUpdate);
        }
    }
}

void UQuestScreen::RemoveObjective_Implementation(const UNerveObjectiveRuntimeData* ObjectiveToRemove)
{
    if (!IsValid(ObjectiveToRemove)) return;

    // Find the quest container that contains this objective
    for (const auto& QuestContainerPair : ActiveQuestContainers)
    {
        if (IsValid(QuestContainerPair.Value))
        {
            QuestContainerPair.Value->RemoveObjectiveItem(ObjectiveToRemove);
        }
    }
}

UPanelWidget* UQuestScreen::GetQuestContainer_Implementation()
{ return nullptr; }

TSubclassOf<UGameQuestObjectiveContainer> UQuestScreen::GetQuestObjectiveClassContainer_Implementation()
{ return nullptr; }