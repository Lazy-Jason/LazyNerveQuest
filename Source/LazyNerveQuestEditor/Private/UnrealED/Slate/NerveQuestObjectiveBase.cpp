// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealED/Slate/NerveQuestObjectiveBase.h"
#include "BlueprintNodeHelpers.h"
#include "GraphEditorSettings.h"
#include "IDocumentation.h"
#include "LazyNerveQuestStyle.h"
#include "SCommentBubble.h"
#include "SGraphPin.h"
#include "SlateOptMacros.h"
#include "TutorialMetaData.h"
#include "Objects/Nodes/Objective/NerveQuestRuntimeObjectiveBase.h"
#include "UnrealED/Node/NerveQuestObjectiveNodeBase.h"
#include "UnrealED/Slate/NerveQuestObjectiveGraphPin.h"
#include "UnrealED/Slate/NerveQuestObjectiveProperty.h"
#include "Widgets/Images/SImage.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

const TArray<TSharedPtr<FProperty>>* operator*(const TArray<FProperty*>& Array);

void SNerveQuestObjectiveBase::Construct(const FArguments& InArgs, UNerveQuestObjectiveNodeBase* InNode)
{
	GraphNode = Cast<UEdGraphNode>(InNode);
	this->SetCursor(EMouseCursor::CardinalCross);
	this->StyleSetting = GetDefault<UNerveQuestEditorStyleSetting>();
	this->UpdateGraphNode();
	this->SetDefaultTitleAreaWidget(SNew(SOverlay));
}

void SNerveQuestObjectiveBase::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();
	
	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	TSharedPtr<SVerticalBox> MainVerticalBox;
	SetupErrorReporting();

	if (!SWidget::GetToolTip().IsValid())
	{
		const TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip
		(
			TAttribute<FText>(this, &SGraphNode::GetNodeTooltip), nullptr, GraphNode->GetDocumentationLink(),
			GraphNode->GetDocumentationExcerptName());
		SetToolTip(DefaultToolTip);
	}

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	PopulateMetaTag(&TagMeta);

	this->ContentScale.Bind( this, &SGraphNode::GetContentScale );

	const TSharedPtr<SVerticalBox> InnerVerticalBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSpacer)
			.Size(FVector2D(0, 15))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			CreateNodeContentArea()
		];

	const TSharedPtr<SWidget> EnabledStateWidget = GetEnabledStateWidget();
	if (EnabledStateWidget.IsValid())
	{
		InnerVerticalBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(FMargin(2, 0))
			[
				EnabledStateWidget.ToSharedRef()
			];
	}

	InnerVerticalBox->AddSlot()
		.AutoHeight()
		.Padding(Settings->GetNonPinNodeBodyPadding())
		[
			ErrorReporting->AsWidget()
		];



	this->GetOrAddSlot( ENodeZone::Center )
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(MainVerticalBox, SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SOverlay)
				.AddMetaData<FGraphNodeMetaData>(TagMeta)
				+SOverlay::Slot()
				.Padding(Settings->GetNonPinNodeBodyPadding())
				[
					SNew(SImage)
					.Image(GetNodeBodyBrush())
					.ColorAndOpacity(this, &SGraphNode::GetNodeBodyColor)
				]
				+SOverlay::Slot()
				[
					 SAssignNew(GraphNodeBody, SBox)
					.MinDesiredWidth(this, &SNerveQuestObjectiveBase::GetNodeMinWidth)
					.MaxDesiredWidth(this, &SNerveQuestObjectiveBase::GetNodeMaxWidth)
					.MinDesiredHeight(this, &SNerveQuestObjectiveBase::GetNodeMinHeight)
					.MaxDesiredHeight(this, &SNerveQuestObjectiveBase::GetNodeMaxHeight)
					[
						SNew(SBorder)
						.BorderImage(FLazyNerveQuestStyle::Get().GetBrush("NerveQuestNodeBorderStyle_V2"))
						.BorderBackgroundColor(this, &SNerveQuestObjectiveBase::GetNodeBodyColor)
						[
							SNew(SBorder)
							.BorderImage(FLazyNerveQuestStyle::Get().GetBrush("NerveQuestNodeBorderStyle_V2_Gradient"))
							.BorderBackgroundColor(this, &SNerveQuestObjectiveBase::GetNodeBodyGradientColor)
							[
								SNew(SBorder)
								.BorderImage(FLazyNerveQuestStyle::Get().GetBrush("QuestNodeTopBorder"))
								.BorderBackgroundColor(this, &SNerveQuestObjectiveBase::GetNodeBodyHighlightColor)
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.FillHeight(1.0f)
									[
										InnerVerticalBox.ToSharedRef()
									]
								]
							]
						]
					]
				]
			]			
		];

	bool SupportsBubble = true;
	if (GraphNode != nullptr)
	{
		SupportsBubble = GraphNode->SupportsCommentBubble();
	}

	if (SupportsBubble)
	{
		// Create comment bubble
		TSharedPtr<SCommentBubble> CommentBubble;
		const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

		SAssignNew(CommentBubble, SCommentBubble)
			.GraphNode(GraphNode)
			.Text(this, &SGraphNode::GetNodeComment)
			.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
			.OnToggled(this, &SGraphNode::OnCommentBubbleToggled)
			.ColorAndOpacity(CommentColor)
			.AllowPinning(true)
			.EnableTitleBarBubble(true)
			.EnableBubbleCtrls(true)
			.GraphLOD(this, &SGraphNode::GetCurrentLOD)
			.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

		GetOrAddSlot(ENodeZone::TopCenter)
			.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
			.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
			.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
			.VAlign(VAlign_Top)
			[
				CommentBubble.ToSharedRef()
			];
	}

	CreateBelowWidgetControls(MainVerticalBox);
	CreatePinWidgets();
	CreateInputSideAddButton(LeftNodeBox);
	CreateOutputSideAddButton(RightNodeBox);
}

