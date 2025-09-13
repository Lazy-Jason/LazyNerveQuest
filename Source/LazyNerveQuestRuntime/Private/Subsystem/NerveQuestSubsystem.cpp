#include "Subsystem/NerveQuestSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Interface/NerveQuestReceiver.h"
#include "Widget/ObjectiveProgressTracker.h"
#include "Kismet/GameplayStatics.h"
#include "Setting/NerveQuestRuntimeSetting.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"
#include "Objects/Nodes/Objective/NerveEntryObjective.h"
#include "Objects/Pin/NerveQuestRuntimePin.h"
#include "Objects/Rewards/NerveQuestRewardBase.h"
#include "Widget/QuestScreen.h"

void UNerveQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// Load default quest runtime settings
	QuestRuntimeSetting = GetDefault<UNerveQuestRuntimeSetting>();
	// Set up initial quest UI
	SetupQuestScreen();
}

void UNerveQuestSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("NerveQuestSubsystem: Beginning shutdown cleanup"));
	
	// Use existing comprehensive cleanup
	ResetAllQuests();
	
	// Clear any remaining references
	QuestRuntimeSetting = nullptr;
	
	// Call parent cleanup
	Super::Deinitialize();
	
	UE_LOG(LogTemp, Log, TEXT("NerveQuestSubsystem: Shutdown cleanup completed"));
}

void UNerveQuestSubsystem::SetupQuestScreen()
{
	// Validate settings and screen class
	if (!IsValid(QuestRuntimeSetting) || !IsValid(QuestRuntimeSetting->NerveQuestScreen))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupQuestScreen: Invalid settings or screen class"));
		return;
	}

	// Create quest screen widget
	NerveQuestScreen = CreateWidget<UQuestScreen>(GetWorld(), QuestRuntimeSetting->NerveQuestScreen);
	if (!IsValid(NerveQuestScreen))
	{
		UE_LOG(LogTemp, Error, TEXT("SetupQuestScreen: Failed to create quest screen widget"));
		return;
	}

	// Restore tracking for currently tracked quest
	if (IsValid(GetCurrentlyTrackedQuest()) && GetCurrentlyTrackedQuest()->bIsTracked)
	{
		TrackQuest(GetCurrentlyTrackedQuest()->QuestAsset);
	}
}

void UNerveQuestSubsystem::AddQuestAsync(TSoftObjectPtr<UNerveQuestAsset> Quest, const bool bTrackQuest, UObject* WorldContextObject, FOnQuestAddedDelegate OnComplete)
{
    // Validate inputs
    if (!IsValid(WorldContextObject) || Quest.IsNull())
    {
        UE_LOG(LogTemp, Warning, TEXT("AddQuestAsync: Invalid world context or null quest asset"));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    // Create async loading handle
    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

    const TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad
	(
        Quest.ToSoftObjectPath(),
        FStreamableDelegate::CreateUObject(this, &UNerveQuestSubsystem::OnQuestAssetLoaded, Quest, bTrackQuest, WorldContextObject, OnComplete)
    );
    
    if (!Handle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("AddQuestAsync: Failed to create async load handle"));
        OnComplete.ExecuteIfBound(false);
    }
}

void UNerveQuestSubsystem::OnQuestAssetLoaded(TSoftObjectPtr<UNerveQuestAsset> Quest, bool bTrackQuest, UObject* WorldContextObject, FOnQuestAddedDelegate OnComplete)
{
    UNerveQuestAsset* LoadedQuest = Quest.Get();
    if (!IsValid(LoadedQuest))
    {
        UE_LOG(LogTemp, Error, TEXT("OnQuestAssetLoaded: Quest asset failed to load"));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    // Now proceed with the original logic
    bool bSuccess = AddQuestInternal(LoadedQuest, bTrackQuest, WorldContextObject);
    OnComplete.ExecuteIfBound(bSuccess);
}

bool UNerveQuestSubsystem::AddQuest(const TSoftObjectPtr<UNerveQuestAsset> Quest, const bool bTrackQuest, UObject* WorldContextObject)
{
	// Load the quest asset synchronously
	UNerveQuestAsset* LoadedQuest = Quest.LoadSynchronous();
	if (!IsValid(LoadedQuest))
	{
		UE_LOG(LogTemp, Error, TEXT("AddQuest: Failed to load quest asset"));
		return false;
	}

	return AddQuestInternal(LoadedQuest, bTrackQuest, WorldContextObject);
}

bool UNerveQuestSubsystem::AddQuestInternal(UNerveQuestAsset* LoadedQuest, const bool bTrackQuest, UObject* WorldContextObject)
{
	// Validate inputs
	if (!IsValid(WorldContextObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddQuest: Invalid world context or null quest asset"));
		return false;
	}

	// Load the quest asset synchronously
	if (!IsValid(LoadedQuest))
	{
		UE_LOG(LogTemp, Error, TEXT("AddQuest: Failed to load quest asset"));
		return false;
	}

	// Set world context using weak pointer with validation
	if (IsValid(WorldContextObject))
	{
		QuestWorldContextObject = WorldContextObject;
		UE_LOG(LogTemp, Log, TEXT("AddQuestInternal: Set world context object %s"), *WorldContextObject->GetName());
	}
	else if (UWorld* CurrentWorld = GetWorld())
	{
		QuestWorldContextObject = CurrentWorld;
		UE_LOG(LogTemp, Log, TEXT("AddQuestInternal: Using subsystem world as fallback context"));
	}

	// Create and initialize quest runtime data
	UNerveQuestRuntimeData* NewQuestRuntimeData = NewObject<UNerveQuestRuntimeData>(this);
	if (!IsValid(NewQuestRuntimeData))
	{
		UE_LOG(LogTemp, Error, TEXT("AddQuest: Failed to create quest runtime data"));
		return false;
	}

	NewQuestRuntimeData->Initialize(LoadedQuest, this, bTrackQuest);
	QuestRuntimeDataMap.Add(LoadedQuest, NewQuestRuntimeData);

	// Rest of your code...
	NewQuestRuntimeData->OnQuestCompleted.AddDynamic(this, &UNerveQuestSubsystem::QuestCompleted);
	NewQuestRuntimeData->StartQuest();

	if (bTrackQuest)
	{
		TrackQuest(LoadedQuest);
	}

	OnQuestChanged.Broadcast(LoadedQuest);
	OnQuestAdded.Broadcast(LoadedQuest);
    
	UE_LOG(LogTemp, Log, TEXT("AddQuest: Successfully added quest %s"), *LoadedQuest->GetName());
	return true;
}

bool UNerveQuestSubsystem::RemoveQuest(UNerveQuestAsset* QuestToRemove)
{
	// Validate quest
	if (!IsValid(QuestToRemove))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveQuest: Invalid quest asset"));
		return false;
	}

	// Get runtime data
	UNerveQuestRuntimeData* QuestRuntimeData = GetQuestRuntimeData(QuestToRemove);
	if (!IsValid(QuestRuntimeData))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveQuest: Quest not found in system - %s"), *QuestToRemove->GetName());
		return false;
	}

	// Clean up runtime data
	QuestRuntimeData->Uninitialize();
	QuestRuntimeDataMap.Remove(QuestToRemove);

	// Broadcast events
	OnQuestChanged.Broadcast(QuestToRemove);
	OnQuestRemoved.Broadcast(QuestToRemove);

	// Mark for garbage collection
	QuestRuntimeData->ConditionalBeginDestroy();

	UE_LOG(LogTemp, Log, TEXT("RemoveQuest: Successfully removed quest %s"), *QuestToRemove->GetName());
	return true;
}

void UNerveQuestSubsystem::ResetAllQuests()
{
	UE_LOG(LogTemp, Log, TEXT("ResetQuestSystem: Resetting entire quest system"));
	
	for (auto& Pair : QuestRuntimeDataMap)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Uninitialize();
			Pair.Value->ConditionalBeginDestroy();
		}
	}
	QuestRuntimeDataMap.Empty();

	for (auto& Pair : ActiveOptionalObjectives)
	{
		for (FOptionalObjectiveData& Data : Pair.Value.ObjectiveData)
		{
			if (IsValid(Data.OptionalObjective))
			{
				Data.OptionalObjective->Uninitialize();
				Data.OptionalObjective->ConditionalBeginDestroy();
			}
		}
	}
	ActiveOptionalObjectives.Empty();

	for (auto& Pair : TrackedSubQuests)
	{
		if (IsValid(Pair.Key))
		{
			Pair.Key->Uninitialize();
			Pair.Key->ConditionalBeginDestroy();
		}
	}
	TrackedSubQuests.Empty();
	
	DisplayedObjectives.Empty();
	
	RegisteredEventReceivers.Empty();
	RegisteredTagReceivers.Empty();
	
	CurrentlyTrackedQuest = nullptr;

	UE_LOG(LogTemp, Log, TEXT("ResetQuestSystem: Quest system fully reset"));
}

