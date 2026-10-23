// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderCheckpointSubsystem.h"
#include "ThunderCheckpoint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

void UThunderCheckpointSubsystem::SetStartIfUnset(const FTransform& Start)
{
	if (!bHasRespawn)
	{
		RespawnTransform = Start;
		bHasRespawn = true;
	}
}

bool UThunderCheckpointSubsystem::Activate(AThunderCheckpoint* Checkpoint, const FTransform& Respawn)
{
	if (ActiveCheckpoint.Get() == Checkpoint)
	{
		return false;
	}

	ActiveCheckpoint = Checkpoint;
	RespawnTransform = Respawn;
	bHasRespawn = true;
	return true;
}

void UThunderCheckpointSubsystem::Respawn(ACharacter* Character)
{
	if (!Character || !bHasRespawn)
	{
		return;
	}

	Character->GetCharacterMovement()->StopMovementImmediately();
	Character->TeleportTo(RespawnTransform.GetLocation(), RespawnTransform.Rotator(), false, true);

	if (AController* Controller = Character->GetController())
	{
		Controller->SetControlRotation(RespawnTransform.Rotator());
	}

	UE_LOG(LogTemp, Log, TEXT("Checkpoint: respawned %s at %s"), *Character->GetActorNameOrLabel(), *RespawnTransform.GetLocation().ToString());
}
