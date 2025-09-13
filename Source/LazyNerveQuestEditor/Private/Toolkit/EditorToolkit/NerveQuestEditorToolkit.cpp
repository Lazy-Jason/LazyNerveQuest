#include "Toolkit/EditorToolkit//NerveQuestEditorToolkit.h"

#include "ContentBrowserModule.h"
#include "EdGraphUtilities.h"
#include "IContentBrowserSingleton.h"
#include "LazyNerveQuestEditor.h"
#include "LazyNerveQuestStyle.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Commands/GenericCommands.h"
#include "Helper/LazyNerveValidator.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Objects/Graph/NerveQuestRuntimeGraph.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"
#include "Objects/Nodes/Objective/NerveEntryObjective.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"
#include "Objects/Pin/NerveQuestRuntimePin.h"
#include "Objects/Rewards/NerveQuestRewardBase.h"
#include "Toolkit/EditorMode/NerveQuestEditorMode.h"
#include "UnrealED/Schema/NerveQuestGraphSchema.h"
#include "UnrealED/Graph/NerveQuestGraphEditor.h"
#include "UnrealED/Node/NerveQuestObjectiveNodeBase.h"
#include "UnrealED/Node/NerveQuestRootObjective.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

DEFINE_LOG_CATEGORY_STATIC(NerveQuestEditorToolkitLog, Log, All);

NerveQuestEditorToolkit::NerveQuestEditorToolkit()
{
	if (UEditorEngine* Editor = static_cast<UEditorEngine*>(GEngine); Editor != nullptr)
	{
		Editor->RegisterForUndo(this);
	}
}

NerveQuestEditorToolkit::~NerveQuestEditorToolkit()
{
	if (UEditorEngine* Editor = static_cast<UEditorEngine*>(GEngine))
	{
		Editor->UnregisterForUndo(this);
	}
}

void NerveQuestEditorToolkit::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	FWorkflowCentricApplication::OnToolkitHostingStarted(Toolkit);
}

void NerveQuestEditorToolkit::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	FWorkflowCentricApplication::OnToolkitHostingFinished(Toolkit);
}

void NerveQuestEditorToolkit::PostUndo(bool bSuccess)
{
	if (bSuccess)
	{
		// Clear selection, to avoid holding refs to nodes that go away
		if (TSharedPtr<SGraphEditor> CurrentGraphEditor = CurrentWorkingGraphEditor)
		{
			CurrentGraphEditor->ClearSelectionSet();
			CurrentGraphEditor->NotifyGraphChanged();
		}
		FSlateApplication::Get().DismissAllMenus();
	}
}


void NerveQuestEditorToolkit::PostRedo(bool bSuccess)
{
	if (bSuccess)
	{
		// Clear selection, to avoid holding refs to nodes that go away
		if (TSharedPtr<SGraphEditor> CurrentGraphEditor = CurrentWorkingGraphEditor)
		{
			CurrentGraphEditor->ClearSelectionSet();
			CurrentGraphEditor->NotifyGraphChanged();
		}
		FSlateApplication::Get().DismissAllMenus();
	}
}

void NerveQuestEditorToolkit::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
	FProperty* PropertyThatChanged)
{
	FNotifyHook::NotifyPostChange(PropertyChangedEvent, PropertyThatChanged);
}

void NerveQuestEditorToolkit::OnClose()
{
	if(IsValid(CurrentSelectedNerveDialogueGraphNode) && IsValid(CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance()))
	{
		CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance()->StopObjectivePreview(GEditor->GetEditorWorldContext().World());
	}
	SaveGraphToAsset();
	CurrentWorkingAsset->SetPreSaveListener(nullptr);
	CurrentWorkingAsset->SetPreEditChangeListener(nullptr);
	
	FWorkflowCentricApplication::OnClose();
}

void NerveQuestEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& Tab)
{
	FWorkflowCentricApplication::RegisterTabSpawners(Tab);
}

void NerveQuestEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& Tab)
{
	FWorkflowCentricApplication::UnregisterTabSpawners(Tab);
}

