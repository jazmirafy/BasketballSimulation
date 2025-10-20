// Copyright Epic Games, Inc. All Rights Reserved.

#include "XRBasketballSimCharacter.h"
#include "XRBasketballSimProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "Pickup.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputSubsystems.h"


//////////////////////////////////////////////////////////////////////////
// AXRBasketballSimCharacter

AXRBasketballSimCharacter::AXRBasketballSimCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));


	HoldingComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HoldingComponent"));
	HoldingComponent->SetRelativeLocation(FVector(100.0f, 0.0f, -30.0f));
	HoldingComponent->SetupAttachment(FirstPersonCameraComponent);

	CurrentItem = NULL;
	bCanMove = true;
	bInspecting = false;

}

void AXRBasketballSimCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			Subsystem->AddMappingContext(GameplayMappingContext, 1);
		}
	}

	PitchMax = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax;
	PitchMin = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin;

}
void AXRBasketballSimCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	Start = FirstPersonCameraComponent->GetComponentLocation();
	ForwardVector = FirstPersonCameraComponent->GetForwardVector();
	End = ((ForwardVector * 300.0f) + Start);

	DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 1, 0, 1);

	
	if (!bHoldingItem) {
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, DefaultComponentQueryParams, DefaultResponseParams)) {
			if (Hit.GetActor()->GetClass()->IsChildOf(APickup::StaticClass())) {

				CurrentItem = Cast<APickup>(Hit.GetActor());

			}
		}
		else {
			CurrentItem = NULL;
		}

	}
	
	//controls camera zoom between inspecting and holding items
	if (bInspecting) {
		if (bHoldingItem) {
			FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 90.0f, 0.1f));
			HoldingComponent->SetRelativeLocation(FVector(0.0f, 50.0f, 50.0f));
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax = 179.90000002f;
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin = -179.90000002f;
			CurrentItem->RotateActor();
		}
		else {

			FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 45.0f, 0.1f));

		}
	}
	else {

		FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 90.0f, 0.1f));
		if (bHoldingItem) {
			HoldingComponent->SetRelativeLocation(FVector(100.0f, 0.0f, -30.0f));
		}
	}
}
//////////////////////////////////////////////////////////////////////////// Input

void AXRBasketballSimCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AXRBasketballSimCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AXRBasketballSimCharacter::Look);

		//Pickup/Shoot
		EnhancedInputComponent->BindAction(ActionAction, ETriggerEvent::Triggered, this, &AXRBasketballSimCharacter::Action);

		//Inspect
		EnhancedInputComponent->BindAction(InspectAction, ETriggerEvent::Triggered, this, &AXRBasketballSimCharacter::Inspect);
		EnhancedInputComponent->BindAction(InspectAction, ETriggerEvent::Completed, this, &AXRBasketballSimCharacter::StopInspecting);

	}
}


void AXRBasketballSimCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr && bCanMove)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AXRBasketballSimCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AXRBasketballSimCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AXRBasketballSimCharacter::GetHasRifle()
{
	return bHasRifle;
}
void AXRBasketballSimCharacter::Action() {
	if (CurrentItem && !bInspecting) {
		ToggleItemPickup();
	}
}
void AXRBasketballSimCharacter::Inspect() {
	if (bHoldingItem) {
		LastRotation = GetControlRotation();
		ToggleMovement();
	}
	else {
		bInspecting = true;
	}
}

void AXRBasketballSimCharacter::StopInspecting() {
	if (bInspecting && bHoldingItem) {
		GetController()->SetControlRotation(LastRotation);
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax = PitchMax;
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin = PitchMin;
		ToggleMovement();
	} 
	else {
		bInspecting = false;
	}
}
void AXRBasketballSimCharacter::ToggleMovement() {
	bCanMove = !bCanMove;
	bInspecting = !bInspecting;
	FirstPersonCameraComponent->bUsePawnControlRotation = !FirstPersonCameraComponent->bUsePawnControlRotation;
	bUseControllerRotationYaw = !bUseControllerRotationYaw;

}
void AXRBasketballSimCharacter::ToggleItemPickup() {
	if (CurrentItem) {
		bHoldingItem = !bHoldingItem;
		CurrentItem->Pickup();

		if (!bHoldingItem) {
			CurrentItem = NULL;
		}
	}
}