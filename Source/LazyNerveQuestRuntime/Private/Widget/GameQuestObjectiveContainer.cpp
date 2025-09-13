// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/GameQuestObjectiveContainer.h"
#include "Components/PanelWidget.h"
#include "Subsystem/NerveQuestSubsystem.h"
#include "Widget/GameQuestObjectiveItem.h"

void UGameQuestObjectiveContainer::InitQuestObjective_Implementation(const UNerveQuestRuntimeData* QuestObjective, const TArray<UNerveObjectiveRuntimeData*>& DisplayableObjectives)
{
    if (!IsValid(QuestObjective)) return;

    // Set quest title
    SetQuestTitle(QuestObjective->QuestAsset->QuestTitle);
    
    // Clear existing objective items
    ObjectiveItems.Empty();
    if (IsValid(GetQuestObjectiveContainer()))
    {
        GetQuestObjectiveContainer()->ClearChildren();
    }
    if (IsValid(GetMainObjectiveContainer()))
    {
        GetMainObjectiveContainer()->ClearChildren();
    }
    if (IsValid(GetOptionalObjectiveContainer()))
    {
        GetOptionalObjectiveContainer()->ClearChildren();
    }
    
    // Generate all objectives
    GenerateQuestObjectives(DisplayableObjectives);
}

void UGameQuestObjectiveContainer::InitQuestObjectiveSingle_Implementation(const UNerveQuestRuntimeData* QuestObjective)
{
    if (!IsValid(QuestObjective) || !IsValid(QuestObjective->CurrentObjective)) return;
    
    TArray<UNerveObjectiveRuntimeData*> SingleObjective;
    SingleObjective.Add(QuestObjective->CurrentObjective);
    InitQuestObjective(QuestObjective, SingleObjective);
}

void UGameQuestObjectiveContainer::UnInitQuestObjective_Implementation(const UNerveQuestRuntimeData* QuestObjective)
{
    // Clean up all objective items
    for (auto& ObjectiveItemPair : ObjectiveItems)
    {
        if (IsValid(ObjectiveItemPair.Value))
        {
            ObjectiveItemPair.Value->UnInitializeObjectiveItem();
        }
    }
    ObjectiveItems.Empty();

    // Clear containers
    if (IsValid(GetQuestObjectiveContainer()))
    {
        GetQuestObjectiveContainer()->ClearChildren();
    }
    if (IsValid(GetMainObjectiveContainer()))
    {
        GetMainObjectiveContainer()->ClearChildren();
    }
    if (IsValid(GetOptionalObjectiveContainer()))
    {
        GetOptionalObjectiveContainer()->ClearChildren();
    }

    // Clear quest title
    SetQuestTitle(FString());
}

void UGameQuestObjectiveContainer::AddObjectiveItem_Implementation(UNerveObjectiveRuntimeData* ObjectiveToAdd)
{
    if (!IsValid(ObjectiveToAdd)) return;

    // Don't add if already exists
    if (ObjectiveItems.Contains(ObjectiveToAdd)) return;

    // Determine which container to use and which class
    UPanelWidget* TargetContainer;
    TSubclassOf<UGameQuestObjectiveItem> ItemClass;

    if (ObjectiveToAdd->IsOptionalObjective())
    {
        TargetContainer = GetOptionalObjectiveContainer();
        ItemClass = GetOptionalObjectiveItemClass();
        
        // Fallback to main containers if optional containers not available
        if (!IsValid(TargetContainer))
        {
            TargetContainer = GetQuestObjectiveContainer();
        }
        if (!ItemClass)
        {
            ItemClass = GetQuestObjectiveItemClass();
        }
    }
    else
    {
        TargetContainer = GetMainObjectiveContainer();
        ItemClass = GetMainObjectiveItemClass();
        
        // Fallback to main containers if specific containers not available
        if (!IsValid(TargetContainer))
        {
            TargetContainer = GetQuestObjectiveContainer();
        }
        if (!ItemClass)
        {
            ItemClass = GetQuestObjectiveItemClass();
        }
    }

    if (!IsValid(TargetContainer) || !ItemClass) return;

    // Create objective item
    UGameQuestObjectiveItem* NewObjectiveItem = CreateWidget<UGameQuestObjectiveItem>(GetOwningPlayer(), ItemClass);
    if (!IsValid(NewObjectiveItem)) return;

    NewObjectiveItem->InitializeObjectiveItem(ObjectiveToAdd);
    TargetContainer->AddChild(NewObjectiveItem);
    ObjectiveItems.Add(ObjectiveToAdd, NewObjectiveItem);
}

