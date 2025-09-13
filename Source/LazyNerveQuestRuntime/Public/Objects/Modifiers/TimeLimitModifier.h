// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "NerveObjectiveModifier.h"
#include "TimeLimitModifier.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UTimeLimitModifier : public UNerveObjectiveModifier
{
	GENERATED_BODY()

	/* in seconds */
	UPROPERTY(EditAnywhere, Category="Time Limit")
	float TimeLimit = 60.0f;
	
	float StartTime = 0.0f;

public:
	virtual void OnObjectiveStart(UNerveQuestRuntimeObjectiveBase* Objective) override;
	virtual bool CheckCondition(UNerveQuestRuntimeObjectiveBase* Objective) override;
};
