#include "AssetTypeAction/NerveQuestAssetTpeAction.h"
#include "Toolkit/EditorToolkit/NerveQuestEditorToolkit.h"

NerveQuestAssetTpeAction::NerveQuestAssetTpeAction(EAssetTypeCategories::Type AssetCategory, const FText& DisplayName,
const FColor& Color) : AssetCategory(AssetCategory), NewName(DisplayName), Color(Color)
{}

void NerveQuestAssetTpeAction::OpenAssetEditor(const TArray<UObject*>& InObjects,
TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Element : InObjects)
	{
		if(UNerveQuestAsset* NewElement = Cast<UNerveQuestAsset>(Element))
		{
			const TSharedPtr<NerveQuestEditorToolkit> NerveToolkit(new NerveQuestEditorToolkit());
			NerveToolkit->InitNerveQuestEditor(Mode, EditWithinLevelEditor, NewElement);
		}
	}
}
