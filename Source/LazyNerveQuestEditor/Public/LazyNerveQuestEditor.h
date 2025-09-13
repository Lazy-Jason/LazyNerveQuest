#pragma once

#include "CoreMinimal.h"
#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include "IAssetTypeActions.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"

struct FNerveQuestGraphNodeFactory;
struct FGraphNodeClassHelper;
class IAssetTools;
class FAssetToolsModule;

class FLazyNerveQuestEditorModule : public IModuleInterface
{
    TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
    EAssetTypeCategories::Type NerveCategory = EAssetTypeCategories::None;

private:
    inline static TSharedPtr<FSlateStyleSet> NerveDialogueStyleSet = nullptr;
    TSharedPtr<FNerveQuestGraphNodeFactory> GraphNodeFactory;
    
public:
    inline static const FName NerveQuestEditorModeName = FName(TEXT("NerveQuestEditorMode"));
    inline static const FName NerveQuestStyleName = FName(TEXT("NerveQuestEditorStyle"));

    // --- Tab Names --- //
    inline static const FName NerveQuestViewportTabName = FName(TEXT("NerveQuestViewportTab"));
    inline static const FName NerveQuestEditorTabName = FName(TEXT("NerveQuestEditorTab"));
    inline static const FName NerveQuestDetailsTabName = FName(TEXT("NerveQuestDetailsTab"));
    inline static const FName NerveQuestEditorTabLayout = FName(TEXT("NerveQuestEditorMode_Layout_v1"));
    inline static const FName NerveQuestSequencePinCategory = FName(TEXT("NerveQuestSequencePin"));
    inline static const FName NerveQuestOptionalPinCategory = FName(TEXT("NerveQuestOptionalPin"));

private:
    template<class T>
    void RegisterAssetTypeAction(const FString& Name)
    {
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        const auto Action = MakeShared<T>(NerveCategory, FText::FromString(Name), FColor::Orange);
        RegisteredAssetTypeActions.Emplace(Action);

        AssetTools.RegisterAssetTypeActions(Action);
    }
    TSharedPtr<FGraphNodeClassHelper, ESPMode::ThreadSafe> ClassCache_QuestObjectiveClass;
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    static FLazyNerveQuestEditorModule& Get();
    TSharedPtr<FGraphNodeClassHelper, ESPMode::ThreadSafe> GetClassCache();
};