void NerveQuestEditorToolkit::InitNerveQuestEditor(EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& NewToolkitHost, UObject* Object)
{
    TArray<UObject*> ObjectsToEdit;
    ObjectsToEdit.Add(Object);
    ExtendAssetToolBar();

    CurrentWorkingAsset = Cast<UNerveQuestAsset>(Object);
    CurrentWorkingAsset->SetPreSaveListener([this]() { OnQuestAssetPreSave(); });
    CurrentWorkingAsset->SetPostEditChangeListener([this]() { OnQuestAssetPostEdit(); });

    CurrentWorkingGraph = FBlueprintEditorUtils::CreateNewGraph
	(
        CurrentWorkingAsset,
        NAME_None,
        UNerveQuestGraphEditor::StaticClass(),
        UNerveQuestGraphSchema::StaticClass()
    );

    InitAssetEditor
	(
        Mode,
        NewToolkitHost,
        FName(TEXT("NerveQuestEditor")),
        FTabManager::FLayout::NullLayout,
        true,
        true,
        ObjectsToEdit
    );

    CreateGraphCommandList();

    AddApplicationMode(FLazyNerveQuestEditorModule::NerveQuestEditorModeName, MakeShareable(new NerveQuestEditorMode(SharedThis(this))));
    SetCurrentMode(FLazyNerveQuestEditorModule::NerveQuestEditorModeName);

    RestoreGraphFromSaveAsset();
}

void NerveQuestEditorToolkit::CreateGraphCommandList()
{
    GraphEditorCommands = MakeShareable(new FUICommandList);

    GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
        FExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::DeleteSelectedNodes),
        FCanExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::CanDeleteNodes));

    GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
        FExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::CopySelectedNodes),
        FCanExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::CanCopyNodes));

    GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
        FExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::PasteNodes),
        FCanExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::CanPasteNodes));

    GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
        FExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::DuplicateNodes),
        FCanExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::CanDuplicateNodes));

    GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
        FExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::SelectAllNodes),
        FCanExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::CanSelectAllNodes));
}

TSharedRef<SGraphEditor> NerveQuestEditorToolkit::CreateGraphEditorWidget()
{
	if (!GraphEditorCommands.IsValid())
	{
		CreateGraphCommandList();
	}

	FGraphAppearanceInfo NerveGraphAppearance;
	NerveGraphAppearance.CornerText = FText::FromString(TEXT("Nerve Quest"));
	NerveGraphAppearance.InstructionText = FText::FromString(TEXT("Construct Objective flow."));

	TSharedRef<SWidget> TitleBarWidget = 
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("Graph.TitleBackground")))
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("NerveQuestEditorToolkit", "NerveQuestEditorToolkit", "Nerve Quest"))
				.TextStyle(FAppStyle::Get(), TEXT("GraphBreadcrumbButtonText"))
			]
		];
    
	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &NerveQuestEditorToolkit::OnGraphSelectionChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &NerveQuestEditorToolkit::OnNodeDoubleClicked);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &NerveQuestEditorToolkit::OnNodeTitleCommitted);

	CurrentWorkingGraphEditor = SNew(SGraphEditor)
		.Appearance(NerveGraphAppearance)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.GraphToEdit(CurrentWorkingGraph)
		.TitleBar(TitleBarWidget)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(false);

	return CurrentWorkingGraphEditor.ToSharedRef();
}

void NerveQuestEditorToolkit::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	if (Node->CanJumpToDefinition())
	{
		Node->JumpToDefinition();
	}
	/*if (UNerveQuestObjectiveNodeBase* QuestNode = Cast<UNerveQuestObjectiveNodeBase>(Node))
	{
		UClass* NodeClass = QuestNode->GetRuntimeObjectiveClass();
		if (NodeClass && NodeClass->ClassGeneratedBy)
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(NodeClass->ClassGeneratedBy);
			if (IsValid(Blueprint))
			{
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);
			}
		}
	}*/
}

void NerveQuestEditorToolkit::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged && CommitInfo == ETextCommit::OnEnter)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NerveQuestEditor", "RenameNode", "Rename Node"));
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

FGraphPanelSelectionSet NerveQuestEditorToolkit::GetSelectedNodes() const
{
    if (CurrentWorkingGraphEditor.IsValid())
    {
        return CurrentWorkingGraphEditor->GetSelectedNodes();
    }
    return FGraphPanelSelectionSet();
}

