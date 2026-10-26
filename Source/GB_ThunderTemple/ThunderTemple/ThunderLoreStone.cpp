// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderLoreStone.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AThunderLoreStone::AThunderLoreStone()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMaterial(TEXT("/Game/ThunderTemple/Materials/MI_GB_Yellow_Glow.MI_GB_Yellow_Glow"));

	// a squat, tilted pebble shape about 1 m across, so every copy already glows yellow when placed
	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Stone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stone"));
	Stone->SetupAttachment(Root);
	Stone->SetRelativeLocation(FVector(0.0f, 0.0f, 25.0f));
	Stone->SetStaticMesh(SphereMesh.Object);
	Stone->SetRelativeScale3D(FVector(1.1f, 0.8f, 0.6f));
	Stone->SetRelativeRotation(FRotator(8.0f, 0.0f, -6.0f));
	if (GlowMaterial.Succeeded())
	{
		Stone->SetMaterial(0, GlowMaterial.Object);
	}

	Title = FText::FromString(TEXT("\u77F3\u7891"));
	Message = FText::FromString(TEXT("\u5728\u8FD9\u91CC\u8F93\u5165\u6587\u5B57\u3002"));
}
