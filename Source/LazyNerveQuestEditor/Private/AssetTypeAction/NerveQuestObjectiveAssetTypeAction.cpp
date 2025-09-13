#include "AssetTypeAction/NerveQuestObjectiveAssetTypeAction.h"

#include "BlueprintEditorModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Factories/BlueprintFactory.h"
#include "Factory/NerveQuestObjectiveFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

NerveQuestObjectiveAssetTypeAction::NerveQuestObjectiveAssetTypeAction(EAssetTypeCategories::Type AssetCategory, const FText& DisplayName,
const FColor& Color) : AssetCategory(AssetCategory), NewName(DisplayName), Color(Color)
{}

UFactory* NerveQuestObjectiveAssetTypeAction::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UBlueprintFactory* BlueprintFactory = NewObject<UNerveQuestObjectiveFactory>();
	BlueprintFactory->ParentClass = UNerveQuestRuntimeObjectiveBase::StaticClass();
	return BlueprintFactory;
}
