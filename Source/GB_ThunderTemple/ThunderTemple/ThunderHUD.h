// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Widgets/SWidget.h"
#include "ThunderHUD.generated.h"

class SWidget;
class SBox;
class STextBlock;

/**
 *  Gray-box game UI, built in Slate so it needs no widget assets:
 *  the level title at the start, brass plate and battery markers in the top-right corner,
 *  and a completion screen with Restart and Quit buttons.
 */
UCLASS()
class AThunderHUD : public AHUD
{
	GENERATED_BODY()

protected:

	/** Title shown when the game starts */
	UPROPERTY(EditAnywhere, Category="UI")
	FText LevelTitle = FText::FromString(TEXT("THUNDER TEMPLE"));

	/** How long the title stays fully visible */
	UPROPERTY(EditAnywhere, Category="UI", meta = (ClampMin = 0, Units = "s"))
	float TitleHoldTime = 3.0f;

	/** How long the title takes to fade out */
	UPROPERTY(EditAnywhere, Category="UI", meta = (ClampMin = 0.01, Units = "s"))
	float TitleFadeTime = 1.5f;

	/** Root of all the UI */
	TSharedPtr<SWidget> Root;

	/** Level title */
	TSharedPtr<SWidget> Title;

	/** Brass plate marker */
	TSharedPtr<SWidget> PlateMarker;

	/** Brass plate count next to the marker */
	TSharedPtr<STextBlock> PlateCount;

	/** Battery marker */
	TSharedPtr<SWidget> BatteryMarker;

	/** Completion screen */
	TSharedPtr<SWidget> CompleteScreen;

	/** Time since the game started */
	float Age = 0.0f;

public:

	AThunderHUD();

	/** Fades the title and refreshes the inventory markers */
	virtual void Tick(float DeltaSeconds) override;

	/** Shows the completion screen and hands the mouse to the player */
	UFUNCTION(BlueprintCallable, Category="UI")
	void ShowComplete();

protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Reloads the current level */
	void Restart();

	/** Quits the game */
	void Quit();
};
