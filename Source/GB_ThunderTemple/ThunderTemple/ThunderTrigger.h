// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderTrigger.generated.h"

class ALightningRodSub;
class UBoxComponent;
class UPrimitiveComponent;

/**
 *  A box that strikes all of its linked rods at once when the player walks into it.
 */
UCLASS()
class AThunderTrigger : public AActor
{
	GENERATED_BODY()

	/** Trigger volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Box;

protected:

	/** Rods struck together when the player enters */
	UPROPERTY(EditAnywhere, Category="Thunder Trigger")
	TArray<TObjectPtr<ALightningRodSub>> Rods;

	/** Box half size, in cm */
	UPROPERTY(EditAnywhere, Category="Thunder Trigger")
	FVector BoxExtent = FVector(450.0f, 270.0f, 200.0f);

	/** If true, the trigger only fires the first time the player enters */
	UPROPERTY(EditAnywhere, Category="Thunder Trigger")
	bool bOnce = false;

	/** Set once the trigger has fired */
	bool bFired = false;

public:

	/** Constructor */
	AThunderTrigger();

	/** Sizes the box */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Strikes every linked rod that is ready */
	UFUNCTION(BlueprintCallable, Category="Thunder Trigger")
	void Fire();

protected:

	UFUNCTION()
	void OnBoxBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
