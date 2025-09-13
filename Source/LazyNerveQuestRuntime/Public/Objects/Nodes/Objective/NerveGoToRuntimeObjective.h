// Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "NerveQuestRuntimeObjectiveBase.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "Engine/TimerHandle.h"
#include "Templates/SubclassOf.h"
#include "NerveGoToRuntimeObjective.generated.h"

class AGoToWorldPing;
class UWorldGotoPing;
/**
 * Enum representing the type of location for the "Go To" quest objective.
 */
UENUM(BlueprintType)
enum class EGoToQuestLocationType : uint8
{
	/** The quest requires the player to go to a specific location. */
	SpecificLocation,

	/** The quest requires the player to go to the location of a specific actor. */
	ActorLocation,
};

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UNerveGoToRuntimeObjective : public UNerveQuestRuntimeObjectiveBase
{
	GENERATED_BODY()

protected:
	/** The type of location for this objective: either a specific point or an actor's location. */
	UPROPERTY(EditAnywhere, Category="GOTO Objective")
	EGoToQuestLocationType LocationType = EGoToQuestLocationType::ActorLocation;
	
	/** The specific location the player must go to, used when LocationType is set to SpecificLocation. */
	UPROPERTY(EditAnywhere, Category="GOTO Objective", meta=(EditCondition = "LocationType == EGoToQuestLocationType::SpecificLocation",
	EditConditionHides = "LocationType == EGoToQuestLocationType::SpecificLocation"))
	FVector SpecificLocation = FVector();
	
	/** The actor whose location the player must go to, used when LocationType is set to ActorLocation. */
	UPROPERTY(EditAnywhere, Category="GOTO Objective", meta=(EditCondition = "LocationType == EGoToQuestLocationType::ActorLocation",
	EditConditionHides = "LocationType == EGoToQuestLocationType::ActorLocation"))
	TSoftObjectPtr<AActor> LocationActor = nullptr;
	
	/** The acceptable distance from the target location within which the objective will be considered complete.
	* This allows the player to be within a certain radius of the target location rather than needing to be exactly on it. */
	UPROPERTY(EditAnywhere, Category="GOTO Objective")
	float AcceptableRadialOffset = 0.0f;
	
	/** Whether to use the exact Z (height) coordinate of the target location.
	* If true, the player must reach the precise Z coordinate (useful for high-altitude locations). 
	* If false, the Z coordinate will be adjusted (snapped to ground level), allowing the player to reach the location more easily. */
	UPROPERTY(EditAnywhere, Category="GOTO Objective")
	bool bApplyAbsoluteZ = true;

	/**
	* Maximum distance to trace downwards when finding the ground level.
	* Only used when bApplyAbsoluteZ is false.
	* Set this value based on your world's scale and the maximum expected height difference.
	*/
	UPROPERTY(EditAnywhere, Category="GOTO Objective", meta=(EditCondition = "!bApplyAbsoluteZ", EditConditionHides = "!bApplyAbsoluteZ"))
	float GroundLevelTraceDistance = 10000000.0f;
	
	/** Whether or not to apply a world marker to the objective location. */
	UPROPERTY(EditAnywhere, Category="GOTO Objective")
	bool bApplyWorldMarker = true;

	UPROPERTY(EditAnywhere, Category="GOTO Objective", meta=(EditCondition = "bApplyWorldMarker == true",
	EditConditionHides = "bApplyWorldMarker == true"))
	TSubclassOf<UWorldGotoPing> PingWidgetClass = nullptr;

	/** The unit of measurement for displaying the distance to the objective. */
	UPROPERTY(EditAnywhere, Category="GOTO Objective", meta=(EditCondition = "bApplyWorldMarker == true",
	EditConditionHides = "bApplyWorldMarker == true"))
	ENerveDistanceConversionMethod DistanceConversion = ENerveDistanceConversionMethod::Centimeter;

	/** The rate at which the player location is tracked. */
	UPROPERTY(EditAnywhere, Category="GOTO Objective")
	float TrackingRate = 0.01f;

private:
	UPROPERTY()
	APawn* TrackingPlayer = nullptr;
	UPROPERTY()
	TObjectPtr<class APingManager> PingManager = nullptr;
	UPROPERTY()
	int32 CurrentPingID = -1;
	FTimerHandle TimerHandle;
	FTimerHandle DebugDrawTimerHandle;

public:
	UNerveGoToRuntimeObjective();

	virtual FText GetObjectiveName_Implementation() override;
	virtual FText GetObjectiveDescription_Implementation() override;
	virtual FText GetObjectiveCategory_Implementation() override;
	virtual FSlateBrush GetObjectiveBrush_Implementation() const override;

	virtual void ExecuteObjective_Implementation(UNerveQuestAsset* NerveQuestAsset) override;
	virtual void MarkAsTracked_Implementation(bool TrackValue) override;
	void ListenToPlayerLocation();
	FVector GetTargetLocationByLocationType(bool& Success) const;
	FVector FindGroundLevel(const UWorld* World, const FVector& StartLocation) const;
	virtual void CleanUpObjective_Implementation() override;
	virtual void BeginDestroy() override;
	void CleanupPing();

#if WITH_EDITOR
	virtual void StartObjectivePreview_Implementation(UObject* PreviewWorldContextObject) override;
	void DrawDebugVisuals(const UWorld* World) const;
	virtual void StopObjectivePreview_Implementation(UObject* PreviewWorldContextObject) override;
#endif
};
