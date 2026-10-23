// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BrassPlatePickup.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPrimitiveComponent;

/**
 *  A brass lightning plate lying in the level. Walking over it puts it in the player's inventory.
 */
UCLASS()
class ABrassPlatePickup : public AActor
{
	GENERATED_BODY()

	/** Plate mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Plate;

	/** Pickup range */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* PickupArea;

protected:

	/** Number of plates given */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 1))
	int32 Count = 1;

public:

	/** Constructor */
	ABrassPlatePickup();

protected:

	UFUNCTION()
	void OnPickupBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
