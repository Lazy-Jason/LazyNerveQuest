// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NerveObjectiveModifier.generated.h"

class UNerveQuestRuntimeObjectiveBase;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class LAZYNERVEQUESTRUNTIME_API UNerveObjectiveModifier : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	const UObject* WorldContextObject = nullptr;

public:
	virtual UWorld* GetWorld() const override;
	
	/** Called when the objective starts */
	virtual void OnObjectiveStart(UNerveQuestRuntimeObjectiveBase* Objective);
    
	/** Called to check if the modifier's conditions are met */
	virtual bool CheckCondition(UNerveQuestRuntimeObjectiveBase* Objective) { return true; }
    
	/** Called when the objective updates progress */
	virtual void OnObjectiveProgress(UNerveQuestRuntimeObjectiveBase* Objective);
    
	/** Called when the objective is completed to verify if the modifier conditions were met */
	virtual bool ValidateCompletion(UNerveQuestRuntimeObjectiveBase* Objective) { return true; }
	
	const UObject* GetWorldContextObject() const { return WorldContextObject; }
	
	void SetWorldContextObject(const UObject* NewWorldContextObject) { WorldContextObject = NewWorldContextObject; }
};
