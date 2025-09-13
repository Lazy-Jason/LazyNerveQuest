#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

class NerveQuestDetailsTabFactory : public FWorkflowTabFactory
{
public:
	NerveQuestDetailsTabFactory( const TSharedPtr<class NerveQuestEditorToolkit>& Toolkit );
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override { return FText::FromString("Details view for quest objectives"); }

private:
	TWeakPtr<class NerveQuestEditorToolkit> DetailsToolKit;
};
