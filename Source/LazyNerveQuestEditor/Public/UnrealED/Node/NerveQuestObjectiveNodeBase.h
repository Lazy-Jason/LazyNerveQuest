#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "NerveQuestObjectiveNodeBase.generated.h"

class UNerveQuestRuntimeObjectiveBase;

UCLASS()
class LAZYNERVEQUESTEDITOR_API UNerveQuestObjectiveNodeBase : public UEdGraphNode
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    UClass* RuntimeObjectiveClass = nullptr;

    UPROPERTY()
    UNerveQuestRuntimeObjectiveBase* RuntimeObjectiveInstance = nullptr;

    UPROPERTY()
    UObject* OuterObject = nullptr;

    /** Cached state of whether this node is being used as an optional */
    UPROPERTY()
    bool bIsUsedAsOptional = false;

protected:
    /** Reference to the parent graph editor */
    TWeakPtr<SGraphEditor> ParentGraphEditor = nullptr;

    TArray<UEdGraphPin*> OptionalPins = TArray<UEdGraphPin*>();

public:
    /** Initialize the runtime objective instance */
    void InitObjectiveClassInstance();

    /** Clean up the runtime objective instance */
    void UnInitObjectiveClassInstance();
    
    void OnNodePropertyChange(const FPropertyChangedEvent& PropertyChangedEvent);

    virtual void RefreshOptionalPins();
    
    /** Check if this objective is currently being used as an optional */
    UFUNCTION(BlueprintPure, Category = "Quest Node")
    bool IsUsedAsOptional() const { return bIsUsedAsOptional; }

    UFUNCTION(BlueprintCallable, Category = "Quest Node")
    void SetIsUsedAsOptional(const bool NewOptional) { bIsUsedAsOptional = NewOptional; }

    /** Update the optional status based on pin connections */
    void UpdateOptionalStatus();

    static bool IsOptionalPin(const UEdGraphPin* Pin);

    // UEdGraphNode interface
    virtual void AllocateDefaultPins() override;
    virtual void ReconstructNode() override;
    virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
    virtual FText GetTooltipText() const override;
    virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
    virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor(FColor::Blue); }
    virtual bool CanUserDeleteNode() const override { return true; }
    virtual bool CanDuplicateNode() const override { return true; }
    virtual FLinearColor GetNodeBodyTintColor() const override { return FLinearColor(FColor::Purple); }
    virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
    virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
    virtual void NodeConnectionListChanged() override;
    void AddOptionalPins(UEdGraphPin* NewPin);
    virtual UObject* GetJumpTargetForDoubleClick() const override;
    virtual bool CanJumpToDefinition() const override;
    virtual void JumpToDefinition() const override;
    virtual FString GetDocumentationLink() const override;

    // Getters
    UObject* GetOuterObject() const { return OuterObject; }
    TArray<UEdGraphPin*> GetOptionalPins() const { return OptionalPins; }
    UClass* GetRuntimeObjectiveClass() const { return RuntimeObjectiveClass; }
    UNerveQuestRuntimeObjectiveBase* GetRuntimeObjectiveInstance() const { return RuntimeObjectiveInstance; }

    // Setters
    void SetRuntimeObjectiveInstance(UNerveQuestRuntimeObjectiveBase* NewInstance) { RuntimeObjectiveInstance = NewInstance; }
    void SetRuntimeObjectiveClass(UClass* NewClass) { RuntimeObjectiveClass = NewClass; }
    void SetOuterObject(UObject* NewOuter) { OuterObject = NewOuter; }
    void SetParentGraphEditor(const TSharedPtr<SGraphEditor>& NewParentGraphEditor) { ParentGraphEditor = NewParentGraphEditor; }

protected:
    virtual void GenerateOptionalPins();
    virtual void ClearOptionalPins();

    /** Helper function to find the Blueprint graph for ExecuteObjective, if it exists */
    UEdGraph* GetExecuteObjectiveGraph() const;
};