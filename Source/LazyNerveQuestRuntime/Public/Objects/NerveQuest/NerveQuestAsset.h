// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "Objects/Graph/NerveQuestRuntimeGraph.h"
#include "UObject/Object.h"
#include "NerveQuestAsset.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LAZYNERVEQUESTRUNTIME_API UNerveQuestAsset : public UObject
{
	GENERATED_BODY()

public:
	/** The title of the quest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Information")
	FString QuestTitle = FString(TEXT("Default Quest Title"));

	/** Detailed description of the quest objectives and context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Information", meta = (MultiLine = "true"))
	FString QuestDescription = FString(TEXT("Default Quest Description"));

	/** The type of quest (Main or Side) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Information")
	ENerveQuestTypes QuestType = ENerveQuestTypes::MainQuest;

	/** Indicates if the quest can be repeated after completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Information")
	bool bRepeatable = false;

	/** The difficulty level of the quest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Information")
	ENerveQuestDifficulty QuestDifficulty = ENerveQuestDifficulty::Medium;

	/** The rewards granted from completing this quest */
	UPROPERTY(EditAnywhere, Instanced,  BlueprintReadWrite, Category = "Quest Information")
	TArray<TObjectPtr<class UNerveQuestRewardBase>> QuestRewards = TArray<TObjectPtr<class UNerveQuestRewardBase>>();
	
	UPROPERTY()
	TObjectPtr<UNerveQuestRuntimeGraph> RuntimeGraph = nullptr;

public:
	
	UNerveQuestRuntimeGraph* GetRuntimeGraph() const { return RuntimeGraph.Get(); }
	void SetRuntimeGraph(UNerveQuestRuntimeGraph* NewRuntimeGraph) { if(IsValid(NewRuntimeGraph)) RuntimeGraph = NewRuntimeGraph; }

	UFUNCTION(BlueprintPure, Category="QuestAsset Helper")
	TArray<UNerveQuestRuntimeObjectiveBase*> GetQuestObjectives() const;
	
#if WITH_EDITOR // WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void SetPreEditChangeListener(const std::function<void()>& NewPreEditChangeListener) { PreEditListener = NewPreEditChangeListener; }
	void SetPostEditChangeListener(const std::function<void()>& NewPostSaveListener) { PostEditListener = NewPostSaveListener; }
#endif // WITH_EDITOR

	// Start UObject Save
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	void SetPreSaveListener(const std::function<void()>& NewPreSaveListener) { PreSaveListener = NewPreSaveListener; }

	// End UObject Save
protected:
	std::function<void()> PreSaveListener = nullptr;
	std::function<void()> PreEditListener = nullptr;
	std::function<void()> PostEditListener = nullptr;
};