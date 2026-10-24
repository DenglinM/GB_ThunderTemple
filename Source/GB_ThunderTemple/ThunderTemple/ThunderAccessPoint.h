// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderAccessPoint.generated.h"

class AThunderGate;
class UStaticMeshComponent;

/**
 *  A conductor plate wired to a gate. Lightning striking anywhere within its radius opens the gate for a short time.
 */
UCLASS()
class AThunderAccessPoint : public AActor
{
	GENERATED_BODY()

	/** Floor plate showing the access point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Plate;

	/** Thin disc on the floor marking the pickup radius */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RangeMarker;

protected:

	/** Gate opened by a strike */
	UPROPERTY(EditAnywhere, Category="Access Point")
	TObjectPtr<AThunderGate> Gate;

	/** Lightning landing within this ground distance opens the gate, in cm */
	UPROPERTY(EditAnywhere, Category="Access Point", meta = (ClampMin = 0, Units = "cm"))
	float Radius = 400.0f;

	/** How long the gate stays open */
	UPROPERTY(EditAnywhere, Category="Access Point", meta = (ClampMin = 0.1, Units = "s"))
	float OpenTime = 2.0f;

public:

	/** Constructor */
	AThunderAccessPoint();

	/** Sizes the range marker */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called by every strike in the level; opens the gate if the lightning landed in range */
	void NotifyLightning(const FVector& StrikeLocation);
};
