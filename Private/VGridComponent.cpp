// Fill out your copyright notice in the Description page of Project Settings.


#include "VGridComponent.h"
#include "MarchingCubesUtil.h"
#include "Engine.h"

// Sets default values for this component's properties
UVGridComponent::UVGridComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// ...
}


// Called when the game starts
void UVGridComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UVGridComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UVGridComponent::InitStorage(UMarchingCubesUtil* MCUtil, int resolutionOfChunks, int voxelResInChunk)
{
	if (GetOwnerRole() == ROLE_Authority) {
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Initializing storage"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Initializing storage"));
	}

	MarchingCubesUtil = MCUtil;
	chunkResolution = resolutionOfChunks;
	voxelResolutionPerChunk = voxelResInChunk;
}

FVoxel UVGridComponent::GetVoxel(int x, int y, int z)
{
	//Get correct chunk
	int xChunk = x / voxelResolutionPerChunk;
	int yChunk = y / voxelResolutionPerChunk;
	int zChunk = z / voxelResolutionPerChunk;

	//Get relative coords in chunk
	int relX = x % voxelResolutionPerChunk;
	int relY = y % voxelResolutionPerChunk;
	int relZ = z % voxelResolutionPerChunk;

	//UE_LOG(LogTemp, Warning, TEXT("chunk %d for chunk %d %d %d for voxel %d %d %d"), iChunk, xChunk, yChunk, zChunk, relX, relY, relZ);
	uint32 morton = MarchingCubesUtil->mortonEncode(relX, relY, relZ, voxelResolutionPerChunk);

	FChunk* chunk = Chunks.Find(getChunkId(xChunk, yChunk, zChunk));
	return chunk->voxelArray[morton];
}

void UVGridComponent::FillVoxel(int x, int y, int z, FPoint point)
{
	TArray<FVector> voxelCoords = {
	FVector(x, y, z),
	FVector(x + 1, y, z),
	FVector(x + 1, y + 1, z),
	FVector(x, y + 1, z),
	FVector(x, y, z + 1),
	FVector(x + 1, y, z + 1),
	FVector(x + 1, y + 1, z + 1),
	FVector(x, y + 1, z + 1)
	};
	for (int i = 0; i < 8; i++) {
		x = voxelCoords[i].X;
		y = voxelCoords[i].Y;
		z = voxelCoords[i].Z;

		SetPoint(x, y, z, point);
	}
}

void UVGridComponent::RemoveVoxel(int x, int y, int z)
{
	TArray<FVector> voxelCoords = {
		FVector(x, y, z),
		FVector(x + 1, y, z),
		FVector(x + 1, y + 1, z),
		FVector(x, y + 1, z),
		FVector(x, y, z + 1),
		FVector(x + 1, y, z + 1),
		FVector(x + 1, y + 1, z + 1),
		FVector(x, y + 1, z + 1)
	};
	for (int i = 0; i < 8; i++) {
		x = voxelCoords[i].X;
		y = voxelCoords[i].Y;
		z = voxelCoords[i].Z;

		SetPoint(x, y, z, FPoint());
	}
}

void UVGridComponent::SetPoint(int x, int y, int z, FPoint point)
{
	TArray<FVector> voxelCoords = {
		FVector(x, y, z),
		FVector(x - 1, y, z),
		FVector(x - 1, y - 1, z),
		FVector(x, y - 1, z),
		FVector(x, y, z - 1),
		FVector(x - 1, y, z - 1),
		FVector(x - 1, y - 1, z - 1),
		FVector(x, y - 1, z - 1)
	};
	//For each voxel to edit
	for (int i = 0; i < 8; i++) {

		x = voxelCoords[i].X;
		y = voxelCoords[i].Y;
		z = voxelCoords[i].Z;

		if (x >= 0 && y >= 0 && z >= 0) {
			//Get correct chunk
			int xChunk = x / voxelResolutionPerChunk;
			int yChunk = y / voxelResolutionPerChunk;
			int zChunk = z / voxelResolutionPerChunk;

			//Get relative coords in chunk
			int relX = x % voxelResolutionPerChunk;
			int relY = y % voxelResolutionPerChunk;
			int relZ = z % voxelResolutionPerChunk;

			if (Chunks.Contains(getChunkId(xChunk, yChunk, zChunk)))
			{
				FChunk* chunk = Chunks.Find(getChunkId(xChunk, yChunk, zChunk));
				uint32 morton = MarchingCubesUtil->mortonEncode(relX, relY, relZ, voxelResolutionPerChunk);

				if (chunk != NULL)
				{
					chunk->voxelArray[morton].pointArray[i].density = point.density;
					chunk->voxelArray[morton].pointArray[i].type = point.type;

					if (point.type != EVoxelType::Air) {
						chunk->bIsEmpty = false;
					}

					chunk->voxelArray[morton].calcShape();
					changedChunksSet.Add(chunk);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("NULL chunk"));
				}
			}
		}
	}
}

