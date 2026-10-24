// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderFinale.h"
#include "LightningRodMain.h"
#include "Stalker.h"
#include "ThunderGate.h"
#include "AudioDevice.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AThunderFinale::AThunderFinale()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	// the wave is purely visual; reaching things is checked by distance
	Wave = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wave"));
	Wave->SetupAttachment(Root);
	Wave->SetStaticMesh(SphereMesh.Object);
	Wave->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Wave->SetCastShadow(false);
	Wave->SetHiddenInGame(true);

	Flash = CreateDefaultSubobject<UPointLightComponent>(TEXT("Flash"));
	Flash->SetupAttachment(Root);
	Flash->SetRelativeLocation(FVector(0.0f, 0.0f, 800.0f));
	Flash->SetIntensity(0.0f);
	Flash->SetAttenuationRadius(12000.0f);
	Flash->SetLightColor(FLinearColor(0.75f, 0.85f, 1.0f));
	Flash->SetCastShadows(false);
}

void AThunderFinale::BeginPlay()
{
	Super::BeginPlay();

	if (WaveMaterial)
	{
		Wave->SetMaterial(0, WaveMaterial);
	}

	// sit on the nearest main rod so the wave comes from it
	if (!MainRod)
	{
		float BestDist = TNumericLimits<float>::Max();
		for (TActorIterator<ALightningRodMain> It(GetWorld()); It; ++It)
		{
			const float Dist = FVector::DistSquared(It->GetActorLocation(), GetActorLocation());
			if (Dist < BestDist)
			{
				BestDist = Dist;
				MainRod = *It;
			}
		}
	}
	if (MainRod)
	{
		SetActorLocation(MainRod->GetActorLocation());
	}
}

void AThunderFinale::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// the audio device outlives the play session in the editor, so never leave it muted
	if (bStarted && !bPlayerReached)
	{
		SetHearing(true);
	}

	Super::EndPlay(EndPlayReason);
}

void AThunderFinale::StartFinale()
{
	if (bStarted)
	{
		return;
	}
	bStarted = true;

	UE_LOG(LogTemp, Log, TEXT("Finale: battery pulled, the main rod discharges"));

	// the discharge freezes every stalker until the wave reaches it
	for (TActorIterator<AStalker> It(GetWorld()); It; ++It)
	{
		It->Freeze();
		FrozenStalkers.Add(*It);
	}

	// every gate opens and stays open
	for (TActorIterator<AThunderGate> It(GetWorld()); It; ++It)
	{
		It->OpenFor(1.0e6f);
	}

	Flash->SetIntensity(FlashIntensity);
	GetWorldTimerManager().SetTimer(FlashTimer, this, &AThunderFinale::EndFlash, 0.25f, false);

	// then silence
	SetHearing(false);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, SilenceTime + 3.0f, FColor::Cyan, TEXT("The rod discharges... silence. Run before the thunder catches you."));
	}

	GetWorldTimerManager().SetTimer(WaveTimer, this, &AThunderFinale::ReleaseWave, FMath::Max(SilenceTime, 0.01f), false);
}

void AThunderFinale::EndFlash()
{
	Flash->SetIntensity(0.0f);
}

void AThunderFinale::ReleaseWave()
{
	UE_LOG(LogTemp, Log, TEXT("Finale: the thunder wave sets off at %.0f cm/s"), WaveSpeed);

	Radius = 0.0f;
	bWaveMoving = true;
	Wave->SetHiddenInGame(false);
	Wave->SetWorldScale3D(FVector(0.01f));
	SetActorTickEnabled(true);
}

void AThunderFinale::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bWaveMoving)
	{
		return;
	}

	Radius += WaveSpeed * DeltaSeconds;

	// the engine sphere is 100 cm wide
	Wave->SetWorldScale3D(FVector(Radius / 50.0f));

	const FVector Center = GetActorLocation();

	// stalkers the thunder reaches wake up and hunt
	for (int32 Index = FrozenStalkers.Num() - 1; Index >= 0; --Index)
	{
		AStalker* Stalker = FrozenStalkers[Index].Get();
		if (!Stalker)
		{
			FrozenStalkers.RemoveAt(Index);
		}
		else if (FVector::Dist(Stalker->GetActorLocation(), Center) < Radius)
		{
			Stalker->Awaken();
			FrozenStalkers.RemoveAt(Index);
		}
	}

	// the player hears again once the thunder catches them
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!bPlayerReached && Player && FVector::Dist(Player->GetActorLocation(), Center) < Radius)
	{
		bPlayerReached = true;
		SetHearing(true);
		UE_LOG(LogTemp, Log, TEXT("Finale: the thunder wave reached the player at %.0f m"), Radius / 100.0f);

		if (WaveThunder.Sound)
		{
			UGameplayStatics::PlaySound2D(this, WaveThunder.Sound, 1.0f, 1.0f, WaveThunder.StartTime);
		}
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("The thunder caught you: you can hear again, and so can they"));
		}
	}

	if (Radius >= MaxRadius)
	{
		UE_LOG(LogTemp, Log, TEXT("Finale: the thunder wave faded out"));
		bWaveMoving = false;
		Wave->SetHiddenInGame(true);
		SetActorTickEnabled(false);
	}
}

void AThunderFinale::SetHearing(bool bHear)
{
	if (FAudioDevice* AudioDevice = GetWorld() ? GetWorld()->GetAudioDeviceRaw() : nullptr)
	{
		AudioDevice->SetTransientPrimaryVolume(bHear ? 1.0f : 0.0f);
	}
}
