// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderTrigger.h"
#include "LightningRodSub.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

AThunderTrigger::AThunderTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Box->ShapeColor = FColor::Yellow;
	Box->OnComponentBeginOverlap.AddDynamic(this, &AThunderTrigger::OnBoxBegin);
}

void AThunderTrigger::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Box->SetBoxExtent(BoxExtent);
}

void AThunderTrigger::Fire()
{
	if (bOnce && bFired)
	{
		return;
	}
	bFired = true;

	UE_LOG(LogTemp, Log, TEXT("ThunderTrigger %s: fired"), *GetActorNameOrLabel());

	for (ALightningRodSub* Rod : Rods)
	{
		if (Rod && Rod->CanStrike())
		{
			Rod->Strike();
		}
	}
}

void AThunderTrigger::OnBoxBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		Fire();
	}
}
