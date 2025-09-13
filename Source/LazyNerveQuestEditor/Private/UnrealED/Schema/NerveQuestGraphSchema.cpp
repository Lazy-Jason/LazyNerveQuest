// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealED/Schema/NerveQuestGraphSchema.h"
#include "AIGraphTypes.h"
#include "LazyNerveQuestEditor.h"
#include "Factory/Graph/NerveQuestGraphNodeMapper.h"
#include "Objects/Nodes/Objective/NerveEntryObjective.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"
#include "UnrealED/Node/NerveQuestObjectiveNodeBase.h"
#include "UnrealED/Node/NerveQuestRootObjective.h"

UEdGraphNode* FNerveQuestGraphSchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
const FVector2D Location, bool bSelectNewNode)
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "NerveQuestObjective_AddNode", "Add Node"));

	// Run the new objective to be created through a mapper to get the right node class to initialize
	const TSubclassOf<UNerveQuestObjectiveNodeBase> GraphNodeClass = NerveQuestGraphNodeMapper::CreateNewGraphNodeForRuntimeInstance(NodeClass);
	if (!IsValid(GraphNodeClass)) return nullptr;
	
	// Create a single instance of the node
	UNerveQuestObjectiveNodeBase* QuestNode = NewObject<UNerveQuestObjectiveNodeBase>(ParentGraph, GraphNodeClass);
	QuestNode->CreateNewGuid();
	QuestNode->SetRuntimeObjectiveClass(NodeClass);
	QuestNode->SetOuterObject(OuterObject);
	QuestNode->InitObjectiveClassInstance();
	QuestNode->SetFlags(RF_Transactional);
	QuestNode->AllocateDefaultPins();
	QuestNode->NodePosX = Location.X;
	QuestNode->NodePosY = Location.Y;

	if (FromPin != nullptr)
	{
		if (UEdGraphPin* NewNodeInputPin = QuestNode->FindPin(UEdGraphSchema_K2::PN_Execute))
		{
			QuestNode->GetSchema()->TryCreateConnection(FromPin, NewNodeInputPin);
		}
	}

	ParentGraph->Modify();
	ParentGraph->AddNode(QuestNode, true, bSelectNewNode);

	return QuestNode;
}

void UNerveQuestGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	Super::GetGraphContextActions(ContextMenuBuilder);

	const FText AddToolTip = NSLOCTEXT("NerveQuestSchema", "NewNerveQuestNodeTooltip", "Add a {NodeType} node here");

	TArray<FGraphNodeClassData> NodeClasses;
	FGraphNodeClassHelper* ClassCache = FLazyNerveQuestEditorModule::Get().GetClassCache().Get();
	ClassCache->GatherClasses(UNerveQuestRuntimeObjectiveBase::StaticClass(), NodeClasses);
	
	for (FGraphNodeClassData& NodeClass : NodeClasses)
	{
		if (NodeClass.IsAbstract() || NodeClass.GetClass() == UNerveEntryObjective::StaticClass()) continue;
		
		FText NodeTitle;
		FText NodeCategory;
		
		UClass* NewNodeClass = NodeClass.GetClass();
		if(IsValid(NewNodeClass) && NewNodeClass->GetDefaultObject<UNerveQuestRuntimeObjectiveBase>())
		{
			NodeTitle = NewNodeClass->GetDefaultObject<UNerveQuestRuntimeObjectiveBase>()->GetObjectiveName();
			NodeCategory = NewNodeClass->GetDefaultObject<UNerveQuestRuntimeObjectiveBase>()->GetObjectiveCategory();
		}
		else
		{
			NodeTitle = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));
			NodeCategory = FText::FromString("Quest Objective Nodes");
		}
		const FText MenuDesc = FText::Format(NSLOCTEXT("NerveDialogueGraphNode", "AddObjective", "Add {0} Objective"), NodeTitle);
		FText Tooltip = FText::Format(AddToolTip, NodeTitle);

		TSharedPtr<FNerveQuestGraphSchemaAction> Action = MakeShared<FNerveQuestGraphSchemaAction>
		(
			NodeCategory, 
			MenuDesc, 
			Tooltip, 
			0,
			NewNodeClass,
			OuterObject
		);

		ContextMenuBuilder.AddAction(Action);
	}
}

