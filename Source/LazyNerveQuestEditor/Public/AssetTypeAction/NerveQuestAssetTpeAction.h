#pragma once
#include "AssetTypeActions_Base.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"

class LAZYNERVEQUESTEDITOR_API NerveQuestAssetTpeAction : public FAssetTypeActions_Base
{
private:
	EAssetTypeCategories::Type AssetCategory;
	const FText NewName = FText::FromString("Default");
	const FColor Color;
public:
	explicit NerveQuestAssetTpeAction(EAssetTypeCategories::Type AssetCategory, const FText& DisplayName = FText::FromString("Default"), const FColor& Color = FColor::Black);
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;

	virtual UClass* GetSupportedClass() const override { return UNerveQuestAsset::StaticClass(); }
	virtual FText GetName() const override { return NewName; }
	virtual FColor GetTypeColor() const override { return Color; }
	virtual uint32 GetCategories() override { return AssetCategory; }
};
