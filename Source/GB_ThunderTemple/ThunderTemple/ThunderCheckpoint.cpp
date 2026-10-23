// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderCheckpoint.h"
#include "ThunderCheckpointSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AThunderCheckpoint::AThunderCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	Pad = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pad"));
	Pad->SetupAttachment(Root);
	Pad->SetStaticMesh(CubeMesh.Object);

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Root);
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trigger->ShapeColor = FColor::Green;
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AThunderCheckpoint::OnTriggerBegin);
}

void AThunderCheckpoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// the engine cube is 100 cm on each side, centered on its pivot
	Pad->SetRelativeLocation(FVector(0.0f, 0.0f, PadSize.Z * 0.5f));
	Pad->SetRelativeScale3D(PadSize / 100.0f);

	Trigger->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	Trigger->SetBoxExtent(FVector(PadSize.X * 0.5f, PadSize.Y * 0.5f, 100.0f));
}

void AThunderCheckpoint::OnTriggerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	// respawn standing on the pad, facing the way the checkpoint faces
	const FTransform Respawn(GetActorRotation(), GetActorLocation() + FVector(0.0f, 0.0f, 100.0f));
	UThunderCheckpointSubsystem* Checkpoints = GetWorld()->GetSubsystem<UThunderCheckpointSubsystem>();
	if (Checkpoints && Checkpoints->Activate(this, Respawn))
	{
		UE_LOG(LogTemp, Log, TEXT("Checkpoint %s: saved"), *GetActorNameOrLabel());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Checkpoint saved"));
		}
	}
}
