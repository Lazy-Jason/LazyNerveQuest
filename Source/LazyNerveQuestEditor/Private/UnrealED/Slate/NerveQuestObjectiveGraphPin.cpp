// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealED/Slate/NerveQuestObjectiveGraphPin.h"
#include "SlateOptMacros.h"
#include "EditorStyleSet.h"
#include "LazyNerveQuestStyle.h"
#include "UnrealED/Node/NerveQuestObjectiveNodeBase.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SNerveQuestObjectiveGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
    // Call parent construct to get standard pin behavior
    SGraphPin::Construct(SGraphPin::FArguments(), InPin);

    // Get style settings
    StyleSetting = GetDefault<UNerveQuestEditorStyleSetting>();

    // Cache quest-specific brushes - you can create custom brushes or reuse existing ones
    // For now, using standard exec pin brushes but you can replace with quest-specific ones
    CachedImg_QuestExecPin_ConnectedHovered = FLazyNerveQuestStyle::Get().GetBrush(TEXT("PinShape.Full"));
    CachedImg_QuestExecPin_Connected = FLazyNerveQuestStyle::Get().GetBrush(TEXT("PinShape.Full"));
    CachedImg_QuestExecPin_DisconnectedHovered = FLazyNerveQuestStyle::Get().GetBrush(TEXT("PinShape.Empty"));
    CachedImg_QuestExecPin_Disconnected = FLazyNerveQuestStyle::Get().GetBrush(TEXT("PinShape.Empty"));

    // Optional pin brushes - could be different style to indicate they're optional
    CachedImg_QuestOptionalPin_ConnectedHovered = FLazyNerveQuestStyle::Get().GetBrush(TEXT("PinShape.Full"));
    CachedImg_QuestOptionalPin_Connected = FLazyNerveQuestStyle::Get().GetBrush(TEXT("PinShape.Full"));
    CachedImg_QuestOptionalPin_DisconnectedHovered = FLazyNerveQuestStyle::Get().GetBrush(TEXT("PinShape.Empty"));
    CachedImg_QuestOptionalPin_Disconnected = FLazyNerveQuestStyle::Get().GetBrush(TEXT("PinShape.Empty"));
}

FSlateColor SNerveQuestObjectiveGraphPin::GetPinColor() const
{
    // Check if this is an execution pin (input "Exec" or output "Completed")
    if (GraphPinObj->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
    {
        // Use your custom colors from style settings
        return FSlateColor(IsHovered() ? StyleSetting->PinHoverColor : StyleSetting->PinUnHoverColor);
    }
    
    // For optional pins
    if (IsOptionalPin())
    {
        // Could use a muted version of your colors or different colors entirely
        FLinearColor OptionalColor = IsHovered() ? StyleSetting->PinHoverColor.GetSpecifiedColor() : StyleSetting->PinUnHoverColor.GetSpecifiedColor();
        OptionalColor.A *= 0.7f; // Make it slightly transparent to indicate optional
        return FSlateColor(OptionalColor);
    }

    // Default fallback
    return FSlateColor(IsHovered() ? StyleSetting->PinHoverColor : StyleSetting->PinUnHoverColor);
}

const FSlateBrush* SNerveQuestObjectiveGraphPin::GetPinIcon() const
{
    // Handle optional pins
    if (IsOptionalPin())
    {
        return IsConnected() ? (IsHovered() ? CachedImg_QuestOptionalPin_ConnectedHovered : CachedImg_QuestOptionalPin_Connected)
        : (IsHovered() ? CachedImg_QuestOptionalPin_DisconnectedHovered : CachedImg_QuestOptionalPin_Disconnected);
    }

    // Default fallback
    return IsConnected() ? (IsHovered() ? CachedImg_QuestExecPin_ConnectedHovered : CachedImg_QuestExecPin_Connected)
        : (IsHovered() ? CachedImg_QuestExecPin_DisconnectedHovered : CachedImg_QuestExecPin_Disconnected);
}

FSlateColor SNerveQuestObjectiveGraphPin::GetPinTextColor() const
{
    return StyleSetting->NodeHeaderFontColor;
}

bool SNerveQuestObjectiveGraphPin::IsOptionalPin() const
{
    // Check if the pin name contains "Optional" or has a specific category
    return UNerveQuestObjectiveNodeBase::IsOptionalPin(GraphPinObj);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION