// Fill out your copyright notice in the Description page of Project Settings.


#include "VObject.h"
#include "Engine.h"
#include "Async/Async.h"
#include "ProceduralMeshComponent.h"
#include "MarchingCubesUtil.h"
#include "VGridComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AVObject::AVObject()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false;
	bAlwaysRelevant = true;

	root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = root;

	//Storage Component
	storage = CreateDefaultSubobject<UVGridComponent>(TEXT("VoxelStorage"));

}

// Called when the game starts or when spawned
void AVObject::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AVObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVObject::initializeObject(FVObjectSettings parameters)
{
	params = parameters;
	if (HasAuthority()) {
		UE_LOG(LogTemp, Display, TEXT("[SERVER] VObject initializing params"));
	}
	else {
		UE_LOG(LogTemp, Display, TEXT("[CLIENT] VObject initializing params   chunk res: %d    vox res per chunk: %d"), params.chunkResolution, params.voxelResPerChunk);
	}
	//Create MC utilities
	MarchingCubesUtil = NewObject<UMarchingCubesUtil>();

	mapVoxelTypeToMaterial = GenerateColorMap();

	//Initialize storage component
	storage->InitStorage(MarchingCubesUtil, params.chunkResolution, params.voxelResPerChunk);

}

FTypeToMaterialMap AVObject::GenerateColorMap() {
	FTypeToMaterialMap map = FTypeToMaterialMap();

	for (auto name : params.MaterialsForVoxelTypes->GetRowNames()) {
		FVoxelTypeMaterial* dataRow = params.MaterialsForVoxelTypes->FindRow<FVoxelTypeMaterial>(name, FString("test"));
		map.mapTypeToColor(dataRow->type, dataRow->material);
	}

	return map;
}

void AVObject::FillVoxel(int x, int y, int z, EVoxelType type)
{
	storage->FillVoxel(x, y, z, FPoint(type, params.densityValue));
}

FVector AVObject::voxelPointFromWorldPosition(int x, int y, int z)
{
	FVector voxPoint = (FVector(x, y, z) - GetActorLocation()) / params.unitScale;
	return voxPoint;
}

void AVObject::RemoveVoxel(int x, int y, int z)
{
	storage->RemoveVoxel(x, y, z);
}

void AVObject::SetPoint(int x, int y, int z, FPoint point)
{
	storage->SetPoint(x, y, z, point);
}

void AVObject::SetPointInChunk(int x, int y, int z, int chunkId, FPoint point)
{
	FVector offset = storage->getChunk(chunkId)->offset;
	UE_LOG(LogTemp, Warning, TEXT("offset of chunk %f, %f %f %f"), chunkId, offset.X, offset.Y, offset.Z);
	SetPoint(x + params.voxelResPerChunk * offset.X, y + params.voxelResPerChunk * offset.Y, z + params.voxelResPerChunk * offset.Z, point);
}

FPoint AVObject::GetPoint(int x, int y, int z)
{
	return storage->GetPoint(x, y, z);
}

void AVObject::SetChunk(int x, int y, int z, TArray<uint8> densities, TArray<uint8> materials)
{
	int iChunk = x + (y * storage->GetChunkResolution()) + (z * storage->GetChunkResolution() * storage->GetChunkResolution());
	storage->SetChunk(x, y, z, densities, materials);
}

bool AVObject::containsChunk(int x, int y, int z)
{
	return storage->containsChunk(x, y, z);
}

FVector AVObject::getChunkCoordinatesFromWorldLocation(FVector worldLocation)
{
	FVector voxPoint = (worldLocation - GetActorLocation()) / params.unitScale;

	int xChunk = voxPoint.X / params.voxelResPerChunk;
	int yChunk = voxPoint.Y / params.voxelResPerChunk;
	int zChunk = voxPoint.Z / params.voxelResPerChunk;

	return FVector(xChunk, yChunk, zChunk);

}

FVector AVObject::getChunkCoordinatesFromVoxelPoint(FVector point)
{

	int xChunk = point.X / params.voxelResPerChunk;
	int yChunk = point.Y / params.voxelResPerChunk;
	int zChunk = point.Z / params.voxelResPerChunk;

	return FVector(xChunk, yChunk, zChunk);
}

bool AVObject::isInRenderDistance(int x, int y, int z)
{
	int distX = this->GetCenterChunk().X - x;
	int distY = this->GetCenterChunk().Y - y;
	int distZ = this->GetCenterChunk().Z - z;

	if (FMath::Sqrt((distX * distX) + (distY * distY) + (distZ * distZ)) > RENDER_RADIUS){
		return false;
	} else {
		return true;
	}

}

