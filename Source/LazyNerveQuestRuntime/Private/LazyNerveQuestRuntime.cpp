// Copyright Epic Games, Inc. All Rights Reserved.

#include "..\Public\LazyNerveQuestRuntime.h"

#include "LazyNerveRuntimeQuestStyle.h"

#define LOCTEXT_NAMESPACE "FLazyNerveQuestRuntimeModule"

void FLazyNerveQuestRuntimeModule::StartupModule()
{
	FLazyNerveRuntimeQuestStyle::Initialize();
	FLazyNerveRuntimeQuestStyle::ReloadTextures();
}

void FLazyNerveQuestRuntimeModule::ShutdownModule()
{
	FLazyNerveRuntimeQuestStyle::Shutdown();
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLazyNerveQuestRuntimeModule, LazyNerveQuestRuntime)