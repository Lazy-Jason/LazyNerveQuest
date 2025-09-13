// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NerveQuestRewardBase.generated.h"

/**
 * @brief A base class for quest rewards in the Lazy Nerve Quests system.
 * 
 * This abstract class is intended to be subclassed to define different types of quest rewards. It provides a 
 * standardized interface for retrieving reward details such as the label, value, and icon, and a method for 
 * applying the reward to an entity when a quest is completed.
 * 
 * The class is Blueprintable, allowing designers to extend functionality in Blueprints, and supports 
 * in-line editing, instance default behavior, and spawning in components.
 */
UCLASS(Blueprintable, Abstract, EditInlineNew, DefaultToInstanced, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LAZYNERVEQUESTRUNTIME_API UNerveQuestRewardBase : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * @brief Retrieves the reward's label as text.
	 * 
	 * This function provides a human-readable label for the reward, useful for UI display or logging.
	 * Can be overridden in Blueprint or native C++ subclasses.
	 * 
	 * @return The label describing this reward.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category="Quest Rewards")
	FText GetRewardLabel();

	/**
	 * @brief Retrieves the value or description of the reward.
	 * 
	 * This function returns a detailed value or description of the reward, which might include its 
	 * quantity or special attributes. Can be overridden in subclasses.
	 * 
	 * @return The value of the reward, such as an amount of currency or experience points.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category="Quest Rewards")
	FText GetRewardValue();

	/**
	 * @brief Retrieves the icon representing the reward, primarily used in UI.
	 * 
	 * This function provides an image associated with the reward, which can be displayed in the user interface. 
	 * It is intended to help players easily identify the type of reward being offered.
	 * 
	 * @return A pointer to a UTexture2D representing the icon for this reward.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category="Quest Rewards")
	UTexture2D* GetRewardIcon();

	/**
	 * @brief Grants the reward to the entity completing the quest.
	 * 
	 * This method performs the necessary logic to apply the reward to the player or entity who 
	 * completed the quest. It is the key function responsible for delivering the reward.
	 * 
	 * Can be overridden in subclasses to define specific reward-granting logic, whether it be 
	 * adding items to an inventory, granting experience points, or other in-game effects.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Quest Rewards")
	void GrantReward(APlayerController* PlayerController);
};
