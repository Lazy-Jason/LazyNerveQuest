#include "Factory/TabFactory/NerveQuestDetailsTabFactory.h"
#include "LazyNerveQuestEditor.h"
#include "Toolkit/EditorToolkit/NerveQuestEditorToolkit.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"

NerveQuestDetailsTabFactory::NerveQuestDetailsTabFactory(const TSharedPtr<NerveQuestEditorToolkit>& Toolkit)
	:FWorkflowTabFactory(FLazyNerveQuestEditorModule::NerveQuestDetailsTabName, Toolkit)
{
	DetailsToolKit = Toolkit;
	TabLabel = FText::FromString(TEXT("Details"));
	ViewMenuDescription = FText::FromString(TEXT("Display's details of currently selected quest objectives"));
	ViewMenuTooltip = FText::FromString(TEXT("Shows the details view"));
}

TSharedRef<SWidget> NerveQuestDetailsTabFactory::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	const TSharedPtr<NerveQuestEditorToolkit> Tool = DetailsToolKit.Pin();
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bSearchInitialKeyFocus = true;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.NotifyHook = nullptr;
	DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.bShowModifiedPropertiesOption = false;
	DetailsViewArgs.bShowScrollBar = false;

	const TSharedPtr<IDetailsView> GenericDialogueSettingsDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	GenericDialogueSettingsDetailsView->SetObject(Tool->GetCurrentWorkingAsset());
	Tool->SetCurrentSelectedEditorDetailsView(GenericDialogueSettingsDetailsView);
	
	return GenericDialogueSettingsDetailsView.ToSharedRef();
}