// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightningRodMain.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 *  The main lightning rod. Every bolt starts at its tip and travels to a sub rod.
 *  Thunder delays are measured from this rod, in rings of fixed width around it.
 */
UCLASS()
class ALightningRodMain : public AActor
{
	GENERATED_BODY()

	/** Base of the rod */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Rod pole */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Pole;

	/** Rod cap */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Cap;

protected:

	/** Height of the pole, in cm */
	UPROPERTY(EditAnywhere, Category="Lightning Rod", meta = (ClampMin = 100, Units = "cm"))
	float PoleHeight = 2000.0f;

	/** Radius of the pole, in cm */
	UPROPERTY(EditAnywhere, Category="Lightning Rod", meta = (ClampMin = 5, Units = "cm"))
	float PoleRadius = 60.0f;

	/** Width of each ring around this rod; crossing one ring adds one step to the thunder delay */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Timing", meta = (ClampMin = 100, Units = "cm"))
	float RingSpacing = 1000.0f;

	/** Thunder delay added per ring, in seconds */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Timing", meta = (ClampMin = 0, Units = "s"))
	float SecondsPerRing = 1.0f;

	/** If true, the delay grows linearly with the bolt length instead of in ring steps */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Timing")
	bool bLinearDelay = false;

	/** Bolt length covered per second of delay when using the linear mode */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Timing", meta = (ClampMin = 100, Units = "cm", EditCondition = "bLinearDelay"))
	float LinearDistancePerSecond = 1000.0f;

	/** Draws the timing rings in the editor viewport */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Debug")
	bool bDrawRings = true;

	/** Number of rings to draw */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Debug", meta = (ClampMin = 1, EditCondition = "bDrawRings"))
	int32 RingsToDraw = 8;

public:

	/** Constructor */
	ALightningRodMain();

	/** Resizes the meshes to match the pole settings */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Draws the timing rings while editing */
	virtual bool ShouldTickIfViewportsOnly() const override { return bDrawRings; }
	virtual void Tick(float DeltaSeconds) override;

	/** World position bolts start from */
	UFUNCTION(BlueprintPure, Category="Lightning Rod")
	FVector GetStrikeOrigin() const;

	/** Thunder delay, in seconds, for a bolt that ends at the given world position */
	UFUNCTION(BlueprintPure, Category="Lightning Rod")
	float GetThunderDelayTo(const FVector& Target) const;
};
