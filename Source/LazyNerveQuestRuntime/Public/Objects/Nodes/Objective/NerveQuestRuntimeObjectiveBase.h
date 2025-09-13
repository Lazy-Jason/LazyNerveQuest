// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "Styling/SlateBrush.h"
#include "UObject/Object.h"
#include "NerveQuestRuntimeObjectiveBase.generated.h"

class UNerveObjectiveModifier;
class UObjectiveProgressTracker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNerveQuestObjectiveAction, UNerveQuestRuntimeObjectiveBase*, ObjectiveBase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNerveQuestObjectiveProgressAction, UNerveQuestRuntimeObjectiveBase*, ObjectiveBase, float, NewProgressValue, float, MaxProgressValue);

/**
 * 
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class LAZYNERVEQUESTRUNTIME_API UNerveQuestRuntimeObjectiveBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<class UNerveQuestRuntimePin> InputPin;

	UPROPERTY()
	TArray<TObjectPtr<class UNerveQuestRuntimePin>> OutPutPin;

	UPROPERTY()
	TArray<TObjectPtr<class UNerveQuestRuntimePin>> OutOptionalPins;

	UPROPERTY()
	FVector2D Location = FVector2D();

	UPROPERTY()
	UClass* EditorClass = nullptr;

	/** Whether this objective is an optional objective */
	UPROPERTY()
	bool bIsOptionalObjective = false;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FNerveQuestObjectiveAction OnObjectiveCompleted;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FNerveQuestObjectiveAction OnObjectiveFailed;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FNerveQuestObjectiveProgressAction OnProgressChanged;

protected:

	/** The label that will be displayed for this objective in the UI. */
	UPROPERTY(EditAnywhere, Category="Generic")
	FString DisplayLabel = FString(TEXT("Default Label"));

	/** A brief tip or description for this objective, to be shown in the UI. */
	UPROPERTY(EditAnywhere, Category="Generic")
	FString DisplayTip = FString(TEXT("Default Tip for this objective"));

	/** Defines how to handle failure for this objective. */
	UPROPERTY(EditAnywhere, Category="Generic")
	EObjectiveFailureResponse FailureResponse = EObjectiveFailureResponse::FailQuest;
	
	/** Modifiers that add additional conditions to this objective */
	// should be when one fails then the whole thing fails.
    UPROPERTY(EditAnywhere, Instanced, Category="Generic")
    TArray<UNerveObjectiveModifier*> Modifiers = TArray<UNerveObjectiveModifier*>();

	/** Optional objectives to perform. */
	UPROPERTY(EditAnywhere, Category="Optional Objectives")
	TArray<FText> Optionals = TArray<FText>();

	/** How optional objective completion affects the parent */
	UPROPERTY(EditAnywhere, Category="Optional Objectives")
	EOptionalObjectiveResponse OptionalCompletionResponse = EOptionalObjectiveResponse::NoEffect;

	/** How optional objective failure affects the parent */
	UPROPERTY(EditAnywhere, Category="Optional Objectives")
	EOptionalObjectiveResponse OptionalFailureResponse = EOptionalObjectiveResponse::NoEffect;

	/** Priority for UI display (higher numbers display first) */
	UPROPERTY(EditAnywhere, Category="UI")
	int32 DisplayPriority = 0;

	/** Whether this objective should be shown in the UI when tracked */
	UPROPERTY(EditAnywhere, Category="UI")
	bool bShowInUI = true;

	/** Whether or not a progress tracker should be generated for this objective, creating a visual representation in the UI. */
	UPROPERTY(EditAnywhere, Category="UI")
	bool bGenerateProgressTracker = false;

	/** The class of the progress tracker to be used if bGenerateProgressTracker is true. This will be hidden when bGenerateProgressTracker is false. */
	UPROPERTY(EditAnywhere, Category="UI", meta=(EditCondition = "bGenerateProgressTracker", EditConditionHides = "bGenerateProgressTracker"))
	TSubclassOf<UObjectiveProgressTracker> ProgressTrackerClass = nullptr;

	/** Positioning rule for the UI to follow when inserting the progress tracking. */
	UPROPERTY(EditAnywhere, Category="UI", meta=(EditCondition = "bGenerateProgressTracker", EditConditionHides = "bGenerateProgressTracker"))
	ENerveObjectiveTrackerPosition ProgressTrackerPositioning = ENerveObjectiveTrackerPosition::Inline;
	
	/** properties that should be copied */
	TArray<FProperty*> PropertyData;

	UPROPERTY()
	TObjectPtr<class UNerveQuestAsset> ParentQuestAsset = nullptr;

	UPROPERTY()
	bool bIsConnectedAsOptional = false;

	UPROPERTY()
	TWeakObjectPtr<const UObject> WorldContextObject = nullptr;

