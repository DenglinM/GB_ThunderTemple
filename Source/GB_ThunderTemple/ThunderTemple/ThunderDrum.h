// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderDrum.generated.h"

class UStaticMeshComponent;
class USoundBase;

/**
 *  A drum the player can beat. The noise draws every stalker within hearing range to the drum.
 */
UCLASS()
class AThunderDrum : public AActor
{
	GENERATED_BODY()

	/** Drum body */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Body;

protected:

	/** Stalkers within this distance hear the drum, in cm */
	UPROPERTY(EditAnywhere, Category="Drum", meta = (ClampMin = 0, Units = "cm"))
	float HearingRadius = 2500.0f;

	/** Minimum time between two beats */
	UPROPERTY(EditAnywhere, Category="Drum", meta = (ClampMin = 0, Units = "s"))
	float Cooldown = 1.5f;

	/** Sound of a beat */
	UPROPERTY(EditAnywhere, Category="Drum")
	TObjectPtr<USoundBase> BeatSound;

	/** Time of the last beat */
	float LastBeatTime = -1000.0f;

public:

	/** Constructor */
	AThunderDrum();

	/** Beats the drum: plays the sound and calls nearby stalkers over */
	UFUNCTION(BlueprintCallable, Category="Drum")
	void Beat();
};
