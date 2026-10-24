// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderGate.generated.h"

class ALightningRodSub;
class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPrimitiveComponent;

/**
 *  A gate that only opens while thunder is sounding.
 *  It sinks into the floor when a linked rod's thunder arrives and rises again when the thunder ends.
 *  If the player waits at the closed gate, it calls lightning on a nearby rod so the lesson always plays out.
 */
UCLASS()
class AThunderGate : public AActor
{
	GENERATED_BODY()

	/** Base of the gate, at floor level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Door slab */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Door;

	/** Area in front of the gate where a waiting player calls the lightning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* WaitArea;

protected:

	/** Rods whose thunder opens the gate. If empty, every sub rod in the level opens it */
	UPROPERTY(EditAnywhere, Category="Thunder Gate")
	TArray<TObjectPtr<ALightningRodSub>> Rods;

	/** If false, thunder does not open the gate; only OpenFor does (e.g. from an access point) */
	UPROPERTY(EditAnywhere, Category="Thunder Gate")
	bool bOpenOnThunder = true;

	/** If false, a player waiting at the closed gate does not call lightning */
	UPROPERTY(EditAnywhere, Category="Thunder Gate")
	bool bCallLightningWhenWaiting = true;

	/** Rod struck when the player waits at the closed gate. If empty, the nearest rod is used */
	UPROPERTY(EditAnywhere, Category="Thunder Gate")
	TObjectPtr<ALightningRodSub> WaitRod;

	/** Door size, in cm (X thickness, Y width, Z height) */
	UPROPERTY(EditAnywhere, Category="Thunder Gate")
	FVector DoorSize = FVector(40.0f, 450.0f, 350.0f);

	/** Depth of the wait area in front of the door, in cm */
	UPROPERTY(EditAnywhere, Category="Thunder Gate", meta = (ClampMin = 50, Units = "cm"))
	float WaitAreaDepth = 600.0f;

	/** Time for the door to sink or rise */
	UPROPERTY(EditAnywhere, Category="Thunder Gate", meta = (ClampMin = 0.01, Units = "s"))
	float MoveTime = 0.3f;

	/** Delay before calling lightning again if the player is still waiting after the gate closes */
	UPROPERTY(EditAnywhere, Category="Thunder Gate", meta = (ClampMin = 0, Units = "s"))
	float RetryDelay = 1.5f;

	/** World time at which the gate closes again */
	float CloseTime = -1.0f;

	/** 0 = closed, 1 = fully sunk */
	float OpenAlpha = 0.0f;

	/** True while the player stands in the wait area */
	bool bPlayerWaiting = false;

	FTimerHandle RetryTimer;

public:

	/** Constructor */
	AThunderGate();

	/** Sizes the door and wait area */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Moves the door */
	virtual void Tick(float DeltaSeconds) override;

	/** Opens the gate for the given time, extending it if already open */
	UFUNCTION(BlueprintCallable, Category="Thunder Gate")
	void OpenFor(float Seconds);

	/** True while the gate is open or opening */
	UFUNCTION(BlueprintPure, Category="Thunder Gate")
	bool IsOpen() const;

protected:

	virtual void BeginPlay() override;

	/** Thunder handler bound to the rods */
	UFUNCTION()
	void OnRodThunder(float ThunderDelay, float ThunderDuration);

	/** Calls lightning on the wait rod if the player is waiting at the closed gate */
	void TryCallLightning();

	UFUNCTION()
	void OnWaitAreaBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnWaitAreaEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
