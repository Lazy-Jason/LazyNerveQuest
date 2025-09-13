// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/Factory.h"
#include "NerveQuestObjectiveFactory.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTEDITOR_API UNerveQuestObjectiveFactory : public UBlueprintFactory
{
	GENERATED_BODY()
	
public:
	UNerveQuestObjectiveFactory(const FObjectInitializer& ObjectInitializer);
	virtual bool CanCreateNew() const override { return true; }
};
