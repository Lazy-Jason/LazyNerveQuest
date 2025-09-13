// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectiveProgressTracker.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class LAZYNERVEQUESTRUNTIME_API UObjectiveProgressTracker : public UUserWidget
{
	GENERATED_BODY()

	float Current = 0.0f;
	float Max = 0.0f;

public:
	UFUNCTION(BlueprintPure, Category="Current")
	float GetCurrent() const { return Current; }

	UFUNCTION(BlueprintPure, Category="Max")
	float GetMax() const { return Max; }

	UFUNCTION(BlueprintNativeEvent, Category="Current")
	void SetCurrent(const float NewCurrent);

	UFUNCTION(BlueprintNativeEvent, Category="Max")
	void SetMax(const float NewMax);
};