void NerveQuestEditorToolkit::DeleteSelectedNodes() const
{
    if (!CurrentWorkingGraphEditor.IsValid() || !CurrentWorkingGraph) return;

    const FScopedTransaction Transaction(NSLOCTEXT("NerveQuestEditor", "DeleteNodes", "Delete Selected Nodes"));
    CurrentWorkingGraph->Modify();

    const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
    CurrentWorkingGraphEditor->ClearSelectionSet();

    for (const auto& SelectedNode : SelectedNodes)
    {
        if (UNerveQuestObjectiveNodeBase* Node = Cast<UNerveQuestObjectiveNodeBase>(SelectedNode))
        {
            if (Node->CanUserDeleteNode())
            {
                Node->Modify();
                Node->DestroyNode();
            }
        }
    }

    CurrentWorkingGraphEditor->NotifyGraphChanged();
}

bool NerveQuestEditorToolkit::CanDeleteNodes() const
{
    const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
    for (const auto& SelectedNode : SelectedNodes)
    {
        if (const UNerveQuestObjectiveNodeBase* Node = Cast<UNerveQuestObjectiveNodeBase>(SelectedNode))
        {
            if (Node->CanUserDeleteNode()) return true;
        }
    }
    return false;
}

void NerveQuestEditorToolkit::CopySelectedNodes() const
{
    const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
    if (SelectedNodes.Num() == 0) return;

    for (const auto& SelectedNode : SelectedNodes)
    {
        if (UNerveQuestObjectiveNodeBase* Node = Cast<UNerveQuestObjectiveNodeBase>(SelectedNode))
        {
            Node->PrepareForCopying();
        }
    }

    FString ExportedText;
    FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
    FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool NerveQuestEditorToolkit::CanCopyNodes() const
{
    const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
    for (const auto& SelectedNode : SelectedNodes)
    {
        if (const UNerveQuestObjectiveNodeBase* Node = Cast<UNerveQuestObjectiveNodeBase>(SelectedNode))
        {
            if (Node->CanDuplicateNode())
            {
                return true;
            }
        }
    }
    return false;
}

void NerveQuestEditorToolkit::PasteNodes() const
{
    if (!CurrentWorkingGraphEditor.IsValid()) return;
    PasteNodesHere(CurrentWorkingGraphEditor->GetPasteLocation());
}

void NerveQuestEditorToolkit::PasteNodesHere(const FVector2D& Location) const
{
    if (!CurrentWorkingGraph || !CurrentWorkingGraphEditor.IsValid()) return;

    const FScopedTransaction Transaction(NSLOCTEXT("NerveQuestEditor", "PasteNodes", "Paste Nodes"));
    CurrentWorkingGraph->Modify();

    FString ClipboardContent;
    FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
    if (ClipboardContent.IsEmpty()) return;

    TSet<UEdGraphNode*> PastedNodes;
    FEdGraphUtilities::ImportNodesFromText(CurrentWorkingGraph, ClipboardContent, PastedNodes);

    FVector2D AvgPosition(0.0f, 0.0f);
    int32 NumNodes = 0;
    for (UEdGraphNode* Node : PastedNodes)
    {
        AvgPosition.X += Node->NodePosX;
        AvgPosition.Y += Node->NodePosY;
        NumNodes++;
    }
    if (NumNodes > 0)
    {
        AvgPosition /= NumNodes;
    }

    CurrentWorkingGraphEditor->ClearSelectionSet();
    for (UEdGraphNode* Node : PastedNodes)
    {
        Node->NodePosX += Location.X - AvgPosition.X;
        Node->NodePosY += Location.Y - AvgPosition.Y;
        Node->SnapToGrid(16);

        if (UNerveQuestObjectiveNodeBase* QuestNode = Cast<UNerveQuestObjectiveNodeBase>(Node))
        {
        	QuestNode->CreateNewGuid();
            QuestNode->SetParentGraphEditor(CurrentWorkingGraphEditor);
            QuestNode->InitObjectiveClassInstance();
        }

        CurrentWorkingGraphEditor->SetNodeSelection(Node, true);
    }

    CurrentWorkingGraph->NotifyGraphChanged();
}

bool NerveQuestEditorToolkit::CanPasteNodes() const
{
    if (!CurrentWorkingGraphEditor.IsValid()) return false;

    FString ClipboardContent;
    FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
    return !ClipboardContent.IsEmpty() && FEdGraphUtilities::CanImportNodesFromText(CurrentWorkingGraph, ClipboardContent);
}

void NerveQuestEditorToolkit::DuplicateNodes()
{
    CopySelectedNodes();
    PasteNodes();
}

bool NerveQuestEditorToolkit::CanDuplicateNodes() const
{
    return CanCopyNodes();
}

void NerveQuestEditorToolkit::SelectAllNodes() const
{
    if (CurrentWorkingGraphEditor.IsValid())
    {
        CurrentWorkingGraphEditor->SelectAllNodes();
    }
}

bool NerveQuestEditorToolkit::CanSelectAllNodes() const
{
    return CurrentWorkingGraphEditor.IsValid();
}

void NerveQuestEditorToolkit::OnGraphSelectionChanged(const FGraphPanelSelectionSet& NewPanelSelectionSet)
{
	UNerveQuestObjectiveNodeBase* SelectedNode = GetSelectedGraphNode(NewPanelSelectionSet);
	if(!IsValid(SelectedNode))
	{
		if(IsValid(CurrentSelectedNerveDialogueGraphNode))
		{
			if(IsValid(CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance()))
			{
				CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance()->StopObjectivePreview(GEditor->GetEditorWorldContext().World());
			}
			CurrentSelectedNerveDialogueGraphNode->SetParentGraphEditor(nullptr);
		}
		CurrentSelectedNerveDialogueGraphNode = nullptr;
		CurrentSelectedEditorDetailsView->SetObject(GetCurrentWorkingAsset());
		return;
	}

	if(SelectedNode != CurrentSelectedNerveDialogueGraphNode && IsValid(CurrentSelectedNerveDialogueGraphNode))
	{
		CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance()->StopObjectivePreview(GEditor->GetEditorWorldContext().World());
		CurrentSelectedNerveDialogueGraphNode->SetParentGraphEditor(nullptr);
	}
	CurrentSelectedNerveDialogueGraphNode = SelectedNode;
	CurrentSelectedNerveDialogueGraphNode->SetParentGraphEditor(CurrentWorkingGraphEditor);
	// Set the objects in the details view
	if(CurrentSelectedEditorDetailsView != nullptr)
	{
		if(IsValid(CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance()))
		{
			CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance()->StartObjectivePreview(GEditor->GetEditorWorldContext().World());
		}
		const bool IsCosmetic = CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance()->IsCosmetic();
		UObject* DetailViewObject = IsCosmetic? nullptr : CurrentSelectedNerveDialogueGraphNode->GetRuntimeObjectiveInstance();
		CurrentSelectedEditorDetailsView->SetObject(DetailViewObject, true);
	}
}

void NerveQuestEditorToolkit::RestoreGraphFromSaveAsset() const
{
	// if we don't have an asset to work with or a runtime graph
	if (!IsValid(CurrentWorkingAsset) || !IsValid(CurrentWorkingAsset->RuntimeGraph))
	{
		const UNerveQuestGraphSchema* Schema = Cast<UNerveQuestGraphSchema>(CurrentWorkingGraph->GetSchema());
		if(!IsValid(Schema)) return;
		Schema->CreateQuestDefaultNodesForGraph(*CurrentWorkingGraph, GetCurrentWorkingAsset());
		return;
	}

	// Create all the nodes/pins first
	TArray<std::pair<FGuid, FGuid>> Connections;
	TMap<FGuid, UEdGraphPin*> IdToPinMap;
	
	for (UNerveQuestRuntimeObjectiveBase* RuntimeNode : CurrentWorkingAsset->RuntimeGraph->GraphNodes)
	{
		if(!IsValid(RuntimeNode)) continue;
		
		UClass* EditorNodeClass = RuntimeNode->EditorClass ? RuntimeNode->EditorClass : UNerveQuestObjectiveNodeBase::StaticClass();

		// Special handling for entry/root nodes
		if (RuntimeNode->IsA<UNerveEntryObjective>()) 
		{
			EditorNodeClass = UNerveQuestRootObjective::StaticClass();
		}

		UNerveQuestObjectiveNodeBase* QuestNode = NewObject<UNerveQuestObjectiveNodeBase>(CurrentWorkingGraph, EditorNodeClass);
		QuestNode->CreateNewGuid();
		QuestNode->SetOuterObject(GetCurrentWorkingAsset());
		QuestNode->SetRuntimeObjectiveClass(RuntimeNode->GetClass());
		QuestNode->SetRuntimeObjectiveInstance(RuntimeNode);
		QuestNode->SetFlags(RF_Transactional);
		QuestNode->NodePosX = RuntimeNode->Location.X;
		QuestNode->NodePosY = RuntimeNode->Location.Y;
		QuestNode->SetIsUsedAsOptional(RuntimeNode->bIsOptionalObjective);
		
		// Handle input pin
        if (IsValid(RuntimeNode->InputPin))
        {
            UNerveQuestRuntimePin* RuntimePin = RuntimeNode->InputPin;
            UEdGraphPin* InPin = QuestNode->CreatePin(EGPD_Input, RuntimePin->PinCategory, RuntimePin->PinName);
            InPin->PinName = RuntimePin->PinName;
            InPin->PinType.PinSubCategory = RuntimePin->PinSubCategory;
            InPin->PinId = RuntimePin->PinId;

            // Store connections for later processing
            if(!RuntimePin->Connection.IsEmpty())
            {
                for (TWeakObjectPtr ConnectedPin : RuntimePin->Connection)
                {
                    if(ConnectedPin.IsValid())
                    {
                        Connections.Add(std::make_pair(RuntimePin->PinId, ConnectedPin.Get()->PinId));
                    }
                }
            }
            IdToPinMap.Add(RuntimePin->PinId, InPin);
        }

        // Handle regular output pins
        for (UNerveQuestRuntimePin* RuntimeOutputPin : RuntimeNode->OutPutPin)
        {
            if (!IsValid(RuntimeOutputPin)) continue;
            
            UEdGraphPin* OutPin = QuestNode->CreatePin(EGPD_Output, RuntimeOutputPin->PinCategory, RuntimeOutputPin->PinName);
            OutPin->PinName = RuntimeOutputPin->PinName;
            OutPin->PinType.PinSubCategory = RuntimeOutputPin->PinSubCategory;
            OutPin->PinId = RuntimeOutputPin->PinId;

            // Store connections for later processing
            if(!RuntimeOutputPin->Connection.IsEmpty())
            {
                for (TWeakObjectPtr<UNerveQuestRuntimePin> ConnectedPin : RuntimeOutputPin->Connection)
                {
                    if(ConnectedPin.IsValid())
                    {
                        Connections.Add(std::make_pair(RuntimeOutputPin->PinId, ConnectedPin.Get()->PinId));
                    }
                }
            }
            IdToPinMap.Add(RuntimeOutputPin->PinId, OutPin);
        }

        // Handle optional output pins separately
        for (UNerveQuestRuntimePin* RuntimeOptionalPin : RuntimeNode->OutOptionalPins)
        {
            if (!IsValid(RuntimeOptionalPin)) continue;
            
            UEdGraphPin* OptionalPin = QuestNode->CreatePin(EGPD_Output, RuntimeOptionalPin->PinCategory, RuntimeOptionalPin->PinName);
            OptionalPin->PinName = RuntimeOptionalPin->PinName;
            OptionalPin->PinType.PinSubCategory = RuntimeOptionalPin->PinSubCategory;
            OptionalPin->PinId = RuntimeOptionalPin->PinId;

            // Add to the QuestNode's OptionalPins array
            QuestNode->AddOptionalPins(OptionalPin);

            // Store connections for later processing
            if(!RuntimeOptionalPin->Connection.IsEmpty())
            {
                for (TWeakObjectPtr ConnectedPin : RuntimeOptionalPin->Connection)
                {
                    if(ConnectedPin.IsValid())
                    {
                        Connections.Add(std::make_pair(RuntimeOptionalPin->PinId, ConnectedPin.Get()->PinId));
                    }
                }
            }
            IdToPinMap.Add(RuntimeOptionalPin->PinId, OptionalPin);
        }

        // Don't call RefreshOptionalPins here since we're manually restoring them
        CurrentWorkingGraph->AddNode(QuestNode, true, false);
	}

	// Process all connections
	TSet<TPair<FGuid, FGuid>> ProcessedConnections;
	for (const std::pair<FGuid, FGuid>& Connection : Connections)
	{
		// Avoid duplicate connections
		TPair<FGuid, FGuid> SortedConnection = Connection.first < Connection.second ? 
			TPair<FGuid, FGuid>(Connection.first, Connection.second) : 
			TPair<FGuid, FGuid>(Connection.second, Connection.first);
			
		if (ProcessedConnections.Contains(SortedConnection)) continue;
		ProcessedConnections.Add(SortedConnection);
		
		UEdGraphPin* PinA = IdToPinMap.FindRef(Connection.first);
		UEdGraphPin* PinB = IdToPinMap.FindRef(Connection.second);
		
		if (PinA != nullptr && PinB != nullptr)
		{
			if (!PinA->LinkedTo.Contains(PinB))
			{
				PinA->LinkedTo.Add(PinB);
			}
			if (!PinB->LinkedTo.Contains(PinA))
			{
				PinB->LinkedTo.Add(PinA);
			}
			
			UE_LOG(LogTemp, Log, TEXT("Restored connection between %s and %s"), 
			*PinA->PinName.ToString(), *PinB->PinName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to restore connection: Pin %s or %s not found"), 
			*Connection.first.ToString(), *Connection.second.ToString());
		}
	}
}

void NerveQuestEditorToolkit::SaveGraphToAsset() const
{
	if (!IsValid(CurrentWorkingAsset) || !IsValid(CurrentWorkingGraph)) return;

	// Update the state we need into our save-able format
	UNerveQuestRuntimeGraph* RuntimeGraph = NewObject<UNerveQuestRuntimeGraph>(CurrentWorkingAsset);
	CurrentWorkingAsset->RuntimeGraph = RuntimeGraph;

	TArray<TPair<FGuid, FGuid>> NodeConnections;
	TMap<FGuid, UNerveQuestRuntimePin*> IdToPinMap;

	// First pass: Create all runtime nodes and pins
    for (const TObjectPtr<UEdGraphNode>& NodeConnection : CurrentWorkingGraph->Nodes)
    {
        UNerveQuestObjectiveNodeBase* GraphNode = Cast<UNerveQuestObjectiveNodeBase>(NodeConnection);
        if(!IsValid(GraphNode) || !IsValid(GraphNode->GetRuntimeObjectiveClass())) continue;
        
        UNerveQuestRuntimeObjectiveBase* RuntimeNode = GraphNode->GetRuntimeObjectiveInstance();
        if(!IsValid(RuntimeNode)) 
        {
            RuntimeNode = NewObject<UNerveQuestRuntimeObjectiveBase>(RuntimeGraph, GraphNode->GetRuntimeObjectiveClass());
        }
        
        // Clear existing connections
        RuntimeNode->EditorClass = GraphNode->GetClass();
        RuntimeNode->InputPin = nullptr;
        RuntimeNode->OutPutPin.Empty();
        RuntimeNode->OutOptionalPins.Empty();
        RuntimeNode->Location = FVector2D(GraphNode->NodePosX, GraphNode->NodePosY);
        RuntimeNode->bIsOptionalObjective = GraphNode->IsUsedAsOptional();

        for (UEdGraphPin* NodeConnectionPin : GraphNode->Pins)
        {
        	if (!LazyNerveValidator::ValidateEditorGraphPin(NodeConnectionPin)) continue;
        	
            UNerveQuestRuntimePin* RuntimePin = NewObject<UNerveQuestRuntimePin>(RuntimeNode);
            RuntimePin->PinName = NodeConnectionPin->PinName;
            RuntimePin->PinCategory = NodeConnectionPin->PinType.PinCategory;
            RuntimePin->PinSubCategory = NodeConnectionPin->PinType.PinSubCategory;
            RuntimePin->PinId = NodeConnectionPin->PinId;
            RuntimePin->ParentNode = RuntimeNode;
            RuntimePin->Connection.Empty();

            // Store all connections (both directions)
            if (NodeConnectionPin->LinkedTo.Num() > 0)
            {
                for (UEdGraphPin* LinkedPin : NodeConnectionPin->LinkedTo)
                {
                    NodeConnections.Emplace(NodeConnectionPin->PinId, LinkedPin->PinId);
                }
            }

            IdToPinMap.Add(NodeConnectionPin->PinId, RuntimePin);
            
            // Assign pins to their respective arrays
            if (NodeConnectionPin->Direction == EGPD_Input)
            {
                RuntimeNode->InputPin = RuntimePin;
            }
            else if (NodeConnectionPin->Direction == EGPD_Output)
            {
            	if (!UNerveQuestObjectiveNodeBase::IsOptionalPin(NodeConnectionPin))
            	{
            		RuntimeNode->OutPutPin.Add(RuntimePin);
            	}
                if (LazyNerveValidator::ValidateEditorOptionalGraphPin(NodeConnectionPin, GraphNode))
                {
                    RuntimeNode->OutOptionalPins.Add(RuntimePin);
                }
            }
        }
        
        RuntimeGraph->GraphNodes.Add(RuntimeNode);
    }

	// Second pass: Establish all connections
	for (const TPair<FGuid, FGuid>& PairConnections : NodeConnections)
	{
		UNerveQuestRuntimePin* PinA = IdToPinMap.FindRef(PairConnections.Key);
		UNerveQuestRuntimePin* PinB = IdToPinMap.FindRef(PairConnections.Value);
		
		if (IsValid(PinA) && IsValid(PinB))
		{
			// Add bidirectional connections to ensure both pins know about each other
			if (!PinA->Connection.Contains(PinB))
			{
				PinA->Connection.Add(PinB);
			}
			if (!PinB->Connection.Contains(PinA))
			{
				PinB->Connection.Add(PinA);
			}
			
			UE_LOG(LogTemp, Log, TEXT("Connected Pin %s to Pin %s"), 
			*PinA->PinName.ToString(), *PinB->PinName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to find pins for connection: %s -> %s"), 
			*PairConnections.Key.ToString(), *PairConnections.Value.ToString());
		}
	}
	
	// Debug logging to verify connections
	for (UNerveQuestRuntimeObjectiveBase* RuntimeNode : RuntimeGraph->GraphNodes)
	{
		if (!IsValid(RuntimeNode)) continue;
		
		UE_LOG(LogTemp, Log, TEXT("Node %s:"), *RuntimeNode->GetClass()->GetName());
		
		if (IsValid(RuntimeNode->InputPin))
		{
			UE_LOG(LogTemp, Log, TEXT("  Input Pin '%s' has %d connections"), 
			*RuntimeNode->InputPin->PinName.ToString(), RuntimeNode->InputPin->Connection.Num());
		}
		
		for (UNerveQuestRuntimePin* OutPin : RuntimeNode->OutPutPin)
		{
			if (IsValid(OutPin))
			{
				UE_LOG(LogTemp, Log, TEXT("  Output Pin '%s' has %d connections"), 
					   *OutPin->PinName.ToString(), 
					   OutPin->Connection.Num());
			}
		}
	}
}

void NerveQuestEditorToolkit::OnQuestAssetPreSave() const
{
	SaveGraphToAsset();
}

void NerveQuestEditorToolkit::OnQuestAssetPostEdit()
{}

void NerveQuestEditorToolkit::PopulateToolBar(FToolBarBuilder& ToolBarBuilder) const
{
	ToolBarBuilder.BeginSection(FName(TEXT("Nerve Quest Tools")));
	{
		const FText NewObjectiveLabel = NSLOCTEXT("Objective_Label", "New Objective", "Create New Objective");
		const FText NewObjectiveTooltip = NSLOCTEXT("Objective_ToolTip", "Create a new Objective","Create a new objective node blueprint from a base class");
		const FSlateIcon NewObjectiveIcon = FSlateIcon(FLazyNerveQuestStyle::GetStyleSetName(), "ClassThumbnail.NerveQuestRuntimeObjectiveBase");

		const FText NewRewardLabel = NSLOCTEXT("Reward_Label", "New Reward", "New Reward");
		const FText NewRewardTooltip = NSLOCTEXT("Reward_ToolTip", "Create a new reward","Create a new quest reward blueprint to be granted when a quest is completed from a base class");
		const FSlateIcon NewRewardIcon = FSlateIcon(FLazyNerveQuestStyle::GetStyleSetName(), "ClassThumbnail.NerveQuestRewardBase");

		ToolBarBuilder.AddToolBarButton
		(
			FUIAction(
				FExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::HandleNewNodeClassPicked, UNerveQuestRuntimeObjectiveBase::StaticClass()),
				FCanExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::CanCreateObjective),
				FIsActionChecked()
			),
			NAME_None,
			NewObjectiveLabel,
			NewObjectiveTooltip,
			NewObjectiveIcon
		);

		ToolBarBuilder.AddToolBarButton
		(
			FUIAction(
				FExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::HandleNewNodeClassPicked, UNerveQuestRewardBase::StaticClass()),
				FCanExecuteAction::CreateSP(this, &NerveQuestEditorToolkit::CanCreateReward),
				FIsActionChecked()
			),
			NAME_None,
			NewRewardLabel,
			NewRewardTooltip,
			NewRewardIcon
		);
	}
	ToolBarBuilder.EndSection();
}

