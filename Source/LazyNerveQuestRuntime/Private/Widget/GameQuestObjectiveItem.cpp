// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/GameQuestObjectiveItem.h"
#include "Components/PanelWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Subsystem/NerveQuestSubsystem.h"

void UGameQuestObjectiveItem::InitializeObjectiveItem_Implementation(UNerveObjectiveRuntimeData* PerformingObjective)
{
    if (!IsValid(PerformingObjective) || !IsValid(PerformingObjective->ParentObjective)) return;

    ParentPerformingObjective = PerformingObjective;
    
    ParentPerformingObjective->OnObjectiveCompleted.AddDynamic(this, &ThisClass::OnObjectiveCompleted);
    ParentPerformingObjective->OnObjectiveFailed.AddDynamic(this, &ThisClass::OnObjectiveFailed);

    ConstructObjectiveLook();
    GenerateOptionalObjectives();
}

void UGameQuestObjectiveItem::UnInitializeObjectiveItem_Implementation()
{
    ParentPerformingObjective->OnObjectiveCompleted.RemoveDynamic(this, &ThisClass::OnObjectiveCompleted);
    ParentPerformingObjective->OnObjectiveFailed.RemoveDynamic(this, &ThisClass::OnObjectiveFailed);
    
    ParentPerformingObjective = nullptr;
    for (const auto Element : AllOptionalObjectiveWidget)
    {
        Element->UnInitializeObjectiveItem();
    }
}

void UGameQuestObjectiveItem::ConstructObjectiveLook_Implementation()
{
    // Set objective title
    if (IsValid(GetObjectiveTitleBlock()))
    {
        if (GetParentPerformingObjective()->ParentObjective->GetObjectiveDisplayLabel().IsEmpty())
        {
            GetObjectiveTitleBlock()->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            GetObjectiveTitleBlock()->SetText(GetParentPerformingObjective()->ParentObjective->GetObjectiveDisplayLabel());
            GetObjectiveTitleBlock()->SetVisibility(ESlateVisibility::Visible);
        }
    }

    // Set objective tip/description
    if (IsValid(GetObjectiveTipBlock()))
    {
        if (GetParentPerformingObjective()->ParentObjective->GetObjectiveDisplayTip().IsEmpty())
        {
            GetObjectiveTipBlock()->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            if (UTextBlock* NormalText = Cast<UTextBlock>(GetObjectiveTipBlock()))
            {
                NormalText->SetText(GetParentPerformingObjective()->ParentObjective->GetObjectiveDisplayTip());
            }
            else if (URichTextBlock* RichText = Cast<URichTextBlock>(GetObjectiveTipBlock()))
            {
                RichText->SetText(GetParentPerformingObjective()->ParentObjective->GetObjectiveDisplayTip());
            }
            GetObjectiveTipBlock()->SetVisibility(ESlateVisibility::Visible);
        }
    }

    RecalculateObjectiveState();
    ConstructOptionalLook();
}

void UGameQuestObjectiveItem::ConstructOptionalLook_Implementation()
{
    // Align the optional objectives to the right of the screen and scaled down
    // this is a preference and should be overriden
    if (GetParentPerformingObjective()->IsOptionalObjective())
    {
        SetRenderTransformPivot(FVector2D(1, 0));
        SetRenderScale(FVector2D(0.7, 0.7));
    }
}

void UGameQuestObjectiveItem::GenerateOptionalObjectives_Implementation()
{
    // Optional objectives cant generate optionals
    if (GetParentPerformingObjective()->IsOptionalObjective()) return;
    if (!IsValid(GetOptionalObjectiveClass())) return;
    if (!IsValid(GetOptionalObjectiveContainer())) return;

    AllOptionalObjectiveWidget.Empty();

    TArray<UNerveObjectiveRuntimeData*> Optionals = GetParentPerformingObjective()->GetOptionalObjectives();

    int32 Index = 0;
    for (const auto Optional : Optionals)
    {
        UGameQuestObjectiveItem* NewObjectiveItem = CreateWidget<UGameQuestObjectiveItem>(GetOwningPlayer(), GetOptionalObjectiveClass());
        if (!IsValid(NewObjectiveItem)) continue;

        AllOptionalObjectiveWidget.Add(NewObjectiveItem);

        UUniformGridPanel* NewUniformGrid = Cast<UUniformGridPanel>(GetOptionalObjectiveContainer());
        if (IsValid(NewUniformGrid))
        {
            UUniformGridSlot* GridSlot = NewUniformGrid->AddChildToUniformGrid(NewObjectiveItem, Index, 0);
            if (IsValid(GridSlot))
            {
                // personal preference override to make custom
                GridSlot->SetHorizontalAlignment(HAlign_Right);
                GridSlot->SetVerticalAlignment(VAlign_Top);
            }
        }
        else
        {
            GetOptionalObjectiveContainer()->AddChild(NewObjectiveItem);
        }
        NewObjectiveItem->InitializeObjectiveItem(Optional);

        Index++;
    }
}

void UGameQuestObjectiveItem::RecalculateObjectiveState_Implementation()
{ /* implement check for completed or failed and change the UI based on that */ }

bool UGameQuestObjectiveItem::IsOptionalObjective() const
{
    return IsValid(ParentPerformingObjective) && ParentPerformingObjective->IsOptionalObjective();
}

UTextBlock* UGameQuestObjectiveItem::GetObjectiveTitleBlock_Implementation()
{ return nullptr; }

UTextLayoutWidget* UGameQuestObjectiveItem::GetObjectiveTipBlock_Implementation()
{ return nullptr; }

TSubclassOf<UGameQuestObjectiveItem> UGameQuestObjectiveItem::GetOptionalObjectiveClass_Implementation()
{ return nullptr; }

UPanelWidget* UGameQuestObjectiveItem::GetOptionalObjectiveContainer_Implementation()
{ return nullptr; }

void UGameQuestObjectiveItem::OnObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* ObjectiveBase)
{
    RecalculateObjectiveState();
}

void UGameQuestObjectiveItem::OnObjectiveFailed(UNerveQuestRuntimeObjectiveBase* ObjectiveBase)
{
    RecalculateObjectiveState();
}
