// Copyright (C) 2024 Job Omondiale - All Rights Reserved

#include "Objects/Nodes/Objective/NerveGoToRuntimeObjective.h"
#include "LazyNerveRuntimeQuestStyle.h"
#include "TimerManager.h"
#include "Actors/GoToWorldPing.h"
#include "GameFramework/Pawn.h"
#include "Widget/WorldGotoPing.h"
#include "Kismet/GameplayStatics.h"

UNerveGoToRuntimeObjective::UNerveGoToRuntimeObjective()
{}

FText UNerveGoToRuntimeObjective::GetObjectiveName_Implementation()
{
	return FText::FromString(TEXT("GoTo"));
}

FText UNerveGoToRuntimeObjective::GetObjectiveDescription_Implementation()
{
	return FText::FromString(TEXT("Travel to a specific location within the game world."));
}

FText UNerveGoToRuntimeObjective::GetObjectiveCategory_Implementation()
{
	return FText::FromString(TEXT("Primitive Objectives"));
}

FSlateBrush UNerveGoToRuntimeObjective::GetObjectiveBrush_Implementation() const
{
	const FSlateBrush* Brush = FLazyNerveRuntimeQuestStyle::Get().GetBrush("QuestPOIBrush");
	return *Brush;
}

void UNerveGoToRuntimeObjective::ExecuteObjective_Implementation(UNerveQuestAsset* NerveQuestAsset)
{
    UNerveQuestRuntimeObjectiveBase::ExecuteObjective_Implementation(NerveQuestAsset);
    
    // Check if the World is valid
    if (!IsValid(GetWorld()))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid World in UNerveGoToRuntimeObjective::ExecuteObjective_Implementation"));
        FailObjective();
        return;
    }

    // Get the player pawn
    const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!IsValid(PlayerController))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid PlayerController in UNerveGoToRuntimeObjective::ExecuteObjective_Implementation"));
        FailObjective();
        return;
    }

    TrackingPlayer = PlayerController->GetPawn();

    if (!IsValid(TrackingPlayer))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid TrackingPlayer in UNerveGoToRuntimeObjective::ExecuteObjective_Implementation"));
        FailObjective();
        return;
    }

    // Clear any existing timer
    if (TimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    }

    // Get or create the ping manager
    if (!IsValid(PingManager))
    {
        // Try to find existing ping manager in the world
        PingManager = APingManager::GetOrCreatePingManager(GetWorld());
        
        // If none exists, create one
        if (!IsValid(PingManager))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create PingManager"));
            FailObjective();
            return;
        }
    }

    // Create the ping if we should apply world marker
    if (bApplyWorldMarker && IsValid(PingWidgetClass))
    {
        // Get initial target location
        bool Success;
        FVector TargetLocation = GetTargetLocationByLocationType(Success);
        
        if (Success)
        {
            if (!bApplyAbsoluteZ)
            {
                TargetLocation = FindGroundLevel(GetWorld(), TargetLocation);
            }
            
            CurrentPingID = PingManager->CreatePing(TargetLocation, PingWidgetClass);
            if (CurrentPingID == -1)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create ping"));
            }
        }
    }
    
    // Set up the timer to periodically check player location
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UNerveGoToRuntimeObjective::ListenToPlayerLocation, TrackingRate, true);
}

void UNerveGoToRuntimeObjective::MarkAsTracked_Implementation(const bool TrackValue)
{
    if (!IsValid(PingManager) || CurrentPingID == -1) return;
        
    PingManager->SetPingVisibility(CurrentPingID, TrackValue);
}

