// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "Engine/DeveloperSettings.h"
#include "NerveQuestRuntimeSetting.generated.h"

/**
 * 
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Nerve Quest System"))
class LAZYNERVEQUESTRUNTIME_API UNerveQuestRuntimeSetting : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UNerveQuestRuntimeSetting(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(config, EditAnywhere, Category="Quest")
	TSubclassOf<class UQuestScreen> NerveQuestScreen = nullptr;

	UPROPERTY(config, EditAnywhere, Category="Quest")
	int32 QuestScreenZOrder = 0;

	// New property for distance conversion settings
	UPROPERTY(config, EditAnywhere, Category="Distance Conversion")
	FNerveDistanceConversionSettings DistanceConversions;
};
