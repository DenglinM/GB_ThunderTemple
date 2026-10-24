// Copyright Epic Games, Inc. All Rights Reserved.

#include "GB_ThunderTempleCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GB_ThunderTemple.h"
#include "ThunderCheckpointSubsystem.h"
#include "ThunderDrum.h"
#include "BatteryPickup.h"
#include "Stalker.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "InputAction.h"
#include "UObject/ConstructorHelpers.h"

AGB_ThunderTempleCharacter::AGB_ThunderTempleCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// crouching: held on Ctrl, slower and lower so cover hides the head
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 250.0f;

	static ConstructorHelpers::FObjectFinder<UInputAction> CrouchActionAsset(TEXT("/Game/Input/Actions/IA_Crouch.IA_Crouch"));
	CrouchAction = CrouchActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractActionAsset(TEXT("/Game/Input/Actions/IA_Interact.IA_Interact"));
	InteractAction = InteractActionAsset.Object;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AGB_ThunderTempleCharacter::BeginPlay()
{
	Super::BeginPlay();

	// the character blueprint may carry an older value
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	if (UThunderCheckpointSubsystem* Checkpoints = GetWorld()->GetSubsystem<UThunderCheckpointSubsystem>())
	{
		Checkpoints->SetStartIfUnset(GetActorTransform());
	}
}

void AGB_ThunderTempleCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	// players go back to their last checkpoint; anything else falls as usual
	UThunderCheckpointSubsystem* Checkpoints = GetWorld()->GetSubsystem<UThunderCheckpointSubsystem>();
	if (IsPlayerControlled() && Checkpoints)
	{
		Checkpoints->Respawn(this);
		return;
	}

	Super::FellOutOfWorld(DamageType);
}

void AGB_ThunderTempleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGB_ThunderTempleCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AGB_ThunderTempleCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGB_ThunderTempleCharacter::Look);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AGB_ThunderTempleCharacter::DoCrouchStart);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AGB_ThunderTempleCharacter::DoCrouchEnd);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AGB_ThunderTempleCharacter::DoInteract);
	}
	else
	{
		UE_LOG(LogGB_ThunderTemple, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AGB_ThunderTempleCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AGB_ThunderTempleCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AGB_ThunderTempleCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AGB_ThunderTempleCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AGB_ThunderTempleCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AGB_ThunderTempleCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AGB_ThunderTempleCharacter::DoCrouchStart()
{
	Crouch();
}

void AGB_ThunderTempleCharacter::DoCrouchEnd()
{
	UnCrouch();
}

void AGB_ThunderTempleCharacter::DoInteract()
{
	// the battery is the final goal, so it always wins
	for (TActorIterator<ABatteryPickup> It(GetWorld()); It; ++It)
	{
		if (FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) < InteractRange)
		{
			It->Take();
			return;
		}
	}

	// with a plate in hand, a plate-less stalker within reach comes first, since it usually stands at the drum
	AStalker* Stalker = nullptr;
	float Best = InteractRange;
	for (TActorIterator<AStalker> It(GetWorld()); It; ++It)
	{
		const float Dist = FVector::Dist2D(It->GetActorLocation(), GetActorLocation());
		if (!It->HasBrassPlate() && Dist < Best)
		{
			Best = Dist;
			Stalker = *It;
		}
	}

	if (Stalker && BrassPlates > 0 && Stalker->AttachBrassPlate())
	{
		--BrassPlates;
		return;
	}

	// otherwise beat the nearest drum within reach
	AThunderDrum* Drum = nullptr;
	Best = InteractRange;
	for (TActorIterator<AThunderDrum> It(GetWorld()); It; ++It)
	{
		const float Dist = FVector::Dist2D(It->GetActorLocation(), GetActorLocation());
		if (Dist < Best)
		{
			Best = Dist;
			Drum = *It;
		}
	}

	if (Drum)
	{
		Drum->Beat();
	}
	else if (Stalker && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No brass plate to attach"));
	}
}

void AGB_ThunderTempleCharacter::AddBrassPlates(int32 Count)
{
	BrassPlates += Count;
	UE_LOG(LogTemp, Log, TEXT("%s: picked up a brass plate, now carrying %d"), *GetActorNameOrLabel(), BrassPlates);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, FString::Printf(TEXT("Brass plate x%d"), BrassPlates));
	}
}

bool AGB_ThunderTempleCharacter::UseBrassPlate()
{
	if (BrassPlates <= 0)
	{
		return false;
	}

	--BrassPlates;
	return true;
}
