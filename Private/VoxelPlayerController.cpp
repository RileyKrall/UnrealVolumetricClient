// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelPlayerController.h"
#include "VoxelManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine.h"

AVoxelPlayerController::AVoxelPlayerController() {
	PrimaryActorTick.bCanEverTick = true;
}

void AVoxelPlayerController::BeginPlay() {
	Super::BeginPlay();
	voxelManager = (AVoxelManager*) UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelManager::StaticClass());
	UE_LOG(LogTemp, Warning, TEXT("STARTING VOXEL PLAYER CONTROLLER"));
}

void AVoxelPlayerController::PlayerTick(float DeltaTime) {
	
	Super::PlayerTick(DeltaTime);

	if (voxelManager != nullptr && this->GetPawn() != nullptr) {
		FVector loc = this->GetPawn()->GetActorLocation();
		voxelManager->updatePlayerLocation(loc);
	}

}

void AVoxelPlayerController::DrawChunk(int x, int y, int z) {
	voxelManager->DrawChunk(x, y, z);
}

void AVoxelPlayerController::RequestChunk(int x, int y, int z) {
	voxelManager->requestChunk(x, y, z);
}

void AVoxelPlayerController::PrintChunk(int x, int y, int z) {
	voxelManager->PrintChunk(x, y, z);
}