void UNerveGoToRuntimeObjective::ListenToPlayerLocation()
{
    // Check if the tracking player is still valid
    if (!IsValid(TrackingPlayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("TrackingPlayer became invalid in UNerveGoToRuntimeObjective::ListenToPlayerLocation"));
        if (IsValid(GetWorld()))
        {
            GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
        }

        CleanupPing();
        FailObjective();
        return;
    }

    // Get the target location based on the LocationType
    bool Success;
    FVector TargetLocation = GetTargetLocationByLocationType(Success);
    
    if (!Success)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid LocationActor in UNerveGoToRuntimeObjective::ListenToPlayerLocation"));
        FailObjective();
        return;
    }

    if (!bApplyAbsoluteZ)
    {
        TargetLocation = FindGroundLevel(GetWorld(), TargetLocation);
    }
    
    // Calculate the distance between the player and the target location
    const float Distance = FVector::Distance(TrackingPlayer->GetActorLocation(), TargetLocation);

    // Update ping if it exists
    if (IsValid(PingManager) && CurrentPingID != -1)
    {
        PingManager->UpdatePingLocation(CurrentPingID, TargetLocation);
        PingManager->UpdatePingDistance(CurrentPingID, ConvertDistance(Distance, DistanceConversion), DistanceConversion);
    }

    // Check if the player is within the acceptable range
    if (Distance <= AcceptableRadialOffset)
    {
        UE_LOG(LogTemp, Log, TEXT("Player reached the target location. Completing objective."));
        if (IsValid(GetWorld()))
        {
            GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
        }

        CleanupPing();
        CompleteObjective();
    }
}

FVector UNerveGoToRuntimeObjective::GetTargetLocationByLocationType(bool& Success) const
{
    Success = true;
    if (LocationType == EGoToQuestLocationType::SpecificLocation) 
    {
        return SpecificLocation;
    }
    if (IsValid(LocationActor.LoadSynchronous()))
    {
        return LocationActor.Get()->GetActorLocation();
    }
    Success = false;
    return {};
}

FVector UNerveGoToRuntimeObjective::FindGroundLevel(const UWorld* World, const FVector& StartLocation) const
{
    if (!IsValid(World))
    {
        return StartLocation;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = false;

    const FVector EndLocation = StartLocation - FVector(0, 0, GroundLevelTraceDistance);

    if (FHitResult HitResult; World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    ))
    {
        return HitResult.Location;
    }

    return StartLocation;
}

void UNerveGoToRuntimeObjective::CleanUpObjective_Implementation()
{
    CleanupPing();
}

void UNerveGoToRuntimeObjective::CleanupPing()
{
    if (IsValid(PingManager) && CurrentPingID != -1)
    {
        PingManager->RemovePing(CurrentPingID);
        CurrentPingID = -1;
    }
}

void UNerveGoToRuntimeObjective::BeginDestroy()
{
    CleanupPing();
    Super::BeginDestroy();
}

#if WITH_EDITOR
void UNerveGoToRuntimeObjective::StartObjectivePreview_Implementation(UObject* PreviewWorldContextObject)
{
    if (!IsValid(PreviewWorldContextObject) || !IsValid(PreviewWorldContextObject->GetWorld()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid PreviewWorldContextObject in StartObjectivePreview"));
        return;
    }

    const UWorld* World = PreviewWorldContextObject->GetWorld();

    // Set up a looping timer
    World->GetTimerManager().SetTimer
    (
        DebugDrawTimerHandle,
        FTimerDelegate::CreateUObject(this, &UNerveGoToRuntimeObjective::DrawDebugVisuals, World),
        0.1f,
        true
    );
}

void UNerveGoToRuntimeObjective::DrawDebugVisuals(const UWorld* World) const
{
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid StoredPreviewWorldContextObject in DrawDebugVisuals"));
        return;
    }

    bool Success;
    FVector TargetLocation = GetTargetLocationByLocationType(Success);
    if (Success)
    {
        // Draw a line from the ground to the target location if bApplyAbsoluteZ is false
        if (!bApplyAbsoluteZ)
        {
            TargetLocation = FindGroundLevel(World, TargetLocation);
        }

        // Draw a debug sphere at the target location
        DrawDebugSphere(World, TargetLocation, AcceptableRadialOffset, 32, FColor::Green,
            false, 0.2f, 0, 5.0f);
    }
}

void UNerveGoToRuntimeObjective::StopObjectivePreview_Implementation(UObject* PreviewWorldContextObject)
{
    if (!IsValid(PreviewWorldContextObject) || !IsValid(PreviewWorldContextObject->GetWorld())) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid PreviewWorldContextObject in StopObjectivePreview"));
        return;
    }

    const UWorld* World = PreviewWorldContextObject->GetWorld();
    
    // Clear the timer
    World->GetTimerManager().ClearTimer(DebugDrawTimerHandle);

    FlushPersistentDebugLines(World);
    FlushDebugStrings(World);
}
#endif