void NerveQuestEditorToolkit::ExtendAssetToolBar()
{
	const TSharedPtr<FExtender> NerveQuestExtender = MakeShared<FExtender>();
	NerveQuestExtender->AddToolBarExtension
	(
		FName(TEXT("Asset")),
		EExtensionHook::After,
		ToolkitCommands,
		FToolBarExtensionDelegate::CreateSP(this, &NerveQuestEditorToolkit::PopulateToolBar)
	);
	
	AddToolbarExtender(NerveQuestExtender);
}

void NerveQuestEditorToolkit::SetCurrentSelectedDetailsView(const TSharedPtr<IDetailsView>& NewDetailsView)
{}

void NerveQuestEditorToolkit::SetCurrentSelectedEditorDetailsView(const TSharedPtr<IDetailsView>& NewDetailsView)
{
	CurrentSelectedEditorDetailsView = NewDetailsView;
	CurrentSelectedEditorDetailsView->OnFinishedChangingProperties().AddRaw(this, &NerveQuestEditorToolkit::OnObjectiveDetailsViewPropertyChanged);
	CurrentSelectedEditorDetailsView->SetObject(GetCurrentWorkingAsset());
}

void NerveQuestEditorToolkit::OnObjectiveDetailsViewPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if(CurrentWorkingGraphEditor == nullptr) return;
	UNerveQuestObjectiveNodeBase* SelectedNode = GetSelectedGraphNode(CurrentWorkingGraphEditor->GetSelectedNodes());
	if(IsValid(SelectedNode))
	{
		SelectedNode->OnNodePropertyChange(PropertyChangedEvent);
	}
	
	CurrentWorkingGraphEditor->NotifyGraphChanged();
}

