// // Copyright (C) 2024 Job Omondiale - All Rights Reserved


#include "Objects/Nodes/Objective/NerveSequenceRuntimeObjective.h"
#include "LazyNerveQuestRuntime.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"
#include "Objects/Pin/NerveQuestRuntimePin.h"
#include "Subsystem/NerveQuestSubsystem.h"

UNerveSequenceRuntimeObjective::UNerveSequenceRuntimeObjective():
CurrentQuestAsset(nullptr), FailedChildCount(0)
{
    ExecutionType = EObjectiveExecutionType::Parallel;
    CurrentSequentialIndex = 0;
    CompletedChildCount = 0;
    bSequenceStarted = false;
}

FText UNerveSequenceRuntimeObjective::GetObjectiveName_Implementation()
{
    return FText::FromString(TEXT("Sequence Objective"));
}

FText UNerveSequenceRuntimeObjective::GetObjectiveDescription_Implementation()
{
    FString Description = ExecutionType == EObjectiveExecutionType::Sequential 
        ? TEXT("Complete objectives in order") 
        : TEXT("Complete all objectives");
    return FText::FromString(Description);
}

FText UNerveSequenceRuntimeObjective::GetObjectiveCategory_Implementation()
{
    return FText::FromString(TEXT("Sequence"));
}

FSlateBrush UNerveSequenceRuntimeObjective::GetObjectiveBrush_Implementation() const
{
    return FSlateBrush();
}

void UNerveSequenceRuntimeObjective::ExecuteObjective_Implementation(UNerveQuestAsset* QuestManager)
{
    Super::ExecuteObjective_Implementation(QuestManager);
    
    if (!IsValid(QuestManager))
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveSequenceRuntimeObjective: Invalid QuestManager"));
        return;
    }

    CurrentQuestAsset = QuestManager;
    bSequenceStarted = true;
    
    // Collect all child objectives from output pins
    const bool Success = CollectChildObjectives();
    
    if (Success && ChildObjectives.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("UNerveSequenceRuntimeObjective: No child objectives found"));
        OnObjectiveCompleted.Broadcast(this);
        return;
    }

    // Initialize counters
    CurrentSequentialIndex = 0;
    CompletedChildCount = 0;
    FailedChildCount = 0;

    // Execute based on type
    switch (ExecutionType)
    {
        case EObjectiveExecutionType::Sequential:
            ExecuteSequential();
            break;
        case EObjectiveExecutionType::Parallel:
            ExecuteParallel();
            break;
    }
}

void UNerveSequenceRuntimeObjective::PauseObjective_Implementation()
{
    // Pause all active child objectives
    for (UNerveQuestRuntimeObjectiveBase* Child : ChildObjectives)
    {
        if (IsValid(Child) && ActiveChildObjectives.Contains(Child))
        {
            Child->PauseObjective();
        }
    }
}

void UNerveSequenceRuntimeObjective::ResumeObjective_Implementation()
{
    // Resume all active child objectives
    for (UNerveQuestRuntimeObjectiveBase* Child : ChildObjectives)
    {
        if (IsValid(Child) && ActiveChildObjectives.Contains(Child))
        {
            Child->ResumeObjective();
        }
    }
}

void UNerveSequenceRuntimeObjective::MarkAsTracked_Implementation(bool TrackValue)
{
    Super::MarkAsTracked_Implementation(TrackValue);
    
    // Mark all active child objectives as tracked/untracked
    for (UNerveQuestRuntimeObjectiveBase* Child : ActiveChildObjectives)
    {
        if (IsValid(Child))
        {
            Child->MarkAsTracked(TrackValue);
        }
    }
}

void UNerveSequenceRuntimeObjective::CleanUpObjective_Implementation()
{
    // Clean up all child objectives
    for (UNerveQuestRuntimeObjectiveBase* Child : ChildObjectives)
    {
        if (IsValid(Child))
        {
            // Remove delegates
            Child->OnObjectiveCompleted.RemoveAll(this);
            Child->OnObjectiveFailed.RemoveAll(this);
            Child->CleanUpObjective();
        }
    }
    
    ChildObjectives.Empty();
    ActiveChildObjectives.Empty();
    CompletedChildObjectives.Empty();
    
    Super::CleanUpObjective_Implementation();
}

