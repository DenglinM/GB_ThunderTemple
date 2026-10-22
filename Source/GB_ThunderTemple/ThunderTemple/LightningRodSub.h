// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightningRodSub.generated.h"

class ALightningRodMain;
class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class UPointLightComponent;
class USoundBase;
class UMaterialInterface;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLightningRodEvent, float, ThunderDelay);

/**
 *  A sub lightning rod. When the player comes close, a bolt jumps from the main rod to this rod,
 *  and the thunder follows after a delay that depends on the distance to the main rod.
 */
UCLASS()
class ALightningRodSub : public AActor
{
	GENERATED_BODY()

	/** Base of the rod */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Rod pole */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Pole;

	/** Player detection range */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* Trigger;

	/** Bolt from the main rod to this rod, hidden until a strike */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Bolt;

	/** Flash at the rod tip during a strike */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPointLightComponent* Flash;

protected:

	/** Main rod the bolts come from. If empty, the nearest main rod is used */
	UPROPERTY(EditAnywhere, Category="Lightning Rod")
	TObjectPtr<ALightningRodMain> MainRod;

	/** Height of the pole, in cm */
	UPROPERTY(EditAnywhere, Category="Lightning Rod", meta = (ClampMin = 50, Units = "cm"))
	float PoleHeight = 600.0f;

	/** Radius of the pole, in cm */
	UPROPERTY(EditAnywhere, Category="Lightning Rod", meta = (ClampMin = 5, Units = "cm"))
	float PoleRadius = 30.0f;

	/** Distance at which the player triggers a strike, in cm */
	UPROPERTY(EditAnywhere, Category="Lightning Rod", meta = (ClampMin = 50, Units = "cm"))
	float TriggerRadius = 400.0f;

	/** Minimum time between two strikes on this rod */
	UPROPERTY(EditAnywhere, Category="Lightning Rod", meta = (ClampMin = 0, Units = "s"))
	float Cooldown = 8.0f;

	/** Bolt thickness, in cm */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Effects", meta = (ClampMin = 1, Units = "cm"))
	float BoltRadius = 15.0f;

	/** How long each bolt flicker stays visible */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Effects", meta = (ClampMin = 0.01, Units = "s"))
	float FlickerTime = 0.08f;

	/** Light intensity at the rod tip during a strike */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Effects", meta = (ClampMin = 0))
	float FlashIntensity = 200000.0f;

	/** Material for the bolt */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Effects")
	TObjectPtr<UMaterialInterface> BoltMaterial;

	/** Sound played when the thunder arrives */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Effects")
	TObjectPtr<USoundBase> ThunderSound;

	/** Prints the bolt length and thunder delay on screen when striking */
	UPROPERTY(EditAnywhere, Category="Lightning Rod|Debug")
	bool bShowTiming = true;

	/** Time of the last strike */
	float LastStrikeTime = -1000.0f;

	/** Flicker step counter for the current strike */
	int32 FlickerStep = 0;

	FTimerHandle FlickerTimer;
	FTimerHandle ThunderTimer;

public:

	/** Called when the bolt appears; passes the thunder delay */
	UPROPERTY(BlueprintAssignable, Category="Lightning Rod")
	FLightningRodEvent OnLightning;

	/** Called when the thunder arrives; passes the delay it had */
	UPROPERTY(BlueprintAssignable, Category="Lightning Rod")
	FLightningRodEvent OnThunder;

	/** Constructor */
	ALightningRodSub();

	/** Resizes the meshes and trigger to match the settings */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Fires a bolt from the main rod to this rod, then schedules the thunder */
	UFUNCTION(BlueprintCallable, Category="Lightning Rod")
	void Strike();

	/** Thunder delay for this rod, in seconds */
	UFUNCTION(BlueprintPure, Category="Lightning Rod")
	float GetThunderDelay() const;

	/** World position bolts arrive at */
	UFUNCTION(BlueprintPure, Category="Lightning Rod")
	FVector GetStrikeTarget() const;

protected:

	virtual void BeginPlay() override;

	/** Picks the nearest main rod if none is set */
	void FindMainRod();

	/** Stretches the bolt between the two rod tips */
	void AlignBolt();

	/** Toggles the bolt and flash for the flicker sequence */
	void FlickerTick();

	/** Plays the thunder */
	void Thunder(float Delay);

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
