// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderGate.h"
#include "LightningRodSub.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AThunderGate::AThunderGate()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(Root);
	Door->SetStaticMesh(CubeMesh.Object);

	WaitArea = CreateDefaultSubobject<UBoxComponent>(TEXT("WaitArea"));
	WaitArea->SetupAttachment(Root);
	WaitArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	WaitArea->OnComponentBeginOverlap.AddDynamic(this, &AThunderGate::OnWaitAreaBegin);
	WaitArea->OnComponentEndOverlap.AddDynamic(this, &AThunderGate::OnWaitAreaEnd);
}

void AThunderGate::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// the engine cube is 100 cm on each side, centered on its pivot
	Door->SetRelativeLocation(FVector(0.0f, 0.0f, DoorSize.Z * 0.5f));
	Door->SetRelativeScale3D(DoorSize / 100.0f);

	// the wait area sits on the -X side of the door, which faces the approaching player
	WaitArea->SetRelativeLocation(FVector(-WaitAreaDepth * 0.5f, 0.0f, DoorSize.Z * 0.5f));
	WaitArea->SetBoxExtent(FVector(WaitAreaDepth * 0.5f, DoorSize.Y * 0.5f, DoorSize.Z * 0.5f));
}

void AThunderGate::BeginPlay()
{
	Super::BeginPlay();

	// with no rods listed, listen to every sub rod in the level
	if (Rods.IsEmpty() && (bOpenOnThunder || bCallLightningWhenWaiting))
	{
		for (TActorIterator<ALightningRodSub> It(GetWorld()); It; ++It)
		{
			Rods.Add(*It);
		}
	}

	ALightningRodSub* NearestRod = nullptr;
	float BestDist = TNumericLimits<float>::Max();
	for (ALightningRodSub* Rod : Rods)
	{
		if (!Rod)
		{
			continue;
		}

		Rod->OnThunder.AddDynamic(this, &AThunderGate::OnRodThunder);

		const float Dist = FVector::DistSquared(Rod->GetActorLocation(), GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			NearestRod = Rod;
		}
	}

	// default the wait rod to the nearest one
	if (!WaitRod)
	{
		WaitRod = NearestRod;
	}
}

void AThunderGate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	// poll the wait area too: a pawn spawned inside it overlaps before it is possessed, so the overlap event alone can miss it
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const bool bPlayerInArea = PlayerPawn && WaitArea->IsOverlappingActor(PlayerPawn);
	if (bPlayerInArea != bPlayerWaiting)
	{
		bPlayerWaiting = bPlayerInArea;
		if (bPlayerWaiting)
		{
			TryCallLightning();
		}
	}

	// slide the door towards its target
	const float PrevAlpha = OpenAlpha;
	const float Target = IsOpen() ? 1.0f : 0.0f;
	OpenAlpha = FMath::FInterpConstantTo(OpenAlpha, Target, DeltaSeconds, 1.0f / MoveTime);

	const float Sink = (DoorSize.Z + 10.0f) * OpenAlpha;
	Door->SetRelativeLocation(FVector(0.0f, 0.0f, DoorSize.Z * 0.5f - Sink));

	// once fully closed again, ask for another strike if the player is still waiting
	if (PrevAlpha > 0.0f && OpenAlpha <= 0.0f && bPlayerWaiting)
	{
		GetWorldTimerManager().SetTimer(RetryTimer, this, &AThunderGate::TryCallLightning, FMath::Max(RetryDelay, 0.01f), false);
	}
}

void AThunderGate::OpenFor(float Seconds)
{
	CloseTime = FMath::Max(CloseTime, GetWorld()->GetTimeSeconds() + Seconds);
	UE_LOG(LogTemp, Log, TEXT("ThunderGate %s: open for %.1f s"), *GetActorNameOrLabel(), Seconds);
}

bool AThunderGate::IsOpen() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < CloseTime;
}

void AThunderGate::OnRodThunder(float ThunderDelay, float ThunderDuration)
{
	if (bOpenOnThunder)
	{
		OpenFor(ThunderDuration);
	}
}

void AThunderGate::TryCallLightning()
{
	if (!bCallLightningWhenWaiting || !bPlayerWaiting || IsOpen() || !WaitRod || !WaitRod->CanStrike())
	{
		return;
	}

	WaitRod->Strike();
}

void AThunderGate::OnWaitAreaBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		bPlayerWaiting = true;
		TryCallLightning();
	}
}

void AThunderGate::OnWaitAreaEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		bPlayerWaiting = false;
	}
}
