// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#include "Objects/Nodes/Objective/NerveSubQuestRuntimeObjective.h"
#include "Subsystem/NerveQuestSubsystem.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"
#include "LazyNerveRuntimeQuestStyle.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNerveSubQuestRuntimeObjective::UNerveSubQuestRuntimeObjective()
{
    SubQuestRuntimeData = nullptr;
    QuestSubsystem = nullptr;
    CurrentRestartAttempts = 0;
    bIsCurrentlyTracked = false;
    
    // Set up default display information
    DisplayLabel = TEXT("Execute Sub-Quest");
    DisplayTip = TEXT("Run a nested quest sequence");
}

FText UNerveSubQuestRuntimeObjective::GetObjectiveName_Implementation()
{
    return FText::FromString(TEXT("Sub-Quest"));
}

FText UNerveSubQuestRuntimeObjective::GetObjectiveDescription_Implementation()
{
    return FText::FromString(TEXT("Executes another quest as a nested sequence within this objective."));
}

FText UNerveSubQuestRuntimeObjective::GetObjectiveCategory_Implementation()
{
    return FText::FromString(TEXT("Primitive Objectives|Utility"));
}

FSlateBrush UNerveSubQuestRuntimeObjective::GetObjectiveBrush_Implementation() const
{
    const FSlateBrush* Brush = FLazyNerveRuntimeQuestStyle::Get().GetBrush("QuestSubQuestBrush");
    return Brush ? *Brush : FSlateBrush();
}

void UNerveSubQuestRuntimeObjective::ExecuteObjective_Implementation(UNerveQuestAsset* QuestManager)
{
    UNerveQuestRuntimeObjectiveBase::ExecuteObjective_Implementation(QuestManager);
    
    // Get the quest subsystem
    QuestSubsystem = GetQuestSubsystem();
    if (!IsValid(QuestSubsystem))
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveSubQuestRuntimeObjective: Could not get quest subsystem"));
        FailObjective();
        return;
    }

    // Validate the sub-quest asset
    if (!IsSubQuestValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveSubQuestRuntimeObjective: Invalid sub-quest asset"));
        FailObjective();
        return;
    }

    // Initialize and start the sub-quest
    if (!InitializeSubQuest())
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveSubQuestRuntimeObjective: Failed to initialize sub-quest"));
        FailObjective();
        return;
    }

    StartSubQuest();
}

void UNerveSubQuestRuntimeObjective::PauseObjective_Implementation()
{
    UNerveQuestRuntimeObjectiveBase::PauseObjective_Implementation();
    
    // Pause the sub-quest if it's running
    if (IsValid(SubQuestRuntimeData) && IsValid(SubQuestRuntimeData->CurrentObjective))
    {
        SubQuestRuntimeData->CurrentObjective->ParentObjective->PauseObjective();
    }
}

void UNerveSubQuestRuntimeObjective::ResumeObjective_Implementation()
{
    UNerveQuestRuntimeObjectiveBase::ResumeObjective_Implementation();
    
    // Resume the sub-quest if it's paused
    if (IsValid(SubQuestRuntimeData) && IsValid(SubQuestRuntimeData->CurrentObjective))
    {
        SubQuestRuntimeData->CurrentObjective->ParentObjective->ResumeObjective();
    }
}

void UNerveSubQuestRuntimeObjective::MarkAsTracked_Implementation(bool TrackValue)
{
    UNerveQuestRuntimeObjectiveBase::MarkAsTracked_Implementation(TrackValue);
    
    bIsCurrentlyTracked = TrackValue;
    UpdateSubQuestTracking();
}

void UNerveSubQuestRuntimeObjective::CleanUpObjective_Implementation()
{
    CleanupSubQuest();
    UNerveQuestRuntimeObjectiveBase::CleanUpObjective_Implementation();
}

bool UNerveSubQuestRuntimeObjective::IsSubQuestValid() const
{
    return SubQuestAsset.IsValid() && IsValid(SubQuestAsset.LoadSynchronous());
}

bool UNerveSubQuestRuntimeObjective::RestartSubQuest()
{
    if (CurrentRestartAttempts >= MaxRestartAttempts)
    {
        UE_LOG(LogTemp, Warning, TEXT("UNerveSubQuestRuntimeObjective: Maximum restart attempts reached"));
        return false;
    }

    CurrentRestartAttempts++;
    
    // Clean up current sub-quest
    CleanupSubQuest();
    
    // Initialize and start fresh
    if (InitializeSubQuest())
    {
        StartSubQuest();
        return true;
    }
    
    return false;
}

