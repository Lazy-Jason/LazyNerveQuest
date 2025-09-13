// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"
#include "UObject/Object.h"
#include "NerveQuestRuntimeGraph.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UNerveQuestRuntimeGraph : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<UNerveQuestRuntimeObjectiveBase>> GraphNodes = TArray<TObjectPtr<UNerveQuestRuntimeObjectiveBase>>();
};
