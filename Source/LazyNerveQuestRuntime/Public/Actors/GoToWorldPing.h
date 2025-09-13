// Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "GameFramework/Actor.h"
#include "GoToWorldPing.generated.h"

class UWidgetComponent;
class UWorldGotoPing;

UCLASS()
class LAZYNERVEQUESTRUNTIME_API APingManager : public AActor
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> RootSceneComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ping Management", meta = (AllowPrivateAccess = "true"))
    TArray<FPingComponent> PingComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping Settings", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> DefaultPingWidgetClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping Settings", meta = (AllowPrivateAccess = "true"))
    float EdgeMargin = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping Settings", meta = (AllowPrivateAccess = "true"))
    float UpdateRate = 0.016f; // ~60 FPS

    UPROPERTY()
    FTimerHandle UpdateTimerHandle;

    UPROPERTY()
    int32 NextPingID = 0;

public:
    APingManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintPure, Category = "Ping Management", meta = (WorldContext = "World"))
	static APingManager* GetOrCreatePingManager(UWorld* World);
	
    UFUNCTION(BlueprintCallable, Category = "Ping Management")
    int32 CreatePing(const FVector& WorldLocation, TSubclassOf<UUserWidget> WidgetClass = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Ping Management")
    bool UpdatePingLocation(int32 PingID, const FVector& NewWorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Ping Management")
    bool UpdatePingDistance(int32 PingID, float NewDistance, ENerveDistanceConversionMethod ConversionMethod);

    UFUNCTION(BlueprintCallable, Category = "Ping Management")
    bool SetPingVisibility(int32 PingID, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Ping Management")
    bool RemovePing(int32 PingID);

    UFUNCTION(BlueprintCallable, Category = "Ping Management")
    void RemoveAllPings();

    UFUNCTION(BlueprintCallable, Category = "Ping Management")
    bool IsPingValid(int32 PingID) const;

    UFUNCTION(BlueprintCallable, Category = "Ping Management")
    FPingData GetPingData(int32 PingID) const;

protected:
    UFUNCTION()
    void UpdateAllPings();

    void UpdatePingScreenPosition(FPingComponent& PingComponent);
    FVector2D CalculateEdgePosition(const FVector2D& ScreenPosition, const FVector2D& ViewportSize) const;
    bool IsPositionOnScreen(const FVector2D& ScreenPosition, const FVector2D& ViewportSize) const;
    FPingComponent* FindPingByID(int32 PingID);
    const FPingComponent* FindPingByID(int32 PingID) const;

private:
    UWidgetComponent* CreateWidgetComponent(const TSubclassOf<UUserWidget>& WidgetClass);
    static void CleanupPingComponent(FPingComponent& PingComponent);
};