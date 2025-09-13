// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Nodes/Objective/NerveDestroyActorObjective.h"
#include "Kismet/GameplayStatics.h"

UNerveDestroyActorObjective::UNerveDestroyActorObjective()
{}

FText UNerveDestroyActorObjective::GetObjectiveName_Implementation()
{
	return FText::FromString(TEXT("Destroy Actors"));
}

FText UNerveDestroyActorObjective::GetObjectiveDescription_Implementation()
{
	return FText::FromString(TEXT("Destroy specified actors of class from the game world."));
}

FText UNerveDestroyActorObjective::GetObjectiveCategory_Implementation()
{
	return FText::FromString(TEXT("Primitive Objectives"));
}

FSlateBrush UNerveDestroyActorObjective::GetObjectiveBrush_Implementation() const
{
	return Super::GetObjectiveBrush_Implementation();
}

void UNerveDestroyActorObjective::ExecuteObjective_Implementation(UNerveQuestAsset* NerveQuestAsset)
{
	Super::ExecuteObjective_Implementation(NerveQuestAsset);
	CurrentAmount = 0;
	OutActors.Empty();
	
	// Check if the World is valid
	if (!IsValid(GetWorld()) || !IsValid(ActorToDestroy))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid World in UNerveDestroyActorObjective::ExecuteObjective_Implementation"));
		FailObjective();
		return;
	}
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorToDestroy, OutActors);
	if(OutActors.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No actors to destroy found in UNerveDestroyActorObjective::ExecuteObjective_Implementation"));
		FailObjective();
		return;
	}

	for (AActor* OutActor : OutActors)
	{
		OutActor->OnDestroyed.AddDynamic(this, &UNerveDestroyActorObjective::HandleActorDestroy);
	}
}

void UNerveDestroyActorObjective::MarkAsTracked_Implementation(const bool TrackValue)
{
	Super::MarkAsTracked_Implementation(TrackValue);
}

void UNerveDestroyActorObjective::HandleActorDestroy(AActor* DestroyedActor)
{
	CurrentAmount ++;
	ExecuteProgress(CurrentAmount, AmountToDestroy);
	if(CurrentAmount >= AmountToDestroy)
	{
		CurrentAmount = 0;

		for (AActor* OutActor : OutActors)
		{
			// AT THIS POINT THE ACTOR IS ALREADY DESTROYED BUT IF FOR SOME REASON THE DESTROY DELEGATE IS MANUALLY CALLED
			// AND THE ACTOR ISN'T DESTROYED THEN WE UNBIND.

			if(!IsValid(OutActor)) continue;
			OutActor->OnDestroyed.RemoveDynamic(this, &UNerveDestroyActorObjective::HandleActorDestroy);
		}
		OutActors.Empty();
		CompleteObjective();
		return;
	}
}
