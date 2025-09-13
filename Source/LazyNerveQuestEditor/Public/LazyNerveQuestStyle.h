#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class LAZYNERVEQUESTEDITOR_API FLazyNerveQuestStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static const ISlateStyle& Get();
	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create();
	static TSharedPtr<FSlateStyleSet> StyleInstance;

	FLazyNerveQuestStyle() = delete;
	FLazyNerveQuestStyle(const FLazyNerveQuestStyle&) = delete;
	FLazyNerveQuestStyle& operator=(const FLazyNerveQuestStyle&) = delete;
};