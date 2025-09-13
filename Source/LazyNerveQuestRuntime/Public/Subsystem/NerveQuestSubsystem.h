#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"
#include "Setting/NerveQuestRuntimeSetting.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "NerveQuestSubsystem.generated.h"

class UNerveObjectiveRuntimeData;
class UNerveQuestRuntimeData;
class UNerveQuestAsset;

// Delegate declarations for quest-related events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNerveQuestSubsystemAction, UNerveQuestAsset*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNerveSubQuestSubsystemAction, UNerveQuestAsset*, Quest, bool, Tracked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNerveQuestAction, UNerveQuestRuntimeData*, Quest);

/**
 * @class UNerveQuestSubsystem
 * @brief Manages quest-related functionality for the local player.
 *
 * This subsystem handles quest creation, tracking, progression, and UI integration.
 * It maintains runtime data for quests and objectives, manages optional objectives,
 * and facilitates communication between quest components and UI elements.
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UNerveQuestSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	// --- Delegates for Quest Events ---
	/** Broadcast when any quest state changes */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FNerveQuestSubsystemAction OnQuestChanged;

	/** Broadcast when a new quest is added */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FNerveQuestSubsystemAction OnQuestAdded;

	/** Broadcast when a quest is removed */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FNerveQuestSubsystemAction OnQuestRemoved;

	/** Broadcast when a quest is set as tracked */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FNerveQuestSubsystemAction OnQuestTracked;

	/** Broadcast when a quest is untracked */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FNerveQuestSubsystemAction OnQuestUnTracked;

	/** Broadcast when sub-quest tracking state changes */
	UPROPERTY(BlueprintAssignable, Category = "Sub-Quest|Events")
	FNerveSubQuestSubsystemAction OnSubQuestTrackingChanged;

	// --- Context ---
	/** World context object for quest operations (weak reference to prevent circular dependencies) */
	UPROPERTY()
	TWeakObjectPtr<UObject> QuestWorldContextObject = nullptr;

protected:
	// --- Core Quest Data ---
	/** Mapping of quest assets to their runtime data */
	UPROPERTY()
	TMap<TObjectPtr<UNerveQuestAsset>, TObjectPtr<UNerveQuestRuntimeData>> QuestRuntimeDataMap;

	/** Currently tracked quest */
	UPROPERTY()
	TObjectPtr<UNerveQuestRuntimeData> CurrentlyTrackedQuest;

	/** Runtime settings for quest system */
	UPROPERTY()
	TObjectPtr<const UNerveQuestRuntimeSetting> QuestRuntimeSetting;

	// --- Optional Objectives ---
	/** Active optional objectives per quest */
	UPROPERTY()
	TMap<TObjectPtr<UNerveQuestRuntimeData>, FOptionalObjectiveDataArray> ActiveOptionalObjectives;

	// --- Sub-Quest Management ---
	/** Tracked sub-quests and their UI display status */
	UPROPERTY()
	TMap<TObjectPtr<UNerveQuestRuntimeData>, bool> TrackedSubQuests;

	// --- UI Management ---
	/** Quest screen widget for UI display */
	UPROPERTY()
	TObjectPtr<UQuestScreen> NerveQuestScreen;

	/** Objectives currently displayed in UI */
	UPROPERTY()
	TArray<TObjectPtr<UNerveObjectiveRuntimeData>> DisplayedObjectives;

	// --- Event System ---
	/** Objects registered to receive quest events (using weak references to prevent memory leaks) */
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> RegisteredEventReceivers;

	/** Objects registered to receive gameplay tags (using weak references to prevent memory leaks) */
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> RegisteredTagReceivers;