void UNerveQuestSubsystem::TrackQuest(UNerveQuestAsset* QuestToTrack)
{
	// Validate inputs
	if (!IsValid(QuestToTrack) || !IsValid(NerveQuestScreen))
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackQuest: Invalid quest or screen widget"));
		return;
	}

	// Get runtime data
	CurrentlyTrackedQuest = GetQuestRuntimeData(QuestToTrack);
	if (!IsValid(CurrentlyTrackedQuest))
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackQuest: No runtime data for quest %s"), *QuestToTrack->GetName());
		return;
	}

	// If already tracked and displayed, just refresh UI
	if (CurrentlyTrackedQuest->bIsTracked && NerveQuestScreen->IsInViewport())
	{
		RefreshQuestUI(CurrentlyTrackedQuest);
		UE_LOG(LogTemp, Log, TEXT("TrackQuest: Quest %s already tracked, refreshed UI"), *QuestToTrack->GetName());
		return;
	}

	// Add screen to viewport if needed
	if (!NerveQuestScreen->IsInViewport())
	{
		NerveQuestScreen->AddToViewport(QuestRuntimeSetting->QuestScreenZOrder);
	}

	// Update quest tracking
	CurrentlyTrackedQuest->TrackQuest();
    
	// Broadcast tracking event
	OnQuestTracked.Broadcast(QuestToTrack);
    
	RefreshQuestUI(CurrentlyTrackedQuest);
    
	UE_LOG(LogTemp, Log, TEXT("TrackQuest: Tracking quest %s"), *QuestToTrack->GetName());
}

void UNerveQuestSubsystem::UntrackQuest(UNerveQuestAsset* QuestToUntrack)
{
	// Validate inputs
	if (!IsValid(QuestToUntrack) || !IsValid(NerveQuestScreen) || !IsValid(CurrentlyTrackedQuest) || 
		QuestToUntrack != CurrentlyTrackedQuest->QuestAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("UntrackQuest: Invalid inputs or quest mismatch"));
		return;
	}

	// Remove from UI
	if (NerveQuestScreen->IsInViewport())
	{
		NerveQuestScreen->UnInitQuestObjective(CurrentlyTrackedQuest);
	}

	// Update tracking state
	CurrentlyTrackedQuest->UntrackQuest();
	CurrentlyTrackedQuest = nullptr;
    
	// Broadcast untracking event
	OnQuestUnTracked.Broadcast(QuestToUntrack);
    
	UE_LOG(LogTemp, Log, TEXT("UntrackQuest: Untracked quest %s"), *QuestToUntrack->GetName());
}

bool UNerveQuestSubsystem::StartOptionalObjective(UNerveQuestRuntimeData* ParentQuest, UNerveQuestRuntimeObjectiveBase* OptionalObjectiveBase)
{
	// Validate inputs
	if (!IsValid(ParentQuest) || !IsValid(OptionalObjectiveBase))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartOptionalObjective: Invalid parent quest or objective"));
		return false;
	}

	// Check if already active
	if (IsOptionalObjectiveActive(ParentQuest, OptionalObjectiveBase))
	{
		UE_LOG(LogTemp, Log, TEXT("StartOptionalObjective: Objective already active"));
		return false;
	}

	// Create runtime data
	UNerveObjectiveRuntimeData* OptionalRuntimeData = NewObject<UNerveObjectiveRuntimeData>(this);
	if (!IsValid(OptionalRuntimeData))
	{
		UE_LOG(LogTemp, Error, TEXT("StartOptionalObjective: Failed to create runtime data"));
		return false;
	}

	// Initialize objective with consistent parameters (matching regular objectives)
	OptionalRuntimeData->Initialize(OptionalObjectiveBase, this, OptionalObjectiveBase->bIsOptionalObjective);

	// Set world context (consistent with regular objectives) with fallback
	UObject* WorldContext = QuestWorldContextObject.Get();
	if (!IsValid(WorldContext) && GetWorld())
	{
		WorldContext = GetWorld();
		UE_LOG(LogTemp, Log, TEXT("StartOptionalObjective: Using subsystem world as fallback for optional objective"));
	}
	
	if (IsValid(WorldContext))
	{
		OptionalObjectiveBase->SetWorldContextObject(WorldContext);
		UE_LOG(LogTemp, Log, TEXT("StartOptionalObjective: Set world context for optional objective %s"), *OptionalObjectiveBase->GetName());
	}
	
	// Create and store optional objective data
	FOptionalObjectiveData OptionalData;
	OptionalData.OptionalObjective = OptionalRuntimeData;
	OptionalData.ParentObjective = ParentQuest->CurrentObjective;

	if (!ActiveOptionalObjectives.Contains(ParentQuest))
	{
		ActiveOptionalObjectives.Add(ParentQuest, FOptionalObjectiveDataArray());
	}
	ActiveOptionalObjectives[ParentQuest].ObjectiveData.Add(OptionalData);

	// Bind completion events consistently with regular objectives
	OptionalRuntimeData->OnObjectiveCompleted.AddDynamic(this, &UNerveQuestSubsystem::OnOptionalObjectiveCompleted);
	OptionalRuntimeData->OnObjectiveFailed.AddDynamic(this, &UNerveQuestSubsystem::OnOptionalObjectiveFailed);

	OptionalRuntimeData->ParentObjective->SetWorldContextObject(GetWorld());
	// Execute objective
	OptionalRuntimeData->ExecuteObjective(ParentQuest->QuestAsset);

	// Mark as tracked if parent quest is tracked
	if (ParentQuest->bIsTracked)
	{
		OptionalRuntimeData->MarkAsTracked(ParentQuest->bIsTracked);
		RefreshQuestUI(ParentQuest);
	}

	UE_LOG(LogTemp, Log, TEXT("StartOptionalObjective: Started optional objective for quest %s"), 
		*ParentQuest->QuestAsset->GetName());
	return true;
}

void UNerveQuestSubsystem::StopOptionalObjective(UNerveQuestRuntimeData* ParentQuest, UNerveObjectiveRuntimeData* OptionalObjective)
{
	// Validate inputs
	if (!IsValid(ParentQuest) || !IsValid(OptionalObjective))
	{
		UE_LOG(LogTemp, Warning, TEXT("StopOptionalObjective: Invalid inputs"));
		return;
	}

	// Find and remove objective
	if (ActiveOptionalObjectives.Contains(ParentQuest))
	{
		FOptionalObjectiveDataArray& OptionalDataArray = ActiveOptionalObjectives[ParentQuest];
		for (int32 i = 0; i < OptionalDataArray.ObjectiveData.Num(); ++i)
		{
			if (OptionalDataArray.ObjectiveData[i].OptionalObjective == OptionalObjective)
			{
				// Unbind events
				if (IsValid(OptionalObjective->ParentObjective))
				{
					OptionalObjective->ParentObjective->OnObjectiveCompleted.RemoveDynamic(this, &UNerveQuestSubsystem::OnOptionalObjectiveCompleted);
					OptionalObjective->ParentObjective->OnObjectiveFailed.RemoveDynamic(this, &UNerveQuestSubsystem::OnOptionalObjectiveFailed);
				}
				OptionalDataArray.ObjectiveData.RemoveAt(i);
				break;
			}
		}

		// Clean up empty array
		if (OptionalDataArray.ObjectiveData.IsEmpty())
		{
			ActiveOptionalObjectives.Remove(ParentQuest);
		}

		// Refresh UI if tracked
		if (ParentQuest->bIsTracked)
		{
			RefreshQuestUI(ParentQuest);
		}
		
		UE_LOG(LogTemp, Log, TEXT("StopOptionalObjective: Stopped objective for quest %s"), 
			*ParentQuest->QuestAsset->GetName());
	}
}

bool UNerveQuestSubsystem::IsOptionalObjectiveActive(UNerveQuestRuntimeData* ParentQuest, UNerveQuestRuntimeObjectiveBase* OptionalObjectiveBase) const
{
	// Validate inputs
	if (!IsValid(ParentQuest) || !IsValid(OptionalObjectiveBase))
	{
		return false;
	}

	// Check active objectives
	if (ActiveOptionalObjectives.Contains(ParentQuest))
	{
		const FOptionalObjectiveDataArray& OptionalDataArray = ActiveOptionalObjectives[ParentQuest];
		for (const FOptionalObjectiveData& OptData : OptionalDataArray.ObjectiveData)
		{
			if (IsValid(OptData.OptionalObjective) && OptData.OptionalObjective->ParentObjective == OptionalObjectiveBase)
			{
				return true;
			}
		}
	}
	return false;
}

