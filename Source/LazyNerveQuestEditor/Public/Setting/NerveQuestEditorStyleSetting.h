// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NerveQuestEditorStyleSetting.generated.h"

/**
 * 
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Nerve Quest Editor Style"))
class LAZYNERVEQUESTEDITOR_API UNerveQuestEditorStyleSetting : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UNerveQuestEditorStyleSetting(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(config, EditAnywhere, Category="NodeFontStyle")
	int32 NodeHeaderFontSize = 23;

	UPROPERTY(config, EditAnywhere, Category="NodeFontStyle")
	int32 NodePropertyFontSize = 10;

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	float NodeBodyMinWidth = 400.0f;

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	float NodeBodyMaxWidth = 550.0f;

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	float NodeBodyMinHeight = 150.0f;

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	float NodeBodyMaxHeight = 550.0f;

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FSlateColor NodeHeaderFontColor = FLinearColor::White;

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FSlateColor NodePropertyNameFontColor = FLinearColor::White;

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FSlateColor NodePropertyValueFontColor = FLinearColor::White;

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FSlateColor NodeIconColor = FLinearColor::White;
	
	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FSlateColor NodeBodyColor = FLinearColor(FColor::FromHex("1c1a1e"));

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FSlateColor NodeBodyGradientColor = FLinearColor(FColor::FromHex("945DCC"));

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FMargin NodeBodyGradientPadding = FMargin(15);

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FSlateColor NodeBodyHighlightColor = FLinearColor(FColor::FromHex("4e4854"));

	UPROPERTY(config, EditAnywhere, Category="NodeStyle")
	FSlateColor NodeBodyPropertyBackdropColor = FLinearColor(FColor(0, 0, 0, 40));

	UPROPERTY(config, EditAnywhere, Category="PinStyle")
	FMargin PinPadding = FMargin(15);
	
	UPROPERTY(config, EditAnywhere, Category="PinStyle")
	FSlateColor PinUnHoverColor = FLinearColor(FColor::FromHex("161517"));

	UPROPERTY(config, EditAnywhere, Category="PinStyle")
	FSlateColor PinHoverColor = FLinearColor(FColor::FromHex("312c36"));
};