public:
	// --- Initialization & Cleanup ---
	/** Initializes the subsystem and sets up quest runtime settings */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/** Deinitializes the subsystem and cleans up resources */
	virtual void Deinitialize() override;

	/** Sets up the quest screen widget for UI display */
	UFUNCTION(BlueprintCallable, Category = "Quest|Setup", meta = (WorldContext = "WorldContextObject"))
	void SetupQuestScreen();
	
	// --- Quest Management ---

	DECLARE_DELEGATE_OneParam(FOnQuestAddedDelegate, bool);
	void AddQuestAsync(TSoftObjectPtr<UNerveQuestAsset> Quest, const bool bTrackQuest, UObject* WorldContextObject, FOnQuestAddedDelegate OnComplete);
	void OnQuestAssetLoaded(TSoftObjectPtr<UNerveQuestAsset> Quest, bool bTrackQuest, UObject* WorldContextObject, FOnQuestAddedDelegate OnComplete);


	/**
	 * Adds a new quest to the system
	 * @param Quest The quest asset to add
	 * @param bTrackQuest Whether to track the quest immediately
	 * @param WorldContextObject The world context for the quest
	 * @return True if the quest was successfully added
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Management", meta = (WorldContext = "WorldContextObject"))
	bool AddQuest(TSoftObjectPtr<UNerveQuestAsset> Quest, bool bTrackQuest = true, UObject* WorldContextObject = nullptr);

	bool AddQuestInternal(UNerveQuestAsset* LoadedQuest, const bool bTrackQuest, UObject* WorldContextObject);

	/**
	 * Removes a quest from the system
	 * @param QuestToRemove The quest to remove
	 * @return True if the quest was successfully removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Management")
	bool RemoveQuest(UNerveQuestAsset* QuestToRemove);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Quest|Management")
	void ResetAllQuests();

	/**
	 * Sets a quest as the currently tracked quest
	 * @param QuestToTrack The quest to track
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Management")
	void TrackQuest(UNerveQuestAsset* QuestToTrack);

	/**
	 * Stops tracking a quest
	 * @param QuestToUntrack The quest to untrack
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Management")
	void UntrackQuest(UNerveQuestAsset* QuestToUntrack);

	// --- Sub-Quest Management ---
	/**
	 * Creates runtime data for a sub-quest without registering it
	 * @param SubQuestAsset The sub-quest asset
	 * @param WorldContextObject The world context
	 * @return The created runtime data
	 */
	UFUNCTION(BlueprintCallable, Category = "Sub-Quest|Management")
	UNerveQuestRuntimeData* CreateSubQuestRuntimeData(UNerveQuestAsset* SubQuestAsset, UObject* WorldContextObject = nullptr);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Sub-Quest|Management")
	bool AreAllObjectiveCompleted(UNerveQuestRuntimeData* Quest);

	/**
	 * Tracks a sub-quest
	 * @param SubQuestData The sub-quest runtime data
	 * @param bShowInMainUI Whether to show in main UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Sub-Quest|Management")
	void TrackSubQuest(UNerveQuestRuntimeData* SubQuestData, bool bShowInMainUI = false);

	/**
	 * Untracks a sub-quest
	 * @param SubQuestData The sub-quest to untrack
	 */
	UFUNCTION(BlueprintCallable, Category = "Sub-Quest|Management")
	void UntrackSubQuest(UNerveQuestRuntimeData* SubQuestData);

	// --- Optional Objectives ---
	/**
	 * Starts an optional objective
	 * @param ParentQuest The parent quest
	 * @param OptionalObjectiveBase The optional objective to start
	 * @return True if the objective was started
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Optional Objectives")
	bool StartOptionalObjective(UNerveQuestRuntimeData* ParentQuest, UNerveQuestRuntimeObjectiveBase* OptionalObjectiveBase);

	/**
	 * Stops an optional objective
	 * @param ParentQuest The parent quest
	 * @param OptionalObjective The objective to stop
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Optional Objectives")
	void StopOptionalObjective(UNerveQuestRuntimeData* ParentQuest, UNerveObjectiveRuntimeData* OptionalObjective);

	/**
	 * Checks if an optional objective is active
	 * @param ParentQuest The parent quest
	 * @param OptionalObjectiveBase The objective to check
	 * @return True if the objective is active
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Optional Objectives")
	bool IsOptionalObjectiveActive(UNerveQuestRuntimeData* ParentQuest, UNerveQuestRuntimeObjectiveBase* OptionalObjectiveBase) const;

	// --- UI Management ---
	/**
	 * Refreshes the quest UI for a specific quest
	 * @param QuestData The quest to refresh
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|UI")
	void RefreshQuestUI(UNerveQuestRuntimeData* QuestData);

	/**
	 * Gets objectives that should be displayed in UI
	 * @param QuestData The quest to query
	 * @return Array of displayable objectives
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|UI")
	TArray<UNerveObjectiveRuntimeData*> GetDisplayableObjectives(UNerveQuestRuntimeData* QuestData) const;

	// --- Event Registration ---
	/**
	 * Registers an object to receive quest events
	 * @param RegisteringObject The object to register
	 * @return True if registration was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	bool RegisterToReceiveEventFromObjective(UObject* RegisteringObject);

	/**
	 * Unregisters an object from receiving quest events
	 * @param UnRegisteringObject The object to unregister
	 * @return True if unregistration was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	bool UnRegisterToReceiveEventFromObjective(UObject* UnRegisteringObject);

	/** Clears all registered event receivers */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	void ClearAllReceiversFromReceivingEvent();

	/**
	 * Registers an object to receive gameplay tags
	 * @param RegisteringObject The object to register
	 * @return True if registration was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	bool RegisterToReceiveTagsFromObjective(UObject* RegisteringObject);

	/**
	 * Unregisters an object from receiving gameplay tags
	 * @param UnRegisteringObject The object to unregister
	 * @return True if unregistration was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	bool UnRegisterToReceiveTagsFromObjective(UObject* UnRegisteringObject);

	/** Clears all registered tag receivers */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	void ClearAllReceiversFromReceivingTag();

	/** */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	void SetCurrentlyTrackedQuest(UNerveQuestRuntimeData* NewCurrentTrackedQuest);

	// --- Data Access ---
	/**
	 * Gets runtime data for a quest
	 * @param QuestAsset The quest to query
	 * @return The quest runtime data
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	UNerveQuestRuntimeData* GetQuestRuntimeData(const UNerveQuestAsset* QuestAsset) const;

	/**
	 * Adds quest runtime data
	 * @param NewDataKey The quest asset key
	 * @param NewDataValue The runtime data value
	 * @return True if data was added
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	bool AddQuestData(UNerveQuestAsset* NewDataKey, UNerveQuestRuntimeData* NewDataValue);

	/**
	 * Removes quest runtime data
	 * @param NewDataKey The quest asset key
	 * @param NewDataValue The runtime data value
	 * @return True if data was removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	bool RemoveQuestData(const UNerveQuestAsset* NewDataKey, const UNerveQuestRuntimeData* NewDataValue);

	/**
	 * Checks if a quest is registered
	 * @param QuestAsset The quest to check
	 * @return True if the quest is registered
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	bool IsQuestRegistered(const UNerveQuestAsset* QuestAsset) const;

	/**
	 * Gets all registered quests
	 * @return Array of registered quest assets
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	TArray<UNerveQuestAsset*> GetAllRegisteredQuests() const;

	/**
	 * Gets all active sub-quests
	 * @return Array of active sub-quest runtime data
	 */
	UFUNCTION(BlueprintPure, Category = "Sub-Quest|Query")
	TArray<UNerveQuestRuntimeData*> GetActiveSubQuests() const;

	// --- Helper Functions ---
	/**
	 * Gets the currently tracked quest
	 * @return The tracked quest runtime data
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	const UNerveQuestRuntimeData* GetCurrentlyTrackedQuest() const { return CurrentlyTrackedQuest; }

	/**
	 * Gets the world context object
	 * @return The world context object
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	const UObject* GetQuestWorldContextObject() const { return GetWorld(); }

	/**
	 * Gets all quests and their runtime data (C++ only - smart pointers not Blueprint compatible)
	 * @return Map of quests to runtime data
	 */
	TMap<TObjectPtr<UNerveQuestAsset>, TObjectPtr<UNerveQuestRuntimeData>>& GetAllQuests() { return QuestRuntimeDataMap; }

	/**
	 * Gets all active optional objectives (C++ only - smart pointers not Blueprint compatible)
	 * @return Map of quests to optional objectives
	 */
	TMap<TObjectPtr<UNerveQuestRuntimeData>, FOptionalObjectiveDataArray>& GetAllActiveOptionalObjectives() { return ActiveOptionalObjectives; }

	/**
	 * Gets all quest assets (Blueprint compatible version)
	 * @return Array of quest assets
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	TArray<UNerveQuestAsset*> GetAllQuestAssets() const;

	/**
	 * Gets all quest runtime data (Blueprint compatible version)
	 * @return Array of quest runtime data
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	TArray<UNerveQuestRuntimeData*> GetAllQuestRuntimeData() const;

	/**
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	FOptionalObjectiveDataArray GetAllOptionalObjectiveDataForQuest(const UNerveQuestRuntimeData* ParentQuest);

	/**
	*/
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	TArray<UNerveObjectiveRuntimeData*> GetAllOptionalObjectiveForQuest(const UNerveQuestRuntimeData* ParentQuest, const UNerveObjectiveRuntimeData* OptionalObjective);

	/**
	 * Gets quests of a specific category
	 * @param QuestCategory The category to filter by
	 * @return Array of quests in the category
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	TArray<UNerveQuestAsset*> GetQuestOfCategory(ENerveQuestCategory QuestCategory);

	/**
	 * Gets runtime data by quest asset
	 * @param QuestAsset The quest to query
	 * @return The runtime data
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	UNerveQuestRuntimeData* GetQuestDataByAsset(const UNerveQuestAsset* QuestAsset);

	/**
	 * Gets the next objective for a quest
	 * @param QuestAsset The quest to query
	 * @param OutObjectiveRuntimeData The output objective
	 * @return True if a next objective was found
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	bool GetNextObjectiveForQuest(const UNerveQuestAsset* QuestAsset, UNerveObjectiveRuntimeData*& OutObjectiveRuntimeData);

	/**
	 * Gets the current objective for a quest
	 * @param QuestAsset The quest to query
	 * @param OutObjectiveRuntimeData The output objective
	 * @return True if a current objective was found
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	bool GetCurrentObjectiveForQuest(const UNerveQuestAsset* QuestAsset, UNerveObjectiveRuntimeData*& OutObjectiveRuntimeData);

	/**
	 * Gets the quest screen widget
	 * @return The quest screen widget
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Helper")
	UQuestScreen* GetQuestScreen() const { return NerveQuestScreen; }

	// --- Static Helpers ---
	/**
	 * Finds the entry objective for a quest
	 * @param Quest The quest to query
	 * @return The entry objective
	 */
	static UNerveQuestRuntimeObjectiveBase* FindEntryObjective(const UNerveQuestAsset* Quest);

	/**
	 * Finds the entry objective runtime data
	 * @param Quest The quest runtime data
	 * @return The entry objective runtime data
	 */
	static UNerveObjectiveRuntimeData* FindEntryByObjective(UNerveQuestRuntimeData* Quest);

	// --- Event Broadcasting ---
	/**
	 * Broadcasts events to registered receivers
	 * @param QuestAsset The quest triggering the event
	 * @param ReceivedEventType The event type
	 */
	void BroadcastToEventReceivers(UNerveQuestAsset* QuestAsset, EQuestObjectiveEventType ReceivedEventType);

	/**
	 * Broadcasts gameplay tags to registered receivers
	 * @param QuestAsset The quest triggering the tag
	 * @param ReceivedGameplayTag The gameplay tag
	 */
	void BroadcastToTagReceivers(UNerveQuestAsset* QuestAsset, const FGameplayTag& ReceivedGameplayTag);

	// --- Callbacks ---
	/**
	 * Handles quest completion
	 * @param Quest The completed quest
	 */
	UFUNCTION()
	void QuestCompleted(UNerveQuestRuntimeData* Quest);

	/**
	 * Handles optional objective completion
	 * @param OptionalObjective The completed objective
	 */
	UFUNCTION()
	void OnOptionalObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* OptionalObjective);

	/**
	 * Handles optional objective failure
	 * @param OptionalObjective The failed objective
	 */
	UFUNCTION()
	void OnOptionalObjectiveFailed(UNerveQuestRuntimeObjectiveBase* OptionalObjective);

