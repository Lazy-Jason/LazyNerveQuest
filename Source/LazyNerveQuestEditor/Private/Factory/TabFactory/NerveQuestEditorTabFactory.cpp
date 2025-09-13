#include "Factory/TabFactory/NerveQuestEditorTabFactory.h"
#include "LazyNerveQuestEditor.h"
#include "Toolkit/EditorToolkit/NerveQuestEditorToolkit.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"

NerveQuestEditorTabFactory::NerveQuestEditorTabFactory(const TSharedPtr<NerveQuestEditorToolkit>& Toolkit) :
FWorkflowTabFactory(FLazyNerveQuestEditorModule::NerveQuestEditorTabName, Toolkit)
{
	EditorToolKit = Toolkit;
	TabLabel = FText::FromString(TEXT("Nerve Quest Graph"));
	ViewMenuDescription = FText::FromString(TEXT("Display's a nerve quest editor"));
	ViewMenuTooltip = FText::FromString(TEXT("Shows the nerve quest editor"));
}

TSharedRef<SWidget> NerveQuestEditorTabFactory::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	const TSharedPtr<NerveQuestEditorToolkit> Tool = EditorToolKit.Pin();
	const TSharedPtr<SGraphEditor> DialogueGraphEditor = Tool->CreateGraphEditorWidget();
	
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.FillHeight(1.0f)
	.HAlign(HAlign_Fill)
	[
		DialogueGraphEditor.ToSharedRef()
	];
}

bool NerveQuestEditorTabFactory::GetIsEditable() const
{
	if(EditorToolKit.Pin() == nullptr) return true;
	
	return EditorToolKit.Pin()->GetCanEditGraphEditor();
}