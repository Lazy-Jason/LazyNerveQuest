#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "GraphEditor.h"

class LAZYNERVEQUESTEDITOR_API NerveQuestEditorToolkit : public FWorkflowCentricApplication, public FEditorUndoClient, public FNotifyHook
{
public:
    NerveQuestEditorToolkit();
    ~NerveQuestEditorToolkit();
    virtual FName GetToolkitFName() const override { return FName(TEXT("NerveQuestEditorToolkit")); }
    virtual FText GetBaseToolkitName() const override { return FText::FromString("NerveQuestEditorToolkit"); }
    virtual FString GetWorldCentricTabPrefix() const override { return FString("NerveQuestEditorToolkit"); }
    virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor(0.2, 0.3, 0.5, 0.5); }

    virtual void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) override;
    virtual void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) override;

    // FEditorUndoClient Interface
    virtual void PostUndo(bool bSuccess) override;
    virtual void PostRedo(bool bSuccess) override;

    // FNotifyHook Interface
    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
    virtual void OnClose() override;

    virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& Tab) override;
    virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& Tab) override;

    void InitNerveQuestEditor(EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& NewToolkitHost, UObject* Object);
    void OnGraphSelectionChanged(const FGraphPanelSelectionSet& NewPanelSelectionSet);

    // Graph editing commands
    void CreateGraphCommandList();
    TSharedRef<SGraphEditor> CreateGraphEditorWidget();
    void OnNodeDoubleClicked(UEdGraphNode* Node);
    void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);
    FGraphPanelSelectionSet GetSelectedNodes() const;
    void DeleteSelectedNodes() const;
    bool CanDeleteNodes() const;
    void CopySelectedNodes() const;
    bool CanCopyNodes() const;
    void PasteNodes() const;
    void PasteNodesHere(const FVector2D& Location) const;
    bool CanPasteNodes() const;
    void DuplicateNodes();
    bool CanDuplicateNodes() const;
    void SelectAllNodes() const;
    bool CanSelectAllNodes() const;

protected:
    void RestoreGraphFromSaveAsset() const;
    void SaveGraphToAsset() const;
    void OnQuestAssetPreSave() const;
    void OnQuestAssetPostEdit();
    void PopulateToolBar(FToolBarBuilder& ToolBarBuilder) const;
    virtual void ExtendAssetToolBar();
    void OnObjectiveDetailsViewPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);
    class UNerveQuestObjectiveNodeBase* GetSelectedGraphNode(const FGraphPanelSelectionSet& NewSelection);

    bool CanCreateObjective() const { return true; }
    bool CanCreateReward() const { return true; }
    void HandleNewNodeClassPicked(UClass* InClass) const;

public:
    class UNerveQuestAsset* GetCurrentWorkingAsset() const { return CurrentWorkingAsset; }
    class UEdGraph* GetCurrentWorkingGraph() const { return CurrentWorkingGraph; }
    void SetCurrentWorkingGraphEditor(const TSharedPtr<SGraphEditor>& NewCurrentWorkingGraphEditor) { CurrentWorkingGraphEditor = NewCurrentWorkingGraphEditor; }
    void SetCurrentSelectedDetailsView(const TSharedPtr<class IDetailsView>& NewDetailsView);
    void SetCurrentSelectedEditorDetailsView(const TSharedPtr<class IDetailsView>& NewDetailsView);
    bool GetCanEditGraphEditor() const { return bCanEditGraph; }

private:
    bool bCanEditGraph = true;
    class UNerveQuestAsset* CurrentWorkingAsset = nullptr;
    class UEdGraph* CurrentWorkingGraph = nullptr;
    UNerveQuestObjectiveNodeBase* CurrentSelectedNerveDialogueGraphNode = nullptr;
    TSharedPtr<SGraphEditor> CurrentWorkingGraphEditor = nullptr;
    TSharedPtr<class IDetailsView> CurrentSelectedEditorDetailsView = nullptr;
    TSharedPtr<class IDetailsView> CurrentSelectedDetailsView = nullptr;
    UToolMenu* NerveQuestAssetToolMenu = nullptr;

    /** Command list for graph editing actions */
    TSharedPtr<FUICommandList> GraphEditorCommands;
};