protected:
	/**
	 * Processes optional objective completion
	 * @param OptionalData The optional objective data
	 */
	void ProcessOptionalObjectiveCompletion(const FOptionalObjectiveData& OptionalData);

	/**
	 * Processes optional objective failure
	 * @param OptionalData The optional objective data
	 */
	void ProcessOptionalObjectiveFailure(const FOptionalObjectiveData& OptionalData);

	/**
	 * Finds optional objective data
	 * @param ParentQuest The parent quest
	 * @param OptionalObjective The optional objective
	 * @return The optional objective data
	 */
	FOptionalObjectiveData* FindOptionalObjectiveData(UNerveQuestRuntimeData* ParentQuest, UNerveObjectiveRuntimeData* OptionalObjective);
};

/**
 * @class UNerveQuestRuntimeData
 * @brief Manages runtime data for a single quest.
 *
 * This class tracks quest progress, objectives, and status, and handles
 * quest initialization, completion, and objective progression.
 */
UCLASS(BlueprintType)
class LAZYNERVEQUESTRUNTIME_API UNerveQuestRuntimeData : public UObject
{
	GENERATED_BODY()

public:
	// --- Quest Properties ---
	/** The associated quest asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TObjectPtr<UNerveQuestAsset> QuestAsset;

	/** The current active objective */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TObjectPtr<UNerveObjectiveRuntimeData> CurrentObjective;