bool AVObject::isInStorageDistance(int x, int y, int z)
{
	int distX = this->GetCenterChunk().X - x;
	int distY = this->GetCenterChunk().Y - y;
	int distZ = this->GetCenterChunk().Z - z;

	if (FMath::Sqrt((distX * distX) + (distY * distY) + (distZ * distZ)) > STORAGE_RADIUS) {
		return false;
	}
	else {
		return true;
	}
}

FVector AVObject::GetCenterChunk()
{
	return centerChunk;
}

void AVObject::SetCenterChunk(const FVector& CenterChunk)
{
	centerChunk = CenterChunk;

	//Draw new chunks in render distance
	TSet<FVector> chunksToKeepDrawn;
	for (int x = -RENDER_RADIUS; x <= RENDER_RADIUS; x++)
	{
		for (int y = -RENDER_RADIUS; y <= RENDER_RADIUS; y++)
		{
			for (int z = -RENDER_RADIUS; z <= RENDER_RADIUS; z++)
			{
				if (FMath::Sqrt((x * x) + (y * y) + (z * z)) <= RENDER_RADIUS)
				{
					FVector chunk = FVector(x + CenterChunk.X, y + CenterChunk.Y, z + CenterChunk.Z);
					if (chunk.X >= 0 && chunk.Y >= 0 && chunk.Z >= 0 && !ChunksDrawn.Contains(chunk))
					{
						storage->addChunkToChangedChunkSet(chunk.X, chunk.Y, chunk.Z);
					}
					chunksToKeepDrawn.Add(chunk);
				}
			}
		}
	}
	
	//Remove Meshes for chunks out of draw range
	for (FVector chunk : ChunksDrawn.Array())
	{
		if (!chunksToKeepDrawn.Contains(chunk))
		{
			int iChunk = storage->getChunkId(chunk.X, chunk.Y, chunk.Z);
			if (ChunkMeshMap.Contains(iChunk))
			{
				(*ChunkMeshMap.Find(iChunk))->ClearAllMeshSections();
				ChunkMeshMap.Remove(iChunk);
			}
			ChunksDrawn.Remove(chunk);
		}
	}

	//unload chunks out of storage range
	for (uint32 chunkId : storage->getChunkSet())
	{
		FVector chunk = storage->getChunk(chunkId)->offset;
		int x = chunk.X - CenterChunk.X;
		int y = chunk.Y - CenterChunk.Y;
		int z = chunk.Z - CenterChunk.Z;

		if (FMath::Sqrt((x * x) + (y * y) + (z * z)) > STORAGE_RADIUS)
		{
			UE_LOG(LogTemp, Warning, TEXT("Removing %f %f %f from storage"), chunk.X, chunk.Y, chunk.Z);
			storage->RemoveChunk(chunk.X, chunk.Y, chunk.Z);
		}
	}

	ChangeAffectedChunks();
}

TArray<uint32> AVObject::getChunkSet()
{
	return storage->getChunkSet();
}

uint32 AVObject::getChunkId(int x, int y, int z)
{
	return storage->getChunkId(x,y,z);
}

FVector AVObject::getChunkOffset(uint32 chunkId)
{
	if (storage->containsChunk(chunkId)) {
		return storage->getChunk(chunkId)->offset;
	}
	else {
		return FVector().ZeroVector;
	}
	
}

void AVObject::ChangeAffectedChunks()
{
	//if (HasAuthority()) {
	//	UE_LOG(LogTemp, Warning, TEXT("[SERVER] Changing affected chunks. Count: %d"), storage->changedChunksSet.Num());
	//}
	//else {
	//	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Changing affected chunks. Count: %d"), storage->changedChunksSet.Num());
	//}

	//Change this to start a timer to pop chunks to draw
	//This will prevent many chunks needing to be redrawn in one frame
	//Order from closest radius to further away

	for (auto& chunk : storage->changedChunksSet) {
		//If chunk is within render distance
		if (FVector::Dist(chunk->offset, centerChunk) <= RENDER_RADIUS)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Calling Chunk %d"), MarchingCubesUtil->mortonEncode(chunk->offset.X, chunk->offset.Y, chunk->offset.Z, chunk->resolution));
			int iChunk = chunk->offset.X + (chunk->offset.Y * storage->GetChunkResolution()) + (chunk->offset.Z * storage->GetChunkResolution() * storage->GetChunkResolution());

			//Build procedural mesh if one does not exist
			if (!ChunkMeshMap.Contains(iChunk))
			{
				UProceduralMeshComponent* mesh = NewObject<UProceduralMeshComponent>(this, UProceduralMeshComponent::StaticClass());
				mesh->SetIsReplicated(true);
				ChunkMeshMap.Add(iChunk, mesh);
				(*ChunkMeshMap.Find(iChunk))->AttachToComponent(root, FAttachmentTransformRules::SnapToTargetIncludingScale);
				(*ChunkMeshMap.Find(iChunk))->RegisterComponent();
			}

			//Draw the chunk
			DrawChunk(chunk);
		}
	}

	storage->changedChunksSet.Empty();
}


