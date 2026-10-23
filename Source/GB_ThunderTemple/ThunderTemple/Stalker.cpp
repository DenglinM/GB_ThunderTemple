// Copyright Epic Games, Inc. All Rights Reserved.


#include "Stalker.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	WorldPatrol.Reset();
	for (const FVector& Point : PatrolPoints)
	{
		WorldPatrol.Add(GetActorTransform().TransformPosition(Point));
	}

	if (WorldPatrol.Num() > 0)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AStalker::PatrolStep);
	}
}

void AStalker::PatrolStep()
{
	if (bActive && WorldPatrol.Num() > 0)
	{
		FVector ToTarget = WorldPatrol[PatrolIndex] - GetActorLocation();
		ToTarget.Z = 0.0f;

		if (ToTarget.Size() < AcceptRadius)
		{
			PatrolIndex = (PatrolIndex + 1) % WorldPatrol.Num();
		}
		else
		{
			AddMovementInput(ToTarget.GetSafeNormal());
		}
	}

	// keep stepping every frame, also while paused so the patrol picks up again on resume
	GetWorldTimerManager().SetTimerForNextTick(this, &AStalker::PatrolStep);
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