	/** Whether the quest is currently tracked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsTracked;

	/** Overall progress of the quest (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	float OverallProgress;

	/** Current status of the quest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	ENerveQuestCategory QuestStatus;

	/** Whether the quest is completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsCompleted;

	// --- Delegates ---
	/** Broadcast when the quest is completed */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Quest|Events")
	FNerveQuestAction OnQuestCompleted;

	/** Broadcast when the quest fails */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Quest|Events")
	FNerveQuestAction OnQuestFailed;

private:
	// --- Internal Data ---
	/** All objectives associated with this quest */
	UPROPERTY()
	TArray<TObjectPtr<UNerveObjectiveRuntimeData>> AllNerveObjectiveRuntimeData;

	/** Reference to the quest subsystem */
	UPROPERTY()
	TObjectPtr<UNerveQuestSubsystem> QuestHandlerSubSystem;

public:
	// --- Initialization & Cleanup ---
	/**
	 * Initializes the quest with its asset and subsystem
	 * @param InQuestAsset The quest asset
	 * @param InSubsystem The quest subsystem
	 * @param InIsTracked Initial tracking state
	 */
	void Initialize(UNerveQuestAsset* InQuestAsset, UNerveQuestSubsystem* InSubsystem, bool InIsTracked);

	/** Cleans up quest resources */
	void Uninitialize();
	
