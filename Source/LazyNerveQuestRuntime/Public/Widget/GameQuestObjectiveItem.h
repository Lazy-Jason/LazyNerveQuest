// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameQuestObjectiveItem.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UGameQuestObjectiveItem : public UUserWidget
{
	GENERATED_BODY()

private:
    UPROPERTY()
    UNerveObjectiveRuntimeData* ParentPerformingObjective = nullptr;

    UPROPERTY()
    TArray<UGameQuestObjectiveItem*> AllOptionalObjectiveWidget = TArray<UGameQuestObjectiveItem*>();

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    void InitializeObjectiveItem(UNerveObjectiveRuntimeData* PerformingObjective);
    virtual void InitializeObjectiveItem_Implementation(UNerveObjectiveRuntimeData* PerformingObjective);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    void UnInitializeObjectiveItem();
    virtual void UnInitializeObjectiveItem_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    void ConstructObjectiveLook();
    virtual void ConstructObjectiveLook_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    void ConstructOptionalLook();
    virtual void ConstructOptionalLook_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    void GenerateOptionalObjectives();
    virtual void GenerateOptionalObjectives_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    void RecalculateObjectiveState();
    virtual void RecalculateObjectiveState_Implementation();

    UFUNCTION(BlueprintPure, Category = "Quest Objective Item")
    UNerveObjectiveRuntimeData* GetParentPerformingObjective() const { return ParentPerformingObjective; }

    UFUNCTION(BlueprintPure, Category = "Quest Objective Item")
    TArray<UGameQuestObjectiveItem*> GetAllOptionalObjectiveWidget() const { return AllOptionalObjectiveWidget; }

    UFUNCTION(BlueprintCallable, Category = "Quest Objective Item")
    bool IsOptionalObjective() const;

protected:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    UTextBlock* GetObjectiveTitleBlock();
    virtual UTextBlock* GetObjectiveTitleBlock_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    UTextLayoutWidget* GetObjectiveTipBlock();
    virtual UTextLayoutWidget* GetObjectiveTipBlock_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    TSubclassOf<UGameQuestObjectiveItem> GetOptionalObjectiveClass();
    virtual TSubclassOf<UGameQuestObjectiveItem> GetOptionalObjectiveClass_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Objective Item")
    UPanelWidget* GetOptionalObjectiveContainer();
    virtual UPanelWidget* GetOptionalObjectiveContainer_Implementation();

    UFUNCTION()
    void OnObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* ObjectiveBase);

    UFUNCTION()
    void OnObjectiveFailed(UNerveQuestRuntimeObjectiveBase* ObjectiveBase);
};