void UNerveSubQuestRuntimeObjective::ForceCompleteSubQuest()
{
    if (IsValid(SubQuestRuntimeData))
    {
        SubQuestRuntimeData->MarkQuestComplete();
    }
}

float UNerveSubQuestRuntimeObjective::GetSubQuestProgress() const
{
    if (!IsValid(SubQuestRuntimeData) || SubQuestRuntimeData->GetAllObjectives().IsEmpty())
    {
        return 0.0f;
    }

    const TArray<UNerveObjectiveRuntimeData*>& Objectives = SubQuestRuntimeData->GetAllObjectives();
    int32 CompletedObjectives = 0;
    
    for (const UNerveObjectiveRuntimeData* Objective : Objectives)
    {
        if (Objective && Objective->bIsCompleted)
        {
            CompletedObjectives++;
        }
    }
    
    return static_cast<float>(CompletedObjectives) / static_cast<float>(Objectives.Num());
}

FText UNerveSubQuestRuntimeObjective::GetCurrentSubQuestObjectiveText() const
{
    if (IsValid(SubQuestRuntimeData) && IsValid(SubQuestRuntimeData->CurrentObjective) && 
        IsValid(SubQuestRuntimeData->CurrentObjective->ParentObjective))
    {
        return SubQuestRuntimeData->CurrentObjective->ParentObjective->GetObjectiveDisplayLabel();
    }
    
    return FText::GetEmpty();
}

bool UNerveSubQuestRuntimeObjective::InitializeSubQuest()
{
    if (!IsSubQuestValid() || !IsValid(QuestSubsystem)) return false;

    UNerveQuestAsset* SubQuestAssetPtr = SubQuestAsset.LoadSynchronous();
    
    // Create sub-quest runtime data (not registered in main quest system)
    SubQuestRuntimeData = NewObject<UNerveQuestRuntimeData>(this);
    if (!IsValid(SubQuestRuntimeData))
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveSubQuestRuntimeObjective: Failed to create sub-quest runtime data"));
        return false;
    }

    // Initialize the sub-quest runtime data
    // We pass false for tracking initially - we'll handle tracking separately
    SubQuestRuntimeData->Initialize(SubQuestAssetPtr, QuestSubsystem, false);
    
    // Set world context if inheriting from parent
    if (bInheritWorldContext && GetWorldContextObject())
    {
        // The sub-quest will inherit the world context through the subsystem
    }

    // Bind to sub-quest events
    SubQuestRuntimeData->OnQuestCompleted.AddDynamic(this, &UNerveSubQuestRuntimeObjective::OnSubQuestCompleted);
    SubQuestRuntimeData->OnQuestFailed.AddDynamic(this, &UNerveSubQuestRuntimeObjective::OnSubQuestFailed);

    return true;
}

void UNerveSubQuestRuntimeObjective::CleanupSubQuest()
{
    if (IsValid(SubQuestRuntimeData))
    {
        // Unbind delegates
        SubQuestRuntimeData->OnQuestCompleted.RemoveAll(this);
        SubQuestRuntimeData->OnQuestFailed.RemoveAll(this);
        
        // Clean up sub-quest data
        SubQuestRuntimeData->Uninitialize();
        SubQuestRuntimeData = nullptr;
    }
    
    CurrentRestartAttempts = 0;
}

void UNerveSubQuestRuntimeObjective::StartSubQuest()
{
    if (!IsValid(SubQuestRuntimeData))
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveSubQuestRuntimeObjective: Cannot start invalid sub-quest"));
        FailObjective();
        return;
    }

    // Set up objective-specific completion tracking
    if (CompletionBehavior == ESubQuestCompletionBehavior::CompleteOnSpecificObjective)
    {
        const TArray<UNerveObjectiveRuntimeData*>& Objectives = SubQuestRuntimeData->GetAllObjectives();
        if (Objectives.IsValidIndex(SpecificObjectiveIndex) && IsValid(Objectives[SpecificObjectiveIndex]))
        {
            Objectives[SpecificObjectiveIndex]->OnObjectiveCompleted.AddDynamic(
                this, &UNerveSubQuestRuntimeObjective::OnSubQuestObjectiveCompleted);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UNerveSubQuestRuntimeObjective: Invalid specific objective index %d"), 
                SpecificObjectiveIndex);
        }
    }

    // Update tracking behavior
    UpdateSubQuestTracking();
    
    // Start the sub-quest
    SubQuestRuntimeData->StartQuest();
    
    UE_LOG(LogTemp, Log, TEXT("UNerveSubQuestRuntimeObjective: Started sub-quest '%s'"), 
    SubQuestRuntimeData->QuestAsset ? *SubQuestRuntimeData->QuestAsset->QuestTitle : TEXT("Unknown"));
}