	/** RAII Destructor - ensures cleanup */
	virtual void BeginDestroy() override;

	// --- Objective Management ---
	/**
	 * Adds an objective to the quest
	 * @param Objective The objective to add
	 * @return True if added successfully
	 */
	bool AddObjective(UNerveObjectiveRuntimeData* Objective);

	/**
	 * Removes an objective from the quest
	 * @param Objective The objective to remove
	 * @return True if removed successfully
	 */
	bool RemoveObjective(UNerveObjectiveRuntimeData* Objective);

	/** Clears all objectives from the quest */
	void ClearObjectives();

	// --- Quest Control ---
	/** Starts the quest and its first objective */
	UFUNCTION(BlueprintCallable, Category = "Quest|Control")
	void StartQuest();

	/** Sets the quest as tracked */
	UFUNCTION(BlueprintCallable, Category = "Quest|Control")
	void TrackQuest();

	/** Stops tracking the quest */
	UFUNCTION(BlueprintCallable, Category = "Quest|Control")
	void UntrackQuest();

	/** Marks the quest as complete */
	UFUNCTION(BlueprintCallable, Category = "Quest|Control")
	void MarkQuestComplete();

	/** Marks the quest as failed */
	UFUNCTION(BlueprintCallable, Category = "Quest|Control")
	void MarkQuestFailed();

