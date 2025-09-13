// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "NerveQuestRuntimeObjectiveBase.h"
#include "NerveSequenceRuntimeObjective.generated.h"

UENUM(BlueprintType)
enum class EObjectiveExecutionType : uint8
{
    Sequential,     // One after another
    Parallel,       // All simultaneously
};

/**
 * Sequence objective that can execute child objectives either sequentially or in parallel.
 * Sequential: Executes one child at a time, advancing to the next when current completes
 * Parallel: Executes all children simultaneously, completes when all are done
 */
UCLASS(BlueprintType, Blueprintable)
class LAZYNERVEQUESTRUNTIME_API UNerveSequenceRuntimeObjective : public UNerveQuestRuntimeObjectiveBase
{
    GENERATED_BODY()

public:
    UNerveSequenceRuntimeObjective();

    // Execution type for this sequence
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
    EObjectiveExecutionType ExecutionType = EObjectiveExecutionType::Parallel;

protected:
    // Current quest asset being executed
    UPROPERTY()
    UNerveQuestAsset* CurrentQuestAsset;

    // All child objectives collected from output pins
    UPROPERTY()
    TArray<UNerveQuestRuntimeObjectiveBase*> ChildObjectives;

    // Currently active child objectives
    UPROPERTY()
    TArray<UNerveQuestRuntimeObjectiveBase*> ActiveChildObjectives;

    // Completed child objectives
    UPROPERTY()
    TArray<UNerveQuestRuntimeObjectiveBase*> CompletedChildObjectives;

    // Current index for sequential execution
    UPROPERTY()
    int32 CurrentSequentialIndex;

    // Number of completed children
    UPROPERTY()
    int32 CompletedChildCount;

    // Number of failed children
    UPROPERTY()
    int32 FailedChildCount;

    // Whether the sequence has started
    UPROPERTY()
    bool bSequenceStarted;

public:
    // UNerveQuestRuntimeObjectiveBase interface
    virtual FText GetObjectiveName_Implementation() override;
    virtual FText GetObjectiveDescription_Implementation() override;
    virtual FText GetObjectiveCategory_Implementation() override;
    virtual FSlateBrush GetObjectiveBrush_Implementation() const override;

    virtual void ExecuteObjective_Implementation(UNerveQuestAsset* QuestManager) override;
    virtual void PauseObjective_Implementation() override;
    virtual void ResumeObjective_Implementation() override;
    virtual void MarkAsTracked_Implementation(bool TrackValue) override;
    virtual void CleanUpObjective_Implementation() override;
    virtual bool CanGenerateOptionals_Implementation() override { return true; }

    // Sequence-specific methods
    UFUNCTION(BlueprintCallable, Category = "Sequence")
    TArray<UNerveQuestRuntimeObjectiveBase*> GetActiveChildObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Sequence")
    TArray<UNerveQuestRuntimeObjectiveBase*> GetCompletedChildObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Sequence")
    float GetSequenceProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Sequence")
    bool IsSequenceComplete() const;

protected:
    // Internal methods
    bool CollectChildObjectives();
    void AccumulateAllSequenceChildren(class UNerveQuestRuntimeObjectiveBase* ParentNode);
    void ExecuteSequential();
    void ExecuteParallel();
    void RestartSequence();

    // Child objective event handlers
    UFUNCTION()
    void OnChildObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* CompletedObjective);

    UFUNCTION()
    void OnChildObjectiveFailed(UNerveQuestRuntimeObjectiveBase* FailedObjective);
};