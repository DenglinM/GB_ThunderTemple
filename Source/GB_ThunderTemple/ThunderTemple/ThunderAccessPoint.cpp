// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderAccessPoint.h"
#include "ThunderGate.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AThunderAccessPoint::AThunderAccessPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	// a 1.2 m square, 0.3 m tall plate on the floor
	Plate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plate"));
	RootComponent = Plate;
	Plate->SetStaticMesh(CubeMesh.Object);
	Plate->SetRelativeScale3D(FVector(1.2f, 1.2f, 0.3f));

	// the marker is visual only, so stalkers and the player walk over it
	RangeMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RangeMarker"));
	RangeMarker->SetupAttachment(Plate);
	RangeMarker->SetStaticMesh(CylinderMesh.Object);
	RangeMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RangeMarker->SetCastShadow(false);
	RangeMarker->SetUsingAbsoluteScale(true);
}

void AThunderAccessPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// a 2 cm disc just above the floor; the plate is 30 cm tall and centered on its pivot
	RangeMarker->SetRelativeLocation(FVector(0.0f, 0.0f, -48.0f));
	RangeMarker->SetWorldScale3D(FVector(Radius / 50.0f, Radius / 50.0f, 0.02f));
}

void AThunderAccessPoint::NotifyLightning(const FVector& StrikeLocation)
{
	if (!Gate || FVector::Dist2D(StrikeLocation, GetActorLocation()) > Radius)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AccessPoint %s: lightning in range, opening %s for %.1f s"), *GetActorNameOrLabel(), *Gate->GetActorNameOrLabel(), OpenTime);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, OpenTime, FColor::Cyan, TEXT("Access point charged: gate open"));
	}

	Gate->OpenFor(OpenTime);
}
