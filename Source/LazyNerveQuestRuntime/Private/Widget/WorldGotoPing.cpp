// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/WorldGotoPing.h"

UTextBlock* UWorldGotoPing::GetDistanceTextBlock_Implementation()
{ return nullptr; }

void UWorldGotoPing::OnScreenStateChanged_Implementation(bool bIsOnScreen)
{}

void UWorldGotoPing::UpdateDistance(const float NewDistance, const ENerveDistanceConversionMethod ConversionMethod)
{
	if (!IsValid(GetDistanceTextBlock())) return;

	// Determine the unit string based on the conversion method
	FString UnitString;
	switch (ConversionMethod)
	{
		case ENerveDistanceConversionMethod::Centimeter:
			UnitString = TEXT(" cm");
			break;
		case ENerveDistanceConversionMethod::Meter:
			UnitString = TEXT(" m");
			break;
		case ENerveDistanceConversionMethod::Kilometer:
			UnitString = TEXT(" km");
			break;
		case ENerveDistanceConversionMethod::Foot:
			UnitString = TEXT(" ft");
			break;
		default:
			UnitString = TEXT(""); // Fallback in case of invalid enum value
			break;
	}

	// Format the distance with the unit and update the text block
	const FString DistanceString = FString::Printf(TEXT("%.0f%s"), NewDistance, *UnitString);
	GetDistanceTextBlock()->SetText(FText::FromString(DistanceString));
}

void UWorldGotoPing::SetIsOnScreen(const bool bIsOnScreen)
{
	if (bCurrentlyOnScreen != bIsOnScreen)
	{
		bCurrentlyOnScreen = bIsOnScreen;
		OnScreenStateChanged(bIsOnScreen);
	}
}