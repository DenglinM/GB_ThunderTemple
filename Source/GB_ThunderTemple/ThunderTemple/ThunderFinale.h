// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightningRodSub.h"
#include "ThunderFinale.generated.h"

class ALightningRodMain;
class AStalker;
class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInterface;

/**
 *  The escape at the end. Started when the battery is pulled from the main rod:
 *  the rod discharges and freezes every stalker, the world goes silent, every gate opens and the exit unlocks.
 *  Then the last thunder spreads out from the rod as a slow, visible wave. Stalkers it reaches wake up and hunt the player,
 *  and the player hears again once it catches up with them.
 */
UCLASS()
class AThunderFinale : public AActor
{
	GENERATED_BODY()

	/** Center of the wave, placed at the main rod */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Visible sound wave, a growing sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Wave;

	/** Flash of the discharge */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPointLightComponent* Flash;

protected:

	/** Main rod the wave starts from. If empty, the nearest main rod is used */
	UPROPERTY(EditAnywhere, Category="Finale")
	TObjectPtr<ALightningRodMain> MainRod;

	/** Silence between the discharge and the wave setting off */
	UPROPERTY(EditAnywhere, Category="Finale", meta = (ClampMin = 0, Units = "s"))
	float SilenceTime = 3.0f;

	/** Speed of the wave, in cm/s; the player walks at 500 */
	UPROPERTY(EditAnywhere, Category="Finale", meta = (ClampMin = 1, Units = "cm/s"))
	float WaveSpeed = 350.0f;

	/** The wave fades out once it reaches this radius, in cm */
	UPROPERTY(EditAnywhere, Category="Finale", meta = (ClampMin = 100, Units = "cm"))
	float MaxRadius = 12000.0f;

	/** Wave material, ideally translucent and two-sided so it reads from inside too */
	UPROPERTY(EditAnywhere, Category="Finale")
	TObjectPtr<UMaterialInterface> WaveMaterial;

	/** Thunder heard when the wave reaches the player */
	UPROPERTY(EditAnywhere, Category="Finale")
	FThunderSample WaveThunder;

	/** Light intensity of the discharge flash */
	UPROPERTY(EditAnywhere, Category="Finale", meta = (ClampMin = 0))
	float FlashIntensity = 2000000.0f;

	/** True once the battery has been pulled */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Finale")
	bool bStarted = false;

	/** True while the wave is spreading */
	bool bWaveMoving = false;

	/** Current wave radius, in cm */
	float Radius = 0.0f;

	/** True once the wave has reached the player */
	bool bPlayerReached = false;

	/** Stalkers frozen by the discharge that the wave has not reached yet */
	TArray<TWeakObjectPtr<AStalker>> FrozenStalkers;

	FTimerHandle WaveTimer;
	FTimerHandle FlashTimer;

public:

	/** Constructor */
	AThunderFinale();

	/** Grows the wave */
	virtual void Tick(float DeltaSeconds) override;

	/** Runs the discharge and sets the wave going */
	UFUNCTION(BlueprintCallable, Category="Finale")
	void StartFinale();

	/** True once the finale has started */
	UFUNCTION(BlueprintPure, Category="Finale")
	bool HasStarted() const { return bStarted; }

protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Starts spreading the wave */
	void ReleaseWave();

	/** Ends the discharge flash */
	void EndFlash();

	/** Mutes or restores everything the player hears */
	void SetHearing(bool bHear);
};