FPoint UVGridComponent::GetPoint(int x, int y, int z)
{
	//Get correct chunk
	int xChunk = x / voxelResolutionPerChunk;
	int yChunk = y / voxelResolutionPerChunk;
	int zChunk = z / voxelResolutionPerChunk;

	//Get relative coords in chunk
	int relX = x % voxelResolutionPerChunk;
	int relY = y % voxelResolutionPerChunk;
	int relZ = z % voxelResolutionPerChunk;

	//Index of voxel
	uint32 morton = MarchingCubesUtil->mortonEncode(relX, relY, relZ, voxelResolutionPerChunk);
	if (Chunks.Contains(getChunkId(xChunk, yChunk, zChunk))) {
		return Chunks.Find(getChunkId(xChunk, yChunk, zChunk))->voxelArray[morton].pointArray[0];
	}
	else {
		return FPoint();
	}
	
}

void UVGridComponent::SetChunk(int x, int y, int z, TArray<uint8> densities, TArray<uint8> materials)
{
	Chunks.Add(getChunkId(x, y, z), FChunk(FVector(x, y, z), GetVoxelResolutionPerChunk()));

	FVector voxelOffset = FVector(x, y, z) * voxelResolutionPerChunk;
	for (int i = 0; i < voxelResolutionPerChunk; i++) {
		for (int j = 0; j < voxelResolutionPerChunk; j++) {
			for (int k = 0; k < voxelResolutionPerChunk; k++) {

				int voxIndex = MarchingCubesUtil->mortonEncode(i, j, k, voxelResolutionPerChunk);

				FPoint point = FPoint(static_cast<EVoxelType>(materials[voxIndex]), densities[voxIndex]);
				SetPoint(i + voxelOffset.X, j + voxelOffset.Y, k + voxelOffset.Z, point);
			}
		}
	}

	//Fill seams
	//x y
	for (int i = 0; i <= voxelResolutionPerChunk; i++) {
		for (int j = 0; j <= voxelResolutionPerChunk; j++) {
			FPoint point = GetPoint(i + voxelOffset.X, j + voxelOffset.Y, 32 + voxelOffset.Z);
			SetPoint(i + voxelOffset.X, j + voxelOffset.Y, 32 + voxelOffset.Z, point);
		}
	}

	//y z
	for (int j = 0; j <= voxelResolutionPerChunk; j++) {
		for (int k = 0; k <= voxelResolutionPerChunk; k++) {
			FPoint point = GetPoint(32 + voxelOffset.X, j + voxelOffset.Y, k + voxelOffset.Z);
			SetPoint(32 + voxelOffset.X, j + voxelOffset.Y, k + voxelOffset.Z, point);
		}
	}

	//x z
	for (int i = 0; i <= voxelResolutionPerChunk; i++) {
		for (int k = 0; k <= voxelResolutionPerChunk; k++) {
			FPoint point = GetPoint(i + voxelOffset.X, 32 + voxelOffset.Y, k + voxelOffset.Z);
			SetPoint(i + voxelOffset.X, 32 + voxelOffset.Y, k + voxelOffset.Z, point);
		}
	}

}

FChunk* UVGridComponent::getChunk(int x, int y, int z)
{
	if (containsChunk(x, y, z)) {
		return Chunks.Find(getChunkId(x, y, z));
	}
	else {
		return nullptr;
	}
}

FChunk* UVGridComponent::getChunk(uint32 chunkId)
{
	if (Chunks.Contains(chunkId)) {
		return Chunks.Find(chunkId);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Chunk Id: %d  Does not exist"), chunkId);
		return nullptr;
	}
}

void UVGridComponent::printChunkData(int x, int y, int z)
{
	if (x < 0 || y < 0 || z < 0) {
		UE_LOG(LogTemp, Warning, TEXT("Chunks do not support negative coordinates %d %d %d"), x, y, z);
		return;
	}

	if (x > GetChunkResolution() || y > GetChunkResolution() || z > GetChunkResolution()) {
		UE_LOG(LogTemp, Warning, TEXT("Chunk Coordinates out of bounds %d %d %d"), x, y, z);
		return;
	}

	FChunk* chunk = Chunks.Find(getChunkId(x, y, z));

	UE_LOG(LogTemp, Warning, TEXT("PRINTING CHUNK: %d %d %d"), x, y, z);
	for (FVoxel vox : chunk->voxelArray) {
		UE_LOG(LogTemp, Warning, TEXT("d Value: %d"), vox.pointArray[0].density);
	}
}

void UVGridComponent::RemoveChunk(int x, int y, int z)
{
	Chunks.Remove(getChunkId(x, y, z));
}

bool UVGridComponent::containsChunk(int x, int y, int z)
{
	return Chunks.Contains(getChunkId(x, y, z));
}

bool UVGridComponent::containsChunk(uint32 chunkId)
{
	return Chunks.Contains(chunkId);
}

TArray<uint32> UVGridComponent::getChunkSet()
{
	TArray<uint32> outArray;
	Chunks.GetKeys(outArray);
	return outArray;
}

void UVGridComponent::addChunkToChangedChunkSet(int x, int y, int z)
{
	FChunk* chunk = Chunks.Find(getChunkId(x, y, z));
	if (chunk != NULL)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Adding chunk to change queue %d"), MarchingCubesUtil->mortonEncode(chunk->offset.X, chunk->offset.Y, chunk->offset.Z, chunk->resolution));
		changedChunksSet.Add(chunk);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk %d %d %d was not found in chunk list"), x, y, z);
	}
}

uint32 UVGridComponent::getChunkId(int x, int y, int z) {
	return x + (y * chunkResolution) + (z * chunkResolution * chunkResolution);
}