TSharedRef<SWidget> SNerveQuestObjectiveBase::CreateNodeContentArea()
{
	return SNew(SHorizontalBox)
      
	  // Left area, usually for pins
	  + SHorizontalBox::Slot()
	  .AutoWidth()
	  [
		  SAssignNew(LeftNodeBox, SVerticalBox)
	  ]
      
	  // Central content area
	  + SHorizontalBox::Slot()
	  .FillWidth(1.0f)
	  [
		  CreateQuestCentralNode()
	  ]
      
	  // Right area with two sections for pins
	  + SHorizontalBox::Slot()
	  .AutoWidth()
	  [
		  SNew(SVerticalBox)
		  
		  // Top section for main output pins (Success, etc.)
		  + SVerticalBox::Slot()
		  .AutoHeight()
		  .VAlign(VAlign_Top)
		  [
			  SAssignNew(RightNodeBox, SVerticalBox) // This will be your main output section
		  ]
		  
		  // Spacer to push optional pins to bottom
		  + SVerticalBox::Slot()
		  .AutoHeight()
		  [
			  SNew(SSpacer)
			  .Size(FVector2D(0, 25))
		  ]
		  
		  // Bottom section for optional pins
		  + SVerticalBox::Slot()
		  .FillHeight(1.0)
		  .VAlign(VAlign_Bottom)
		  [
			  SAssignNew(OptionalPinsBox, SVerticalBox) // for optional pins
		  ]
	  ];
}

TSharedRef<SWidget> SNerveQuestObjectiveBase::CreateQuestCentralNode()
{
	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	// Icon Section
	const FSlateBrush* QuestBrush = GetQuestBrush();
	if (QuestBrush != nullptr && QuestBrush->GetResourceName() != FName())
	{
		HorizontalBox->AddSlot()
		.Padding(10)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(70, 70))
			.Image(QuestBrush)
			.ColorAndOpacity(this, &SNerveQuestObjectiveBase::GetNodeIconColor)
		];
	}

	// Title Section
	HorizontalBox->AddSlot()
	.Padding(10)
	.FillWidth(1.0f)
	.VAlign(VAlign_Center)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(this, &SNerveQuestObjectiveBase::GetQuestTitle)
			.Font(this, &SNerveQuestObjectiveBase::GetNodeTitleFontSize)
			.TransformPolicy(ETextTransformPolicy::ToUpper)
			.ColorAndOpacity(this, &SNerveQuestObjectiveBase::GetNodeTitleFontColor)
			.Justification(this, reinterpret_cast<TAttribute<ETextJustify::Type>::FGetter::TConstMethodPtr<SNerveQuestObjectiveBase>>(&
				               SNerveQuestObjectiveBase::GetTitleJustification))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			GenerateObjectiveProperty()
		]
	];

	return HorizontalBox;
}