void UGameQuestObjectiveContainer::RemoveObjectiveItem_Implementation(const UNerveObjectiveRuntimeData* ObjectiveToRemove)
{
    if (!IsValid(ObjectiveToRemove)) return;

    UGameQuestObjectiveItem* ItemToRemove = FindObjectiveItem(ObjectiveToRemove);
    if (!IsValid(ItemToRemove)) return;

    // Remove from appropriate container
    if (ObjectiveToRemove->IsOptionalObjective() && IsValid(GetOptionalObjectiveContainer()))
    {
        GetOptionalObjectiveContainer()->RemoveChild(ItemToRemove);
    }
    else if (IsValid(GetMainObjectiveContainer()))
    {
        GetMainObjectiveContainer()->RemoveChild(ItemToRemove);
    }
    else if (IsValid(GetQuestObjectiveContainer()))
    {
        GetQuestObjectiveContainer()->RemoveChild(ItemToRemove);
    }

    ItemToRemove->UnInitializeObjectiveItem();
    ObjectiveItems.Remove(ObjectiveToRemove);
}

void UGameQuestObjectiveContainer::UpdateObjectiveItem_Implementation(const UNerveObjectiveRuntimeData* ObjectiveToUpdate)
{
    if (!IsValid(ObjectiveToUpdate)) return;

    UGameQuestObjectiveItem* ItemToUpdate = FindObjectiveItem(ObjectiveToUpdate);
    if (!IsValid(ItemToUpdate)) return;

    // Update the item with current objective data
    ItemToUpdate->InitializeObjectiveItem(const_cast<UNerveObjectiveRuntimeData*>(ObjectiveToUpdate));
}

void UGameQuestObjectiveContainer::SetQuestTitle(const FString& NewTitle)
{
    if (!IsValid(GetQuestTitleBlock())) return;
    
    GetQuestTitleBlock()->SetText(FText::FromString(NewTitle));
}

UGameQuestObjectiveItem* UGameQuestObjectiveContainer::FindObjectiveItem(const UNerveObjectiveRuntimeData* Objective) const
{
    if (ObjectiveItems.Contains(Objective))
    {
        return ObjectiveItems[Objective];
    }
    return nullptr;
}

void UGameQuestObjectiveContainer::GenerateQuestObjectives(const TArray<UNerveObjectiveRuntimeData*>& Objectives)
{
    // Sort objectives by display priority and type
    TArray<UNerveObjectiveRuntimeData*> SortedObjectives = Objectives;
    SortedObjectives.Sort([](const UNerveObjectiveRuntimeData& A, const UNerveObjectiveRuntimeData& B)
    {
        // Main objectives first, then optionals
        if (A.IsOptionalObjective() != B.IsOptionalObjective())
        {
            return !A.IsOptionalObjective(); // Main objectives (false) come first
        }
        // Within same type, sort by priority
        return A.GetDisplayPriority() > B.GetDisplayPriority();
    });

    // Add each objective
    for (UNerveObjectiveRuntimeData* Objective : SortedObjectives)
    {
        AddObjectiveItem(Objective);
    }
}

UPanelWidget* UGameQuestObjectiveContainer::GetMainObjectiveContainer_Implementation()
{
    // Default fallback to main container
    return GetQuestObjectiveContainer();
}

UPanelWidget* UGameQuestObjectiveContainer::GetOptionalObjectiveContainer_Implementation()
{
    // Default fallback to main container
    return GetQuestObjectiveContainer();
}

TSubclassOf<UGameQuestObjectiveItem> UGameQuestObjectiveContainer::GetMainObjectiveItemClass_Implementation()
{
    // Default fallback to main class
    return GetQuestObjectiveItemClass();
}

TSubclassOf<UGameQuestObjectiveItem> UGameQuestObjectiveContainer::GetOptionalObjectiveItemClass_Implementation()
{
    // Default fallback to main class
    return GetQuestObjectiveItemClass();
}

UTextBlock* UGameQuestObjectiveContainer::GetQuestTitleBlock_Implementation()
{ return nullptr; }

UPanelWidget* UGameQuestObjectiveContainer::GetQuestObjectiveContainer_Implementation()
{ return nullptr; }

TSubclassOf<UGameQuestObjectiveItem> UGameQuestObjectiveContainer::GetQuestObjectiveItemClass_Implementation()
{ return nullptr; }