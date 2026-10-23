// Copyright Epic Games, Inc. All Rights Reserved.


#include "Stalker.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ThunderCheckpointSubsystem.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AStalker::AStalker()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(45.0f, 125.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	// a 0.9 m wide, 2.5 m tall column filling the capsule
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(GetCapsuleComponent());
	Body->SetStaticMesh(CylinderMesh.Object);
	Body->SetRelativeScale3D(FVector(0.9f, 0.9f, 2.5f));
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// face the walking direction
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 270.0f, 0.0f);
}

void AStalker::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	// never chase or patrol off a platform edge
	GetCharacterMovement()->bCanWalkOffLedges = false;
	HomeLocation = GetActorLocation();

	WorldPatrol.Reset();
	for (const FVector& Point : PatrolPoints)
	{
		WorldPatrol.Add(GetActorTransform().TransformPosition(Point));
	}

	if (WorldPatrol.Num() > 0 || bChasePlayer)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AStalker::PatrolStep);
	}
}

void AStalker::PatrolStep()
{
	// keep stepping every frame, also while paused so the stalker picks up again on resume
	GetWorldTimerManager().SetTimerForNextTick(this, &AStalker::PatrolStep);

	if (!bActive)
	{
		return;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);

	// hunt the player on sight
	if (bChasePlayer && Player && CanSeePlayer(Player))
	{
		if (!bChasing)
		{
			UE_LOG(LogTemp, Log, TEXT("Stalker %s: spotted the player"), *GetActorNameOrLabel());
		}
		bChasing = true;
		LastSeenLocation = Player->GetActorLocation();
		GetCharacterMovement()->MaxWalkSpeed = Player->GetCharacterMovement()->MaxWalkSpeed * ChaseSpeedMultiplier;

		if (WalkTowards(LastSeenLocation) < CatchRadius + GetCapsuleComponent()->GetScaledCapsuleRadius())
		{
			UE_LOG(LogTemp, Log, TEXT("Stalker %s: caught the player"), *GetActorNameOrLabel());
			if (UThunderCheckpointSubsystem* Checkpoints = GetWorld()->GetSubsystem<UThunderCheckpointSubsystem>())
			{
				Checkpoints->Respawn(Player);
			}
			bChasing = false;
		}
		return;
	}

	// lost sight: go and check where the player was last seen
	if (bChasing)
	{
		if (WalkTowards(LastSeenLocation) < AcceptRadius)
		{
			UE_LOG(LogTemp, Log, TEXT("Stalker %s: lost the player"), *GetActorNameOrLabel());
			bChasing = false;
		}
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	if (WorldPatrol.Num() > 0)
	{
		if (WalkTowards(WorldPatrol[PatrolIndex]) < AcceptRadius)
		{
			PatrolIndex = (PatrolIndex + 1) % WorldPatrol.Num();
		}
	}
	else
	{
		// guards go back to their post
		WalkTowards(HomeLocation);
	}
}

float AStalker::WalkTowards(const FVector& Target)
{
	FVector ToTarget = Target - GetActorLocation();
	ToTarget.Z = 0.0f;

	const float Distance = ToTarget.Size();
	if (Distance > AcceptRadius * 0.5f)
	{
		AddMovementInput(ToTarget.GetSafeNormal());
	}
	return Distance;
}

bool AStalker::CanSeePlayer(const ACharacter* Player) const
{
	if (ActivationArea && !ActivationArea->IsOverlappingActor(Player))
	{
		return false;
	}

	// crouching shrinks the range at which the player is noticed
	if (DetectionRadius > 0.0f)
	{
		const float Range = DetectionRadius * (Player->bIsCrouched ? CrouchDetectionScale : 1.0f);
		if (FVector::Dist(Player->GetActorLocation(), GetActorLocation()) > Range)
		{
			return false;
		}
	}

	// line of sight from the top of the stalker to the player's head; cover and walls block it
	const FVector Eye = GetActorLocation() + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 10.0f);
	const FVector Head = Player->GetPawnViewLocation();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(StalkerSight), false, this);
	Params.AddIgnoredActor(Player);
	return !GetWorld()->LineTraceTestByChannel(Eye, Head, ECC_Visibility, Params);
}

void AStalker::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (UMaterialInterface* Material = bActive ? ActiveMaterial : PausedMaterial)
	{
		Body->SetMaterial(0, Material);
	}
}

void AStalker::HitByLightning()
{
	if (!bActive)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Stalker %s: hit by lightning, paused %s"), *GetActorNameOrLabel(),
		PauseDuration > 0.0f ? *FString::Printf(TEXT("for %.1f s"), PauseDuration) : TEXT("until the thunder"));

	SetActive(false);

	if (PauseDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(ResumeTimer, this, &AStalker::Resume, PauseDuration, false);
	}
}

void AStalker::Resume()
{
	GetWorldTimerManager().ClearTimer(ResumeTimer);
	if (!bActive)
	{
		UE_LOG(LogTemp, Log, TEXT("Stalker %s: resumed"), *GetActorNameOrLabel());
	}
	SetActive(true);
}

void AStalker::SetActive(bool bNewActive)
{
	if (bActive == bNewActive)
	{
		return;
	}

	bActive = bNewActive;

	// stop or restart movement and any AI logic
	if (bActive)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}

	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AI->GetBrainComponent())
		{
			if (bActive)
			{
				Brain->ResumeLogic(TEXT("Lightning"));
			}
			else
			{
				Brain->PauseLogic(TEXT("Lightning"));
			}
		}
		if (!bActive)
		{
			AI->StopMovement();
		}
	}

	if (UMaterialInterface* Material = bActive ? ActiveMaterial : PausedMaterial)
	{
		Body->SetMaterial(0, Material);
	}

	OnStateChanged.Broadcast(bActive);
}
