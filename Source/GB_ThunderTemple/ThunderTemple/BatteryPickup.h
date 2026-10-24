// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BatteryPickup.generated.h"

class AThunderFinale;
class UStaticMeshComponent;

/**
 *  The battery at the foot of the main rod, the player's final goal. Taking it starts the finale.
 */
UCLASS()
class ABatteryPickup : public AActor
{
	GENERATED_BODY()

	/** Battery mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Body;

protected:

	/** Finale started when the battery is taken. If empty, the first one in the level is used */
	UPROPERTY(EditAnywhere, Category="Battery")
	TObjectPtr<AThunderFinale> Finale;

public:

	/** Constructor */
	ABatteryPickup();

	/** Takes the battery and starts the finale */
	UFUNCTION(BlueprintCallable, Category="Battery")
	void Take();
};