void SNerveQuestObjectiveBase::CreatePinWidgets()
{
	UNerveQuestObjectiveNodeBase* StateNode = CastChecked<UNerveQuestObjectiveNodeBase>(GraphNode);
    
    if (!StateNode || !IsValid(StateNode))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid state node when creating pin widgets"));
        return;
    }

    // Validate pins before creating widgets
    for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
    {
        UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
        
        // Enhanced pin validation
        if (MyPin == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid pin at index %d"), PinIdx);
            continue;
        }
        
        if (!MyPin->bHidden)
        {
            TSharedPtr<SGraphPin> NewPin = SNew(SNerveQuestObjectiveGraphPin, MyPin);
            
            if (NewPin.IsValid())
            {
                AddPin(NewPin.ToSharedRef());
            }
        }
    }
}

void SNerveQuestObjectiveBase::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

    const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
    
    // Enhanced pin validation
    if (PinObj == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting to add invalid pin to node widget"));
        return;
    }
    
    if (PinObj && PinObj->bAdvancedView)
    {
        PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
    }

    if (PinToAdd->GetDirection() == EGPD_Input)
    {
        if (LeftNodeBox.IsValid())
        {
            LeftNodeBox->AddSlot()
            .HAlign(HAlign_Left)
            .VAlign(VAlign_Top)
            .AutoHeight()
            [
                PinToAdd
            ];
            InputPins.Add(PinToAdd);
        }
    }
    else // Direction == EEdGraphPinDirection::EGPD_Output
    {
        // Check if this is an optional pin with enhanced validation
        const bool bIsOptionalPin = UNerveQuestObjectiveNodeBase::IsOptionalPin(PinObj);
        
        if (bIsOptionalPin)
        {
            // Add to optional pins section (bottom)
            if (OptionalPinsBox.IsValid())
            {
                OptionalPinsBox->AddSlot()
                .HAlign(HAlign_Right)
                .VAlign(VAlign_Bottom)
                .AutoHeight()
                .Padding(FMargin(3))
                [
                    PinToAdd
                ];
            }
        }
        else
        {
            // Add to main output section (top)
            if (RightNodeBox.IsValid())
            {
                RightNodeBox->AddSlot()
                .HAlign(HAlign_Right)
                .VAlign(VAlign_Top)
                .AutoHeight()
                .Padding(FMargin(3))
                [
                    PinToAdd
                ];
            }
        }
        
        OutputPins.Add(PinToAdd);
    }
}

TSharedRef<SWidget> SNerveQuestObjectiveBase::GenerateObjectiveProperty() const
{
	TSharedPtr<SVerticalBox> PropertyBox;
    
	SAssignNew(PropertyBox, SVerticalBox);

	const UNerveQuestObjectiveNodeBase* ObjectiveNode = Cast<UNerveQuestObjectiveNodeBase>(GraphNode);
	if (IsValid(ObjectiveNode))
	{
		const UNerveQuestRuntimeObjectiveBase* RuntimeObjective = ObjectiveNode->GetRuntimeObjectiveInstance();
		if (IsValid(RuntimeObjective))
		{
			TArray<FString> PropertyDescLines = RuntimeObjective->GetPropertyDescription();
            
			for (const FString& Line : PropertyDescLines)
			{
				PropertyBox->AddSlot()
				.AutoHeight()
				.Padding(FMargin(0, 2))
				[
					SNew(SNerveQuestObjectiveProperty)
					.PropertyText(Line)
				];
			}
		}
	}

	return PropertyBox.ToSharedRef();
}

