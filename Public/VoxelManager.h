// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "TcpSocket.h"
#include "GameFramework/Actor.h"
#include "VObject.h"
#include "Net/UnrealNetwork.h"
#include "VoxelManager.generated.h"

class AVoxelTcpSocket;
UCLASS()
class VOXELGAME_API AVoxelManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVoxelManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDataTable* VoxelTypeMaterialList;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		AVObject* createVOjbect(FVector location, FRotator rotation, FVObjectSettings settings);

	UFUNCTION(BlueprintCallable)
		bool connectToVoxelServer();

	UFUNCTION()
		bool areVObjectsInitialized();

	UFUNCTION(BlueprintCallable)
		void InitializeVObjects();

	UFUNCTION()
		void changeCenterChunk(FVector newCenter);

	UFUNCTION()
		void requestChunk(int x, int y, int z);

	UFUNCTION()
		void unregisterChunk(int chunkId);

	UFUNCTION()
		AVObject* getVObject()
	{
		return createdVObjects[0];
	}

	UFUNCTION()
		void updatePlayerLocation(FVector worldCoordinates);

	UFUNCTION()
		void editPoint(int x, int y, int z, FPoint point);

	//USED FOR CONSOLE COMMANDS
	UFUNCTION()
		void DrawChunk(int x, int y, int z);

	UFUNCTION()
		void PrintChunk(int x, int y, int z);


private:

	UPROPERTY()
		uint8 RENDER_RADIUS = 2;

	UPROPERTY()
		uint8 STORAGE_RADIUS = 3;

	UPROPERTY()
		TArray<AVObject*> createdVObjects;

	UPROPERTY()
		bool chunkCurrentlyBeingProcessed = false;

	UPROPERTY()
		TSet<FVector> chunkRequestPending;

	UPROPERTY()
		TArray<FVector> chunkRequestQueue;

	UPROPERTY()
		TArray<AVObject*> LODWatchList;

	UPROPERTY()
		AVoxelTcpSocket* storageServerConnection;

	UPROPERTY()
		bool hasVObjectsInitialized = false;


};
