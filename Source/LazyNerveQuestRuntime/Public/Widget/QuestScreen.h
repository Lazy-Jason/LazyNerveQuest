// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "QuestScreen.generated.h"

class UNerveQuestRuntimeData;
class UGameQuestObjectiveContainer;
/**
 * 
 */
UCLASS(Abstract)
class LAZYNERVEQUESTRUNTIME_API UQuestScreen : public UUserWidget
{
	GENERATED_BODY()

    // Track active quest containers
    UPROPERTY()
    TMap<const UNerveQuestRuntimeData*, UGameQuestObjectiveContainer*> ActiveQuestContainers;
    
public:
    // Enhanced initialization that can handle multiple objectives
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Screen")
    void InitQuestObjective(const UNerveQuestRuntimeData* QuestToTrack, const TArray<UNerveObjectiveRuntimeData*>& DisplayableObjectives);
    virtual void InitQuestObjective_Implementation(const UNerveQuestRuntimeData* QuestToTrack, const TArray<UNerveObjectiveRuntimeData*>& DisplayableObjectives);

    // Backward compatibility - single objective initialization
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Screen")
    void InitQuestObjectiveSingle(const UNerveQuestRuntimeData* QuestToTrack);
    virtual void InitQuestObjectiveSingle_Implementation(const UNerveQuestRuntimeData* QuestToTrack);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Screen")
    void UnInitQuestObjective(const UNerveQuestRuntimeData* QuestToTrack);
    virtual void UnInitQuestObjective_Implementation(const UNerveQuestRuntimeData* QuestToTrack);

    // New function to update specific objective
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Screen")
    void UpdateObjective(const UNerveObjectiveRuntimeData* ObjectiveToUpdate);
    virtual void UpdateObjective_Implementation(const UNerveObjectiveRuntimeData* ObjectiveToUpdate);

    // New function to remove specific objective
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Screen")
    void RemoveObjective(const UNerveObjectiveRuntimeData* ObjectiveToRemove);
    virtual void RemoveObjective_Implementation(const UNerveObjectiveRuntimeData* ObjectiveToRemove);

protected:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Screen")
    UPanelWidget* GetQuestContainer();
    virtual UPanelWidget* GetQuestContainer_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Screen")
    TSubclassOf<UGameQuestObjectiveContainer> GetQuestObjectiveClassContainer();
    virtual TSubclassOf<UGameQuestObjectiveContainer> GetQuestObjectiveClassContainer_Implementation();

    void CleanupInvalidEntries();
};
