// Fill out your copyright notice in the Description page of Project Settings.

#include "CaveGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"


ACaveGameMode::ACaveGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	if (PlayerPawnBPClass.Succeeded())
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}


	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController"));
	if (PlayerControllerBPClass.Succeeded())
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}

void ACaveGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Error, TEXT("Log is working"));

	//assign the ndisplay and player to our variables
	PlayerCameraActor = GetWorld()->GetFirstPlayerController()->GetViewTarget();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag("nDisplay Anchor")) 
		{
			NDisplayCameraActor = *It;
			break;
		}
	}
	//see if we found the ndisplay and the player
	if (PlayerCameraActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player found: %s"), *PlayerCameraActor->GetName());
	
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("No player found!"));
	}
	if (NDisplayCameraActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("nDisplay found: %s"), *NDisplayCameraActor->GetName());

	}
	else {
		UE_LOG(LogTemp, Error, TEXT("No nDisplay found!"));
	}
}
void ACaveGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	SyncCameras(); //run sync cameras each frame so the cave nDisplay accurately represents the player first person view

}
void ACaveGameMode::SyncCameras() {
	//if references to both the player camera and nDisplay are valid, assign the players camera transform to nDisplay
	if (PlayerCameraActor && NDisplayCameraActor) {
		FTransform PlayerTransform = PlayerCameraActor->GetActorTransform();
		NDisplayCameraActor->SetActorTransform(PlayerTransform);
	}
}
