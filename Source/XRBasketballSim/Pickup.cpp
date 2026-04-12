#include "Pickup.h"
#include "XRBasketballSimCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// sets default values for the pickup actor
APickup::APickup()
{
	// enable ticking every frame
	PrimaryActorTick.bCanEverTick = true;

	// create mesh component and enable physics
	MyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyMesh"));
	MyMesh->SetEnableGravity(false);
	MyMesh->SetSimulatePhysics(false);
	MyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// set mesh as root component
	RootComponent = MyMesh;

	// initialize pickup state
	bHolding = true;
	bGravity = false;
}

// called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

	// get the player character
	MyCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);

	// get the player camera component
	PlayerCamera = MyCharacter->FindComponentByClass<UCameraComponent>();

	// find the holding component attached to the player
	TArray<USceneComponent*> Components;
	MyCharacter->GetComponents(Components);

	if (Components.Num() > 0)
	{
		for (auto& Comp : Components)
		{
			// match by name so we know where to attach held objects
			if (Comp->GetName() == "HoldingComponent")
			{
				HoldingComp = Cast<USceneComponent>(Comp);
			}
		}
	}

	//Tells the band that is in the holding component that it is the held ball
	AXRBasketballSimCharacter* BasketballChar = Cast<AXRBasketballSimCharacter>(MyCharacter);
	if (BasketballChar)
	{
		BasketballChar->HeldBall = this;
		BasketballChar->bHoldingItem = true;

		MyMesh->IgnoreActorWhenMoving(MyCharacter, true);
		MyCharacter->MoveIgnoreActorAdd(this);
	}
}

// called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// snap pickup to holding component when held
	if (bHolding && HoldingComp)
	{
		MyMesh->SetSimulatePhysics(false);
		SetActorLocationAndRotation(
			HoldingComp->GetComponentLocation(),
			HoldingComp->GetComponentRotation()
		);
	}
}

// rotates the pickup to match player control rotation
void APickup::RotateActor()
{
	ControlRotation = GetWorld()->GetFirstPlayerController()->GetControlRotation();
	SetActorRotation(FQuat(ControlRotation));
}

// toggles pickup and throw behavior
void APickup::Pickup()
{
	// toggle holding and gravity states
	bHolding = !bHolding;
	bGravity = !bGravity;

	// update physics and collision based on holding state
	MyMesh->SetEnableGravity(bGravity);
	MyMesh->SetSimulatePhysics(bHolding ? false : true);
	MyMesh->SetCollisionEnabled(
		bHolding ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics
	);

	// apply impulse forward when released
	if (!bHolding)
	{
		ForwardVector = PlayerCamera->GetForwardVector();
		MyMesh->AddImpulse(
			ForwardVector * ForceAmount * MyMesh->GetMass()
		);
	}
}

void APickup::Shoot()
{
	// release and fire the ball
	bHolding = false;
	bGravity = true;

	SetActorTickEnabled(false);

	MyMesh->IgnoreActorWhenMoving(MyCharacter, false);
	MyCharacter->MoveIgnoreActorRemove(this);

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	MyMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	MyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MyMesh->SetEnableGravity(true);
	MyMesh->SetSimulatePhysics(true);


	// apply forward impulse
	ForwardVector = PlayerCamera->GetForwardVector();
	MyMesh->AddImpulse(ForwardVector * ForceAmount * MyMesh->GetMass());

}

void APickup::ReturnToHand()
{
	// reset ball state back to held
	bHolding = true;
	bGravity = false;

	// re-enable tick so it snaps to hand
	SetActorTickEnabled(true);

	// disable physics and collision
	MyMesh->SetSimulatePhysics(false);
	MyMesh->SetEnableGravity(false);
	MyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// re-add movement ignore so ball doesn't push player
	MyMesh->IgnoreActorWhenMoving(MyCharacter, true);
	MyCharacter->MoveIgnoreActorAdd(this);

	// re-register with character
	AXRBasketballSimCharacter* BasketballChar = Cast<AXRBasketballSimCharacter>(MyCharacter);
	if (BasketballChar)
	{
		BasketballChar->HeldBall = this;
		BasketballChar->bHoldingItem = true;
	}
}