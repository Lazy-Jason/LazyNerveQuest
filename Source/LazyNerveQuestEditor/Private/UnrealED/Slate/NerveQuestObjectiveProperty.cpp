// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealED/Slate/NerveQuestObjectiveProperty.h"
#include "LazyNerveQuestStyle.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SNerveQuestObjectiveProperty::Construct(const FArguments& InArgs)
{
	PropertyText = InArgs._PropertyText;
	StyleSetting = GetDefault<UNerveQuestEditorStyleSetting>();
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FLazyNerveQuestStyle::Get().GetBrush("WhiteRoundedBrush"))
		.BorderBackgroundColor(this, &SNerveQuestObjectiveProperty::GetNodePropertyBackgroundColor)
		.Padding(FMargin(5))
		[
			SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(5)
            [
            	SNew(STextBlock)
            	.Text(this, &SNerveQuestObjectiveProperty::GetPropertyDisplayName)
            	.Font(this, &SNerveQuestObjectiveProperty::GetNodePropertyNameFontSize)
				.ColorAndOpacity(this, &SNerveQuestObjectiveProperty::GetNodePropertyNameFontColor)
            ]
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(5)
            [
	            SNew(STextBlock)
				.Text(this, &SNerveQuestObjectiveProperty::GetPropertyDisplayValue)
				.Font(this, &SNerveQuestObjectiveProperty::GetNodePropertyValueFontSize)
				.ColorAndOpacity(this, &SNerveQuestObjectiveProperty::GetNodePropertyValueFontColor)
            ]
		]
	];
}

FSlateColor SNerveQuestObjectiveProperty::GetNodePropertyBackgroundColor() const
{
	return StyleSetting->NodeBodyPropertyBackdropColor;
}

FSlateFontInfo SNerveQuestObjectiveProperty::GetNodePropertyNameFontSize() const
{
	return FCoreStyle::GetDefaultFontStyle("Bold", StyleSetting->NodePropertyFontSize);
}

FSlateFontInfo SNerveQuestObjectiveProperty::GetNodePropertyValueFontSize() const
{
	return FCoreStyle::GetDefaultFontStyle("Regular", StyleSetting->NodePropertyFontSize);
}

FSlateColor SNerveQuestObjectiveProperty::GetNodePropertyNameFontColor() const
{
	return StyleSetting->NodePropertyNameFontColor;
}

FSlateColor SNerveQuestObjectiveProperty::GetNodePropertyValueFontColor() const
{
	return StyleSetting->NodePropertyValueFontColor;
}

FText SNerveQuestObjectiveProperty::GetPropertyDisplayName() const
{
	FString Left, Right;
	if (PropertyText.Split(TEXT(":"), &Left, &Right))
	{
		return FText::FromString(Left.TrimStartAndEnd() + ":");
	}
	return FText::FromString(PropertyText);
}

FText SNerveQuestObjectiveProperty::GetPropertyDisplayValue() const
{
	FString Left, Right;
	if (PropertyText.Split(TEXT(":"), &Left, &Right))
	{
		return FText::FromString(Right.TrimStartAndEnd());
	}
	return FText::GetEmpty();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
