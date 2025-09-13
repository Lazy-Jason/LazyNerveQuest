// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NerveQuestRuntimeObjectiveBase.h"
#include "NerveEntryObjective.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UNerveEntryObjective : public UNerveQuestRuntimeObjectiveBase
{
	GENERATED_BODY()

public:
	virtual FText GetObjectiveName_Implementation() override;
	virtual FText GetObjectiveDescription_Implementation() override;
	virtual bool CanGenerateOptionals_Implementation() override { return false; }
	virtual bool IsCosmetic_Implementation() override { return true; }

	virtual void ExecuteObjective_Implementation(UNerveQuestAsset* NerveQuestAsset) override;
};
