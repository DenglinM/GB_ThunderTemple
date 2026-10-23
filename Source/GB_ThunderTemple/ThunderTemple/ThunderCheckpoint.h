// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderCheckpoint.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPrimitiveComponent;

/**
 *  Mid-level save point. Stepping on it makes it the place the player respawns after falling.
 */
UCLASS()
class AThunderCheckpoint : public AActor
{
	GENERATED_BODY()

	/** Base, at floor level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Green floor pad */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Pad;

	/** Area that activates the checkpoint */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Trigger;

protected:

	/** Pad size, in cm */
	UPROPERTY(EditAnywhere, Category="Checkpoint")
	FVector PadSize = FVector(250.0f, 250.0f, 12.0f);

public:

	/** Constructor */
	AThunderCheckpoint();

	/** Sizes the pad and trigger */
	virtual void OnConstruction(const FTransform& Transform) override;

protected:

	UFUNCTION()
	void OnTriggerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
