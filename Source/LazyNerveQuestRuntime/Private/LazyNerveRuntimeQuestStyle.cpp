#include "LazyNerveRuntimeQuestStyle.h"
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

TSharedPtr<FSlateStyleSet> FLazyNerveRuntimeQuestStyle::StyleInstance = nullptr;

void FLazyNerveRuntimeQuestStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        StyleInstance = Create();
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FLazyNerveRuntimeQuestStyle::Shutdown()
{
    if (StyleInstance.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
        ensure(StyleInstance.IsUnique());
        StyleInstance.Reset();
    }
}

void FLazyNerveRuntimeQuestStyle::ReloadTextures()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
    }
}

const ISlateStyle& FLazyNerveRuntimeQuestStyle::Get()
{
    check(StyleInstance.IsValid());
    return *StyleInstance;
}

FName FLazyNerveRuntimeQuestStyle::GetStyleSetName()
{
    return FName(TEXT("NerveQuestRuntimeStyle"));
}

TSharedRef<FSlateStyleSet> FLazyNerveRuntimeQuestStyle::Create()
{
    TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("LazyNerveQuest"));
    check(Plugin.IsValid());
    const FString BaseDirectory = Plugin->GetBaseDir();
    Style->SetContentRoot(BaseDirectory / TEXT("Resources"));
    
    FSlateImageBrush* QuestPOIBrush = new FSlateImageBrush(RootToContentDir(TEXT("POIGoToIcon"), TEXT(".png")), FVector2D(1024, 1024));
    Style->Set("QuestPOIBrush", QuestPOIBrush);

    return Style;
}
