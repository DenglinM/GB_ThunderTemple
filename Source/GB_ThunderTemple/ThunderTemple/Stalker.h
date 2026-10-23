// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Stalker.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStalkerStateEvent, bool, bActive);

/**
 *  Stalker enemy, gray-box stand-in. It walks a patrol loop or stands guard; if set to chase, it hunts the player
 *  on sight (a crouched player is spotted from closer only) and sends them back to their checkpoint on contact.
 *  A lightning strike nearby pauses it until that strike's thunder arrives.
 */
UCLASS()
class AStalker : public ACharacter
{
	GENERATED_BODY()

	/** Gray-box body */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Body;

protected:

	/** 0: a strike pauses the stalker until its thunder arrives. Above 0: resume after this many seconds instead */
	UPROPERTY(EditAnywhere, Category="Stalker", meta = (ClampMin = 0, Units = "s"))
	float PauseDuration = 0.0f;

	/** Patrol loop, relative to the actor. Empty: the stalker stands still */
	UPROPERTY(EditAnywhere, Category="Stalker|Patrol", meta = (MakeEditWidget = true))
	TArray<FVector> PatrolPoints;

	/** Walking speed on patrol, in cm/s */
	UPROPERTY(EditAnywhere, Category="Stalker|Patrol", meta = (ClampMin = 0, Units = "cm/s"))
	float PatrolSpeed = 200.0f;

	/** Distance at which a patrol point counts as reached, in cm */
	UPROPERTY(EditAnywhere, Category="Stalker|Patrol", meta = (ClampMin = 10, Units = "cm"))
	float AcceptRadius = 80.0f;

	/** Patrol points in world space, captured at BeginPlay */
	TArray<FVector> WorldPatrol;

	/** Index of the patrol point being walked to */
	int32 PatrolIndex = 0;

	FTimerHandle PatrolTimer;

	/** If true, the stalker chases the player while it can see them */
	UPROPERTY(EditAnywhere, Category="Stalker|Chase")
	bool bChasePlayer = false;

	/** Detection range for a standing player, in cm. 0: no distance limit, only line of sight */
	UPROPERTY(EditAnywhere, Category="Stalker|Chase", meta = (ClampMin = 0, Units = "cm"))
	float DetectionRadius = 1500.0f;

	/** Detection range multiplier while the player is crouched */
	UPROPERTY(EditAnywhere, Category="Stalker|Chase", meta = (ClampMin = 0, ClampMax = 1))
	float CrouchDetectionScale = 0.5f;

	/** Chase speed as a multiple of the player's walking speed */
	UPROPERTY(EditAnywhere, Category="Stalker|Chase", meta = (ClampMin = 0))
	float ChaseSpeedMultiplier = 1.2f;

	/** Distance at which the player is caught and sent back to their checkpoint, in cm */
	UPROPERTY(EditAnywhere, Category="Stalker|Chase", meta = (ClampMin = 0, Units = "cm"))
	float CatchRadius = 120.0f;

	/** Optional area (e.g. a Trigger Box) the player must be inside before the stalker hunts them */
	UPROPERTY(EditAnywhere, Category="Stalker|Chase")
	TObjectPtr<AActor> ActivationArea;

	/** True while chasing the player or heading to where they were last seen */
	bool bChasing = false;

	/** Where the player was last seen */
	FVector LastSeenLocation = FVector::ZeroVector;

	/** Where a non-patrolling stalker returns after losing the player */
	FVector HomeLocation = FVector::ZeroVector;

	/** Body material while active */
	UPROPERTY(EditAnywhere, Category="Stalker")
	TObjectPtr<UMaterialInterface> ActiveMaterial;

	/** Body material while paused */
	UPROPERTY(EditAnywhere, Category="Stalker")
	TObjectPtr<UMaterialInterface> PausedMaterial;

	/** True while the stalker is active */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stalker")
	bool bActive = true;

	FTimerHandle ResumeTimer;

public:

	/** Called when the stalker is paused or resumes */
	UPROPERTY(BlueprintAssignable, Category="Stalker")
	FStalkerStateEvent OnStateChanged;

	/** Constructor */
	AStalker();

	/** Applies the body material for the current state */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Pauses the stalker after a lightning hit */
	UFUNCTION(BlueprintCallable, Category="Stalker")
	void HitByLightning();

	/** Returns the stalker to its active state */
	UFUNCTION(BlueprintCallable, Category="Stalker")
	void Resume();

	/** True while the stalker is active */
	UFUNCTION(BlueprintPure, Category="Stalker")
	bool IsActive() const { return bActive; }

protected:

	virtual void BeginPlay() override;

	/** Chases, patrols or returns home; reschedules itself every frame */
	void PatrolStep();

	/** True if the player is inside the activation area, within detection range and in clear line of sight */
	bool CanSeePlayer(const ACharacter* Player) const;

	/** Walks towards a point on the ground plane; returns the remaining distance */
	float WalkTowards(const FVector& Target);

	/** Switches movement, AI and visuals to the given state */
	void SetActive(bool bNewActive);
};