void UNerveQuestSubsystem::RefreshQuestUI(UNerveQuestRuntimeData* QuestData)
{
	// Validate inputs
	if (!IsValid(QuestData) || !IsValid(NerveQuestScreen))
	{
		UE_LOG(LogTemp, Warning, TEXT("RefreshQuestUI: Invalid quest data or screen widget"));
		return;
	}

	// Clear existing UI
	NerveQuestScreen->UnInitQuestObjective(QuestData);

	// Get and sort displayable objectives
	TArray<UNerveObjectiveRuntimeData*> DisplayableObjectives = GetDisplayableObjectives(QuestData);
	DisplayableObjectives.Sort([](const UNerveObjectiveRuntimeData& A, const UNerveObjectiveRuntimeData& B)
	{
		return A.GetDisplayPriority() > B.GetDisplayPriority();
	});

	// Update UI
	NerveQuestScreen->InitQuestObjective(QuestData, DisplayableObjectives);
	
	UE_LOG(LogTemp, Log, TEXT("RefreshQuestUI: Refreshed UI for quest %s"), *QuestData->QuestAsset->GetName());
}

TArray<UNerveObjectiveRuntimeData*> UNerveQuestSubsystem::GetDisplayableObjectives(UNerveQuestRuntimeData* QuestData) const
{
	TArray<UNerveObjectiveRuntimeData*> DisplayableObjectives;

	// Validate input
	if (!IsValid(QuestData)) return DisplayableObjectives;

	// Add main objective if visible
	if (IsValid(QuestData->CurrentObjective) && IsValid(QuestData->CurrentObjective->ParentObjective) &&
		QuestData->CurrentObjective->ParentObjective->GetShowInUI())
	{
		DisplayableObjectives.Add(QuestData->CurrentObjective);
	}

	return DisplayableObjectives;
}

