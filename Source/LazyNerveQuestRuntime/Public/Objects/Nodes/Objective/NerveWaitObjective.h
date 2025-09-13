// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "NerveQuestRuntimeObjectiveBase.h"
#include "NerveWaitObjective.generated.h"

/**
 * A quest objective that introduces a delay before completion.
 * This objective pauses quest progression for a specified duration, with an option to keep the UI displayed or hide it during the wait.
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UNerveWaitObjective : public UNerveQuestRuntimeObjectiveBase
{
    GENERATED_BODY()

protected:
    /** The duration in seconds to wait before completing the objective. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wait Objective", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float WaitDuration = 5.0f;

    /** Whether to keep the quest tracking UI displayed during the wait period. If false, the UI will be hidden during the wait. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wait Objective")
    bool bKeepUIDisplayed = false;

    /** The class of the progress tracker to be used if bGenerateProgressTracker is true. This will be hidden when bGenerateProgressTracker is false. */
    UPROPERTY(EditAnywhere, Category="UI", meta=(EditCondition = "bGenerateProgressTracker", EditConditionHides = "bGenerateProgressTracker"))
    float ProgressInterval = 0.02;

    // Timer handle for managing the wait duration
    FTimerHandle WaitTimerHandle;

    float CurrentWaitDuration = 0;

public:
    /** Default constructor. Initializes default values for properties. */
    UNerveWaitObjective();

    // Override base class functions for objective metadata
    /** Returns the display name of the objective. */
    virtual FText GetObjectiveName_Implementation() override;

    /** Returns a brief description of the objective, including the wait duration. */
    virtual FText GetObjectiveDescription_Implementation() override;

    /** Returns the category of the objective for organizational purposes. */
    virtual FText GetObjectiveCategory_Implementation() override;

    /** Returns the icon brush used to represent the objective in the UI. */
    virtual FSlateBrush GetObjectiveBrush_Implementation() const override;

    /**
     * Initializes and starts the objective, setting up the wait timer and handling UI visibility.
     * If the wait duration is zero or negative, the objective completes immediately.
     */
    virtual void ExecuteObjective_Implementation(UNerveQuestAsset* NerveQuestAsset) override;

protected:
    /** Callback function triggered when the wait duration has elapsed. Completes the objective. */
    void OnWaitComplete();

    void UpdateUI(bool Visible);
};