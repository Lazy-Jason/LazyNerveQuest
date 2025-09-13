// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Rewards/NerveQuestRewardBase.h"

FText UNerveQuestRewardBase::GetRewardLabel_Implementation()
{ return FText::FromString(TEXT("Default Reward")); }

FText UNerveQuestRewardBase::GetRewardValue_Implementation()
{ return FText::FromString(TEXT("1")); }

UTexture2D* UNerveQuestRewardBase::GetRewardIcon_Implementation()
{ return nullptr; }

void UNerveQuestRewardBase::GrantReward_Implementation(APlayerController* PlayerController)
{ /*Perform reward granting here*/ }
