#include "LazyNerveQuestStyle.h"
#include "LazyNerveQuestEditor.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

namespace
{
    const FVector2D Icon16x12(16.0f, 12.0f);
}

TSharedPtr<FSlateStyleSet> FLazyNerveQuestStyle::StyleInstance = nullptr;

void FLazyNerveQuestStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        StyleInstance = Create();
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FLazyNerveQuestStyle::Shutdown()
{
    if (StyleInstance.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
        ensure(StyleInstance.IsUnique());
        StyleInstance.Reset();
    }
}

void FLazyNerveQuestStyle::ReloadTextures()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
    }
}

const ISlateStyle& FLazyNerveQuestStyle::Get()
{
    check(StyleInstance.IsValid());
    return *StyleInstance;
}

FName FLazyNerveQuestStyle::GetStyleSetName()
{
    return FLazyNerveQuestEditorModule::NerveQuestStyleName;
}

TSharedRef<FSlateStyleSet> FLazyNerveQuestStyle::Create()
{
    TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("LazyNerveQuest"));
    check(Plugin.IsValid());
    const FString BaseDirectory = Plugin->GetBaseDir();
    Style->SetContentRoot(BaseDirectory / TEXT("Resources"));

    FSlateImageBrush* ThumbnailBrush = new FSlateImageBrush(RootToContentDir(TEXT("QuestClassIcon"), TEXT(".png")), Icon16x12);
    FSlateImageBrush* ObjectiveThumbnailBrush = new FSlateImageBrush(RootToContentDir(TEXT("QuestObjectiveClassIcon"), TEXT(".png")), Icon16x12);
    FSlateImageBrush* RewardThumbnailBrush = new FSlateImageBrush(RootToContentDir(TEXT("QuestRewardClassIcon"), TEXT(".png")), Icon16x12);
    FSlateImageBrush* QuestNodeBorderBrush = new FSlateImageBrush(RootToContentDir(TEXT("QuestNodeBorder"), TEXT(".png")), FVector2D(571, 241));
    FSlateImageBrush* QuestNodeTopBorderBrush = new FSlateImageBrush(RootToContentDir(TEXT("QuestNodeTopBorder"), TEXT(".png")), FVector2D(571, 241));
    
    // Define custom button styles
    const FButtonStyle RoundedButtonStyle = FButtonStyle()
        .SetNormal(FSlateRoundedBoxBrush(FLinearColor::White, 5.0f))
        .SetHovered(FSlateRoundedBoxBrush(FLinearColor::White, 5.0f))
        .SetPressed(FSlateRoundedBoxBrush(FLinearColor::White, 5.0f))
        .SetNormalPadding(FMargin(2.0f));

    Style->Set(TEXT("ClassThumbnail.NerveQuestAsset"), ThumbnailBrush);
    Style->Set(TEXT("ClassIcon.NerveQuestAsset"), ThumbnailBrush);
    Style->Set(TEXT("ClassThumbnail.NerveQuestRuntimeObjectiveBase"), ObjectiveThumbnailBrush);
    Style->Set(TEXT("ClassIcon.NerveQuestRuntimeObjectiveBase"), ObjectiveThumbnailBrush);
    Style->Set(TEXT("ClassThumbnail.NerveQuestRewardBase"), RewardThumbnailBrush);
    Style->Set(TEXT("ClassIcon.NerveQuestRewardBase"), RewardThumbnailBrush);
    Style->Set("WhiteRoundedButton", RoundedButtonStyle);
    Style->Set("WhiteRoundedBrush", new FSlateRoundedBoxBrush(FLinearColor::White, 5.0f));
    Style->Set("QuestNodeBorder", QuestNodeBorderBrush);
    Style->Set("QuestNodeTopBorder", QuestNodeTopBorderBrush);

    Style->Set("NerveQuestNodeBorderStyle_V2", new IMAGE_BRUSH_SVG("NerveQuestNodeBorderStyle_V2", FVector2D(600, 336)));
    Style->Set("NerveQuestNodeBorderStyle_V2_Gradient", new IMAGE_BRUSH("NerveQuestNodeBorderStyle_V2_Gradient", FVector2D(600, 200)));
    
    Style->Set("PinShape.Empty", new IMAGE_BRUSH_SVG("NerveQuestPinShape_Empty", FVector2D(24, 24)));
    Style->Set("PinShape.Full", new IMAGE_BRUSH_SVG("NerveQuestPinShape_Full", FVector2D(24, 24)));

    return Style;
}