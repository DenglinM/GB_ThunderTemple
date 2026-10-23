// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ThunderCheckpointSubsystem.generated.h"

class AThunderCheckpoint;
class ACharacter;

/**
 *  Remembers where the player respawns: the level start until a checkpoint is reached, then the latest checkpoint.
 */
UCLASS()
class UThunderCheckpointSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	/** Where the player comes back */
	FTransform RespawnTransform;

	/** True once a respawn point has been recorded */
	bool bHasRespawn = false;

	/** Latest checkpoint reached */
	TWeakObjectPtr<AThunderCheckpoint> ActiveCheckpoint;

public:

	/** Records the level start, unless a respawn point is already known */
	void SetStartIfUnset(const FTransform& Start);

	/** Makes the given checkpoint the respawn point; returns false if it already was */
	bool Activate(AThunderCheckpoint* Checkpoint, const FTransform& Respawn);

	/** Moves the character back to the respawn point */
	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	void Respawn(ACharacter* Character);

	/** Latest checkpoint reached, if any */
	UFUNCTION(BlueprintPure, Category="Checkpoint")
	AThunderCheckpoint* GetActiveCheckpoint() const { return ActiveCheckpoint.Get(); }
};
