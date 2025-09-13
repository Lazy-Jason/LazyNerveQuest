// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NerveQuestRuntimePin.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTRUNTIME_API UNerveQuestRuntimePin : public UObject
{
	GENERATED_BODY()

public:
	//PinName
	UPROPERTY()
	FName PinName = FName(TEXT("Default"));

	UPROPERTY()
	FName PinCategory = FName(TEXT(""));

	UPROPERTY()
	FName PinSubCategory = FName(TEXT(""));

	//PinId
	UPROPERTY()
	FGuid PinId;

	//Pins this is connected to (using weak references to prevent circular references)
	UPROPERTY()
	TArray<TWeakObjectPtr<UNerveQuestRuntimePin>> Connection = TArray<TWeakObjectPtr<UNerveQuestRuntimePin>>();

	UPROPERTY()
	TWeakObjectPtr<class UNerveQuestRuntimeObjectiveBase> ParentNode;

public:
	/** Safely get parent node, returns nullptr if weak reference is stale */
	UFUNCTION(BlueprintPure, Category = "Pin")
	class UNerveQuestRuntimeObjectiveBase* GetParentNode() const { return ParentNode.Get(); }

	/** Safely get connected pins, automatically filters out stale references */
	UFUNCTION(BlueprintPure, Category = "Pin")
	TArray<UNerveQuestRuntimePin*> GetValidConnections() const
	{
		TArray<UNerveQuestRuntimePin*> ValidConnections;
		for (const TWeakObjectPtr<UNerveQuestRuntimePin>& WeakPin : Connection)
		{
			if (UNerveQuestRuntimePin* Pin = WeakPin.Get())
			{
				ValidConnections.Add(Pin);
			}
		}
		return ValidConnections;
	}

	/** Clean up stale connections */
	void CleanupStaleConnections()
	{
		Connection.RemoveAll([](const TWeakObjectPtr<UNerveQuestRuntimePin>& WeakPin)
		{
			return !WeakPin.IsValid();
		});
	}

	/** Add connection safely */
	void AddConnection(UNerveQuestRuntimePin* Pin)
	{
		if (IsValid(Pin))
		{
			Connection.AddUnique(TWeakObjectPtr<UNerveQuestRuntimePin>(Pin));
		}
	}

	/** Remove connection safely */
	void RemoveConnection(UNerveQuestRuntimePin* Pin)
	{
		if (IsValid(Pin))
		{
			Connection.RemoveAll([Pin](const TWeakObjectPtr<UNerveQuestRuntimePin>& WeakPin)
			{
				return WeakPin.Get() == Pin;
			});
		}
	}
};
