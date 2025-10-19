// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NDisplayCameraSyncer.generated.h"

class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XRBASKETBALLSIM_API UNDisplayCameraSyncer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNDisplayCameraSyncer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UCameraComponent* PlayerCameraComponent;
	void SyncCameras(); //handles copying the player camera position to the nDisplay position so nDisplay is synced with the player first person view

		
};
