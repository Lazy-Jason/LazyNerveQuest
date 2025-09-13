#include "UnrealED/Node/NerveQuestObjectiveNodeBase.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"
#include "EdGraph/EdGraphNode.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "LazyNerveQuestEditor.h"
#include "ToolMenu.h"
#include "Framework/Commands/GenericCommands.h"
#include "EdGraph/EdGraphNode.h"
#include "GraphEditor.h"
#include "ToolMenu.h"
#include "BlueprintEditorSettings.h"
#include "SourceCodeNavigation.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Preferences/UnrealEdOptions.h"
#include "Widgets/Notifications/SNotificationList.h"

void UNerveQuestObjectiveNodeBase::InitObjectiveClassInstance()
{
    if (!IsValid(RuntimeObjectiveClass) || IsValid(RuntimeObjectiveInstance)) return;
    RuntimeObjectiveInstance = NewObject<UNerveQuestRuntimeObjectiveBase>(OuterObject ? OuterObject : this, RuntimeObjectiveClass);
}

void UNerveQuestObjectiveNodeBase::UnInitObjectiveClassInstance()
{
    if (!IsValid(RuntimeObjectiveInstance)) return;
    RuntimeObjectiveInstance = nullptr;
}

void UNerveQuestObjectiveNodeBase::OnNodePropertyChange(const FPropertyChangedEvent& PropertyChangedEvent)
{
    GenerateOptionalPins();
}

void UNerveQuestObjectiveNodeBase::AllocateDefaultPins()
{
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, TEXT("Exec"));
    if (!bIsUsedAsOptional)
    {
        CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Completed"));
        GenerateOptionalPins();
    }
}

void UNerveQuestObjectiveNodeBase::ReconstructNode()
{
    UpdateOptionalStatus();
    Super::ReconstructNode();
}

void UNerveQuestObjectiveNodeBase::AutowireNewNode(UEdGraphPin* FromPin)
{
    Super::AutowireNewNode(FromPin);
}

FText UNerveQuestObjectiveNodeBase::GetTooltipText() const
{
    if (!IsValid(RuntimeObjectiveInstance)) return FText::FromString(TEXT("Default Quest Graph Node Tooltip"));
    return RuntimeObjectiveInstance->GetObjectiveDescription();
}

bool UNerveQuestObjectiveNodeBase::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const
{
    return DesiredSchema != nullptr;
}

void UNerveQuestObjectiveNodeBase::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
    Super::GetNodeContextMenuActions(Menu, Context);
    if (!Context->Node) return;
    FToolMenuSection& Section = Menu->AddSection("NerveQuestGraphNodeActions", FText::FromString("Node Actions"));
    
    Section.AddMenuEntry(FGenericCommands::Get().SelectAll);
    Section.AddMenuEntry(FGenericCommands::Get().Delete);
    Section.AddMenuEntry(FGenericCommands::Get().Copy);
    Section.AddMenuEntry(FGenericCommands::Get().Paste);
    Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
    Section.AddMenuEntry(FGraphEditorCommands::Get().AddBreakpoint);
    Section.AddMenuEntry(FGraphEditorCommands::Get().RemoveBreakpoint);
    Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
}

void UNerveQuestObjectiveNodeBase::NodeConnectionListChanged()
{
    Super::NodeConnectionListChanged();
}

void UNerveQuestObjectiveNodeBase::AddOptionalPins(UEdGraphPin* NewPin)
{
    if (NewPin == nullptr) return;
    OptionalPins.Add(NewPin);
}

UEdGraph* UNerveQuestObjectiveNodeBase::GetExecuteObjectiveGraph() const
{
    if (RuntimeObjectiveClass == nullptr) return nullptr;

    UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(RuntimeObjectiveClass);
    if (BPClass != nullptr)
    {
        UBlueprint* Blueprint = Cast<UBlueprint>(BPClass->ClassGeneratedBy);
        if (Blueprint != nullptr)
        {
            for (UEdGraph* Graph : Blueprint->FunctionGraphs)
            {
                if (Graph->GetFName() == TEXT("ExecuteObjective"))
                {
                    return Graph;
                }
            }
        }
    }
    return nullptr;
}