FText SNerveQuestObjectiveBase::GetQuestTitle() const
{
	if(!IsValid(GraphNode)) return FText::FromString(TEXT("No Title"));

	const UNerveQuestObjectiveNodeBase* ObjectiveProxy = Cast<UNerveQuestObjectiveNodeBase>(GraphNode);
	if(!IsValid(ObjectiveProxy)) return FText::FromString(TEXT("No Title"));

	UNerveQuestRuntimeObjectiveBase* DefaultObjectiveBase = ObjectiveProxy->GetRuntimeObjectiveInstance();

	if(!IsValid(DefaultObjectiveBase)) return FText::FromString(TEXT("No Title"));

	return DefaultObjectiveBase->GetObjectiveName();
}

const FSlateBrush* SNerveQuestObjectiveBase::GetQuestBrush()
{
	if(!IsValid(GraphNode)) return nullptr;

	const UNerveQuestObjectiveNodeBase* ObjectiveProxy = Cast<UNerveQuestObjectiveNodeBase>(GraphNode);
	if(!IsValid(ObjectiveProxy)) return nullptr;

	const UNerveQuestRuntimeObjectiveBase* DefaultObjectiveBase = ObjectiveProxy->GetRuntimeObjectiveInstance();

	if(!IsValid(DefaultObjectiveBase)) return nullptr;
	
	CachedBrush = DefaultObjectiveBase->GetObjectiveBrush();
	return &CachedBrush;
}

ETextJustify::Type SNerveQuestObjectiveBase::GetTitleJustification()
{
	const FSlateBrush* QuestBrush = GetQuestBrush();

	if(QuestBrush == nullptr) return ETextJustify::Center;
	
	return QuestBrush->GetResourceName() == FName()? ETextJustify::Center : ETextJustify::Left;
}

FMargin SNerveQuestObjectiveBase::GetGradientPadding() const
{
	return StyleSetting->NodeBodyGradientPadding;
}


FOptionalSize SNerveQuestObjectiveBase::GetNodeMinWidth() const
{
	return StyleSetting->NodeBodyMinWidth;
}

FOptionalSize SNerveQuestObjectiveBase::GetNodeMaxWidth() const
{
	return StyleSetting->NodeBodyMaxWidth;
}

FOptionalSize SNerveQuestObjectiveBase::GetNodeMinHeight() const
{
	return StyleSetting->NodeBodyMinHeight;
}

FOptionalSize SNerveQuestObjectiveBase::GetNodeMaxHeight() const
{
	return StyleSetting->NodeBodyMaxHeight;
}

FSlateColor SNerveQuestObjectiveBase::GetNodeBodyColor() const
{
	return StyleSetting->NodeBodyColor;
}

FSlateColor SNerveQuestObjectiveBase::GetNodeBodyGradientColor() const
{
	return StyleSetting->NodeBodyGradientColor;
}

FSlateColor SNerveQuestObjectiveBase::GetNodeBodyHighlightColor() const
{
	return StyleSetting->NodeBodyHighlightColor;
}

FSlateColor SNerveQuestObjectiveBase::GetNodeIconColor() const
{
	return StyleSetting->NodeIconColor;
}

FSlateColor SNerveQuestObjectiveBase::GetNodeTitleFontColor() const
{
	return StyleSetting->NodeHeaderFontColor;
}

FSlateFontInfo SNerveQuestObjectiveBase::GetNodeTitleFontSize() const
{
	return FCoreStyle::GetDefaultFontStyle("Bold", StyleSetting->NodeHeaderFontSize);
}

FSlateColor SNerveQuestObjectiveBase::GetNodePropertyBackgroundColor() const
{
	return StyleSetting->NodeBodyPropertyBackdropColor;
}

FSlateFontInfo SNerveQuestObjectiveBase::GetNodePropertyFontSize() const
{
	return FCoreStyle::GetDefaultFontStyle("Regular", StyleSetting->NodePropertyFontSize);
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
