// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderDrum.h"
#include "Stalker.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AThunderDrum::AThunderDrum()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	// a 1.2 m wide, 0.9 m tall drum standing on the floor
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = Body;
	Body->SetStaticMesh(CylinderMesh.Object);
	Body->SetRelativeScale3D(FVector(1.2f, 1.2f, 0.9f));
}

void AThunderDrum::Beat()
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastBeatTime < Cooldown)
	{
		return;
	}
	LastBeatTime = Now;

	if (BeatSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BeatSound, GetActorLocation());
	}

	// every stalker in earshot on this floor comes to look; floors are several meters apart
	const float MaxHeightDifference = 500.0f;
	int32 Heard = 0;
	for (TActorIterator<AStalker> It(GetWorld()); It; ++It)
	{
		const FVector StalkerLocation = It->GetActorLocation();
		if (FVector::Dist2D(StalkerLocation, GetActorLocation()) < HearingRadius && FMath::Abs(StalkerLocation.Z - GetActorLocation().Z) < MaxHeightDifference)
		{
			It->HearNoise(GetActorLocation());
			++Heard;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Drum %s: beaten, %d stalker(s) heard it"), *GetActorNameOrLabel(), Heard);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, FString::Printf(TEXT("Drum: %d stalker(s) heard it"), Heard));
	}
}
