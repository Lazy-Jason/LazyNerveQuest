// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NerveQuestStructsAndEnums.generated.h"

class UWorldGotoPing;
class UWidgetComponent;
class UNerveQuestAsset;
// Quest category enum to track the current state of a quest
UENUM(BlueprintType)
enum class ENerveQuestCategory : uint8
{
	Available UMETA(DisplayName = "Available"),
	InProgress UMETA(DisplayName = "In Progress"),
	Failed UMETA(DisplayName = "Failed"),
	Completed UMETA(DisplayName = "Completed")
};

// Quest types enum to differentiate between main and side quests
UENUM(BlueprintType)
enum class ENerveQuestTypes : uint8
{
	MainQuest UMETA(DisplayName = "Main Quest"),
	SideQuest UMETA(DisplayName = "Side Quest")
};

// Quest difficulty enum to indicate the challenge level of a quest
UENUM(BlueprintType)
enum class ENerveQuestDifficulty : uint8
{
	Easy UMETA(DisplayName = "Easy"),
	Medium UMETA(DisplayName = "Medium"),
	Hard UMETA(DisplayName = "Hard"),
	VeryHard UMETA(DisplayName = "Very Hard")
};

UENUM(BlueprintType)
enum class ENerveDistanceConversionMethod : uint8
{
	Centimeter     UMETA(DisplayName = "Centimeter"),
	Meter          UMETA(DisplayName = "Meter"),
	Kilometer      UMETA(DisplayName = "Kilometer"),
	Foot           UMETA(DisplayName = "Foot"),
};

UENUM(BlueprintType)
enum class ENerveObjectiveTrackerPosition : uint8
{
	Inline     UMETA(DisplayName = "Inline"),
	Full          UMETA(DisplayName = "Full"),
};

UENUM(BlueprintType)
enum class EQuestObjectiveEventType : uint8
{
	QuestStarted,
	QuestCompleted,
	QuestFailed,
	QuestObjectiveStarted,
	QuestObjectiveCompleted,
	QuestObjectiveFailed,
};

UENUM(BlueprintType)
enum class EOptionalObjectiveResponse : uint8
{
	NoEffect           UMETA(DisplayName = "No Effect"),
	CompleteParent     UMETA(DisplayName = "Complete Parent Objective"),
	FailParent         UMETA(DisplayName = "Fail Parent Objective"),
	AdvanceParent      UMETA(DisplayName = "Advance Parent to Next"),
	BlockParent        UMETA(DisplayName = "Block Parent Progress"),
};

UENUM(BlueprintType)
enum class EObjectiveState : uint8
{
	NotStarted     UMETA(DisplayName = "Not Started"),
	InProgress     UMETA(DisplayName = "In Progress"),
	Completed      UMETA(DisplayName = "Completed"),
	Failed         UMETA(DisplayName = "Failed"),
	Optional       UMETA(DisplayName = "Optional"),
	Blocked        UMETA(DisplayName = "Blocked")
};

/** 
 * Specifies what happens when this objective fails.
 * Determines the quest flow control after an objective failure.
 */
UENUM(BlueprintType)
enum class EObjectiveFailureResponse : uint8
{
	/** Fails the entire quest immediately */
	FailQuest,
    
	/** Marks this objective as failed but continues to the next objective */
	ContinueToNextObjective,
    
	/** Restarts the entire quest from the beginning */
	RestartQuest,
};

USTRUCT(BlueprintType)
struct FOptionalObjectiveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optional")
	class UNerveObjectiveRuntimeData* OptionalObjective = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optional")
	class UNerveObjectiveRuntimeData* ParentObjective = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optional")
	bool bIsCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optional")
	bool bHasFailed = false;

	FOptionalObjectiveData()
	{
		OptionalObjective = nullptr;
		ParentObjective = nullptr;
		bIsCompleted = false;
		bHasFailed = false;
	}
};

USTRUCT(BlueprintType)
struct FOptionalObjectiveDataArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optional")
	TArray<FOptionalObjectiveData> ObjectiveData = TArray<FOptionalObjectiveData>();

	FOptionalObjectiveDataArray(): ObjectiveData()
	{}
};

USTRUCT(BlueprintType)
struct FNerveQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString QuestTitle = FString(TEXT("Nerve Title"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	class UNerveQuestRuntimeObjectiveBase* PerformingObjective;

	FNerveQuestObjective()
		: QuestTitle(TEXT("Nerve Title"))
		, PerformingObjective(nullptr)
	{}
	
	FNerveQuestObjective(const FString& NewTitle, class UNerveQuestRuntimeObjectiveBase* NewObjective)
		: QuestTitle(NewTitle)
		,PerformingObjective(NewObjective)
	{}
};

// Struct to define conversion factors from Unreal Units (UU) to other units
USTRUCT(BlueprintType)
struct FNerveDistanceConversionSettings
{
	GENERATED_BODY()

	UPROPERTY(config, EditAnywhere, Category="Distance Conversion", meta=(ClampMin="0.000001", DisplayName="Centimeters per Unreal Unit"))
	float UnrealUnitToCentimeter = 1.0f;

	UPROPERTY(config, EditAnywhere, Category="Distance Conversion", meta=(ClampMin="0.000001", DisplayName="Meters per Unreal Unit"))
	float UnrealUnitToMeter = 0.01f;

	UPROPERTY(config, EditAnywhere, Category="Distance Conversion", meta=(ClampMin="0.000001", DisplayName="Kilometers per Unreal Unit"))
	float UnrealUnitToKilometer = 0.00001f;

	UPROPERTY(config, EditAnywhere, Category="Distance Conversion", meta=(ClampMin="0.000001", DisplayName="Feet per Unreal Unit"))
	float UnrealUnitToFoot = 0.0328084f;

	// Utility function to convert distance using this struct's settings
	float ConvertDistance(float DistanceInUnrealUnits, ENerveDistanceConversionMethod ConversionMethod) const;
};

USTRUCT(BlueprintType)
struct FPingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENerveDistanceConversionMethod DistanceConversion = ENerveDistanceConversionMethod::Meter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnScreen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D EdgePosition = FVector2D::ZeroVector;

	FPingData()
	{
		WorldLocation = FVector::ZeroVector;
		Distance = 0.0f;
		DistanceConversion = ENerveDistanceConversionMethod::Meter;
		bIsVisible = true;
		bIsOnScreen = true;
		ScreenPosition = FVector2D::ZeroVector;
		EdgePosition = FVector2D::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FPingComponent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> WidgetComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWorldGotoPing> PingWidget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPingData PingData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PingID = -1;

	FPingComponent()
	{
		WidgetComponent = nullptr;
		PingWidget = nullptr;
		PingID = -1;
	}
};

// Utility function to convert distance using settings from UNerveQuestRuntimeSetting
LAZYNERVEQUESTRUNTIME_API float ConvertDistance(float DistanceInUnrealUnits, ENerveDistanceConversionMethod ConversionMethod);