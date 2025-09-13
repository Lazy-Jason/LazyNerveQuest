#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class LAZYNERVEQUESTRUNTIME_API FLazyNerveRuntimeQuestStyle
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

	FLazyNerveRuntimeQuestStyle() = delete;
	FLazyNerveRuntimeQuestStyle(const FLazyNerveRuntimeQuestStyle&) = delete;
	FLazyNerveRuntimeQuestStyle& operator=(const FLazyNerveRuntimeQuestStyle&) = delete;
};
