// Fill out your copyright notice in the Description page of Project Settings.


#include "NDisplayCameraSyncer.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"

// Sets default values
UNDisplayCameraSyncer::UNDisplayCameraSyncer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void UNDisplayCameraSyncer::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Log is working"));


	//find the player, once found get the players camera component, then assign it to the appropriate variable
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (PlayerPawn)
	{
		PlayerCameraComponent = PlayerPawn->FindComponentByClass<UCameraComponent>();
		if (PlayerCameraComponent) {

			UE_LOG(LogTemp, Warning, TEXT("Player Camera Component found: %s"), *PlayerCameraComponent->GetName());
		}
		else {

			UE_LOG(LogTemp, Error, TEXT("No Player Camera Component found"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Player Pawn found!"));
	}
}

// Called every frame
void UNDisplayCameraSyncer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SyncCameras(); //run sync cameras each frame so the cave nDisplay accurately represents the player first person view
}
void UNDisplayCameraSyncer::SyncCameras() {
	//if reference to the player camera is valid, assign the players camera transform to the nDisplay anchor
	if (PlayerCameraComponent) {
		FTransform CameraTransform = PlayerCameraComponent->GetComponentTransform();
		GetOwner()->SetActorTransform(CameraTransform);
	}
}

