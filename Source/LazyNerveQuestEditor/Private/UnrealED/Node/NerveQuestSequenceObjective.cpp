// // Copyright (C) 2024 Job Omondiale - All Rights Reserved


#include "UnrealED/Node/NerveQuestSequenceObjective.h"
#include "LazyNerveQuestEditor.h"

void UNerveQuestSequenceObjective::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// Sequence pin to run objective either sequentially or parallel.
	CreatePin(EGPD_Output, FLazyNerveQuestEditorModule::NerveQuestSequencePinCategory, FName(TEXT("Sequence")));
}
