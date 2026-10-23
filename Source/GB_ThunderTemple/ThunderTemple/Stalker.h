// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Stalker.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStalkerStateEvent, bool, bActive);

/**
 *  Blind stalker enemy. Gray-box stand-in for now: it has an active state and a paused state,
 *  and a lightning strike nearby pauses it. Hearing and chasing will be added on top of this.
 */
UCLASS()
class AStalker : public ACharacter
{
	GENERATED_BODY()

	/** Gray-box body */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Body;

protected:

	/** How long a strike pauses the stalker. 0 keeps it paused for good */
	UPROPERTY(EditAnywhere, Category="Stalker", meta = (ClampMin = 0, Units = "s"))
	float PauseDuration = 0.0f;

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

	/** Switches movement, AI and visuals to the given state */
	void SetActive(bool bNewActive);
};