bool UNerveSequenceRuntimeObjective::CollectChildObjectives()
{
    ChildObjectives.Empty();

    UNerveQuestRuntimePin* SequencePin = FindOutPinByCategory(FLazyNerveQuestRuntimeModule::NerveQuestSequencePinCategory);
    if (!IsValid(SequencePin)) return false;

    if (SequencePin->Connection.IsEmpty()) return false;

    // This will maily be connected to one node so no need for a loop
    const TWeakObjectPtr<UNerveQuestRuntimePin> ConnectedPin = SequencePin->Connection[0];

    if (!ConnectedPin.IsValid()) return false;
    
    AccumulateAllSequenceChildren(ConnectedPin.Get()->ParentNode.Get());
    
    UE_LOG(LogTemp, Log, TEXT("UNerveSequenceRuntimeObjective: Collected %d child objectives"), ChildObjectives.Num());
    return true;
}

void UNerveSequenceRuntimeObjective::AccumulateAllSequenceChildren(UNerveQuestRuntimeObjectiveBase* ParentNode)
{
    if (!IsValid(ParentNode)) return;
    ChildObjectives.Add(ParentNode);
    
    if (ParentNode->OutPutPin.IsEmpty()) return;
    const UNerveQuestRuntimePin* ConnectedPin = ParentNode->OutPutPin[0];

    AccumulateAllSequenceChildren(ConnectedPin->ParentNode.Get());
}

void UNerveSequenceRuntimeObjective::ExecuteSequential()
{
    if (CurrentSequentialIndex >= ChildObjectives.Num())
    {
        // All objectives completed
        OnObjectiveCompleted.Broadcast(this);
        return;
    }
    
    UNerveQuestRuntimeObjectiveBase* CurrentChild = ChildObjectives[CurrentSequentialIndex];
    if (!IsValid(CurrentChild))
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveSequenceRuntimeObjective: Invalid child objective at index %d"), CurrentSequentialIndex);
        OnObjectiveFailed.Broadcast(this);
        return;
    }
    
    // Set up delegates for the current child
    CurrentChild->OnObjectiveCompleted.AddDynamic(this, &UNerveSequenceRuntimeObjective::OnChildObjectiveCompleted);
    CurrentChild->OnObjectiveFailed.AddDynamic(this, &UNerveSequenceRuntimeObjective::OnChildObjectiveFailed);
    
    // Add to active objectives and execute
    ActiveChildObjectives.Empty(); // Only one active at a time for sequential
    ActiveChildObjectives.Add(CurrentChild);
    CurrentChild->ExecuteObjective(CurrentQuestAsset);
    
    UE_LOG(LogTemp, Log, TEXT("UNerveSequenceRuntimeObjective: Executing sequential objective %d/%d"), 
    CurrentSequentialIndex + 1, ChildObjectives.Num());
}

void UNerveSequenceRuntimeObjective::ExecuteParallel()
{
    ActiveChildObjectives.Empty();
    
    // Start all child objectives simultaneously
    for (UNerveQuestRuntimeObjectiveBase* Child : ChildObjectives)
    {
        if (!IsValid(Child)) continue;
            
        // Set up delegates
        Child->OnObjectiveCompleted.AddDynamic(this, &UNerveSequenceRuntimeObjective::OnChildObjectiveCompleted);
        Child->OnObjectiveFailed.AddDynamic(this, &UNerveSequenceRuntimeObjective::OnChildObjectiveFailed);
        
        // Add to active and execute
        ActiveChildObjectives.Add(Child);
        Child->ExecuteObjective(CurrentQuestAsset);
    }
    
    UE_LOG(LogTemp, Log, TEXT("UNerveSequenceRuntimeObjective: Executing %d parallel objectives"), ActiveChildObjectives.Num());
}

