// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "NerveQuestObjectiveNodeBase.h"
#include "NerveQuestSequenceObjective.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTEDITOR_API UNerveQuestSequenceObjective : public UNerveQuestObjectiveNodeBase
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
};
