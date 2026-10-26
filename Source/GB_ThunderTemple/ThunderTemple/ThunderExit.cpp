// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderExit.h"
#include "Stalker.h"
#include "ThunderHUD.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AThunderExit::AThunderExit()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cube = CubeMesh.Object;

	auto MakePanel = [this, Cube](const TCHAR* Name)
	{
		UStaticMeshComponent* Panel = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Panel->SetupAttachment(Root);
		Panel->SetStaticMesh(Cube);
		return Panel;
	};

	BackWall = MakePanel(TEXT("BackWall"));
	LeftWall = MakePanel(TEXT("LeftWall"));
	RightWall = MakePanel(TEXT("RightWall"));
	Roof = MakePanel(TEXT("Roof"));
	Door = MakePanel(TEXT("Door"));

	Inside = CreateDefaultSubobject<UBoxComponent>(TEXT("Inside"));
	Inside->SetupAttachment(Root);
	Inside->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AThunderExit::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// the engine cube is 100 cm on each side, centered on its pivot
	const float D = BoothSize.X;
	const float W = BoothSize.Y;
	const float H = BoothSize.Z;
	const float T = WallThickness;

	BackWall->SetRelativeLocation(FVector(-(D + T) * 0.5f, 0.0f, H * 0.5f));
	BackWall->SetRelativeScale3D(FVector(T, W + 2.0f * T, H) / 100.0f);

	LeftWall->SetRelativeLocation(FVector(0.0f, -(W + T) * 0.5f, H * 0.5f));
	LeftWall->SetRelativeScale3D(FVector(D, T, H) / 100.0f);

	RightWall->SetRelativeLocation(FVector(0.0f, (W + T) * 0.5f, H * 0.5f));
	RightWall->SetRelativeScale3D(FVector(D, T, H) / 100.0f);

	Roof->SetRelativeLocation(FVector(0.0f, 0.0f, H + T * 0.5f));
	Roof->SetRelativeScale3D(FVector(D + 2.0f * T, W + 2.0f * T, T) / 100.0f);

	Door->SetRelativeLocation(FVector((D + T) * 0.5f, 0.0f, H * 0.5f));
	Door->SetRelativeScale3D(FVector(T, W + 2.0f * T, H) / 100.0f);

	// the player counts as inside once past the first meter behind the door
	Inside->SetRelativeLocation(FVector(-50.0f, 0.0f, H * 0.5f));
	Inside->SetBoxExtent(FVector(D * 0.5f - 50.0f, W * 0.5f - 20.0f, H * 0.5f));

	ApplyLook();
}

void AThunderExit::ApplyLook()
{
	if (UMaterialInterface* Material = bUnlocked ? UnlockedMaterial : LockedMaterial)
	{
		for (UStaticMeshComponent* Panel : { BackWall, LeftWall, RightWall, Roof, Door })
		{
			Panel->SetMaterial(0, Material);
		}
	}
}

void AThunderExit::Unlock()
{
	if (bUnlocked)
	{
		return;
	}

	bUnlocked = true;
	ApplyLook();
	UE_LOG(LogTemp, Log, TEXT("Exit %s: unlocked"), *GetActorNameOrLabel());
}

void AThunderExit::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	// polled rather than an overlap event, so a player standing still inside is also caught
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (bUnlocked && !bCleared && Player && Inside->IsOverlappingActor(Player))
	{
		Clear();
	}

	// open while unlocked, shut again once the player is in
	const float Target = (bUnlocked && !bCleared) ? 1.0f : 0.0f;
	OpenAlpha = FMath::FInterpConstantTo(OpenAlpha, Target, DeltaSeconds, 1.0f / MoveTime);

	const float Sink = (BoothSize.Z + 10.0f) * OpenAlpha;
	Door->SetRelativeLocation(FVector((BoothSize.X + WallThickness) * 0.5f, 0.0f, BoothSize.Z * 0.5f - Sink));
}

void AThunderExit::Clear()
{
	bCleared = true;
	UE_LOG(LogTemp, Log, TEXT("Exit %s: the player is out, game cleared"), *GetActorNameOrLabel());

	// nothing follows the player in
	for (TActorIterator<AStalker> It(GetWorld()); It; ++It)
	{
		It->Freeze();
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->DisableInput(PC);
		}

		// completion screen with Restart and Quit
		if (AThunderHUD* HUD = PC->GetHUD<AThunderHUD>())
		{
			HUD->ShowComplete();
		}
	}
}