const FPinConnectionResponse UNerveQuestGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
    // Enhanced pin validation
    if (A == nullptr || B == nullptr)
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot Connect Invalid Pins"));
    }
    
    // Validate owning nodes
    UEdGraphNode* NodeA = A->GetOwningNode();
    UEdGraphNode* NodeB = B->GetOwningNode();
    
    if (!IsValid(NodeA) || !IsValid(NodeB))
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot Connect Pins With Invalid Nodes"));
    }

    if (A->Direction == B->Direction)
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Pins Of The Same Direction Cannot Be Connected"));
    }

    // Check if we're connecting an optional pin to an exec input
    const UEdGraphPin* OptionalPin = nullptr;
    const UEdGraphPin* ExecPin = nullptr;
    
    if (UNerveQuestObjectiveNodeBase::IsOptionalPin(A) && B->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
    {
        OptionalPin = A;
        ExecPin = B;
    }
    else if (UNerveQuestObjectiveNodeBase::IsOptionalPin(B) && A->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
    {
        OptionalPin = B;
        ExecPin = A;
    }

    // If connecting optional to exec, validate the target node can become optional
    if (OptionalPin && ExecPin && ExecPin->Direction == EGPD_Input)
    {
        if (UNerveQuestObjectiveNodeBase* TargetNode = Cast<UNerveQuestObjectiveNodeBase>(ExecPin->GetOwningNode()))
        {
            // Additional validation could go here
            return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_AB, TEXT("Connect Optional Pin"));
        }
    }
    
    return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_AB, TEXT("Connect Pin"));
}

void UNerveQuestGraphSchema::CreateQuestDefaultNodesForGraph(UEdGraph& Graph, UObject* Outer) const
{
	if(!IsValid(Outer)) return;
	
	OuterObject = Outer;
	CreateDefaultNodesForGraph(Graph);
}

void UNerveQuestGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	UNerveQuestRootObjective* RootNode = NewObject<UNerveQuestRootObjective>(&Graph);
	if(!IsValid(RootNode)) return;

	RootNode->CreateNewGuid();
	RootNode->SetRuntimeObjectiveClass(UNerveEntryObjective::StaticClass());
	RootNode->SetOuterObject(OuterObject);
	RootNode->InitObjectiveClassInstance();
	RootNode->SetFlags(RF_Transactional);
	RootNode->AllocateDefaultPins();
	RootNode->NodePosX = 0.0f;
	RootNode->NodePosY = 0.0f;

	Graph.AddNode(RootNode);
	Graph.Modify();
}

bool UNerveQuestGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
    // Pre-connection validation
    if (A == nullptr || B == nullptr) return false;

    const bool bConnectionCreated = Super::TryCreateConnection(A, B);
    
    if (bConnectionCreated)
    {
        // Post-connection updates
        UEdGraphNode* NodeA = A->GetOwningNode();
        UEdGraphNode* NodeB = B->GetOwningNode();
        
        if (UNerveQuestObjectiveNodeBase* QuestNodeA = Cast<UNerveQuestObjectiveNodeBase>(NodeA))
        {
            // Defer the update to avoid issues during connection
            if (GEditor)
            {
                GEditor->GetTimerManager()->SetTimerForNextTick([QuestNodeA]()
                {
                    if (IsValid(QuestNodeA))
                    {
                        QuestNodeA->UpdateOptionalStatus();
                    }
                });
            }
        }
        
        if (UNerveQuestObjectiveNodeBase* QuestNodeB = Cast<UNerveQuestObjectiveNodeBase>(NodeB))
        {
            // Defer the update to avoid issues during connection
            if (GEditor)
            {
                GEditor->GetTimerManager()->SetTimerForNextTick([QuestNodeB]()
                {
                    if (IsValid(QuestNodeB))
                    {
                        QuestNodeB->UpdateOptionalStatus();
                    }
                });
            }
        }
    }
    
    return bConnectionCreated;
}