UNerveQuestObjectiveNodeBase* NerveQuestEditorToolkit::GetSelectedGraphNode(const FGraphPanelSelectionSet& NewSelection)
{
	for (const auto Element : NewSelection)
	{
		UNerveQuestObjectiveNodeBase* GraphNode = Cast<UNerveQuestObjectiveNodeBase>(Element);
		if(!IsValid(GraphNode)) continue;

		return GraphNode;
	}

	return nullptr;
}

void NerveQuestEditorToolkit::HandleNewNodeClassPicked(UClass* InClass) const
{
	UE_CLOG(InClass == nullptr, NerveQuestEditorToolkitLog, Error, TEXT("Trying to handle new node of NULL class for Nerve quest %s ")
		, *GetNameSafe(CurrentWorkingAsset));

	if(CurrentWorkingAsset != nullptr && InClass != nullptr && CurrentWorkingAsset->GetOutermost())
	{
		const FString ClassName = FBlueprintEditorUtils::GetClassNameWithoutSuffix(InClass);

		FString PathName = CurrentWorkingAsset->GetOutermost()->GetPathName();
		PathName = FPaths::GetPath(PathName);
		
		// Now that we've generated some reasonable default locations/names for the package, allow the user to have the final say
		// before we create the package and initialize the blueprint inside of it.
		FSaveAssetDialogConfig SaveAssetDialogConfig;
		SaveAssetDialogConfig.DialogTitleOverride = NSLOCTEXT("SaveAssetDialogTitle", "Save Asset As", "Save Asset As");
		SaveAssetDialogConfig.DefaultPath = PathName;
		SaveAssetDialogConfig.DefaultAssetName = ClassName + TEXT("_New");
		SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

		const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		const FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
		if (!SaveObjectPath.IsEmpty())
		{
			const FString SavePackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
			const FString SavePackagePath = FPaths::GetPath(SavePackageName);
			const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

			UPackage* Package = CreatePackage(*SavePackageName);
			if (ensure(Package))
			{
				// Create and init a new Blueprint
				if (UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(InClass, Package, FName(*SaveAssetName), BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass()))
				{
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewBP);

					// Notify the asset registry
					FAssetRegistryModule::AssetCreated(NewBP);

					// Mark the package dirty...
					Package->MarkPackageDirty();
				}
			}
		}
	}

	FSlateApplication::Get().DismissAllMenus();
}
