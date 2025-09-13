// Copyright (C) 2024 Job Omondiale - All Rights Reserved

#include "Actors/GoToWorldPing.h"
#include "Components/WidgetComponent.h"
#include "Widget/WorldGotoPing.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

APingManager::APingManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    EdgeMargin = 50.0f;
    UpdateRate = 0.016f; // ~60 FPS
    NextPingID = 0;
}

void APingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Start the update timer
    if (IsValid(GetWorld()))
    {
        GetWorld()->GetTimerManager().SetTimer
        (
            UpdateTimerHandle,
            this,
            &APingManager::UpdateAllPings,
            UpdateRate,
            true
        );
    }
}

void APingManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear the timer
    if (IsValid(GetWorld()) && UpdateTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }
    
    // Clean up all ping components
    RemoveAllPings();
    
    Super::EndPlay(EndPlayReason);
}

APingManager* APingManager::GetOrCreatePingManager(UWorld* World)
{
    if (!IsValid(World)) return nullptr;
    
    // Try to find existing ping manager in the world
    APingManager* NewPingManager = Cast<APingManager>(UGameplayStatics::GetActorOfClass(World, StaticClass()));
        
    // If none exists, create one
    if (!IsValid(NewPingManager))
    {
        NewPingManager = World->SpawnActor<APingManager>();
        if (!IsValid(NewPingManager))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create PingManager"));
            return nullptr;
        }
    }
    return NewPingManager;
}

int32 APingManager::CreatePing(const FVector& WorldLocation, const TSubclassOf<UUserWidget> WidgetClass)
{
    if (!IsValid(GetWorld()))
    {
        UE_LOG(LogTemp, Error, TEXT("APingManager::CreatePing - Invalid World"));
        return -1;
    }

    // Use provided widget class or fall back to default
    const TSubclassOf<UUserWidget> ActualWidgetClass = WidgetClass ? WidgetClass : DefaultPingWidgetClass;
    if (!IsValid(ActualWidgetClass))
    {
        UE_LOG(LogTemp, Error, TEXT("APingManager::CreatePing - No valid widget class provided"));
        return -1;
    }

    // Create new ping component
    FPingComponent NewPingComponent;
    NewPingComponent.PingID = NextPingID++;
    NewPingComponent.PingData.WorldLocation = WorldLocation;
    
    // Create widget component
    NewPingComponent.WidgetComponent = CreateWidgetComponent(ActualWidgetClass);
    if (!IsValid(NewPingComponent.WidgetComponent))
    {
        UE_LOG(LogTemp, Error, TEXT("APingManager::CreatePing - Failed to create widget component"));
        return -1;
    }

    // Initialize the widget
    NewPingComponent.WidgetComponent->InitWidget();
    NewPingComponent.PingWidget = Cast<UWorldGotoPing>(NewPingComponent.WidgetComponent->GetUserWidgetObject());
    
    if (!IsValid(NewPingComponent.PingWidget))
    {
        UE_LOG(LogTemp, Error, TEXT("APingManager::CreatePing - Failed to cast to UWorldGotoPing"));
        CleanupPingComponent(NewPingComponent);
        return -1;
    }

    // Add to our array
    const int32 PingID = NewPingComponent.PingID;
    PingComponents.Add(MoveTemp(NewPingComponent));
    
    UE_LOG(LogTemp, Log, TEXT("APingManager::CreatePing - Created ping with ID: %d"), PingID);
    return PingID;
}

bool APingManager::UpdatePingLocation(const int32 PingID, const FVector& NewWorldLocation)
{
    FPingComponent* PingComponent = FindPingByID(PingID);
    if (!PingComponent) return false;

    PingComponent->PingData.WorldLocation = NewWorldLocation;
    return true;
}

