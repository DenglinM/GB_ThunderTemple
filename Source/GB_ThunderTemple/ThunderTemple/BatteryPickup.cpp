// Copyright Epic Games, Inc. All Rights Reserved.


#include "BatteryPickup.h"
#include "ThunderFinale.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ABatteryPickup::ABatteryPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	// a 0.6 m wide, 1 m tall cell standing on the floor
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = Body;
	Body->SetStaticMesh(CylinderMesh.Object);
	Body->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.0f));
}

void ABatteryPickup::Take()
{
	if (!Finale)
	{
		for (TActorIterator<AThunderFinale> It(GetWorld()); It; ++It)
		{
			Finale = *It;
			break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Battery %s: taken"), *GetActorNameOrLabel());

	if (Finale)
	{
		Finale->StartFinale();
	}
	Destroy();
}
