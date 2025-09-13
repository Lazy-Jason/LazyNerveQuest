// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "GameQuestObjectiveContainer.generated.h"

class UNerveQuestRuntimeData;
class UGameQuestObjectiveItem;
/**
 * 
 */
UCLASS(Abstract)
class LAZYNERVEQUESTRUNTIME_API UGameQuestObjectiveContainer : public UUserWidget
{
	GENERATED_BODY()

    // Track objective items for easier management
    UPROPERTY()
    TMap<const UNerveObjectiveRuntimeData*, UGameQuestObjectiveItem*> ObjectiveItems;

public:
	// Enhanced initialization for multiple objectives
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    void InitQuestObjective(const UNerveQuestRuntimeData* QuestObjective, const TArray<UNerveObjectiveRuntimeData*>& DisplayableObjectives);
    virtual void InitQuestObjective_Implementation(const UNerveQuestRuntimeData* QuestObjective, const TArray<UNerveObjectiveRuntimeData*>& DisplayableObjectives);

    // Backward compatibility - single objective initialization
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    void InitQuestObjectiveSingle(const UNerveQuestRuntimeData* QuestObjective);
    virtual void InitQuestObjectiveSingle_Implementation(const UNerveQuestRuntimeData* QuestObjective);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    void UnInitQuestObjective(const UNerveQuestRuntimeData* QuestObjective);
    virtual void UnInitQuestObjective_Implementation(const UNerveQuestRuntimeData* QuestObjective);

    // New functions for managing individual objectives
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    void AddObjectiveItem(UNerveObjectiveRuntimeData* ObjectiveToAdd);
    virtual void AddObjectiveItem_Implementation(UNerveObjectiveRuntimeData* ObjectiveToAdd);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    void RemoveObjectiveItem(const UNerveObjectiveRuntimeData* ObjectiveToRemove);
    virtual void RemoveObjectiveItem_Implementation(const UNerveObjectiveRuntimeData* ObjectiveToRemove);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    void UpdateObjectiveItem(const UNerveObjectiveRuntimeData* ObjectiveToUpdate);
    virtual void UpdateObjectiveItem_Implementation(const UNerveObjectiveRuntimeData* ObjectiveToUpdate);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective Container")
    void SetQuestTitle(const FString& NewTitle);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective Container")
    UGameQuestObjectiveItem* FindObjectiveItem(const UNerveObjectiveRuntimeData* Objective) const;

protected:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    UTextBlock* GetQuestTitleBlock();
    virtual UTextBlock* GetQuestTitleBlock_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    UPanelWidget* GetQuestObjectiveContainer();
    virtual UPanelWidget* GetQuestObjectiveContainer_Implementation();

    // New: Container specifically for main objectives
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    UPanelWidget* GetMainObjectiveContainer();
    virtual UPanelWidget* GetMainObjectiveContainer_Implementation();

    // New: Container specifically for optional objectives
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    UPanelWidget* GetOptionalObjectiveContainer();
    virtual UPanelWidget* GetOptionalObjectiveContainer_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    TSubclassOf<UGameQuestObjectiveItem> GetQuestObjectiveItemClass();
    virtual TSubclassOf<UGameQuestObjectiveItem> GetQuestObjectiveItemClass_Implementation();

    // New: Different classes for different objective types
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    TSubclassOf<UGameQuestObjectiveItem> GetMainObjectiveItemClass();
    virtual TSubclassOf<UGameQuestObjectiveItem> GetMainObjectiveItemClass_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Container")
    TSubclassOf<UGameQuestObjectiveItem> GetOptionalObjectiveItemClass();
    virtual TSubclassOf<UGameQuestObjectiveItem> GetOptionalObjectiveItemClass_Implementation();

private:
    void GenerateQuestObjectives(const TArray<UNerveObjectiveRuntimeData*>& Objectives);
};
