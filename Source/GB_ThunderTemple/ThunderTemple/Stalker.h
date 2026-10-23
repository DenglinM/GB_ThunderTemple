// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Stalker.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStalkerStateEvent, bool, bActive);

/**
 *  Blind stalker enemy. Gray-box stand-in for now: it walks a patrol loop, a lightning strike nearby
 *  pauses it, and it resumes when that strike's thunder arrives. Hearing and chasing come later.
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

	/** Steers towards the current patrol point; reschedules itself every frame */
	void PatrolStep();

	/** Switches movement, AI and visuals to the given state */
	void SetActive(bool bNewActive);
};
