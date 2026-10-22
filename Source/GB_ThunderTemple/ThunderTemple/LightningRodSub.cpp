// Copyright Epic Games, Inc. All Rights Reserved.


#include "LightningRodSub.h"
#include "LightningRodMain.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ALightningRodSub::ALightningRodSub()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	Pole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole"));
	Pole->SetupAttachment(Root);
	Pole->SetStaticMesh(CylinderMesh.Object);

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Root);
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ALightningRodSub::OnTriggerOverlap);

	// the bolt is purely visual
	Bolt = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bolt"));
	Bolt->SetupAttachment(Root);
	Bolt->SetStaticMesh(CylinderMesh.Object);
	Bolt->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bolt->SetCastShadow(false);
	Bolt->SetUsingAbsoluteLocation(true);
	Bolt->SetUsingAbsoluteRotation(true);
	Bolt->SetUsingAbsoluteScale(true);
	Bolt->SetHiddenInGame(true);

	Flash = CreateDefaultSubobject<UPointLightComponent>(TEXT("Flash"));
	Flash->SetupAttachment(Root);
	Flash->SetIntensity(0.0f);
	Flash->SetAttenuationRadius(5000.0f);
	Flash->SetLightColor(FLinearColor(0.75f, 0.85f, 1.0f));
	Flash->SetCastShadows(false);
}

void ALightningRodSub::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// the engine cylinder is 100 cm tall and 100 cm wide, centered on its pivot
	Pole->SetRelativeLocation(FVector(0.0f, 0.0f, PoleHeight * 0.5f));
	Pole->SetRelativeScale3D(FVector(PoleRadius / 50.0f, PoleRadius / 50.0f, PoleHeight / 100.0f));

	Trigger->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	Trigger->SetSphereRadius(TriggerRadius);

	Flash->SetRelativeLocation(FVector(0.0f, 0.0f, PoleHeight + 100.0f));

	if (BoltMaterial)
	{
		Bolt->SetMaterial(0, BoltMaterial);
	}

	AlignBolt();
}

void ALightningRodSub::BeginPlay()
{
	Super::BeginPlay();

	FindMainRod();
	AlignBolt();
}

void ALightningRodSub::FindMainRod()
{
	if (MainRod)
	{
		return;
	}

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

void ALightningRodSub::AlignBolt()
{
	if (!MainRod)
	{
		return;
	}

	// stretch the cylinder between the two tips
	const FVector From = MainRod->GetStrikeOrigin();
	const FVector To = GetStrikeTarget();
	const FVector Dir = To - From;

	Bolt->SetWorldLocation((From + To) * 0.5f);
	Bolt->SetWorldRotation(FRotationMatrix::MakeFromZ(Dir.GetSafeNormal()).Rotator());
	Bolt->SetWorldScale3D(FVector(BoltRadius / 50.0f, BoltRadius / 50.0f, Dir.Size() / 100.0f));
}

FVector ALightningRodSub::GetStrikeTarget() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, PoleHeight);
}

float ALightningRodSub::GetThunderDelay() const
{
	return MainRod ? MainRod->GetThunderDelayTo(GetStrikeTarget()) : 0.0f;
}

void ALightningRodSub::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// only the player triggers the rod
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	if (GetWorld()->GetTimeSeconds() - LastStrikeTime < Cooldown)
	{
		return;
	}

	Strike();
}

void ALightningRodSub::Strike()
{
	FindMainRod();
	if (!MainRod)
	{
		return;
	}

	LastStrikeTime = GetWorld()->GetTimeSeconds();
	AlignBolt();

	// show the bolt with a short double flicker
	FlickerStep = 0;
	FlickerTick();

	// the thunder follows after the delay
	const float Delay = GetThunderDelay();
	FTimerDelegate ThunderDelegate = FTimerDelegate::CreateUObject(this, &ALightningRodSub::Thunder, Delay);
	GetWorldTimerManager().SetTimer(ThunderTimer, ThunderDelegate, FMath::Max(Delay, 0.01f), false);

	const float Length = FVector::Dist(MainRod->GetStrikeOrigin(), GetStrikeTarget()) / 100.0f;
	UE_LOG(LogTemp, Log, TEXT("LightningRod %s: lightning, bolt %.1f m, ground distance %.1f m, thunder in %.1f s"),
		*GetActorNameOrLabel(), Length, FVector::Dist2D(MainRod->GetActorLocation(), GetActorLocation()) / 100.0f, Delay);

	if (bShowTiming && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Delay + 2.0f, FColor::Cyan,
			FString::Printf(TEXT("%s: lightning, bolt %.1f m, thunder in %.1f s"), *GetActorNameOrLabel(), Length, Delay));
	}

	OnLightning.Broadcast(Delay);
}

void ALightningRodSub::FlickerTick()
{
	// on, off, on, off
	const bool bOn = (FlickerStep % 2) == 0;
	Bolt->SetHiddenInGame(!bOn);
	Flash->SetIntensity(bOn ? FlashIntensity : 0.0f);

	if (++FlickerStep < 4)
	{
		GetWorldTimerManager().SetTimer(FlickerTimer, this, &ALightningRodSub::FlickerTick, FlickerTime, false);
	}
}

void ALightningRodSub::Thunder(float Delay)
{
	if (ThunderSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ThunderSound, GetStrikeTarget());
	}

	UE_LOG(LogTemp, Log, TEXT("LightningRod %s: thunder (%.1f s after the lightning)"), *GetActorNameOrLabel(), Delay);

	if (bShowTiming && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			FString::Printf(TEXT("%s: thunder (%.1f s after the lightning)"), *GetActorNameOrLabel(), Delay));
	}

	OnThunder.Broadcast(Delay);
}
