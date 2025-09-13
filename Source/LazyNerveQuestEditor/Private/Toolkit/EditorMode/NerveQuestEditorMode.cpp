#include "Toolkit/EditorMode/NerveQuestEditorMode.h"

#include "LazyNerveQuestEditor.h"
#include "Factory/TabFactory/NerveQuestDetailsTabFactory.h"
#include "Factory/TabFactory/NerveQuestEditorTabFactory.h"
#include "Toolkit/EditorToolkit/NerveQuestEditorToolkit.h"

NerveQuestEditorMode::NerveQuestEditorMode(const TSharedPtr<NerveQuestEditorToolkit>& Toolkit):
FApplicationMode(FLazyNerveQuestEditorModule::NerveQuestEditorModeName)
{
	EditorToolKit = Toolkit;

	// register all tabs below
	TabSets.RegisterFactory(MakeShareable(new NerveQuestEditorTabFactory(Toolkit)));
	TabSets.RegisterFactory(MakeShareable(new NerveQuestDetailsTabFactory(Toolkit)));

	TabLayout = FTabManager::NewLayout(FLazyNerveQuestEditorModule::NerveQuestEditorTabLayout)
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.80f)
					->AddTab(FLazyNerveQuestEditorModule::NerveQuestEditorTabName,
					ETabState::OpenedTab)
				)
				->Split
				(
				FTabManager::NewStack()
					->SetSizeCoefficient(0.20f)
					->AddTab(FLazyNerveQuestEditorModule::NerveQuestDetailsTabName,
				       ETabState::OpenedTab)
				)
			)
		);
}

void NerveQuestEditorMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	const TSharedPtr<NerveQuestEditorToolkit> ToolKitApp = EditorToolKit.Pin();
	ToolKitApp->PushTabFactories(TabSets);
	
	FApplicationMode::RegisterTabFactories(InTabManager);
}

void NerveQuestEditorMode::PostActivateMode()
{
	FApplicationMode::PostActivateMode();
}

void NerveQuestEditorMode::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();
}