void AVObject::DrawChunk(int x, int y, int z)
{

	if (x < 0 || y < 0 || z < 0) {
		UE_LOG(LogTemp, Warning, TEXT("Chunks do not support negative coordinates %d %d %d"), x, y, z);
		return;
	}

	if (x > storage->GetChunkResolution() || y > storage->GetChunkResolution() || z > storage->GetChunkResolution()) {
		UE_LOG(LogTemp, Warning, TEXT("Chunk Coordinates out of bounds %d %d %d"), x, y, z);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Drawing Chunk %d %d %d"), x, y, z);
	int iChunk = x + (y * storage->GetChunkResolution()) + (z * storage->GetChunkResolution() * storage->GetChunkResolution());

	//Build procedural mesh if one does not exist
	if (!ChunkMeshMap.Contains(iChunk))
	{
		UProceduralMeshComponent* mesh = NewObject<UProceduralMeshComponent>(this, UProceduralMeshComponent::StaticClass());

		ChunkMeshMap.Add(iChunk, mesh);
		(*ChunkMeshMap.Find(iChunk))->AttachToComponent(root, FAttachmentTransformRules::SnapToTargetIncludingScale);
		(*ChunkMeshMap.Find(iChunk))->RegisterComponent();
	}

	FChunk* chunk = storage->getChunk(x, y, z);
	if (chunk == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("No Chunk Stored At %d %d %d"), x, y, z);
		return;
	}

	//Draw the chunk
	DrawChunk(chunk);
}

void AVObject::PrintChunk(int x, int y, int z)
{
	storage->printChunkData(x, y, z);
}

