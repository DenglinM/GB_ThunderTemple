// Copyright Epic Games, Inc. All Rights Reserved.


#include "LightningRodMain.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"

ALightningRodMain::ALightningRodMain()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	Pole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole"));
	Pole->SetupAttachment(Root);
	Pole->SetStaticMesh(CylinderMesh.Object);

	Cap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cap"));
	Cap->SetupAttachment(Root);
	Cap->SetStaticMesh(CylinderMesh.Object);
}

void ALightningRodMain::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// the engine cylinder is 100 cm tall and 100 cm wide, centered on its pivot
	Pole->SetRelativeLocation(FVector(0.0f, 0.0f, PoleHeight * 0.5f));
	Pole->SetRelativeScale3D(FVector(PoleRadius / 50.0f, PoleRadius / 50.0f, PoleHeight / 100.0f));

	Cap->SetRelativeLocation(FVector(0.0f, 0.0f, PoleHeight + PoleRadius));
	Cap->SetRelativeScale3D(FVector(PoleRadius / 25.0f, PoleRadius / 25.0f, PoleRadius / 50.0f));
}

void ALightningRodMain::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// only draw the rings while editing, never in game
	UWorld* World = GetWorld();
	if (!bDrawRings || !World || World->IsGameWorld())
	{
		return;
	}

	const FVector Center = GetActorLocation();
	for (int32 Ring = 1; Ring <= RingsToDraw; ++Ring)
	{
		DrawDebugCircle(World, Center, RingSpacing * Ring, 64, FColor::Cyan, false, -1.0f, 0, 8.0f, FVector::XAxisVector, FVector::YAxisVector, false);
	}
}

FVector ALightningRodMain::GetStrikeOrigin() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, PoleHeight + PoleRadius * 2.0f);
}

float ALightningRodMain::GetThunderDelayTo(const FVector& Target) const
{
	if (bLinearDelay)
	{
		// use the full bolt length
		return FVector::Dist(GetStrikeOrigin(), Target) / LinearDistancePerSecond;
	}

	// count the rings crossed on the ground plane, rounding up so the first ring is one step
	const float Radius = FVector::Dist2D(GetActorLocation(), Target);
	return FMath::Max(1.0f, FMath::CeilToFloat(Radius / RingSpacing)) * SecondsPerRing;
}
