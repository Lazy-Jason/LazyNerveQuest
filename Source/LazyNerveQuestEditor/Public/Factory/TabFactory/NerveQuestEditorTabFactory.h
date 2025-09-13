#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

class NerveQuestEditorTabFactory : public FWorkflowTabFactory
{
public:
	NerveQuestEditorTabFactory( const TSharedPtr<class NerveQuestEditorToolkit>& Toolkit );
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override { return FText::FromString("Nerve Quest Editor"); }

protected:
	bool GetIsEditable() const;
private:
	TWeakPtr<class NerveQuestEditorToolkit> EditorToolKit;
};
