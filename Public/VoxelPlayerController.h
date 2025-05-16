// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VoxelManager.h"
#include "VoxelPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class VOXELGAME_API AVoxelPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AVoxelPlayerController();

	virtual void BeginPlay() override;

	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(Exec)
	void DrawChunk(int x, int y, int z);

	UFUNCTION(Exec)
	void RequestChunk(int x, int y, int z);

	UFUNCTION(Exec)
	void PrintChunk(int x, int y, int z);

	AVoxelManager* voxelManager;
};
