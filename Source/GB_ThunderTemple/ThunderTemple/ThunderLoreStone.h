// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderLoreStone.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 *  A glowing yellow stone carrying a piece of story or tutorial text.
 *  Place copies around the level and write each one's text in the Details panel; the player reads it with E.
 */
UCLASS()
class AThunderLoreStone : public AActor
{
	GENERATED_BODY()

	/** Base of the stone, at floor level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Stone mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Stone;

protected:

	/** Heading shown above the text; may be left empty */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore Stone")
	FText Title;

	/** The text shown when the player reads the stone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore Stone", meta = (MultiLine = true))
	FText Message;

	/** Distance from which the stone can be read, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore Stone", meta = (ClampMin = 50, Units = "cm"))
	float ReadRadius = 250.0f;

public:

	/** Constructor */
	AThunderLoreStone();

	/** Heading shown above the text */
	const FText& GetTitle() const { return Title; }

	/** The text to show */
	const FText& GetMessage() const { return Message; }

	/** Distance from which the stone can be read */
	float GetReadRadius() const { return ReadRadius; }
};
