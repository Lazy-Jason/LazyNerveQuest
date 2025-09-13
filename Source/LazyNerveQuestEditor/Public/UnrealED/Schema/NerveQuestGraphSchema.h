// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "NerveQuestGraphSchema.generated.h"

USTRUCT()
struct FNerveQuestGraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	UClass* NodeClass;
	
	UPROPERTY()
	UObject* OuterObject = nullptr;

public:
	FNerveQuestGraphSchemaAction(): NodeClass() {}
	FNerveQuestGraphSchemaAction(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping, UClass* InNodeClass, UObject* Outer) 
	: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeClass(InNodeClass), OuterObject(Outer) {}

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
};

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTEDITOR_API UNerveQuestGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

	UPROPERTY()
	mutable UObject* OuterObject = nullptr;

public:
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual void CreateQuestDefaultNodesForGraph(UEdGraph& Graph, UObject* Outer) const;
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual bool CanShowDataTooltipForPin(const UEdGraphPin& Pin) const override { return true; }
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
};
