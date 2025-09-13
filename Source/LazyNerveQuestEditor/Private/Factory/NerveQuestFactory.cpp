// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory/NerveQuestFactory.h"
#include "Objects/NerveQuest/NerveQuestAsset.h"

UNerveQuestFactory::UNerveQuestFactory(const FObjectInitializer& ObjectInitializer)
{
	SupportedClass = UNerveQuestAsset::StaticClass();
}

UObject* UNerveQuestFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	UNerveQuestAsset* NerveAsset_Proxy = NewObject<UNerveQuestAsset>(InParent, SupportedClass, InName, Flags);
	return NerveAsset_Proxy;
}

bool UNerveQuestFactory::CanCreateNew() const { return true; }
