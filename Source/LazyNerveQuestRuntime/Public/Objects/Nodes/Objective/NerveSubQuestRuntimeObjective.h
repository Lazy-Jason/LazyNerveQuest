// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "NerveSubQuestRuntimeObjective.generated.h"

class UNerveQuestAsset;
class UNerveQuestRuntimeData;

UENUM(BlueprintType)
enum class ESubQuestCompletionBehavior : uint8
{
    /** Complete this objective when the sub-quest completes */
    CompleteOnSubQuestComplete UMETA(DisplayName = "Complete on Sub-Quest Complete"),
    
    /** Complete this objective when a specific objective in the sub-quest completes */
    CompleteOnSpecificObjective UMETA(DisplayName = "Complete on Specific Objective"),
    
    /** Manual completion - sub-quest runs but doesn't auto-complete this objective */
    Manual UMETA(DisplayName = "Manual Completion")
};

UENUM(BlueprintType)
enum class ESubQuestTrackingBehavior : uint8
{
    /** Don't track the sub-quest (hidden execution) */
    NoTracking UMETA(DisplayName = "No Tracking"),
    
    /** Track the sub-quest when this objective is tracked */
    TrackWithParent UMETA(DisplayName = "Track with Parent"),
    
    /** Always track the sub-quest regardless of parent tracking */
    AlwaysTrack UMETA(DisplayName = "Always Track")
};

UENUM(BlueprintType)
enum class ESubQuestFailureBehavior : uint8
{
    /** Fail this objective if sub-quest fails */
    FailWithSubQuest UMETA(DisplayName = "Fail with Sub-Quest"),
    
    /** Restart the sub-quest if it fails */
    RestartSubQuest UMETA(DisplayName = "Restart Sub-Quest"),
    
    /** Ignore sub-quest failure and continue */
    IgnoreFailure UMETA(DisplayName = "Ignore Failure")
};

/**
 * An objective that can run another quest as a nested sequence
 * The sub-quest runs independently without being registered in the main quest system
 */
UCLASS(BlueprintType, Blueprintable)
class LAZYNERVEQUESTRUNTIME_API UNerveSubQuestRuntimeObjective : public UNerveQuestRuntimeObjectiveBase
{
    GENERATED_BODY()

public:
    UNerveSubQuestRuntimeObjective();

protected:
    /** The quest asset to run as a sub-quest */
    UPROPERTY(EditAnywhere, Category = "Sub-Quest", meta = (DisplayName = "Sub-Quest Asset"))
    TSoftObjectPtr<UNerveQuestAsset> SubQuestAsset;

    /** How this objective should behave when the sub-quest completes */
    UPROPERTY(EditAnywhere, Category = "Sub-Quest")
    ESubQuestCompletionBehavior CompletionBehavior = ESubQuestCompletionBehavior::CompleteOnSubQuestComplete;

    /** Specific objective index to watch for completion (only used with CompleteOnSpecificObjective) */
    UPROPERTY(EditAnywhere, Category = "Sub-Quest", 
    meta = (EditCondition = "CompletionBehavior == ESubQuestCompletionBehavior::CompleteOnSpecificObjective", 
    EditConditionHides = true))
    int32 SpecificObjectiveIndex = 0;

    /** How the sub-quest should be tracked in the UI */
    UPROPERTY(EditAnywhere, Category = "Sub-Quest")
    ESubQuestTrackingBehavior TrackingBehavior = ESubQuestTrackingBehavior::TrackWithParent;

    /** How to handle sub-quest failures */
    UPROPERTY(EditAnywhere, Category = "Sub-Quest")
    ESubQuestFailureBehavior FailureBehavior = ESubQuestFailureBehavior::FailWithSubQuest;

    /** Whether to pass the parent quest's world context object to the sub-quest */
    UPROPERTY(EditAnywhere, Category = "Sub-Quest")
    bool bInheritWorldContext = true;

    /** Maximum number of times to restart the sub-quest on failure (only for RestartSubQuest behavior) */
    UPROPERTY(EditAnywhere, Category = "Sub-Quest", 
    meta = (EditCondition = "FailureBehavior == ESubQuestFailureBehavior::RestartSubQuest", EditConditionHides = true, ClampMin = "1", ClampMax = "10"))
    int32 MaxRestartAttempts = 3;

private:
    /** Runtime data for the sub-quest (not registered in main quest system) */
    UPROPERTY()
    UNerveQuestRuntimeData* SubQuestRuntimeData;

    /** Reference to the main quest subsystem */
    UPROPERTY()
    class UNerveQuestSubsystem* QuestSubsystem;

    /** Current restart attempt count */
    int32 CurrentRestartAttempts = 0;

    /** Whether this objective is currently being tracked */
    bool bIsCurrentlyTracked = false;

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
    virtual bool CanGenerateOptionals_Implementation() override { return false; }
    virtual bool IsCosmetic_Implementation() override { return false; }

    // Sub-quest specific functions
    UFUNCTION(BlueprintCallable, Category = "Sub-Quest")
    bool IsSubQuestValid() const;

    UFUNCTION(BlueprintCallable, Category = "Sub-Quest")
    UNerveQuestRuntimeData* GetSubQuestRuntimeData() const { return SubQuestRuntimeData; }

    UFUNCTION(BlueprintCallable, Category = "Sub-Quest")
    bool RestartSubQuest();

    UFUNCTION(BlueprintCallable, Category = "Sub-Quest")
    void ForceCompleteSubQuest();

    UFUNCTION(BlueprintPure, Category = "Sub-Quest")
    float GetSubQuestProgress() const;

    UFUNCTION(BlueprintPure, Category = "Sub-Quest")
    FText GetCurrentSubQuestObjectiveText() const;

protected:
    /** Initialize the sub-quest runtime data */
    bool InitializeSubQuest();

    /** Clean up the sub-quest runtime data */
    void CleanupSubQuest();

    /** Start the sub-quest execution */
    void StartSubQuest();

    /** Handle sub-quest completion */
    UFUNCTION()
    void OnSubQuestCompleted(UNerveQuestRuntimeData* CompletedQuest);

    /** Handle sub-quest failure */
    UFUNCTION()
    void OnSubQuestFailed(UNerveQuestRuntimeData* FailedQuest);

    /** Handle sub-quest objective completion (for specific objective tracking) */
    UFUNCTION()
    void OnSubQuestObjectiveCompleted(class UNerveQuestRuntimeObjectiveBase* CompletedObjective);

    /** Update tracking behavior based on settings */
    void UpdateSubQuestTracking();

    /** Get the quest subsystem reference */
    class UNerveQuestSubsystem* GetQuestSubsystem() const;
};