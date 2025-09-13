// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/StructsAndEnums/NerveQuestStructsAndEnums.h"
#include "Setting/NerveQuestRuntimeSetting.h"


float FNerveDistanceConversionSettings::ConvertDistance(float DistanceInUnrealUnits, ENerveDistanceConversionMethod ConversionMethod) const
{
	switch (ConversionMethod)
	{
	case ENerveDistanceConversionMethod::Centimeter:
		return DistanceInUnrealUnits * UnrealUnitToCentimeter;
	case ENerveDistanceConversionMethod::Meter:
		return DistanceInUnrealUnits * UnrealUnitToMeter;
	case ENerveDistanceConversionMethod::Kilometer:
		return DistanceInUnrealUnits * UnrealUnitToKilometer;
	case ENerveDistanceConversionMethod::Foot:
		return DistanceInUnrealUnits * UnrealUnitToFoot;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown conversion method in FNerveDistanceConversionSettings::ConvertDistance. Using centimeters."));
		return DistanceInUnrealUnits * UnrealUnitToCentimeter;
	}
}

float ConvertDistance(const float DistanceInUnrealUnits, const ENerveDistanceConversionMethod ConversionMethod)
{
	const UNerveQuestRuntimeSetting* Settings = GetDefault<UNerveQuestRuntimeSetting>();
	if (!IsValid(Settings))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load NerveQuestRuntimeSetting in ConvertDistance. Using default centimeter conversion."));
		return DistanceInUnrealUnits; // Fallback to 1:1 if settings are unavailable
	}

	return Settings->DistanceConversions.ConvertDistance(DistanceInUnrealUnits, ConversionMethod);
}
