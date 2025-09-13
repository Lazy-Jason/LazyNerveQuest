// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NerveQuestObjectiveProperty.h"
#include "SGraphNode.h"
#include "Setting/NerveQuestEditorStyleSetting.h"
#include "Widgets/SCompoundWidget.h"

class UNerveQuestObjectiveNodeBase;

/**
 * 
 */
class LAZYNERVEQUESTEDITOR_API SNerveQuestObjectiveBase : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SNerveQuestObjectiveBase)
	{}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UNerveQuestObjectiveNodeBase* InNode);
	
protected:
	FSlateBrush CachedBrush;
	TSharedPtr<SBox> GraphNodeBody;
	const UNerveQuestEditorStyleSetting* StyleSetting = nullptr;

	TSharedPtr<SVerticalBox> OptionalPinsBox;

protected:
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual TSharedRef<SWidget> CreateNodeContentArea() override;
	virtual TSharedRef<SWidget> CreateQuestCentralNode();

	TSharedRef<SWidget> GenerateObjectiveProperty() const;
	FText GetQuestTitle() const;
	const FSlateBrush* GetQuestBrush();
	ETextJustify::Type GetTitleJustification();

	FMargin GetGradientPadding() const;

	FOptionalSize GetNodeMinWidth() const;
	FOptionalSize GetNodeMaxWidth() const;
	FOptionalSize GetNodeMinHeight() const;
	FOptionalSize GetNodeMaxHeight() const;

	FSlateColor GetNodeBodyColor() const;
	FSlateColor GetNodeBodyGradientColor() const;
	FSlateColor GetNodeBodyHighlightColor() const;
	
	FSlateColor GetNodeIconColor() const;

	FSlateColor GetNodeTitleFontColor() const;
	FSlateFontInfo GetNodeTitleFontSize() const;
	
	FSlateColor GetNodePropertyBackgroundColor() const;
	FSlateFontInfo GetNodePropertyFontSize() const;
};