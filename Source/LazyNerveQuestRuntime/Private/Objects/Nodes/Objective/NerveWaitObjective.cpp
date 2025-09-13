// // Copyright (C) 2024 Job Omondiale - All Rights Reserved


#include "Objects/Nodes/Objective/NerveWaitObjective.h"
#include "TimerManager.h"
#include "LazyNerveRuntimeQuestStyle.h"
#include "Components/SlateWrapperTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/NerveQuestSubsystem.h"
#include "Widget/QuestScreen.h"

UNerveWaitObjective::UNerveWaitObjective()
{
    // Initialize default values
    WaitDuration = 5.0f;
    bKeepUIDisplayed = false;
}

FText UNerveWaitObjective::GetObjectiveName_Implementation()
{
    return FText::FromString(TEXT("Wait"));
}

FText UNerveWaitObjective::GetObjectiveDescription_Implementation()
{
    // Format the description to include the wait duration dynamically
    return FText::Format(NSLOCTEXT("QuestObjectives", "WaitDescription", "Wait for {0} seconds."), FText::AsNumber(WaitDuration));
}

FText UNerveWaitObjective::GetObjectiveCategory_Implementation()
{
    return FText::FromString(TEXT("Primitive Objectives|Utility"));
}

FSlateBrush UNerveWaitObjective::GetObjectiveBrush_Implementation() const
{
    // Retrieve the wait objective icon from the style set, fallback to default if not found
    const FSlateBrush* Brush = FLazyNerveRuntimeQuestStyle::Get().GetBrush("QuestWaitBrush");
    return Brush ? *Brush : FSlateBrush();
}

void UNerveWaitObjective::ExecuteObjective_Implementation(UNerveQuestAsset* NerveQuestAsset)
{
    Super::ExecuteObjective_Implementation(NerveQuestAsset);

    // Complete immediately if wait duration is invalid (zero or negative)
    if (WaitDuration <= 0.0f)
    {
        CompleteObjective();
        return;
    }

    // Hide the tracking UI if bKeepUIDisplayed is false
    if (!bKeepUIDisplayed)
    {
        UpdateUI(false);
    }

    const float InTimerRate = AllowGenerateProgressTracker()? ProgressInterval : WaitDuration;
    if (AllowGenerateProgressTracker()) ExecuteProgress(CurrentWaitDuration, WaitDuration);

    // Set up a timer to trigger completion after the wait duration
    GetWorld()->GetTimerManager().SetTimer(WaitTimerHandle, this, &UNerveWaitObjective::OnWaitComplete, InTimerRate, AllowGenerateProgressTracker());
}

void UNerveWaitObjective::OnWaitComplete()
{
    if (!AllowGenerateProgressTracker())
    {
        UpdateUI(true);
        CompleteObjective();
        return;
    }

    CurrentWaitDuration += ProgressInterval;
    ExecuteProgress(CurrentWaitDuration, WaitDuration);
    if (CurrentWaitDuration >= WaitDuration)
    {
        // Complete the objective when the timer expires
        UpdateUI(true);
        CompleteObjective();
    }
    
    // Note: The quest system will handle UI updates for the next objective or quest completion
}

void UNerveWaitObjective::UpdateUI(bool Visible)
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveWaitObjective::ExecuteObjective_Implementation - Invalid PlayerController"));
        FailObjective();
        return;
    }
    
    UNerveQuestSubsystem* QuestSubsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UNerveQuestSubsystem>();
    if (!QuestSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UNerveWaitObjective::ExecuteObjective_Implementation - Invalid QuestSubsystem"));
        FailObjective();
        return;
    }
    
    // Hide the tracking UI if bKeepUIDisplayed is false
    if (!bKeepUIDisplayed && QuestSubsystem->GetQuestScreen())
    {
        QuestSubsystem->GetQuestScreen()->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}
