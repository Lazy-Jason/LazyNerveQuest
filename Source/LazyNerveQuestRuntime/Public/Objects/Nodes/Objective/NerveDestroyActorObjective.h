 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NerveQuestRuntimeObjectiveBase.h"
#include "NerveDestroyActorObjective.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UNerveDestroyActorObjective : public UNerveQuestRuntimeObjectiveBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Destroy Objective")
	TSubclassOf<AActor> ActorToDestroy = nullptr;

	UPROPERTY(EditAnywhere, Category="Destroy Objective", meta=(ClampMin = 1))
	int32 AmountToDestroy = 1;

private:
	int32 CurrentAmount = 0;

	UPROPERTY()
	TArray<AActor*> OutActors;

public:
	UNerveDestroyActorObjective();
	
	virtual FText GetObjectiveName_Implementation() override;
	virtual FText GetObjectiveDescription_Implementation() override;
	virtual FText GetObjectiveCategory_Implementation() override;
	virtual FSlateBrush GetObjectiveBrush_Implementation() const override;
	
	virtual void ExecuteObjective_Implementation(UNerveQuestAsset* NerveQuestAsset) override;
	virtual void MarkAsTracked_Implementation(bool TrackValue) override;

protected:
	UFUNCTION()
	void HandleActorDestroy(AActor* DestroyedActor);
};
