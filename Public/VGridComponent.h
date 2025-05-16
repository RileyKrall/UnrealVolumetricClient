// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MarchingCubesUtil.h"
#include "VGridComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VOXELGAME_API UVGridComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVGridComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitStorage(UMarchingCubesUtil* MCUtil, int resolutionOfChunks, int voxelResInChunk);

	int GetChunkResolution() { return chunkResolution; }

	int GetVoxelResolutionPerChunk() { return voxelResolutionPerChunk; }

	int getVoxelResolution() { return voxelResolutionPerChunk * chunkResolution; }

	FVoxel GetVoxel(int x, int y, int z);

	void FillVoxel(int x, int y, int z, FPoint point);

	void RemoveVoxel(int x, int y, int z);

	void SetPoint(int x, int y, int z, FPoint point);

	FPoint GetPoint(int x, int y, int z);

	void SetChunk(int x, int y, int z, TArray<uint8> densities, TArray<uint8> materials);

	FChunk* getChunk(int x, int y, int z);

	FChunk* getChunk(uint32 chunkId);

	void printChunkData(int x, int y, int z);

	void RemoveChunk(int x, int y, int z);

	bool containsChunk(int x, int y, int z);

	bool containsChunk(uint32 chunkId);

	TArray<uint32> getChunkSet();

	void addChunkToChangedChunkSet(int x, int y, int z);

	TSet<FChunk*> changedChunksSet;

	uint32 getChunkId(int x, int y, int z);

private:

	UPROPERTY()
		UMarchingCubesUtil* MarchingCubesUtil;

	UPROPERTY()
		int voxelResolutionPerChunk;

	UPROPERTY()
		int chunkResolution;

	UPROPERTY()
		TMap<uint32, FChunk> Chunks;

};
