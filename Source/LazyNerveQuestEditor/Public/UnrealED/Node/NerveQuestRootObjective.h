// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NerveQuestObjectiveNodeBase.h"
#include "NerveQuestRootObjective.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTEDITOR_API UNerveQuestRootObjective : public UNerveQuestObjectiveNodeBase
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;

protected:
	virtual bool CanUserDeleteNode() const override;
	virtual bool CanDuplicateNode() const override;
	virtual bool CanPasteHere(const UEdGraph* TargetGraph) const override;
};