public:

	UNerveQuestRuntimeObjectiveBase();

	virtual UWorld* GetWorld() const override;

	UNerveQuestRuntimePin* FindOutPinByCategory(FName InCategory);

	UFUNCTION(BlueprintPure, Category="Generic Objective")
	FText GetObjectiveDisplayLabel() const { return FText::FromString(DisplayLabel); }

	UFUNCTION(BlueprintPure, Category="Generic Objective")
	FText GetObjectiveDisplayTip() const { return FText::FromString(DisplayTip); }

	/** Set whether this objective is being used as an optional */
	UFUNCTION(BlueprintCallable, Category = "Quest Objective")
	void SetConnectedAsOptional(const bool bConnected) { bIsConnectedAsOptional = bConnected; }

	/** Check if this objective is being used as an optional */
	UFUNCTION(BlueprintPure, Category = "Quest Objective")
	bool IsConnectedAsOptional() const { return bIsConnectedAsOptional; }
	
	UFUNCTION(BlueprintPure, Category="Generic Objective")
	EObjectiveFailureResponse GetObjectiveFailureResponse() const { return FailureResponse; }

	UFUNCTION(BlueprintPure, Category="Generic Objective")
	TArray<UNerveObjectiveModifier*> GetObjectiveModifiers() const { return Modifiers; }

	UFUNCTION(BlueprintPure, Category="Generic Objective")
	int32 GetDisplayPriority() const { return DisplayPriority; }
	
	UFUNCTION(BlueprintPure, Category="Generic Objective")
	EOptionalObjectiveResponse GetOptionalCompletionResponse() const { return OptionalCompletionResponse; }

	UFUNCTION(BlueprintPure, Category="Generic Objective")
	EOptionalObjectiveResponse GetOptionalFailureResponse() const { return OptionalFailureResponse; }

	UFUNCTION(BlueprintPure, Category="Generic Objective")
	bool GetIsOptionalObjective() const { return bIsOptionalObjective; }

	UFUNCTION(BlueprintPure, Category="Generic Objective")
	bool GetShowInUI() const { return bShowInUI; }

	UFUNCTION(BlueprintPure, Category="Generic Objective")
	TArray<FText> GetObjectiveOptionals();

	UFUNCTION(BlueprintPure, Category="UI Objective")
	bool AllowGenerateProgressTracker() const { return bGenerateProgressTracker; }

	UFUNCTION(BlueprintPure, Category="UI Objective")
	TSubclassOf<UObjectiveProgressTracker> GetProgressTrackerClass() const { return ProgressTrackerClass; }

	UFUNCTION(BlueprintPure, Category="UI Objective")
	ENerveObjectiveTrackerPosition GetProgressTrackerPositioning() const { return ProgressTrackerPositioning; }
	
	UFUNCTION(BlueprintNativeEvent, Category="Objective Editor MetaData")
	FText GetObjectiveName();
	
	UFUNCTION(BlueprintNativeEvent, Category="Objective Editor MetaData")
	FText GetObjectiveDescription();

	UFUNCTION(BlueprintNativeEvent, Category="Objective Editor MetaData")
	FText GetObjectiveCategory();

	UFUNCTION(BlueprintNativeEvent, Category="Objective Editor MetaData")
	FSlateBrush GetObjectiveBrush() const;

	const UObject* GetWorldContextObject() const { return WorldContextObject.Get(); }
	void SetWorldContextObject(const UObject* NewWorldContextObject) { WorldContextObject = NewWorldContextObject; }

	///////// 

	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	void ExecuteObjective(class UNerveQuestAsset* QuestManager);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	void PauseObjective();

	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	void ResumeObjective();

	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	void MarkAsTracked(bool TrackValue);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	void CleanUpObjective();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteObjective();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailObjective();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ExecuteProgress(float NewValue = 1, float MaxValue = 1);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest Editor")
	bool CanGenerateOptionals();

	UFUNCTION(BlueprintNativeEvent, Category = "Quest Editor")
	bool IsCosmetic();

#if WITH_EDITOR
	UFUNCTION(BlueprintNativeEvent, Category = "Quest Editor", meta=(WorldContext = "WorldContextObject"))
	void StartObjectivePreview(UObject* PreviewWorldContextObject);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest Editor", meta=(WorldContext = "WorldContextObject"))
	void StopObjectivePreview(UObject* PreviewWorldContextObject);
#endif
	
public:
	TArray<FString> GetPropertyDescription() const;
};
