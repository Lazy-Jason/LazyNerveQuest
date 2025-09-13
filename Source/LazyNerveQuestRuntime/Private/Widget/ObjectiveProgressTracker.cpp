// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/ObjectiveProgressTracker.h"

void UObjectiveProgressTracker::SetCurrent_Implementation(const float NewCurrent)
{ Current = NewCurrent; }

void UObjectiveProgressTracker::SetMax_Implementation(const float NewMax)
{ Max = NewMax; }
