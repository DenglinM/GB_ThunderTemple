// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GB_ThunderTempleCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AGB_ThunderTempleCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Crouch Input Action, held to stay crouched */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;

	/** Interact Input Action: take the battery, beat a drum or attach a brass plate to a stalker */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;

	/** Reach for interacting with drums and stalkers, in cm */
	UPROPERTY(EditAnywhere, Category="Input", meta = (ClampMin = 0, Units = "cm"))
	float InteractRange = 250.0f;

	/** Brass lightning plates carried; each can be thrown later to call one extra strike */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Inventory")
	int32 BrassPlates = 0;

	/** True once the battery has been taken from the main rod */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Inventory")
	bool bHasBattery = false;

public:

	/** Constructor */
	AGB_ThunderTempleCharacter();	

protected:

	/** Records the level start as the first respawn point */
	virtual void BeginPlay() override;

	/** Respawns at the latest checkpoint instead of being destroyed when falling out of the level */
	virtual void FellOutOfWorld(const class UDamageType& DamageType) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Starts crouching */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchStart();

	/** Stops crouching */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchEnd();

	/** Uses the nearest drum or stalker within reach */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoInteract();

	/** Adds brass plates to the inventory */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void AddBrassPlates(int32 Count = 1);

	/** Removes one brass plate; returns false if none are carried */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool UseBrassPlate();

	/** Number of brass plates carried */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetBrassPlates() const { return BrassPlates; }

	/** True once the battery has been taken */
	UFUNCTION(BlueprintPure, Category="Inventory")
	bool HasBattery() const { return bHasBattery; }

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

