// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "WorldGotoPing.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UWorldGotoPing : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY()
	bool bCurrentlyOnScreen = true;
	
protected:
	UFUNCTION(BlueprintNativeEvent, Category="GoTo Ping")
	UTextBlock* GetDistanceTextBlock();

	// Optional: Override this in Blueprint to handle on-screen/off-screen visual changes
	UFUNCTION(BlueprintNativeEvent, Category="GoTo Ping")
	void OnScreenStateChanged(bool bIsOnScreen);

public:
	UFUNCTION(BlueprintCallable, Category="GoTo Ping")
	void UpdateDistance(float NewDistance, ENerveDistanceConversionMethod ConversionMethod);

	UFUNCTION(BlueprintCallable, Category="GoTo Ping")
	void SetIsOnScreen(bool bIsOnScreen);

	UFUNCTION(BlueprintPure, Category="GoTo Ping")
	bool GetIsOnScreen() const { return bCurrentlyOnScreen; }
};
