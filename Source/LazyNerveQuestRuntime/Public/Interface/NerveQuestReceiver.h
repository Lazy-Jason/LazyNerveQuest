// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "UObject/Interface.h"
#include "NerveQuestReceiver.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNerveQuestReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LAZYNERVEQUESTRUNTIME_API INerveQuestReceiver
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="NerveQuest")
	void ExecuteReceiveEvent(UNerveQuestAsset* QuestAsset, const EQuestObjectiveEventType ReceivedEventType);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="NerveQuest")
	void ExecuteReceiveTag(UNerveQuestAsset* QuestAsset, const FGameplayTag& ReceivedGameplayTag);
};