bool UNerveQuestObjectiveNodeBase::CanJumpToDefinition() const
{
    if (RuntimeObjectiveClass == nullptr) return false;

    if (GetExecuteObjectiveGraph() != nullptr) return true;

    const UFunction* ExecuteObjectiveFunc = RuntimeObjectiveClass->FindFunctionByName(TEXT("ExecuteObjective"));
    if (ExecuteObjectiveFunc != nullptr && ExecuteObjectiveFunc->IsNative())
    {
        return ensure(GUnrealEd) && GUnrealEd->GetUnrealEdOptions()->IsCPPAllowed();
    }

    return false;
}

UObject* UNerveQuestObjectiveNodeBase::GetJumpTargetForDoubleClick() const
{
    return GetExecuteObjectiveGraph();
}

void UNerveQuestObjectiveNodeBase::JumpToDefinition() const
{
    if (!RuntimeObjectiveClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("RuntimeObjectiveClass is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("JumpToDefinition: RuntimeObjectiveClass = %s"), *RuntimeObjectiveClass->GetName());

    // Blueprint graph navigation
    const UEdGraph* Graph = GetExecuteObjectiveGraph();
    if (Graph)
    {
        FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(Graph);
        return;
    }

    // Native C++ navigation
    if (RuntimeObjectiveClass->IsNative())
    {
        UFunction* TargetFunction = RuntimeObjectiveClass->FindFunctionByName(TEXT("ExecuteObjective"));
        if (TargetFunction && FSourceCodeNavigation::NavigateToFunction(TargetFunction))
        {
            return;
        }

        FString HeaderPath;
        if (FSourceCodeNavigation::FindClassHeaderPath(RuntimeObjectiveClass, HeaderPath))
        {
            FString AbsHeaderPath = FPaths::ConvertRelativePathToFull(HeaderPath);
            FSourceCodeNavigation::OpenSourceFile(AbsHeaderPath);
        }
    }
    else
    {
        // Blueprint class without graph, fall back to native parent
        FString NativeParentClassHeaderPath;
        if (FSourceCodeNavigation::FindClassHeaderPath(RuntimeObjectiveClass, NativeParentClassHeaderPath))
        {
            FString AbsPath = FPaths::ConvertRelativePathToFull(NativeParentClassHeaderPath);
            FSourceCodeNavigation::OpenSourceFile(AbsPath);
        }
    }
}

FString UNerveQuestObjectiveNodeBase::GetDocumentationLink() const
{
    return Super::GetDocumentationLink();
}

void UNerveQuestObjectiveNodeBase::UpdateOptionalStatus()
{
    const bool bWasOptional = bIsUsedAsOptional;
    bIsUsedAsOptional = false;

    if (UEdGraphPin* InputPin = FindPin(TEXT("Exec"), EGPD_Input))
    {
        for (UEdGraphPin* LinkedPin : InputPin->LinkedTo)
        {
            if (LinkedPin != nullptr && IsOptionalPin(LinkedPin))
            {
                bIsUsedAsOptional = true;
                break;
            }
        }
    }

    if (UNerveQuestRuntimeObjectiveBase* RuntimeInstance = GetRuntimeObjectiveInstance())
    {
        RuntimeInstance->SetConnectedAsOptional(bIsUsedAsOptional);
    }

    if (bWasOptional != bIsUsedAsOptional)
    {
        GenerateOptionalPins();
        if (UEdGraph* Graph = GetGraph())
        {
            Graph->NotifyGraphChanged();
        }
    }
}

void UNerveQuestObjectiveNodeBase::PinConnectionListChanged(UEdGraphPin* Pin)
{
    Super::PinConnectionListChanged(Pin);

    if (Pin == nullptr || !Pin->GetOwningNode() || !IsValid(Pin->GetOwningNode())) return;
    
    if (GEditor)
    {
        GEditor->GetTimerManager()->SetTimerForNextTick([this]()
        {
            if (IsValid(this))
            {
                UpdateOptionalStatus();
            }
        });
    }
    else
    {
        UpdateOptionalStatus();
    }
}

void UNerveQuestObjectiveNodeBase::GenerateOptionalPins()
{
    if (bIsUsedAsOptional)
    {
        ClearOptionalPins();
        return;
    }

    if (!IsValid(GetRuntimeObjectiveInstance()) || !GetRuntimeObjectiveInstance()->CanGenerateOptionals()) return;

    ClearOptionalPins();

    TArray<FText> Optionals = GetRuntimeObjectiveInstance()->GetObjectiveOptionals();
    
    OptionalPins.Reserve(Optionals.Num());
    
    for (int32 i = 0; i < Optionals.Num(); i++)
    {
        FString OptionalName = Optionals[i].IsEmpty() ? 
            FString::Printf(TEXT("Optional %d"), i) : 
            Optionals[i].ToString();

        if (UEdGraphPin* NewPin = CreatePin(EGPD_Output, FLazyNerveQuestEditorModule::NerveQuestOptionalPinCategory, FName(*OptionalName)))
        {
            NewPin->PinFriendlyName = FText::FromString(FString::Printf(TEXT("Optional %d"), i));
            OptionalPins.Add(NewPin);
        }
    }
}

void UNerveQuestObjectiveNodeBase::ClearOptionalPins()
{
    TArray<UEdGraphPin*> PinsToRemove = OptionalPins;
    OptionalPins.Empty();
    
    for (UEdGraphPin* Pin : PinsToRemove)
    {
        if (Pin != nullptr)
        {
            Pin->BreakAllPinLinks();
            Pins.Remove(Pin);
            Pin->MarkAsGarbage();
        }
    }
    
    ReconstructNode();
}

void UNerveQuestObjectiveNodeBase::RefreshOptionalPins()
{
    if (!IsValid(GetRuntimeObjectiveInstance())) return;
    
    TArray<FText> CurrentOptionals = GetRuntimeObjectiveInstance()->GetObjectiveOptionals();
    
    bool bNeedsUpdate = (CurrentOptionals.Num() != OptionalPins.Num());
    
    if (!bNeedsUpdate)
    {
        for (int32 i = 0; i < CurrentOptionals.Num(); i++)
        {
            FString ExpectedName = CurrentOptionals[i].IsEmpty() ? 
                FString::Printf(TEXT("Optional %d"), i) : 
                CurrentOptionals[i].ToString();
                
            if (i < OptionalPins.Num() && OptionalPins[i] && 
                OptionalPins[i]->PinName.ToString() != ExpectedName)
            {
                bNeedsUpdate = true;
                break;
            }
        }
    }
    
    if (bNeedsUpdate)
    {
        GenerateOptionalPins();
    }
}

bool UNerveQuestObjectiveNodeBase::IsOptionalPin(const UEdGraphPin* Pin)
{
    if (Pin == nullptr) return false;
    
    UEdGraphNode* OwningNode = Pin->GetOwningNode();
    if (!OwningNode || !IsValid(OwningNode)) return false;
    
    if (OwningNode->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed)) return false;
    
    if (Pin->PinType.PinCategory == FLazyNerveQuestEditorModule::NerveQuestOptionalPinCategory) return true;

    const FString PinName = Pin->PinName.IsValid() ? Pin->PinName.ToString() : FString();
    const FString PinFriendlyName = Pin->PinFriendlyName.IsEmpty() ? FString() : Pin->PinFriendlyName.ToString();
    
    if (PinName.Contains(TEXT("Optional")) || PinFriendlyName.Contains(TEXT("Optional"))) return true;

    return false;
}