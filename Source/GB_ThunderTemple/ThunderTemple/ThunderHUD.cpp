// Copyright Epic Games, Inc. All Rights Reserved.


#include "ThunderHUD.h"
#include "GB_ThunderTempleCharacter.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace ThunderHUD
{
	const FLinearColor Brass(1.0f, 0.62f, 0.12f);
	const FLinearColor Battery(1.0f, 0.86f, 0.15f);
	const FLinearColor TitleColor(0.75f, 0.88f, 1.0f);
	const FLinearColor Panel(0.0f, 0.0f, 0.0f, 0.45f);

	const FSlateBrush* White() { return FCoreStyle::Get().GetBrush("GenericWhiteBox"); }

	/** A solid colored rectangle */
	TSharedRef<SWidget> Block(const FLinearColor& Color, float Width, float Height)
	{
		return SNew(SBox).WidthOverride(Width).HeightOverride(Height)
			[
				SNew(SBorder).BorderImage(White()).BorderBackgroundColor(Color)
			];
	}

	/** Marker label */
	TSharedRef<STextBlock> Label(const FString& Text, int32 Size)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Text))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", Size))
			.ColorAndOpacity(FLinearColor::White)
			.ShadowOffset(FVector2D(1.0f, 1.0f));
	}

	/** Menu button */
	TSharedRef<SWidget> Button(const FString& Text, FOnClicked OnClicked)
	{
		return SNew(SBox).WidthOverride(260.0f).HeightOverride(64.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(OnClicked)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Text))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.ColorAndOpacity(FLinearColor::Black)
				]
			];
	}
}

AThunderHUD::AThunderHUD()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AThunderHUD::BeginPlay()
{
	Super::BeginPlay();

	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	using namespace ThunderHUD;

	// brass plate: a flat brass square and a count
	TSharedRef<STextBlock> Count = Label(TEXT("x1"), 20);
	PlateCount = Count;
	PlateMarker = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			Block(Brass, 34.0f, 34.0f)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight() [ Label(TEXT("BRASS PLATE"), 12) ]
			+ SVerticalBox::Slot().AutoHeight() [ Count ]
		];

	// battery: a cell with a cap on top
	BatteryMarker = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) [ Block(Battery, 10.0f, 5.0f) ]
			+ SVerticalBox::Slot().AutoHeight() [ Block(Battery, 22.0f, 36.0f) ]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			Label(TEXT("BATTERY"), 14)
		];

	Title = SNew(STextBlock)
		.Text(LevelTitle)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 64))
		.ColorAndOpacity(TitleColor)
		.ShadowOffset(FVector2D(2.0f, 2.0f));

	CompleteScreen = SNew(SBorder)
		.BorderImage(White())
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("GAME COMPLETE")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 56))
				.ColorAndOpacity(TitleColor)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 0.0f, 0.0f, 40.0f)
			[
				Label(TEXT("You escaped the Thunder Temple"), 20)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 0.0f, 0.0f, 16.0f)
			[
				Button(TEXT("Restart"), FOnClicked::CreateLambda([this]() { Restart(); return FReply::Handled(); }))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				Button(TEXT("Quit"), FOnClicked::CreateLambda([this]() { Quit(); return FReply::Handled(); }))
			]
		];

	Root = SNew(SOverlay)
		// level title near the top
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.0f, 140.0f, 0.0f, 0.0f)
		[
			Title.ToSharedRef()
		]
		// inventory in the top-right corner; the battery takes the plate's place once the plate is used
		+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.0f, 32.0f, 32.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBorder).BorderImage(White()).BorderBackgroundColor(Panel).Padding(10.0f)
				.Visibility_Lambda([this]() { return PlateMarker->GetVisibility(); })
				[
					PlateMarker.ToSharedRef()
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(12.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBorder).BorderImage(White()).BorderBackgroundColor(Panel).Padding(10.0f)
				.Visibility_Lambda([this]() { return BatteryMarker->GetVisibility(); })
				[
					BatteryMarker.ToSharedRef()
				]
			]
		]
		+ SOverlay::Slot()
		[
			CompleteScreen.ToSharedRef()
		];

	PlateMarker->SetVisibility(EVisibility::Collapsed);
	BatteryMarker->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Root), 10);
}

void AThunderHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Root.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(Root.ToSharedRef());
	}
	Root.Reset();

	Super::EndPlay(EndPlayReason);
}

void AThunderHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Root.IsValid())
	{
		return;
	}

	// hold the title, then fade it out
	Age += DeltaSeconds;
	const float TitleAlpha = 1.0f - FMath::Clamp((Age - TitleHoldTime) / TitleFadeTime, 0.0f, 1.0f);
	Title->SetRenderOpacity(TitleAlpha);
	Title->SetVisibility(TitleAlpha > 0.0f ? EVisibility::HitTestInvisible : EVisibility::Collapsed);

	const AGB_ThunderTempleCharacter* Player = Cast<AGB_ThunderTempleCharacter>(GetOwningPawn());
	const int32 Plates = Player ? Player->GetBrassPlates() : 0;
	PlateMarker->SetVisibility(Plates > 0 ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
	PlateCount->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Plates)));
	BatteryMarker->SetVisibility(Player && Player->HasBattery() ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
}

void AThunderHUD::ShowComplete()
{
	if (!CompleteScreen.IsValid())
	{
		return;
	}

	CompleteScreen->SetVisibility(EVisibility::Visible);

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(CompleteScreen);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
}

void AThunderHUD::Restart()
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void AThunderHUD::Quit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayerController(), EQuitPreference::Quit, false);
}
