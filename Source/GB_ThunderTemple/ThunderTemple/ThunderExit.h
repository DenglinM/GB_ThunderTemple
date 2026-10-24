// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderExit.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPrimitiveComponent;
class UMaterialInterface;

/**
 *  The way out at the end. A booth whose door faces +X: dark and shut until the finale unlocks it,
 *  then bright and open. When the player steps inside, the door shuts behind them and the game is cleared.
 */
UCLASS()
class AThunderExit : public AActor
{
	GENERATED_BODY()

	/** Base of the booth, at floor level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Back wall */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BackWall;

	/** Left side wall */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LeftWall;

	/** Right side wall */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RightWall;

	/** Roof */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Roof;

	/** Door on the +X side; sinks into the floor when open */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Door;

	/** Inside of the booth */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Inside;

protected:

	/** Inner size of the booth, in cm (X depth, Y width, Z height) */
	UPROPERTY(EditAnywhere, Category="Exit")
	FVector BoothSize = FVector(400.0f, 600.0f, 400.0f);

	/** Wall thickness, in cm */
	UPROPERTY(EditAnywhere, Category="Exit", meta = (ClampMin = 5, Units = "cm"))
	float WallThickness = 40.0f;

	/** Time for the door to sink or rise */
	UPROPERTY(EditAnywhere, Category="Exit", meta = (ClampMin = 0.01, Units = "s"))
	float MoveTime = 0.6f;

	/** Look while locked */
	UPROPERTY(EditAnywhere, Category="Exit")
	TObjectPtr<UMaterialInterface> LockedMaterial;

	/** Look once unlocked */
	UPROPERTY(EditAnywhere, Category="Exit")
	TObjectPtr<UMaterialInterface> UnlockedMaterial;

	/** True once the finale has unlocked the exit */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Exit")
	bool bUnlocked = false;

	/** True once the player made it inside */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Exit")
	bool bCleared = false;

	/** 0 = shut, 1 = fully sunk */
	float OpenAlpha = 0.0f;

public:

	/** Constructor */
	AThunderExit();

	/** Builds the booth from the size settings */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Moves the door and watches for the player */
	virtual void Tick(float DeltaSeconds) override;

	/** Lights the exit up and opens the door */
	UFUNCTION(BlueprintCallable, Category="Exit")
	void Unlock();

	/** True once the player made it inside */
	UFUNCTION(BlueprintPure, Category="Exit")
	bool IsCleared() const { return bCleared; }

protected:

	/** Shuts the player in and ends the game */
	void Clear();

	/** Applies the locked or unlocked look */
	void ApplyLook();
};
