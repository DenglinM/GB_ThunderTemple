// Copyright Epic Games, Inc. All Rights Reserved.


#include "BrassPlatePickup.h"
#include "GB_ThunderTempleCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ABrassPlatePickup::ABrassPlatePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupArea = CreateDefaultSubobject<USphereComponent>(TEXT("PickupArea"));
	RootComponent = PickupArea;
	PickupArea->SetSphereRadius(120.0f);
	PickupArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	PickupArea->OnComponentBeginOverlap.AddDynamic(this, &ABrassPlatePickup::OnPickupBegin);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	// an 80 x 80 x 15 cm plate floating slightly above the floor
	Plate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plate"));
	Plate->SetupAttachment(PickupArea);
	Plate->SetStaticMesh(CubeMesh.Object);
	Plate->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.15f));
	Plate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABrassPlatePickup::OnPickupBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGB_ThunderTempleCharacter* Player = Cast<AGB_ThunderTempleCharacter>(OtherActor);
	if (!Player || !Player->IsPlayerControlled())
	{
		return;
	}

	Player->AddBrassPlates(Count);
	Destroy();
}