bool APingManager::UpdatePingDistance(const int32 PingID, const float NewDistance, const ENerveDistanceConversionMethod ConversionMethod)
{
    FPingComponent* PingComponent = FindPingByID(PingID);
    if (!PingComponent || !IsValid(PingComponent->PingWidget)) return false;

    PingComponent->PingData.Distance = NewDistance;
    PingComponent->PingData.DistanceConversion = ConversionMethod;
    
    // Update the widget
    PingComponent->PingWidget->UpdateDistance(NewDistance, ConversionMethod);
    return true;
}

bool APingManager::SetPingVisibility(const int32 PingID, const bool bVisible)
{
    FPingComponent* PingComponent = FindPingByID(PingID);
    if (!PingComponent || !IsValid(PingComponent->WidgetComponent)) return false;

    PingComponent->PingData.bIsVisible = bVisible;
    PingComponent->WidgetComponent->SetVisibility(bVisible);
    return true;
}

bool APingManager::RemovePing(const int32 PingID)
{
    for (int32 i = PingComponents.Num() - 1; i >= 0; --i)
    {
        if (PingComponents[i].PingID == PingID)
        {
            CleanupPingComponent(PingComponents[i]);
            PingComponents.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("APingManager::RemovePing - Removed ping with ID: %d"), PingID);
            return true;
        }
    }
    return false;
}

void APingManager::RemoveAllPings()
{
    for (FPingComponent& PingComponent : PingComponents)
    {
        CleanupPingComponent(PingComponent);
    }
    PingComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("APingManager::RemoveAllPings - Removed all pings"));
}

bool APingManager::IsPingValid(const int32 PingID) const
{
    return FindPingByID(PingID) != nullptr;
}

FPingData APingManager::GetPingData(const int32 PingID) const
{
    if (const FPingComponent* PingComponent = FindPingByID(PingID))
    {
        return PingComponent->PingData;
    }
    return FPingData();
}

void APingManager::UpdateAllPings()
{
    if (!IsValid(GetWorld())) return;

    const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!IsValid(PlayerController)) return;

    // Update each ping's screen position and visibility
    for (FPingComponent& PingComponent : PingComponents)
    {
        if (!IsValid(PingComponent.WidgetComponent) || !PingComponent.PingData.bIsVisible) continue;

        UpdatePingScreenPosition(PingComponent);
    }
}

void APingManager::UpdatePingScreenPosition(FPingComponent& PingComponent)
{
    const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!IsValid(PlayerController)) return;

    // Get viewport size
    int32 ViewportSizeX, ViewportSizeY;
    PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
    const FVector2D ViewportSize(ViewportSizeX, ViewportSizeY);

    // Project world location to screen
    FVector2D ScreenPosition;
    const bool bProjected = PlayerController->ProjectWorldLocationToScreen(
        PingComponent.PingData.WorldLocation,
        ScreenPosition,
        true // bPlayerViewportRelative
    );

    if (!bProjected)
    {
        // If we can't project, hide the ping
        PingComponent.WidgetComponent->SetVisibility(false);
        return;
    }

    // Update ping data
    PingComponent.PingData.ScreenPosition = ScreenPosition;
    PingComponent.PingData.bIsOnScreen = IsPositionOnScreen(ScreenPosition, ViewportSize);

    FVector2D FinalPosition;
    if (PingComponent.PingData.bIsOnScreen)
    {
        // Use actual screen position
        FinalPosition = ScreenPosition;
    }
    else
    {
        // Calculate edge position
        FinalPosition = CalculateEdgePosition(ScreenPosition, ViewportSize);
        PingComponent.PingData.EdgePosition = FinalPosition;
    }

    // Update widget position
    PingComponent.WidgetComponent->SetWorldLocation(PingComponent.PingData.WorldLocation);
    
    // Make sure the ping is visible
    if (!PingComponent.WidgetComponent->IsVisible())
    {
        PingComponent.WidgetComponent->SetVisibility(true);
    }

    // Update the widget with on-screen status if it supports it
    if (IsValid(PingComponent.PingWidget))
    {
        PingComponent.PingWidget->SetIsOnScreen(PingComponent.PingData.bIsOnScreen);
    }
}

