// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "NerveQuestFactory.generated.h"

/**
 * 
 */
UCLASS()
class LAZYNERVEQUESTEDITOR_API UNerveQuestFactory : public UFactory
{
	GENERATED_BODY()

public:
	UNerveQuestFactory(const FObjectInitializer& ObjectInitializer);
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
};
