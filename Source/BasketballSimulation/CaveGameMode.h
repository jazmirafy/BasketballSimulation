// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CaveGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BASKETBALLSIMULATION_API ACaveGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ACaveGameMode();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:

	AActor* PlayerCameraActor;
	AActor* NDisplayCameraActor;
	void SyncCameras(); //handles copying the player camera position to the nDisplay position so nDisplay is synced with the player first person view
	
};
