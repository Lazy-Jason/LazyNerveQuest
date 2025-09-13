// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Setting/NerveQuestEditorStyleSetting.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class LAZYNERVEQUESTEDITOR_API SNerveQuestObjectiveProperty : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNerveQuestObjectiveProperty){}
	SLATE_ARGUMENT(FString, PropertyText)
	SLATE_END_ARGS()


	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

protected:
	FSlateColor GetNodePropertyBackgroundColor() const;
	FSlateFontInfo GetNodePropertyNameFontSize() const;
	FSlateFontInfo GetNodePropertyValueFontSize() const;
	FSlateColor GetNodePropertyNameFontColor() const;
	FSlateColor GetNodePropertyValueFontColor() const;
	
	FText GetPropertyDisplayName() const;
	FText GetPropertyDisplayValue() const;

private:
    FString PropertyText = FString();
	const UNerveQuestEditorStyleSetting* StyleSetting = nullptr;
};
