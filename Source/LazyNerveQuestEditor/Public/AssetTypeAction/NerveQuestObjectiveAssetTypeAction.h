#pragma once

#include "AssetTypeActions_Base.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"

class LAZYNERVEQUESTEDITOR_API NerveQuestObjectiveAssetTypeAction : public FAssetTypeActions_Blueprint
{
private:
	EAssetTypeCategories::Type AssetCategory;
	const FText NewName = FText::FromString("Default");
	const FColor Color;
public:
	explicit NerveQuestObjectiveAssetTypeAction(EAssetTypeCategories::Type AssetCategory, const FText& DisplayName = FText::FromString("Default"), const FColor& Color = FColor::Black);
	/*virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;*/

	virtual UClass* GetSupportedClass() const override { return UNerveQuestRuntimeObjectiveBase::StaticClass(); }
	virtual FText GetName() const override { return NewName; }
	virtual FColor GetTypeColor() const override { return Color; }
	virtual uint32 GetCategories() override { return AssetCategory; }

	virtual bool CanFilter() override { return true; }
	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBlueprint) const override;
};