FVector2D APingManager::CalculateEdgePosition(const FVector2D& ScreenPosition, const FVector2D& ViewportSize) const
{
    const FVector2D Center = ViewportSize * 0.5f;
    const FVector2D Direction = (ScreenPosition - Center).GetSafeNormal();
    
    // Calculate intersection with viewport edges considering margin
    const float MaxX = ViewportSize.X - EdgeMargin;
    const float MaxY = ViewportSize.Y - EdgeMargin;
    const float MinX = EdgeMargin;
    const float MinY = EdgeMargin;
    
    FVector2D EdgePosition = Center;
    
    if (FMath::Abs(Direction.X) > FMath::Abs(Direction.Y))
    {
        // Intersect with left or right edge
        if (Direction.X > 0)
        {
            EdgePosition.X = MaxX;
            EdgePosition.Y = Center.Y + Direction.Y * (MaxX - Center.X) / Direction.X;
        }
        else
        {
            EdgePosition.X = MinX;
            EdgePosition.Y = Center.Y + Direction.Y * (MinX - Center.X) / Direction.X;
        }
        
        // Clamp Y to screen bounds
        EdgePosition.Y = FMath::Clamp(EdgePosition.Y, MinY, MaxY);
    }
    else
    {
        // Intersect with top or bottom edge
        if (Direction.Y > 0)
        {
            EdgePosition.Y = MaxY;
            EdgePosition.X = Center.X + Direction.X * (MaxY - Center.Y) / Direction.Y;
        }
        else
        {
            EdgePosition.Y = MinY;
            EdgePosition.X = Center.X + Direction.X * (MinY - Center.Y) / Direction.Y;
        }
        
        // Clamp X to screen bounds
        EdgePosition.X = FMath::Clamp(EdgePosition.X, MinX, MaxX);
    }
    
    return EdgePosition;
}

bool APingManager::IsPositionOnScreen(const FVector2D& ScreenPosition, const FVector2D& ViewportSize) const
{
    return ScreenPosition.X >= EdgeMargin && 
           ScreenPosition.X <= ViewportSize.X - EdgeMargin &&
           ScreenPosition.Y >= EdgeMargin && 
           ScreenPosition.Y <= ViewportSize.Y - EdgeMargin;
}

FPingComponent* APingManager::FindPingByID(const int32 PingID)
{
    for (FPingComponent& PingComponent : PingComponents)
    {
        if (PingComponent.PingID == PingID)
        {
            return &PingComponent;
        }
    }
    return nullptr;
}

const FPingComponent* APingManager::FindPingByID(const int32 PingID) const
{
    for (const FPingComponent& PingComponent : PingComponents)
    {
        if (PingComponent.PingID == PingID)
        {
            return &PingComponent;
        }
    }
    return nullptr;
}


UWidgetComponent* APingManager::CreateWidgetComponent(const TSubclassOf<UUserWidget>& WidgetClass)
{
    if (!WidgetClass)  return nullptr;

    // Create a unique name for the widget component
    const FString ComponentName = FString::Printf(TEXT("PingWidget_%d"), NextPingID);
    
    // Use NewObject instead of CreateDefaultSubobject for runtime creation
    UWidgetComponent* WidgetComponent = NewObject<UWidgetComponent>(this, *ComponentName);
    if (!IsValid(WidgetComponent)) return nullptr;

    // Configure the widget component
    WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    WidgetComponent->SetDrawAtDesiredSize(true);
    WidgetComponent->SetWidgetClass(WidgetClass);
    
    // Attach to root component
    WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    
    // Register the component with the actor
    WidgetComponent->RegisterComponent();
    
    return WidgetComponent;
}

void APingManager::CleanupPingComponent(FPingComponent& PingComponent)
{
    if (IsValid(PingComponent.WidgetComponent))
    {
        PingComponent.WidgetComponent->DestroyComponent();
        PingComponent.WidgetComponent = nullptr;
    }
    PingComponent.PingWidget = nullptr;
}