void UNerveSubQuestRuntimeObjective::OnSubQuestCompleted(UNerveQuestRuntimeData* CompletedQuest)
{
    if (CompletedQuest != SubQuestRuntimeData) return;

    UE_LOG(LogTemp, Log, TEXT("UNerveSubQuestRuntimeObjective: Sub-quest completed"));
    
    // Handle completion based on behavior setting
    switch (CompletionBehavior)
    {
        case ESubQuestCompletionBehavior::CompleteOnSubQuestComplete:
            CompleteObjective();
            break;
            
        case ESubQuestCompletionBehavior::CompleteOnSpecificObjective:
            // This is handled by OnSubQuestObjectiveCompleted
            break;
            
        case ESubQuestCompletionBehavior::Manual:
            // Do nothing - manual completion required
            break;
    }
}

void UNerveSubQuestRuntimeObjective::OnSubQuestFailed(UNerveQuestRuntimeData* FailedQuest)
{
    if (FailedQuest != SubQuestRuntimeData) return;

    UE_LOG(LogTemp, Log, TEXT("UNerveSubQuestRuntimeObjective: Sub-quest failed"));
    
    // Handle failure based on behavior setting
    switch (FailureBehavior)
    {
        case ESubQuestFailureBehavior::FailWithSubQuest:
            FailObjective();
            break;
            
        case ESubQuestFailureBehavior::RestartSubQuest:
            if (!RestartSubQuest())
            {
                UE_LOG(LogTemp, Warning, TEXT("UNerveSubQuestRuntimeObjective: Failed to restart sub-quest, failing objective"));
                FailObjective();
            }
            break;
            
        case ESubQuestFailureBehavior::IgnoreFailure:
            // Continue execution - could complete manually or through other means
            break;
    }
}

void UNerveSubQuestRuntimeObjective::OnSubQuestObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* CompletedObjective)
{
    if (CompletionBehavior == ESubQuestCompletionBehavior::CompleteOnSpecificObjective)
    {
        UE_LOG(LogTemp, Log, TEXT("UNerveSubQuestRuntimeObjective: Specific objective completed"));
        CompleteObjective();
    }
}

void UNerveSubQuestRuntimeObjective::UpdateSubQuestTracking()
{
    if (!IsValid(SubQuestRuntimeData) || !IsValid(QuestSubsystem)) return;

    bool bShouldTrack = false;
    
    switch (TrackingBehavior)
    {
        case ESubQuestTrackingBehavior::NoTracking:
            bShouldTrack = false;
            break;
            
        case ESubQuestTrackingBehavior::TrackWithParent:
            bShouldTrack = bIsCurrentlyTracked;
            break;
            
        case ESubQuestTrackingBehavior::AlwaysTrack:
            bShouldTrack = true;
            break;
    }

    // Note: Since this is a sub-quest, we handle tracking differently
    // We don't use the main quest system's tracking, but we can still
    // mark the sub-quest as tracked for UI purposes
    if (bShouldTrack && IsValid(SubQuestRuntimeData->CurrentObjective))
    {
        SubQuestRuntimeData->TrackQuest();
    }
    else if (IsValid(SubQuestRuntimeData->CurrentObjective))
    {
        SubQuestRuntimeData->UntrackQuest();
    }
}

UNerveQuestSubsystem* UNerveSubQuestRuntimeObjective::GetQuestSubsystem() const
{
    if (!IsValid(GetWorld())) return nullptr;

    const APlayerController* NewPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!IsValid(NewPlayerController)) return nullptr;

    const ULocalPlayer* LocalPlayer = NewPlayerController->GetLocalPlayer();
    if (!IsValid(LocalPlayer)) return nullptr;
    
    return LocalPlayer->GetSubsystem<UNerveQuestSubsystem>();
    
}