	UFUNCTION(BlueprintCallable, Category = "Quest|Control")
	void SetQuestHandlerSubSystem(UNerveQuestSubsystem* QuestSubsystem);

	/**
	 * Advances to the next objective
	 * @param NextNodeIndex The index of the next node
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Control")
	void AdvanceToNextObjective(int32 NextNodeIndex = 0);

	/**
	 * Executes an objective from a pin
	 * @param OutPin The output pin to execute
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Control")
	void ExecuteObjectiveFromPin(UNerveQuestRuntimePin* OutPin);

	// --- Optional Objectives ---
	/** Starts all optional objectives for the current objective */
	UFUNCTION(BlueprintCallable, Category = "Quest|Optional Objectives")
	void StartOptionalObjectives();

	/** Stops all optional objectives */
	UFUNCTION(BlueprintCallable, Category = "Quest|Optional Objectives")
	void StopAllOptionalObjectives();

	// --- Data Access ---
	/**
	 * Gets all objectives for the quest
	 * @return Array of objectives
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	TArray<UNerveObjectiveRuntimeData*> GetAllObjectives() { return AllNerveObjectiveRuntimeData; }

	/**
	 * Gets quest rewards
	 * @return Array of rewards
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	TArray<UNerveQuestRewardBase*> GetQuestRewards() const;

	/**
	 * Finds objective data by base objective
	 * @param ObjectiveBase The base objective to find
	 * @return The objective runtime data
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	UNerveObjectiveRuntimeData* FindObjectiveData(const UNerveQuestRuntimeObjectiveBase* ObjectiveBase);

	/**
	 * Checks if the quest is completed
	 * @return True if completed
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	bool GetIsCompleted() const { return bIsCompleted; }

	// --- Callbacks ---
	/**
	 * Handles objective completion
	 * @param Objective The completed objective
	 */
	UFUNCTION()
	void OnObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* Objective);

	/**
	 * Handles objective failure
	 * @param Objective The failed objective
	 */
	UFUNCTION()
	void OnObjectiveFailed(UNerveQuestRuntimeObjectiveBase* Objective);

private:
	/**
	 * Starts optional objectives from a pin
	 * @param OptionalPin The optional pin to start from
	 */
	void StartOptionalObjectivesFromPin(UNerveQuestRuntimePin* OptionalPin);

	/**
	 * Accumulates objectives for the quest
	 * @param Objective The objective to accumulate
	 */
	void AccumulateObjectives(UNerveQuestRuntimeObjectiveBase* Objective);
};

/**
 * @class UNerveObjectiveRuntimeData
 * @brief Manages runtime data for a single quest objective.
 *
 * This class tracks objective status, progress, and UI integration for
 * both main and optional objectives.
 */
UCLASS(BlueprintType)
class LAZYNERVEQUESTRUNTIME_API UNerveObjectiveRuntimeData : public UObject
{
	GENERATED_BODY()

public:
	// --- Objective Properties ---
	/** The parent objective base */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	TObjectPtr<UNerveQuestRuntimeObjectiveBase> ParentObjective;

	/** Whether the objective is completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsCompleted;

	/** Whether the objective has failed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bHasFailed;

	/** Whether this is an optional objective */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsOptionalObjective;

