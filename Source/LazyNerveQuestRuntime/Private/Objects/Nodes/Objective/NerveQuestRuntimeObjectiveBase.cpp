// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"
#include "BlueprintNodeHelpers.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"
#include "Objects/Pin/NerveQuestRuntimePin.h"
#include "Setting/NerveQuestRuntimeSetting.h"

UNerveQuestRuntimeObjectiveBase::UNerveQuestRuntimeObjectiveBase()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		BlueprintNodeHelpers::CollectPropertyData(this, StaticClass(), PropertyData);
	}
}

UWorld* UNerveQuestRuntimeObjectiveBase::GetWorld() const
{
	// 1. Try stored world context object
	if (const UObject* WorldContext = WorldContextObject.Get())
	{
		if (UWorld* World = WorldContext->GetWorld())
		{
			return World;
		}
	}
	
	// 2. Try parent quest asset's world
	if (IsValid(ParentQuestAsset))
	{
		if (UWorld* World = ParentQuestAsset->GetWorld())
		{
			return World;
		}
	}
	
	// 3. Try to get world from outer chain
	if (UWorld* World = Super::GetWorld())
	{
		return World;
	}
	
	// 4. Last resort - try to find any valid world
	if (UWorld* World = GWorld)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld: Using GWorld as fallback for objective %s"), *GetName());
		return World;
	}
	
	return nullptr;
}

UNerveQuestRuntimePin* UNerveQuestRuntimeObjectiveBase::FindOutPinByCategory(const FName InCategory)
{
	if (OutPutPin.IsEmpty()) return nullptr;

	for (const auto Element : OutPutPin)
	{
		if (Element->PinCategory != InCategory) continue;
		return Element;
	}
	return nullptr;
}

TArray<FText> UNerveQuestRuntimeObjectiveBase::GetObjectiveOptionals()
{
	// Return empty array if this objective is being used as an optional
	if (bIsConnectedAsOptional)
	{
		return TArray<FText>();
	}
    
	return Optionals; 
}

FText UNerveQuestRuntimeObjectiveBase::GetObjectiveName_Implementation()
{
	return FText::FromString(GetName());
}

FText UNerveQuestRuntimeObjectiveBase::GetObjectiveDescription_Implementation()
{
	return FText::FromString(FString(TEXT("Default Description")));
}

FText UNerveQuestRuntimeObjectiveBase::GetObjectiveCategory_Implementation()
{
	return FText::FromString(FString(TEXT("Misc")));
}

FSlateBrush UNerveQuestRuntimeObjectiveBase::GetObjectiveBrush_Implementation() const
{ return FSlateBrush(); }

void UNerveQuestRuntimeObjectiveBase::ExecuteObjective_Implementation(UNerveQuestAsset* QuestManager)
{
	ParentQuestAsset = QuestManager;
}

void UNerveQuestRuntimeObjectiveBase::PauseObjective_Implementation()
{}

void UNerveQuestRuntimeObjectiveBase::ResumeObjective_Implementation()
{}

void UNerveQuestRuntimeObjectiveBase::CleanUpObjective_Implementation()
{}

void UNerveQuestRuntimeObjectiveBase::MarkAsTracked_Implementation(bool TrackValue)
{}

void UNerveQuestRuntimeObjectiveBase::CompleteObjective()
{
	OnObjectiveCompleted.Broadcast(this);
}

void UNerveQuestRuntimeObjectiveBase::FailObjective()
{
	OnObjectiveFailed.Broadcast(this);
}

void UNerveQuestRuntimeObjectiveBase::ExecuteProgress(const float NewValue, const float MaxValue)
{
	OnProgressChanged.Broadcast(this, NewValue, MaxValue);
}

#if WITH_EDITOR
void UNerveQuestRuntimeObjectiveBase::StartObjectivePreview_Implementation(UObject* PreviewWorldContextObject)
{}

void UNerveQuestRuntimeObjectiveBase::StopObjectivePreview_Implementation(UObject* PreviewWorldContextObject)
{}
#endif

bool UNerveQuestRuntimeObjectiveBase::CanGenerateOptionals_Implementation()
{ return !bIsConnectedAsOptional; }

bool UNerveQuestRuntimeObjectiveBase::IsCosmetic_Implementation()
{ return false; }

TArray<FString> UNerveQuestRuntimeObjectiveBase::GetPropertyDescription() const
{
	TArray<FString> ReturnDescLines;
	const UNerveQuestRuntimeObjectiveBase* PropertyClass = static_cast<UNerveQuestRuntimeObjectiveBase*>(GetClass()->GetDefaultObject());
	if (!IsValid(PropertyClass)) return ReturnDescLines;

	const UClass* StopAtClass = StaticClass();
	const FString PropertyDesc = BlueprintNodeHelpers::CollectPropertyDescription(this, StopAtClass, PropertyClass->PropertyData);
    
	if (!PropertyDesc.IsEmpty())
	{
		PropertyDesc.ParseIntoArray(ReturnDescLines, TEXT("\n"), true);
	}

	return ReturnDescLines;
}
