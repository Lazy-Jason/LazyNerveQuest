#include "LazyNerveQuestEditor.h"

#include "AIGraphTypes.h"
#include "AssetToolsModule.h"
#include "EdGraphUtilities.h"
#include "IAssetTools.h"
#include "LazyNerveQuestStyle.h"
#include "AssetTypeAction/NerveQuestAssetTpeAction.h"
#include "AssetTypeAction/NerveQuestObjectiveAssetTypeAction.h"
#include "Factory/Graph/NerveQuestGraphNodeFactory.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"

#define LOCTEXT_NAMESPACE "FLazyNerveQuestEditorModule"

void FLazyNerveQuestEditorModule::StartupModule()
{
	IAssetTools& AssetTools = IAssetTools::Get();
	NerveCategory = AssetTools.RegisterAdvancedAssetCategory(FName("NerveQuestCategory"), FText::FromString("Nerve Quest"));

	RegisterAssetTypeAction<NerveQuestAssetTpeAction>("Nerve Quest Asset");
	RegisterAssetTypeAction<NerveQuestObjectiveAssetTypeAction>("Nerve Quest Objective");

	FLazyNerveQuestStyle::Initialize();
	FLazyNerveQuestStyle::ReloadTextures();

	GraphNodeFactory = MakeShareable(new FNerveQuestGraphNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphNodeFactory);
}

void FLazyNerveQuestEditorModule::ShutdownModule()
{
	const FAssetToolsModule* AssetToolsModulePtr = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");
	if(!AssetToolsModulePtr) return;

	IAssetTools& AssetTools = AssetToolsModulePtr->Get();
	for (auto Action : RegisteredAssetTypeActions)
	{
		AssetTools.UnregisterAssetTypeActions(Action);
	}

	FEdGraphUtilities::UnregisterVisualNodeFactory(GraphNodeFactory);
	GraphNodeFactory.Reset();
	
	FLazyNerveQuestStyle::Shutdown();
}

FLazyNerveQuestEditorModule& FLazyNerveQuestEditorModule::Get()
{
	return FModuleManager::LoadModuleChecked<FLazyNerveQuestEditorModule>("LazyNerveQuestEditor");
}

TSharedPtr<FGraphNodeClassHelper, ESPMode::ThreadSafe> FLazyNerveQuestEditorModule::GetClassCache()
{
	if (!ClassCache_QuestObjectiveClass.IsValid())
	{
		ClassCache_QuestObjectiveClass = MakeShared<FGraphNodeClassHelper>(UNerveQuestRuntimeObjectiveBase::StaticClass());
		ClassCache_QuestObjectiveClass->AddObservedBlueprintClasses(UNerveQuestRuntimeObjectiveBase::StaticClass());
		ClassCache_QuestObjectiveClass->UpdateAvailableBlueprintClasses();
	}
	return ClassCache_QuestObjectiveClass;
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FLazyNerveQuestEditorModule, LazyNerveQuestEditor)