	/** Display priority for UI sorting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 DisplayPriority;

	/** Parent main objective for optional objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	TObjectPtr<UNerveObjectiveRuntimeData> ParentMainObjective;

	// --- Delegates ---
	/** Broadcast when the objective is completed */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective|Events")
	FNerveQuestObjectiveAction OnObjectiveCompleted;

	/** Broadcast when the objective fails */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective|Events")
	FNerveQuestObjectiveAction OnObjectiveFailed;

private:
	// --- Internal Data ---
	/** Progress tracking widget */
	UPROPERTY()
	TObjectPtr<UObjectiveProgressTracker> TrackingWidget;

	/** Reference to the quest subsystem */
	UPROPERTY()
	TObjectPtr<UNerveQuestSubsystem> QuestHandlerSubSystem;

	/** Parent quest asset */
	UPROPERTY()
	mutable TObjectPtr<UNerveQuestAsset> ParentQuestAsset;

public:
	// --- Initialization & Cleanup ---
	/**
	 * Initializes the objective
	 * @param Objective The base objective
	 * @param QuestSubsystem The quest subsystem
	 * @param bAsOptional Whether this is an optional objective
	 * @param MainParent The parent main objective
	 */
	void Initialize(UNerveQuestRuntimeObjectiveBase* Objective, UNerveQuestSubsystem* QuestSubsystem, bool bAsOptional = false, UNerveObjectiveRuntimeData* MainParent = nullptr);

	/** Cleans up objective resources */
	void Uninitialize();
	
	/** RAII Destructor - ensures cleanup */
	virtual void BeginDestroy() override;

	// --- Objective Control ---
	/**
	 * Executes the objective
	 * @param QuestAsset The associated quest asset
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective|Control")
	void ExecuteObjective(UNerveQuestAsset* QuestAsset) const;

	/**
	 * Sets the tracking state
	 * @param Value The tracking state
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective|Control")
	void MarkAsTracked(const bool Value) const;

	// --- Data Access ---
	/**
	 * Checks if this is an optional objective
	 * @return True if optional
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective|Query")
	bool IsOptionalObjective() const { return bIsOptionalObjective; }

	/**
	 * Gets the display priority
	 * @return The display priority
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective|Query")
	int32 GetDisplayPriority() const { return DisplayPriority; }

	/**
	 * Checks if the objective is completed
	 * @return True if completed
	 */
	UFUNCTION(BlueprintPure, Category = "Objective|Query")
	bool GetIsCompleted() const { return bIsCompleted; }

	/**
	 * Checks if the objective has failed
	 * @return True if failed
	 */
	UFUNCTION(BlueprintPure, Category = "Objective|Query")
	bool GetIsFailed() const { return bHasFailed; }

	/**
	 * Gets the progress tracker widget
	 * @return The tracker widget
	 */
	UFUNCTION(BlueprintPure, Category = "Objective|Query")
	UObjectiveProgressTracker* GetObjectiveTrackerWidget() const { return TrackingWidget; }

	/** */
	UFUNCTION(BlueprintPure, Category = "Objective|Query")
	TArray<UNerveObjectiveRuntimeData*> GetOptionalObjectives() const;

	// --- Callbacks ---
	/**
	 * Handles objective completion
	 * @param Objective The completed objective
	 */
	UFUNCTION()
	void ObjectiveCompleted(UNerveQuestRuntimeObjectiveBase* Objective);

	/**
	 * Handles objective failure
	 * @param Objective The failed objective
	 */
	UFUNCTION()
	void ObjectiveFailed(UNerveQuestRuntimeObjectiveBase* Objective);

	/**
	 * Handles objective progress updates
	 * @param ObjectiveBase The objective
	 * @param NewProgressValue The new progress
	 * @param MaxProgressValue The maximum progress
	 */
	UFUNCTION()
	void ObjectiveProgress(UNerveQuestRuntimeObjectiveBase* ObjectiveBase, float NewProgressValue, float MaxProgressValue);
};