void UNerveSequenceRuntimeObjective::OnChildObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* CompletedObjective)
{
    if (!IsValid(CompletedObjective)) return;
    
    // Remove from active and add to completed
    ActiveChildObjectives.Remove(CompletedObjective);
    CompletedChildObjectives.AddUnique(CompletedObjective);
    CompletedChildCount++;
    
    // Remove delegates
    CompletedObjective->OnObjectiveCompleted.RemoveAll(this);
    CompletedObjective->OnObjectiveFailed.RemoveAll(this);
    
    UE_LOG(LogTemp, Log, TEXT("UNerveSequenceRuntimeObjective: Child objective completed (%d/%d)"), 
    CompletedChildCount, ChildObjectives.Num());
    
    // Handle completion based on execution type
    if (ExecutionType == EObjectiveExecutionType::Sequential)
    {
        CurrentSequentialIndex++;
        if (CurrentSequentialIndex >= ChildObjectives.Num())
        {
            // All sequential objectives completed
            OnObjectiveCompleted.Broadcast(this);
        }
        else
        {
            // Execute next objective in sequence
            ExecuteSequential();
        }
    }
    else // Parallel
    {
        if (CompletedChildCount >= ChildObjectives.Num())
        {
            // All parallel objectives completed
            OnObjectiveCompleted.Broadcast(this);
        }
    }
    
    // Broadcast progress update
    const float Progress = static_cast<float>(CompletedChildCount) / static_cast<float>(ChildObjectives.Num());
    OnProgressChanged.Broadcast(this, Progress, 1.0f);
}

void UNerveSequenceRuntimeObjective::OnChildObjectiveFailed(UNerveQuestRuntimeObjectiveBase* FailedObjective)
{
    if (!IsValid(FailedObjective)) return;
    
    // Remove from active
    ActiveChildObjectives.Remove(FailedObjective);
    FailedChildCount++;
    
    // Remove delegates
    FailedObjective->OnObjectiveCompleted.RemoveAll(this);
    FailedObjective->OnObjectiveFailed.RemoveAll(this);
    
    UE_LOG(LogTemp, Warning, TEXT("UNerveSequenceRuntimeObjective: Child objective failed"));
    
    // Handle failure based on execution type and failure response
    const EObjectiveFailureResponse ChildFailureResponse = FailedObjective->GetObjectiveFailureResponse();
    
    switch (ChildFailureResponse)
    {
        case EObjectiveFailureResponse::FailQuest:
            // If any child fails with FailQuest, the entire sequence fails
            OnObjectiveFailed.Broadcast(this);
            break;
            
        case EObjectiveFailureResponse::ContinueToNextObjective:
            if (ExecutionType == EObjectiveExecutionType::Sequential)
            {
                // Skip to next objective in sequence
                CurrentSequentialIndex++;
                if (CurrentSequentialIndex >= ChildObjectives.Num())
                {
                    OnObjectiveCompleted.Broadcast(this);
                }
                else
                {
                    ExecuteSequential();
                }
            }
            else // Parallel
            {
                // For parallel, continue with remaining objectives
                // Complete the sequence if all remaining objectives are done
                if (ActiveChildObjectives.IsEmpty())
                {
                    if (CompletedChildCount > 0)
                    {
                        OnObjectiveCompleted.Broadcast(this);
                    }
                    else
                    {
                        OnObjectiveFailed.Broadcast(this);
                    }
                }
            }
            break;
            
        case EObjectiveFailureResponse::RestartQuest:
            // Restart the entire sequence
            RestartSequence();
            break;
    }
}

void UNerveSequenceRuntimeObjective::RestartSequence()
{
    // Clean up current state
    for (UNerveQuestRuntimeObjectiveBase* Child : ActiveChildObjectives)
    {
        if (IsValid(Child))
        {
            Child->OnObjectiveCompleted.RemoveAll(this);
            Child->OnObjectiveFailed.RemoveAll(this);
            Child->CleanUpObjective();
        }
    }
    
    // Reset state
    ActiveChildObjectives.Empty();
    CompletedChildObjectives.Empty();
    CurrentSequentialIndex = 0;
    CompletedChildCount = 0;
    FailedChildCount = 0;
    
    // Restart execution
    ExecuteObjective_Implementation(CurrentQuestAsset);
}

TArray<UNerveQuestRuntimeObjectiveBase*> UNerveSequenceRuntimeObjective::GetActiveChildObjectives() const
{
    return ActiveChildObjectives;
}

TArray<UNerveQuestRuntimeObjectiveBase*> UNerveSequenceRuntimeObjective::GetCompletedChildObjectives() const
{
    return CompletedChildObjectives;
}

float UNerveSequenceRuntimeObjective::GetSequenceProgress() const
{
    if (ChildObjectives.IsEmpty()) return 0.0f;
    
    return static_cast<float>(CompletedChildCount) / static_cast<float>(ChildObjectives.Num());
}

bool UNerveSequenceRuntimeObjective::IsSequenceComplete() const
{
    return CompletedChildCount >= ChildObjectives.Num();
}