bool UNerveQuestSubsystem::RegisterToReceiveEventFromObjective(UObject* RegisteringObject)
{
	// Validate input
	if (!IsValid(RegisteringObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterToReceiveEventFromObjective: Invalid object"));
		return false;
	}

	// Add unique registration using weak pointer
	RegisteredEventReceivers.AddUnique(TWeakObjectPtr<UObject>(RegisteringObject));
	
	UE_LOG(LogTemp, Log, TEXT("RegisterToReceiveEventFromObjective: Registered object %s"), *RegisteringObject->GetName());
	return true;
}

bool UNerveQuestSubsystem::UnRegisterToReceiveEventFromObjective(UObject* UnRegisteringObject)
{
	// Validate input
	if (!IsValid(UnRegisteringObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("UnRegisterToReceiveEventFromObjective: Invalid object"));
		return false;
	}

	// Remove registration using weak pointer comparison
	const int32 RemovedCount = RegisteredEventReceivers.RemoveAll([UnRegisteringObject](const TWeakObjectPtr<UObject>& WeakObj)
	{
		return WeakObj.Get() == UnRegisteringObject;
	});
	
	UE_LOG(LogTemp, Log, TEXT("UnRegisterToReceiveEventFromObjective: Unregistered object %s"), *UnRegisteringObject->GetName());
	return RemovedCount > 0;
}

void UNerveQuestSubsystem::ClearAllReceiversFromReceivingEvent()
{
	RegisteredEventReceivers.Empty();
	UE_LOG(LogTemp, Log, TEXT("ClearAllReceiversFromReceivingEvent: Cleared all event receivers"));
}

void UNerveQuestSubsystem::BroadcastToEventReceivers(UNerveQuestAsset* QuestAsset, const EQuestObjectiveEventType ReceivedEventType)
{
	// Early return if no receivers
	if (RegisteredEventReceivers.IsEmpty())
	{
		return;
	}

	// Broadcast to valid receivers, cleaning up stale weak pointers
	for (int32 i = RegisteredEventReceivers.Num() - 1; i >= 0; --i)
	{
		if (UObject* Receiver = RegisteredEventReceivers[i].Get())
		{
			if (Receiver->Implements<UNerveQuestReceiver>())
			{
				INerveQuestReceiver::Execute_ExecuteReceiveEvent(Receiver, QuestAsset, ReceivedEventType);
			}
		}
		else
		{
			// Remove stale weak pointer
			RegisteredEventReceivers.RemoveAtSwap(i);
		}
	}
}

bool UNerveQuestSubsystem::RegisterToReceiveTagsFromObjective(UObject* RegisteringObject)
{
	// Validate input
	if (!IsValid(RegisteringObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterToReceiveTagsFromObjective: Invalid object"));
		return false;
	}

	// Add unique registration using weak pointer
	RegisteredTagReceivers.AddUnique(TWeakObjectPtr<UObject>(RegisteringObject));
	
	UE_LOG(LogTemp, Log, TEXT("RegisterToReceiveTagsFromObjective: Registered object %s"), *RegisteringObject->GetName());
	return true;
}

bool UNerveQuestSubsystem::UnRegisterToReceiveTagsFromObjective(UObject* UnRegisteringObject)
{
	// Validate input
	if (!IsValid(UnRegisteringObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("UnRegisterToReceiveTagsFromObjective: Invalid object"));
		return false;
	}

	// Remove registration using weak pointer comparison
	const int32 RemovedCount = RegisteredTagReceivers.RemoveAll([UnRegisteringObject](const TWeakObjectPtr<UObject>& WeakObj)
	{
		return WeakObj.Get() == UnRegisteringObject;
	});
	
	UE_LOG(LogTemp, Log, TEXT("UnRegisterToReceiveTagsFromObjective: Unregistered object %s"), *UnRegisteringObject->GetName());
	return RemovedCount > 0;
}

void UNerveQuestSubsystem::ClearAllReceiversFromReceivingTag()
{
	RegisteredTagReceivers.Empty();
	UE_LOG(LogTemp, Log, TEXT("ClearAllReceiversFromReceivingTag: Cleared all tag receivers"));
}

void UNerveQuestSubsystem::BroadcastToTagReceivers(UNerveQuestAsset* QuestAsset, const FGameplayTag& ReceivedGameplayTag)
{
	// Early return if no receivers
	if (RegisteredTagReceivers.IsEmpty()) return;

	// Batch cleanup during iteration to maintain performance
	bool bNeedsCleanup = false;

	// Broadcast to valid receivers, cleaning up stale weak pointers
	for (int32 i = RegisteredTagReceivers.Num() - 1; i >= 0; --i)
	{
		if (UObject* Receiver = RegisteredTagReceivers[i].Get())
		{
			if (Receiver->Implements<UNerveQuestReceiver>())
			{
				INerveQuestReceiver::Execute_ExecuteReceiveTag(Receiver, QuestAsset, ReceivedGameplayTag);
			}
		}
		else
		{
			// Remove stale weak pointer
			RegisteredTagReceivers.RemoveAtSwap(i);
			bNeedsCleanup = true;
		}
	}
}

UNerveQuestRuntimeData* UNerveQuestSubsystem::GetQuestRuntimeData(const UNerveQuestAsset* QuestAsset) const
{
	return QuestRuntimeDataMap.FindRef(QuestAsset);
}

bool UNerveQuestSubsystem::AddQuestData(UNerveQuestAsset* NewDataKey, UNerveQuestRuntimeData* NewDataValue)
{
	// Validate inputs
	if (!IsValid(NewDataKey) || !IsValid(NewDataValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddQuestData: Invalid inputs"));
		return false;
	}

	// Check for existing data
	if (QuestRuntimeDataMap.Contains(NewDataKey))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddQuestData: Quest already exists"));
		return false;
	}

	// Add data
	QuestRuntimeDataMap.Add(NewDataKey, NewDataValue);
	
	UE_LOG(LogTemp, Log, TEXT("AddQuestData: Added data for quest %s"), *NewDataKey->GetName());
	return true;
}

bool UNerveQuestSubsystem::RemoveQuestData(const UNerveQuestAsset* NewDataKey, const UNerveQuestRuntimeData* NewDataValue)
{
	// Validate inputs
	if (!IsValid(NewDataKey) || !IsValid(NewDataValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveQuestData: Invalid inputs"));
		return false;
	}

	// Check for existing data
	if (!QuestRuntimeDataMap.Contains(NewDataKey))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveQuestData: Quest not found"));
		return false;
	}

	// Remove data
	QuestRuntimeDataMap.Remove(NewDataKey);
	
	UE_LOG(LogTemp, Log, TEXT("RemoveQuestData: Removed data for quest %s"), *NewDataKey->GetName());
	return true;
}

UNerveQuestRuntimeData* UNerveQuestSubsystem::CreateSubQuestRuntimeData(UNerveQuestAsset* SubQuestAsset, UObject* WorldContextObject)
{
	// Validate input
	if (!IsValid(SubQuestAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateSubQuestRuntimeData: Invalid sub-quest asset"));
		return nullptr;
	}

	// Determine context, safely handling weak pointer
	UObject* ContextObject = WorldContextObject ? WorldContextObject : QuestWorldContextObject.Get();
	if (!IsValid(ContextObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateSubQuestRuntimeData: No valid world context"));
		return nullptr;
	}

	// Create runtime data
	UNerveQuestRuntimeData* SubQuestRuntimeData = NewObject<UNerveQuestRuntimeData>(this);
	if (!IsValid(SubQuestRuntimeData))
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSubQuestRuntimeData: Failed to create runtime data"));
		return nullptr;
	}

	// Initialize without tracking
	SubQuestRuntimeData->Initialize(SubQuestAsset, this, false);
	
	UE_LOG(LogTemp, Log, TEXT("CreateSubQuestRuntimeData: Created sub-quest %s"), *SubQuestAsset->QuestTitle);
	return SubQuestRuntimeData;
}

bool UNerveQuestSubsystem::AreAllObjectiveCompleted(UNerveQuestRuntimeData* Quest)
{
	if (!IsValid(Quest)) return false;

	for (const auto Element : Quest->GetAllObjectives())
	{
		if (!Element->bIsCompleted) return false;
	}

	return true;
}

bool UNerveQuestSubsystem::IsQuestRegistered(const UNerveQuestAsset* QuestAsset) const
{
	return QuestAsset && QuestRuntimeDataMap.Contains(QuestAsset);
}

TArray<UNerveQuestAsset*> UNerveQuestSubsystem::GetAllRegisteredQuests() const
{
	TArray<UNerveQuestAsset*> RegisteredQuests;
	for (const auto& Pair : QuestRuntimeDataMap)
	{
		if (UNerveQuestAsset* QuestAsset = Pair.Key.Get())
		{
			RegisteredQuests.Add(QuestAsset);
		}
	}
	return RegisteredQuests;
}

void UNerveQuestSubsystem::TrackSubQuest(UNerveQuestRuntimeData* SubQuestData, bool bShowInMainUI)
{
	// Validate input
	if (!IsValid(SubQuestData) || !IsValid(SubQuestData->QuestAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackSubQuest: Invalid sub-quest data"));
		return;
	}

	// Update tracking
	SubQuestData->TrackQuest();
	TrackedSubQuests.Add(SubQuestData, bShowInMainUI);

	// Broadcast event
	OnSubQuestTrackingChanged.Broadcast(SubQuestData->QuestAsset, true);
	
	UE_LOG(LogTemp, Log, TEXT("TrackSubQuest: Tracking sub-quest %s (Show in UI: %s)"), 
		*SubQuestData->QuestAsset->QuestTitle, bShowInMainUI ? TEXT("Yes") : TEXT("No"));
}

void UNerveQuestSubsystem::UntrackSubQuest(UNerveQuestRuntimeData* SubQuestData)
{
	// Validate input
	if (!IsValid(SubQuestData))
	{
		UE_LOG(LogTemp, Warning, TEXT("UntrackSubQuest: Invalid sub-quest data"));
		return;
	}

	// Update tracking
	if (TrackedSubQuests.Contains(SubQuestData))
	{
		SubQuestData->UntrackQuest();
		TrackedSubQuests.Remove(SubQuestData);

		// Broadcast event
		if (IsValid(SubQuestData->QuestAsset))
		{
			OnSubQuestTrackingChanged.Broadcast(SubQuestData->QuestAsset, false);
		}
		
		UE_LOG(LogTemp, Log, TEXT("UntrackSubQuest: Untracked sub-quest %s"), 
			SubQuestData->QuestAsset ? *SubQuestData->QuestAsset->QuestTitle : TEXT("Unknown"));
	}
}

TArray<UNerveQuestRuntimeData*> UNerveQuestSubsystem::GetActiveSubQuests() const
{
	TArray<UNerveQuestRuntimeData*> ActiveSubQuests;
	
	// Extract valid quest runtime data from the map
	for (const auto& Pair : TrackedSubQuests)
	{
		if (UNerveQuestRuntimeData* SubQuest = Pair.Key.Get())
		{
			// Filter out completed quests
			if (IsValid(SubQuest) && !SubQuest->GetIsCompleted())
			{
				ActiveSubQuests.Add(SubQuest);
			}
		}
	}
	
	return ActiveSubQuests;
}

void UNerveQuestSubsystem::SetCurrentlyTrackedQuest(UNerveQuestRuntimeData* NewCurrentTrackedQuest)
{
	CurrentlyTrackedQuest = NewCurrentTrackedQuest;
}

TArray<UNerveQuestAsset*> UNerveQuestSubsystem::GetAllQuestAssets() const
{
	TArray<UNerveQuestAsset*> QuestAssets;
	for (const auto& Pair : QuestRuntimeDataMap)
	{
		if (UNerveQuestAsset* QuestAsset = Pair.Key.Get())
		{
			QuestAssets.Add(QuestAsset);
		}
	}
	return QuestAssets;
}

TArray<UNerveQuestRuntimeData*> UNerveQuestSubsystem::GetAllQuestRuntimeData() const
{
	TArray<UNerveQuestRuntimeData*> RuntimeData;
	for (const auto& Pair : QuestRuntimeDataMap)
	{
		if (UNerveQuestRuntimeData* Data = Pair.Value.Get())
		{
			RuntimeData.Add(Data);
		}
	}
	return RuntimeData;
}

FOptionalObjectiveDataArray UNerveQuestSubsystem::GetAllOptionalObjectiveDataForQuest(const UNerveQuestRuntimeData* ParentQuest)
{
	FOptionalObjectiveDataArray* NewOptionalObjectiveDataArray = nullptr;
	
	// Validate inputs
	if (!IsValid(ParentQuest)) return *NewOptionalObjectiveDataArray;

	// Find matching objective
	if (ActiveOptionalObjectives.Contains(ParentQuest))
	{
		NewOptionalObjectiveDataArray = &ActiveOptionalObjectives[ParentQuest];
	}
	
	return *NewOptionalObjectiveDataArray;
}

TArray<UNerveObjectiveRuntimeData*> UNerveQuestSubsystem::GetAllOptionalObjectiveForQuest(const UNerveQuestRuntimeData* ParentQuest,
const UNerveObjectiveRuntimeData* OptionalObjective)
{
	TArray<UNerveObjectiveRuntimeData*> AllOptionalObjectiveDataArray;
	
	if (!IsValid(ParentQuest) || !IsValid(OptionalObjective)) return AllOptionalObjectiveDataArray;

	// Find matching objective
	if (ActiveOptionalObjectives.Contains(ParentQuest))
	{
		FOptionalObjectiveDataArray& OptionalDataArray = ActiveOptionalObjectives[ParentQuest];
		for (FOptionalObjectiveData& OptData : OptionalDataArray.ObjectiveData)
		{
			if (OptData.ParentObjective != OptionalObjective) continue;

			AllOptionalObjectiveDataArray.Add(OptData.OptionalObjective);
		}
	}

	return AllOptionalObjectiveDataArray;
}

TArray<UNerveQuestAsset*> UNerveQuestSubsystem::GetQuestOfCategory(const ENerveQuestCategory QuestCategory)
{
	TArray<UNerveQuestAsset*> Quests;

	// Filter quests by category
	for (const auto& Element : QuestRuntimeDataMap)
	{
		if (IsValid(Element.Key) && Element.Value->QuestStatus == QuestCategory)
		{
			Quests.Add(Element.Key);
		}
	}
	
	return Quests;
}

UNerveQuestRuntimeData* UNerveQuestSubsystem::GetQuestDataByAsset(const UNerveQuestAsset* QuestAsset)
{
	// Validate input
	if (!IsValid(QuestAsset))
	{
		return nullptr;
	}

	// Find matching quest
	for (const auto& Element : QuestRuntimeDataMap)
	{
		if (Element.Key == QuestAsset)
		{
			return Element.Value;
		}
	}
	
	return nullptr;
}

bool UNerveQuestSubsystem::GetNextObjectiveForQuest(const UNerveQuestAsset* QuestAsset, UNerveObjectiveRuntimeData*& OutObjectiveRuntimeData)
{
	// Validate input
	if (!IsValid(QuestAsset))
	{
		return false;
	}

	// Get runtime data
	UNerveQuestRuntimeData* QuestRuntimeData = GetQuestDataByAsset(QuestAsset);
	if (!IsValid(QuestRuntimeData))
	{
		return false;
	}

	// Check objectives
	if (QuestRuntimeData->GetAllObjectives().IsEmpty())
	{
		return false;
	}

	// Find first incomplete objective
	for (UNerveObjectiveRuntimeData* Objective : QuestRuntimeData->GetAllObjectives())
	{
		if (!Objective->bIsCompleted)
		{
			OutObjectiveRuntimeData = Objective;
			return true;
		}
	}
	
	return false;
}

bool UNerveQuestSubsystem::GetCurrentObjectiveForQuest(const UNerveQuestAsset* QuestAsset, UNerveObjectiveRuntimeData*& OutObjectiveRuntimeData)
{
	// Validate input
	if (!IsValid(QuestAsset)) return false;

	// Get runtime data
	UNerveQuestRuntimeData* QuestRuntimeData = GetQuestDataByAsset(QuestAsset);
	if (!IsValid(QuestRuntimeData) || !IsValid(QuestRuntimeData->CurrentObjective))
	{
		return false;
	}

	OutObjectiveRuntimeData = QuestRuntimeData->CurrentObjective;
	return true;
}

void UNerveQuestSubsystem::QuestCompleted(UNerveQuestRuntimeData* Quest)
{
	// Untrack if currently tracked
	if (CurrentlyTrackedQuest == Quest)
	{
		UntrackQuest(CurrentlyTrackedQuest->QuestAsset);
	}
	
	UE_LOG(LogTemp, Log, TEXT("QuestCompleted: Quest %s completed"), 
		Quest->QuestAsset ? *Quest->QuestAsset->QuestTitle : TEXT("Unknown"));
}

void UNerveQuestSubsystem::OnOptionalObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* OptionalObjective)
{
	// Find matching objective
	for (auto& QuestOptionals : ActiveOptionalObjectives)
	{
		for (FOptionalObjectiveData& OptData : QuestOptionals.Value.ObjectiveData)
		{
			if (IsValid(OptData.OptionalObjective) && OptData.OptionalObjective->ParentObjective == OptionalObjective)
			{
				OptData.bIsCompleted = true;
				ProcessOptionalObjectiveCompletion(OptData);
				
				UE_LOG(LogTemp, Log, TEXT("OnOptionalObjectiveCompleted: Objective completed for quest %s"), 
					*QuestOptionals.Key->QuestAsset->GetName());
				return;
			}
		}
	}
}

void UNerveQuestSubsystem::OnOptionalObjectiveFailed(UNerveQuestRuntimeObjectiveBase* OptionalObjective)
{
	// Find matching objective
	for (auto& QuestOptionals : ActiveOptionalObjectives)
	{
		for (FOptionalObjectiveData& OptData : QuestOptionals.Value.ObjectiveData)
		{
			if (IsValid(OptData.OptionalObjective) && OptData.OptionalObjective->ParentObjective == OptionalObjective)
			{
				OptData.bHasFailed = true;

				// Update UI
				if (IsValid(NerveQuestScreen))
				{
					NerveQuestScreen->UpdateObjective(OptData.OptionalObjective);
				}

				// Process failure
				ProcessOptionalObjectiveFailure(OptData);
				
				UE_LOG(LogTemp, Log, TEXT("OnOptionalObjectiveFailed: Objective failed for quest %s"), 
					*QuestOptionals.Key->QuestAsset->GetName());
				return;
			}
		}
	}
}

void UNerveQuestSubsystem::ProcessOptionalObjectiveCompletion(const FOptionalObjectiveData& OptionalData)
{
	// Validate inputs
	if (!IsValid(OptionalData.OptionalObjective) || !IsValid(OptionalData.OptionalObjective->ParentObjective)) return;

	// Handle completion response
	const EOptionalObjectiveResponse Response = OptionalData.OptionalObjective->ParentObjective->GetOptionalCompletionResponse();
	switch (Response)
	{
	case EOptionalObjectiveResponse::CompleteParent:
		if (IsValid(OptionalData.ParentObjective))
		{
			OptionalData.ParentObjective->ParentObjective->OnObjectiveCompleted.Broadcast(OptionalData.ParentObjective->ParentObjective);
		}
		break;
	case EOptionalObjectiveResponse::AdvanceParent:
		// Find and advance parent quest
		for (auto& QuestData : QuestRuntimeDataMap)
		{
			if (QuestData.Value->CurrentObjective == OptionalData.ParentObjective)
			{
				QuestData.Value->AdvanceToNextObjective();
				break;
			}
		}
		break;
	case EOptionalObjectiveResponse::NoEffect:
	default:
		break;
	}
}

void UNerveQuestSubsystem::ProcessOptionalObjectiveFailure(const FOptionalObjectiveData& OptionalData)
{
	// Currently a no-op, can be extended for failure effects
}

FOptionalObjectiveData* UNerveQuestSubsystem::FindOptionalObjectiveData(UNerveQuestRuntimeData* ParentQuest, UNerveObjectiveRuntimeData* OptionalObjective)
{
	// Validate inputs
	if (!IsValid(ParentQuest) || !IsValid(OptionalObjective)) return nullptr;

	// Find matching objective
	if (ActiveOptionalObjectives.Contains(ParentQuest))
	{
		FOptionalObjectiveDataArray& OptionalDataArray = ActiveOptionalObjectives[ParentQuest];
		for (FOptionalObjectiveData& OptData : OptionalDataArray.ObjectiveData)
		{
			if (OptData.OptionalObjective == OptionalObjective)
			{
				return &OptData;
			}
		}
	}
	
	return nullptr;
}

UNerveQuestRuntimeObjectiveBase* UNerveQuestSubsystem::FindEntryObjective(const UNerveQuestAsset* Quest)
{
	// Validate input
	if (!IsValid(Quest) || !IsValid(Quest->RuntimeGraph))
	{
		UE_LOG(LogTemp, Error, TEXT("FindEntryObjective: Invalid quest or runtime graph"));
		return nullptr;
	}

	// Find entry objective
	for (UObject* Node : Quest->RuntimeGraph->GraphNodes)
	{
		if (Node && Node->GetClass() == UNerveEntryObjective::StaticClass())
		{
			return Cast<UNerveQuestRuntimeObjectiveBase>(Node);
		}
	}
	
	return nullptr;
}

UNerveObjectiveRuntimeData* UNerveQuestSubsystem::FindEntryByObjective(UNerveQuestRuntimeData* Quest)
{
	// Validate input
	if (!IsValid(Quest))
	{
		UE_LOG(LogTemp, Error, TEXT("FindEntryByObjective: Invalid quest"));
		return nullptr;
	}

	// Find entry objective
	for (UNerveObjectiveRuntimeData* Objective : Quest->GetAllObjectives())
	{
		if (IsValid(Objective->ParentObjective) && Objective->ParentObjective->GetClass() == UNerveEntryObjective::StaticClass())
		{
			return Objective;
		}
	}
	
	return nullptr;
}

void UNerveQuestRuntimeData::Initialize(UNerveQuestAsset* InQuestAsset, UNerveQuestSubsystem* InSubsystem, const bool InIsTracked)
{
	// Set initial properties
	QuestAsset = InQuestAsset;
	QuestHandlerSubSystem = InSubsystem;
	bIsTracked = InIsTracked;
	QuestStatus = ENerveQuestCategory::Available;

	// Initialize objectives
	if (!QuestAsset->GetQuestObjectives().IsEmpty())
	{
		UNerveQuestRuntimeObjectiveBase* EntryObjective = QuestAsset->GetQuestObjectives()[0];
		if (IsValid(EntryObjective))
		{
			AccumulateObjectives(EntryObjective);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Initialize: Initialized quest %s"), *InQuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::Uninitialize()
{
	for (auto Element : AllNerveObjectiveRuntimeData)
	{
		Element->Uninitialize();
	}
	ClearObjectives();
	
	QuestAsset = nullptr;
	QuestHandlerSubSystem = nullptr;
	
	UE_LOG(LogTemp, Log, TEXT("Uninitialize: Cleaned up quest runtime data"));
}

void UNerveQuestRuntimeData::BeginDestroy()
{
	// Ensure cleanup happens even if Uninitialize wasn't called
	if (IsValid(QuestHandlerSubSystem) || AllNerveObjectiveRuntimeData.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginDestroy: Quest runtime data not properly uninitialized, performing emergency cleanup"));
		Uninitialize();
	}
	
	Super::BeginDestroy();
}

bool UNerveQuestRuntimeData::AddObjective(UNerveObjectiveRuntimeData* Objective)
{
	// Validate input
	if (!IsValid(Objective))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddObjective: Invalid objective"));
		return false;
	}

	// Add objective
	AllNerveObjectiveRuntimeData.Add(Objective);
	
	UE_LOG(LogTemp, Log, TEXT("AddObjective: Added objective to quest %s"), *QuestAsset->QuestTitle);
	return true;
}

bool UNerveQuestRuntimeData::RemoveObjective(UNerveObjectiveRuntimeData* Objective)
{
	// Validate input
	if (!IsValid(Objective))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveObjective: Invalid objective"));
		return false;
	}

	// Remove objective
	AllNerveObjectiveRuntimeData.Remove(Objective);
	
	UE_LOG(LogTemp, Log, TEXT("RemoveObjective: Removed objective from quest %s"), *QuestAsset->QuestTitle);
	return true;
}

void UNerveQuestRuntimeData::ClearObjectives()
{
	AllNerveObjectiveRuntimeData.Empty();
	UE_LOG(LogTemp, Log, TEXT("ClearObjectives: Cleared all objectives for quest %s"), *QuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::StartQuest()
{
	// Validate objectives
	if (GetAllObjectives().IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("StartQuest: No objectives found for quest %s"), *QuestAsset->QuestTitle);
		return;
	}

	if (!IsValid(QuestHandlerSubSystem)) return;

	if (QuestHandlerSubSystem->AreAllObjectiveCompleted(this))
	{
		MarkQuestComplete();
		return;
	}

	// Set initial objective
	if (!IsValid(CurrentObjective))
	{
		CurrentObjective = GetAllObjectives()[0];
	}

	if (!IsValid(CurrentObjective))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartQuest: No valid entry objective for quest %s"), *QuestAsset->QuestTitle);
		return;
	}
	
	// Validate parent objective exists
	if (!IsValid(CurrentObjective->ParentObjective))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartQuest: Current objective has no valid parent for quest %s"), *QuestAsset->QuestTitle);
		return;
	}

	// Validate input pin exists
	if (!IsValid(CurrentObjective->ParentObjective->InputPin))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartQuest: Current objective has no input pin for quest %s"), *QuestAsset->QuestTitle);
		return;
	}

	// Get valid connections from input pin
	TArray<UNerveQuestRuntimePin*> InputConnections = CurrentObjective->ParentObjective->InputPin->GetValidConnections();
	if (InputConnections.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("StartQuest: Current objective input pin has no valid connections for quest %s"), *QuestAsset->QuestTitle);
		return;
	}

	// Get the entry node pin (what's connected to current objective's input)
	UNerveQuestRuntimePin* EntryNodePin = InputConnections[0];
	if (!IsValid(EntryNodePin))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartQuest: Invalid entry node pin for quest %s"), *QuestAsset->QuestTitle);
		return;
	}

	// Broadcast start event
	QuestHandlerSubSystem->BroadcastToEventReceivers(QuestAsset, EQuestObjectiveEventType::QuestStarted);
	ExecuteObjectiveFromPin(EntryNodePin);
	
	UE_LOG(LogTemp, Log, TEXT("StartQuest: Started quest %s"), *QuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::TrackQuest()
{
	// Validate current objective
	if (!IsValid(CurrentObjective))
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackQuest: No current objective"));
		return;
	}

	// Update tracking state
	bIsTracked = true;
	CurrentObjective->MarkAsTracked(bIsTracked);
	
	UE_LOG(LogTemp, Log, TEXT("TrackQuest: Tracking quest %s"), *QuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::UntrackQuest()
{
	// Validate current objective
	if (!IsValid(CurrentObjective))
	{
		UE_LOG(LogTemp, Warning, TEXT("UntrackQuest: No current objective"));
		return;
	}

	// Update tracking state
	bIsTracked = false;
	CurrentObjective->MarkAsTracked(bIsTracked);
	
	UE_LOG(LogTemp, Log, TEXT("UntrackQuest: Untracked quest %s"), *QuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::StartOptionalObjectives()
{
	// Validate current objective
	if (!IsValid(CurrentObjective) || !IsValid(CurrentObjective->ParentObjective))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartOptionalObjectives: Invalid current objective"));
		return;
	}

	// Start optional objectives
	for (UNerveQuestRuntimePin* OptionalPin : CurrentObjective->ParentObjective->OutOptionalPins)
	{
		if (IsValid(OptionalPin))
		{
			StartOptionalObjectivesFromPin(OptionalPin);
		}
	}
}

void UNerveQuestRuntimeData::StopAllOptionalObjectives()
{
	// Validate subsystem
	if (!IsValid(QuestHandlerSubSystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("StopAllOptionalObjectives: Invalid subsystem"));
		return;
	}

	// Clear optional objectives
	if (QuestHandlerSubSystem->GetAllActiveOptionalObjectives().Contains(this))
	{
		FOptionalObjectiveDataArray& OptionalDataArray = QuestHandlerSubSystem->GetAllActiveOptionalObjectives()[this];
		for (FOptionalObjectiveData& OptData : OptionalDataArray.ObjectiveData)
		{
			if (IsValid(OptData.OptionalObjective) && IsValid(OptData.OptionalObjective->ParentObjective))
			{
				OptData.OptionalObjective->ParentObjective->OnObjectiveCompleted.RemoveDynamic(QuestHandlerSubSystem, &UNerveQuestSubsystem::OnOptionalObjectiveCompleted);
				OptData.OptionalObjective->ParentObjective->OnObjectiveFailed.RemoveDynamic(QuestHandlerSubSystem, &UNerveQuestSubsystem::OnOptionalObjectiveFailed);
			}
		}
		OptionalDataArray.ObjectiveData.Empty();
		QuestHandlerSubSystem->GetAllActiveOptionalObjectives().Remove(this);

		// Refresh UI
		if (bIsTracked)
		{
			QuestHandlerSubSystem->RefreshQuestUI(this);
		}
		
		UE_LOG(LogTemp, Log, TEXT("StopAllOptionalObjectives: Stopped all optional objectives for quest %s"), *QuestAsset->QuestTitle);
	}
}

void UNerveQuestRuntimeData::MarkQuestComplete()
{
	// Update quest state
	bIsTracked = false;
	QuestStatus = ENerveQuestCategory::Completed;
	bIsCompleted = true;

	// Untrack if needed
	if (IsValid(QuestHandlerSubSystem))
	{
		QuestHandlerSubSystem->UntrackQuest(QuestAsset);
	}

	// Grant rewards
	if (!QuestAsset->QuestRewards.IsEmpty())
	{
		for (UNerveQuestRewardBase* Reward : QuestAsset->QuestRewards)
		{
			if (IsValid(Reward) && IsValid(QuestHandlerSubSystem))
			{
				Reward->GrantReward(QuestHandlerSubSystem->GetLocalPlayer()->GetPlayerController(GetWorld()));
			}
		}
	}

	// Broadcast completion
	OnQuestCompleted.Broadcast(this);
	
	UE_LOG(LogTemp, Log, TEXT("MarkQuestComplete: Quest %s completed"), *QuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::MarkQuestFailed()
{
	// Update quest state
	QuestStatus = ENerveQuestCategory::Failed;
	bIsTracked = false;
	bIsCompleted = false;

	// Untrack if needed
	if (IsValid(QuestHandlerSubSystem))
	{
		QuestHandlerSubSystem->UntrackQuest(QuestAsset);
	}

	// Broadcast failure
	OnQuestFailed.Broadcast(this);
	
	UE_LOG(LogTemp, Log, TEXT("MarkQuestFailed: Quest %s failed"), *QuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::SetQuestHandlerSubSystem(UNerveQuestSubsystem* QuestSubsystem)
{
	QuestHandlerSubSystem = QuestSubsystem;
}

void UNerveQuestRuntimeData::AdvanceToNextObjective(const int32 NextNodeIndex)
{
	// Validate current objective
	if (!IsValid(CurrentObjective) || NextNodeIndex > CurrentObjective->ParentObjective->OutPutPin.Num() - 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("AdvanceToNextObjective: Invalid current objective or index"));
		return;
	}

	// Get next pin
	UNerveQuestRuntimePin* OutPin = CurrentObjective->ParentObjective->OutPutPin[NextNodeIndex];
	if (!IsValid(OutPin) || OutPin->Connection.IsEmpty())
	{
		CurrentObjective->Uninitialize();
		CurrentObjective = nullptr;
		MarkQuestComplete();
		return;
	}

	// Execute next objective
	ExecuteObjectiveFromPin(OutPin);
}

void UNerveQuestRuntimeData::ExecuteObjectiveFromPin(UNerveQuestRuntimePin* OutPin)
{
	// Validate inputs
	if (!IsValid(OutPin))
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteObjectiveFromPin: Invalid OutPin"));
		return;
	}

	// Get valid connections from output pin
	TArray<UNerveQuestRuntimePin*> OutputConnections = OutPin->GetValidConnections();
	if (OutputConnections.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteObjectiveFromPin: No valid connections"));
		MarkQuestComplete();
		return;
	}

	UNerveQuestRuntimePin* NextPin = OutputConnections[0];
	UNerveQuestRuntimeObjectiveBase* NextParentNode = NextPin ? NextPin->GetParentNode() : nullptr;
	if (!IsValid(NextPin) || !IsValid(NextParentNode))
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteObjectiveFromPin: Invalid connection or parent node"));
		return;
	}

	if (!IsValid(QuestHandlerSubSystem))
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteObjectiveFromPin: Invalid subsystem"));
		return;
	}

	// Find next objective
	UNerveObjectiveRuntimeData* NextObjective = FindObjectiveData(NextParentNode);
	if (!IsValid(NextObjective) || !IsValid(NextObjective->ParentObjective))
	{
		UE_LOG(LogTemp, Log, TEXT("ExecuteObjectiveFromPin: Quest %s completed or reached end"), *QuestAsset->QuestTitle);
		MarkQuestComplete();
		return;
	}

	// Clean up current objective
	if (IsValid(CurrentObjective))
	{
		CurrentObjective->OnObjectiveCompleted.RemoveAll(this);
		CurrentObjective->OnObjectiveFailed.RemoveAll(this);
	}

	// Set up new objective
	CurrentObjective = NextObjective;
	const UObject* WorldContextObject = nullptr;
	
	// 1. Try subsystem's stored context
	if (const UObject* StoredContext = QuestHandlerSubSystem->QuestWorldContextObject.Get())
	{
		WorldContextObject = StoredContext;
		UE_LOG(LogTemp, Log, TEXT("ExecuteObjectiveFromPin: Using stored world context"));
	}
	// 2. Try subsystem's world
	else if (const UWorld* SubsystemWorld = QuestHandlerSubSystem->GetWorld())
	{
		WorldContextObject = SubsystemWorld;
		UE_LOG(LogTemp, Log, TEXT("ExecuteObjectiveFromPin: Using subsystem world as fallback"));
	}
	
	// Set world context if we found one
	if (IsValid(WorldContextObject))
	{
		CurrentObjective->ParentObjective->SetWorldContextObject(WorldContextObject);
	}

	// Bind events
	CurrentObjective->OnObjectiveCompleted.AddDynamic(this, &UNerveQuestRuntimeData::OnObjectiveCompleted);
	CurrentObjective->OnObjectiveFailed.AddDynamic(this, &UNerveQuestRuntimeData::OnObjectiveFailed);

	// Execute objective
	if (IsValid(QuestAsset))
	{
		CurrentObjective->ExecuteObjective(QuestAsset);
	}

	// Update tracking
	if (bIsTracked && IsValid(QuestHandlerSubSystem) && QuestHandlerSubSystem->GetQuestScreen())
	{
		CurrentObjective->MarkAsTracked(bIsTracked);
		QuestHandlerSubSystem->RefreshQuestUI(this);
	}

	// Start optional objectives
	if (IsValid(CurrentObjective) && IsValid(CurrentObjective->ParentObjective))
	{
		StartOptionalObjectives();
	}
}

TArray<UNerveQuestRewardBase*> UNerveQuestRuntimeData::GetQuestRewards() const
{
	if (!IsValid(QuestAsset))
	{
		return TArray<UNerveQuestRewardBase*>();
	}

	// Convert smart pointers to raw pointers for Blueprint compatibility
	TArray<UNerveQuestRewardBase*> Rewards;
	for (const TObjectPtr<UNerveQuestRewardBase>& RewardPtr : QuestAsset->QuestRewards)
	{
		if (UNerveQuestRewardBase* Reward = RewardPtr.Get())
		{
			Rewards.Add(Reward);
		}
	}
	return Rewards;
}

UNerveObjectiveRuntimeData* UNerveQuestRuntimeData::FindObjectiveData(const UNerveQuestRuntimeObjectiveBase* ObjectiveBase)
{
	// Find matching objective
	for (UNerveObjectiveRuntimeData* Objective : AllNerveObjectiveRuntimeData)
	{
		if (IsValid(Objective) && Objective->ParentObjective == ObjectiveBase)
		{
			return Objective;
		}
	}
	
	return nullptr;
}

void UNerveQuestRuntimeData::OnObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* Objective)
{
	// Validate input
	if (!IsValid(Objective) || !IsValid(QuestHandlerSubSystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnObjectiveCompleted: Invalid objective"));
		return;
	}

	// Broadcast event
	QuestHandlerSubSystem->BroadcastToEventReceivers(QuestAsset, EQuestObjectiveEventType::QuestCompleted);

	// Clean up bindings
	Objective->OnObjectiveCompleted.RemoveDynamic(this, &UNerveQuestRuntimeData::OnObjectiveCompleted);
	Objective->OnObjectiveFailed.RemoveDynamic(this, &UNerveQuestRuntimeData::OnObjectiveFailed);

	// Update UI if tracked
	if (IsValid(QuestHandlerSubSystem) && bIsTracked)
	{
		Objective->MarkAsTracked(false);
		if (IsValid(QuestHandlerSubSystem->GetQuestScreen()))
		{
			QuestHandlerSubSystem->GetQuestScreen()->UnInitQuestObjective(this);
		}
	}

	StopAllOptionalObjectives();
	
	// Advance to next objective
	AdvanceToNextObjective();
	
	UE_LOG(LogTemp, Log, TEXT("OnObjectiveCompleted: Objective completed for quest %s"), *QuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::OnObjectiveFailed(UNerveQuestRuntimeObjectiveBase* Objective)
{
	// Validate input
	if (!IsValid(Objective))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnObjectiveFailed: Invalid objective"));
		return;
	}

	// Broadcast event
	QuestHandlerSubSystem->BroadcastToEventReceivers(QuestAsset, EQuestObjectiveEventType::QuestFailed);

	// Clean up bindings
	Objective->OnObjectiveCompleted.RemoveDynamic(this, &UNerveQuestRuntimeData::OnObjectiveCompleted);
	Objective->OnObjectiveFailed.RemoveDynamic(this, &UNerveQuestRuntimeData::OnObjectiveFailed);

	// Update UI if tracked
	if (IsValid(QuestHandlerSubSystem) && bIsTracked)
	{
		Objective->MarkAsTracked(false);
		QuestHandlerSubSystem->GetQuestScreen()->UnInitQuestObjective(this);
	}

	StopAllOptionalObjectives();

	// Handle failure response
	switch (Objective->GetObjectiveFailureResponse())
	{
	case EObjectiveFailureResponse::FailQuest:
		MarkQuestFailed();
		break;
	case EObjectiveFailureResponse::ContinueToNextObjective:
		AdvanceToNextObjective();
		break;
	case EObjectiveFailureResponse::RestartQuest:
		CurrentObjective = nullptr;
		StartQuest();
		break;
	}
	
	UE_LOG(LogTemp, Log, TEXT("OnObjectiveFailed: Objective failed for quest %s"), *QuestAsset->QuestTitle);
}

void UNerveQuestRuntimeData::StartOptionalObjectivesFromPin(UNerveQuestRuntimePin* OptionalPin)
{
	// Validate input
	if (!IsValid(OptionalPin)) return;

	// Get valid connections and cleanup stale ones
	OptionalPin->CleanupStaleConnections();
	TArray<UNerveQuestRuntimePin*> ValidConnections = OptionalPin->GetValidConnections();
	
	if (ValidConnections.IsEmpty()) return;

	// Start connected objectives
	for (UNerveQuestRuntimePin* Connection : ValidConnections)
	{
		if (UNerveQuestRuntimeObjectiveBase* ParentNode = Connection->GetParentNode())
		{
			QuestHandlerSubSystem->StartOptionalObjective(this, ParentNode);
		}
	}
}

void UNerveQuestRuntimeData::AccumulateObjectives(UNerveQuestRuntimeObjectiveBase* Objective)
{
	// Validate inputs
	if (!IsValid(Objective) || !Objective->OutPutPin.IsValidIndex(0) || !IsValid(Objective->OutPutPin[0]))
	{
		UE_LOG(LogTemp, Warning, TEXT("AccumulateObjectives: Invalid objective or missing output pin"));
		return;
	}

	// Get valid connections using safe accessor
	TArray<UNerveQuestRuntimePin*> ValidConnections = Objective->OutPutPin[0]->GetValidConnections();
	if (ValidConnections.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AccumulateObjectives: No valid connections found"));
		return;
	}

	// Get parent node safely
	UNerveQuestRuntimeObjectiveBase* ParentNode = ValidConnections[0]->GetParentNode();
	if (!IsValid(ParentNode))
	{
		UE_LOG(LogTemp, Warning, TEXT("AccumulateObjectives: Invalid parent node"));
		return;
	}

	// Create and initialize objective
	UNerveObjectiveRuntimeData* NewObjective = NewObject<UNerveObjectiveRuntimeData>(this);
	if (IsValid(NewObjective) && IsValid(QuestHandlerSubSystem))
	{
		NewObjective->Initialize(ParentNode, QuestHandlerSubSystem, Objective->bIsOptionalObjective);
		AllNerveObjectiveRuntimeData.Add(NewObjective);
		AccumulateObjectives(ParentNode);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AccumulateObjectives: Failed to create or initialize new objective"));
	}
}

void UNerveObjectiveRuntimeData::Initialize(UNerveQuestRuntimeObjectiveBase* Objective, UNerveQuestSubsystem* QuestSubsystem, const bool bAsOptional, UNerveObjectiveRuntimeData* MainParent)
{
	// Set initial properties
	ParentObjective = Objective;
	QuestHandlerSubSystem = QuestSubsystem;
	bIsOptionalObjective = bAsOptional;
	ParentMainObjective = MainParent;

	// Set display priority
	if (IsValid(ParentObjective))
	{
		DisplayPriority = ParentObjective->GetDisplayPriority();
	}

	// Ensure objective has proper world context set
	if (IsValid(Objective) && IsValid(QuestSubsystem))
	{
		// Try to get world context from subsystem
		if (UObject* WorldContext = QuestSubsystem->QuestWorldContextObject.Get())
		{
			Objective->SetWorldContextObject(WorldContext);
		}
		// Fallback to subsystem's world
		else if (UWorld* SubsystemWorld = QuestSubsystem->GetWorld())
		{
			Objective->SetWorldContextObject(SubsystemWorld);
		}
	}

	// Create progress tracker if allowed
	if (IsValid(ParentObjective) && ParentObjective->AllowGenerateProgressTracker())
	{
		TrackingWidget = CreateWidget<UObjectiveProgressTracker>
		(
			UGameplayStatics::GetPlayerController(GetWorld(), 0), 
			ParentObjective->GetProgressTrackerClass());

		if (IsValid(TrackingWidget))
		{
			TrackingWidget->SetCurrent(0);
			TrackingWidget->SetMax(1);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Initialize: Initialized objective %s"), *ParentObjective->GetName());
}

void UNerveObjectiveRuntimeData::Uninitialize()
{
	// Clean up bindings
	if (IsValid(ParentObjective))
	{
		ParentObjective->OnObjectiveCompleted.RemoveDynamic(this, &UNerveObjectiveRuntimeData::ObjectiveCompleted);
		ParentObjective->OnObjectiveFailed.RemoveDynamic(this, &UNerveObjectiveRuntimeData::ObjectiveFailed);
		ParentObjective->OnProgressChanged.RemoveDynamic(this, &UNerveObjectiveRuntimeData::ObjectiveProgress);

		ParentObjective->CleanUpObjective();
	}
	
	UE_LOG(LogTemp, Log, TEXT("Uninitialize: Cleaned up objective %s"), ParentObjective ? *ParentObjective->GetName() : TEXT("Unknown"));
}

void UNerveObjectiveRuntimeData::BeginDestroy()
{
	// Ensure cleanup happens even if Uninitialize wasn't called
	if (IsValid(ParentObjective) || IsValid(QuestHandlerSubSystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginDestroy: Objective runtime data not properly uninitialized, performing emergency cleanup"));
		Uninitialize();
	}
	
	Super::BeginDestroy();
}

void UNerveObjectiveRuntimeData::ExecuteObjective(UNerveQuestAsset* QuestAsset) const
{
	// Validate inputs
	if (!IsValid(ParentObjective) || !IsValid(QuestAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteObjective: Invalid objective or quest asset"));
		return;
	}

	// Set parent quest
	ParentQuestAsset = QuestAsset;

	// Bind events
	ParentObjective->OnObjectiveCompleted.AddDynamic(this, &UNerveObjectiveRuntimeData::ObjectiveCompleted);
	ParentObjective->OnObjectiveFailed.AddDynamic(this, &UNerveObjectiveRuntimeData::ObjectiveFailed);
	ParentObjective->OnProgressChanged.AddDynamic(this, &UNerveObjectiveRuntimeData::ObjectiveProgress);

	// Broadcast start event
	if (IsValid(QuestHandlerSubSystem))
	{
		QuestHandlerSubSystem->BroadcastToEventReceivers(ParentQuestAsset, EQuestObjectiveEventType::QuestObjectiveStarted);
	}

	// Execute objective
	ParentObjective->ExecuteObjective(QuestAsset);
	
	UE_LOG(LogTemp, Log, TEXT("ExecuteObjective: Executed objective %s for quest %s"), 
		*ParentObjective->GetName(), *QuestAsset->QuestTitle);
}

void UNerveObjectiveRuntimeData::MarkAsTracked(const bool Value) const
{
	// Validate input
	if (!IsValid(ParentObjective))
	{
		UE_LOG(LogTemp, Warning, TEXT("MarkAsTracked: Invalid parent objective"));
		return;
	}

	// Update tracking state
	ParentObjective->MarkAsTracked(Value);
	
	UE_LOG(LogTemp, Log, TEXT("MarkAsTracked: Set tracking to %s for objective %s"), 
		Value ? TEXT("true") : TEXT("false"), *ParentObjective->GetName());
}

TArray<UNerveObjectiveRuntimeData*> UNerveObjectiveRuntimeData::GetOptionalObjectives() const
{
	TArray<UNerveObjectiveRuntimeData*> Optionals;
	if (!IsValid(QuestHandlerSubSystem)) return Optionals;
	if (!IsValid(ParentQuestAsset)) return Optionals;

	Optionals = QuestHandlerSubSystem->GetAllOptionalObjectiveForQuest
	(QuestHandlerSubSystem->GetQuestDataByAsset(ParentQuestAsset), this);

	return Optionals;
}

void UNerveObjectiveRuntimeData::ObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* Objective)
{
	// Update state
	bIsCompleted = true;
	bHasFailed = false;
	
	OnObjectiveCompleted.Broadcast(Objective);

	// Broadcast event
	if (IsValid(QuestHandlerSubSystem) && IsValid(ParentQuestAsset))
	{
		QuestHandlerSubSystem->BroadcastToEventReceivers(ParentQuestAsset, EQuestObjectiveEventType::QuestObjectiveCompleted);
	}
	
	UE_LOG(LogTemp, Log, TEXT("ObjectiveCompleted: Objective %s completed"), *Objective->GetName());
}

void UNerveObjectiveRuntimeData::ObjectiveFailed(UNerveQuestRuntimeObjectiveBase* Objective)
{
	// Update state
	bIsCompleted = false;
	bHasFailed = true;

	// Broadcast failure
	OnObjectiveFailed.Broadcast(Objective);

	// Broadcast event
	if (IsValid(QuestHandlerSubSystem) && IsValid(ParentQuestAsset))
	{
		QuestHandlerSubSystem->BroadcastToEventReceivers(ParentQuestAsset, EQuestObjectiveEventType::QuestObjectiveFailed);
	}
	
	UE_LOG(LogTemp, Log, TEXT("ObjectiveFailed: Objective %s failed"), *Objective->GetName());
}

void UNerveObjectiveRuntimeData::ObjectiveProgress(UNerveQuestRuntimeObjectiveBase* ObjectiveBase, const float NewProgressValue, const float MaxProgressValue)
{
	// Validate input
	if (!IsValid(TrackingWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectiveProgress: Invalid tracking widget"));
		return;
	}

	// Update progress
	TrackingWidget->SetCurrent(NewProgressValue);
	TrackingWidget->SetMax(MaxProgressValue);
	
	UE_LOG(LogTemp, Log, TEXT("ObjectiveProgress: Updated progress for objective %s to %f/%f"), 
	*ObjectiveBase->GetName(), NewProgressValue, MaxProgressValue);
}