void AVObject::DrawChunk(FChunk* chunk)
{
	UE_LOG(LogTemp, Warning, TEXT("DRAWING CHUNK %f %f %f"), chunk->offset.X, chunk->offset.Y, chunk->offset.Z);
	//Get chunk offset
	FVector offset = chunk->offset;

	//Get corresponding procedural mesh
	int iChunk = offset.X + (offset.Y * storage->GetChunkResolution()) + (offset.Z * storage->GetChunkResolution() * storage->GetChunkResolution());

	if (chunk->bIsEmpty) {
		return;
	}

	//Storage for calculating results
	TArray<TFuture<TMap<EVoxelType, FTypeBuffer>>> waitingSubBuffers;
	int threadCount = 8;
	for (int threadId = 0; threadId < threadCount; threadId++) {
		int startX = (chunk->resolution / threadCount) * threadId;
		//<TMap<EVoxelType, FTypeBuffer>>
		waitingSubBuffers.Add(Async(EAsyncExecution::ThreadPool, [&, chunk, startX]() {
			TMap<EVoxelType, FTypeBuffer> subTypeBuffers;

			FVector offset = chunk->offset;

			for (int x = startX; x < chunk->resolution; x++) {
				for (int y = 0; y < chunk->resolution; y++) {
					for (int z = 0; z < chunk->resolution; z++) {
						FVoxel voxel = storage->GetVoxel(offset.X * chunk->resolution + x, offset.Y * chunk->resolution + y, offset.Z * chunk->resolution + z);
						int shapeIndex = voxel.shape;
						if (shapeIndex != 0 && shapeIndex != 255) {
							//Get type for whole voxel. Based on the lowest index vertex thats not air.
							EVoxelType voxelType = EVoxelType::Air;
							for (int v = 0; v < 8; v++) {
								if (voxel.pointArray[v].type != EVoxelType::Air) {
									voxelType = voxel.pointArray[v].type;
									break;
								}
							}

							//Get triangle data for shape
							TArray<int> voxelVertices;
							if (MarchingCubesUtil) {
								voxelVertices = MarchingCubesUtil->GetMCTrianglePoints(shapeIndex);
							}
							else {
								UE_LOG(LogTemp, Warning, TEXT("MC util not valid"));
							}

							if (!subTypeBuffers.Contains(voxelType)) {
								//UE_LOG(LogTemp, Warning, TEXT("Buffer type %d"), type);
								subTypeBuffers.Add(voxelType, FTypeBuffer());
							}
							FTypeBuffer* curBuffer = subTypeBuffers.Find(voxelType);

							//Insert triangle data into buffers for mesh
							for (int vi = 0; vi < voxelVertices.Num(); vi++) {
								int v = voxelVertices[vi];

								//Calc edge intersection point for "smoother" surface
								FVector2D points = MarchingCubesUtil->GetVerticesForMidPoints(v);

								FVector P;
								FVector P1 = MarchingCubesUtil->GetVoxelVertex(points.X);
								FVector P2 = MarchingCubesUtil->GetVoxelVertex(points.Y);

								//Check if interpolation is needed
								if (params.bUseVoxelInterpolation) {
									uint8 V1 = voxel.pointArray[points.X].density;
									uint8 V2 = voxel.pointArray[points.Y].density;
									//P = P1 + ((params.densityValue - V1) * (P2 - P1) / (V2 - V1));

									if (V1 != 0) {
										P1 = P1 * (V1 / 100);
									}
									
									if (V2 != 0) {
										P2 = P2 * (V2 / 100);
									}

									P = (P1 + P2) / 2;
								}
								else {
									P = (P1 + P2) / 2;
								}

								FVector vertex = (offset * chunk->resolution + FVector(x, y, z) + P) * params.unitScale;
								curBuffer->ShiftedVertices.Add(vertex);
								curBuffer->Triangles.Add(curBuffer->NumOfVertices);
								curBuffer->vertexColors.Add(FLinearColor::Black);
								curBuffer->NumOfVertices++;
							}

							//For each triangle in voxel
							for (int i = 0; i < voxelVertices.Num() / 3; i++) {
								curBuffer->UV0.Add(FVector2D(0, 0));
								curBuffer->UV0.Add(FVector2D(1, 0));
								curBuffer->UV0.Add(FVector2D(0, 1));
							}
						}
					}
				}
			}

			for (auto& buffer : subTypeBuffers) {
				for (int ti = 0; ti < buffer.Value.ShiftedVertices.Num() / 3; ti++) {
					FVector va = buffer.Value.ShiftedVertices[(3 * ti)];
					FVector vb = buffer.Value.ShiftedVertices[(3 * ti) + 1];
					FVector vc = buffer.Value.ShiftedVertices[(3 * ti) + 2];

					FVector e1 = va - vb;
					FVector e2 = vc - vb;
					FVector no = FVector().CrossProduct(e1, e2);
					no.Normalize();

					buffer.Value.normals.Add(no);
					buffer.Value.normals.Add(no);
					buffer.Value.normals.Add(no);
				}
			}

			return subTypeBuffers;
			}));
	}
	//End creating threads

	//Buffer map. Master buffer map that all sub buffers will be added into
	TMap<EVoxelType, FTypeBuffer> typeBuffers;
	for (int threadId = 0; threadId < threadCount; threadId++) {
		while (!waitingSubBuffers[threadId].IsReady()) {
			//Wait until ready
		}

		//UE_LOG(LogTemp, Warning, TEXT("Thread %d begin merging into chunk id: %d"), threadId, iChunk);
		for (auto& bufferPair : waitingSubBuffers[threadId].Get()) {
			//Check if buffer of needed type has been created yet.
			if (!typeBuffers.Contains(bufferPair.Key)) {
				typeBuffers.Add(bufferPair.Key, FTypeBuffer());
			}

			//Get associated buffer
			FTypeBuffer* buffer = typeBuffers.Find(bufferPair.Key);

			//Add sub buffer to master buffer
			buffer->NumOfVertices += bufferPair.Value.NumOfVertices;
			buffer->ShiftedVertices += bufferPair.Value.ShiftedVertices;
			buffer->Triangles += bufferPair.Value.Triangles;
			buffer->normals += bufferPair.Value.normals;
			buffer->UV0 += bufferPair.Value.UV0;
			buffer->vertexColors += bufferPair.Value.vertexColors;
		}
	}

	//Per buffer, draw buffer
	int meshSections = 0;
	for (auto& bufferPair : typeBuffers) {
		EVoxelType key = bufferPair.Key;
		UE_LOG(LogTemp, Warning, TEXT("Before Coloring"));
		(*ChunkMeshMap.Find(iChunk))->CreateMeshSection_LinearColor(meshSections, bufferPair.Value.ShiftedVertices, bufferPair.Value.Triangles, bufferPair.Value.normals, bufferPair.Value.UV0, bufferPair.Value.vertexColors, bufferPair.Value.tangents, params.bCalcCollision);
		UE_LOG(LogTemp, Warning, TEXT("Before collision setting"));
		(*ChunkMeshMap.Find(iChunk))->ContainsPhysicsTriMeshData(params.bCalcCollision);
		UE_LOG(LogTemp, Warning, TEXT("before mat check"));
		if (mapVoxelTypeToMaterial.materialMap.Num() > 0) {
			UE_LOG(LogTemp, Warning, TEXT("before setting material"));
			(*ChunkMeshMap.Find(iChunk))->SetMaterial(meshSections, *mapVoxelTypeToMaterial.materialMap.Find(key));
			UE_LOG(LogTemp, Warning, TEXT("after setting material"));
		}
		else {
			if (HasAuthority()) {
				UE_LOG(LogTemp, Warning, TEXT("[SERVER] Material NULL"));
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Material NULL"));
			}
		}

		meshSections++;
	}
	typeBuffers.Empty();

	ChunksDrawn.Add(chunk->offset);
}
