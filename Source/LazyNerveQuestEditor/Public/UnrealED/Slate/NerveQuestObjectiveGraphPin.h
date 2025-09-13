// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"
#include "Setting/NerveQuestEditorStyleSetting.h"

/**
 * Custom pin widget for quest objective nodes that behaves like standard Unreal pins
 * but with custom styling for quest-specific visual feedback
 */
class LAZYNERVEQUESTEDITOR_API SNerveQuestObjectiveGraphPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SNerveQuestObjectiveGraphPin)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

	// Override pin appearance
	virtual FSlateColor GetPinColor() const override;
	virtual const FSlateBrush* GetPinIcon() const override;
	virtual FSlateColor GetPinTextColor() const override;
    
protected:
	// Style settings reference
	const UNerveQuestEditorStyleSetting* StyleSetting = nullptr;

	// Cached brushes for different pin states
	mutable const FSlateBrush* CachedImg_QuestExecPin_ConnectedHovered = nullptr;
	mutable const FSlateBrush* CachedImg_QuestExecPin_Connected = nullptr;
	mutable const FSlateBrush* CachedImg_QuestExecPin_DisconnectedHovered = nullptr;
	mutable const FSlateBrush* CachedImg_QuestExecPin_Disconnected = nullptr;

	mutable const FSlateBrush* CachedImg_QuestOptionalPin_ConnectedHovered = nullptr;
	mutable const FSlateBrush* CachedImg_QuestOptionalPin_Connected = nullptr;
	mutable const FSlateBrush* CachedImg_QuestOptionalPin_DisconnectedHovered = nullptr;
	mutable const FSlateBrush* CachedImg_QuestOptionalPin_Disconnected = nullptr;

private:
	// Helper to determine if this is an optional pin
	bool IsOptionalPin() const;
};