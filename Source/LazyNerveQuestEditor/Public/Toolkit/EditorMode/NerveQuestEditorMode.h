#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class NerveQuestEditorMode : public FApplicationMode
{
public:
	NerveQuestEditorMode( const TSharedPtr<class NerveQuestEditorToolkit>& Toolkit );
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	virtual void PostActivateMode() override;
	virtual void PreDeactivateMode() override;

private:
	TWeakPtr<class NerveQuestEditorToolkit> EditorToolKit;
	FWorkflowAllowedTabSet TabSets;
};
