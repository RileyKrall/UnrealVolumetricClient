// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelManager.h"
#include <string>
#include "TcpSocket.h"
#include "VoxelTcpSocket.h"
#include "Engine.h"
#include "VObject.h"

// Sets default values
AVoxelManager::AVoxelManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;


}

// Called when the game starts or when spawned
void AVoxelManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AVoxelManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Check for data from storage server
	if (IsValid(storageServerConnection))
	{
		if (storageServerConnection->getPayloadQueue().Num() > 0)
		{
			//UE_LOG(LogTemp, Log, TEXT("Processing data from server. Current Queue Size: %d"), storageServerConnection->getPayloadQueue().Num());
			FNetPayload netPayload = storageServerConnection->PopFromPayloadQueue();

			//Convert payload data to correct struct
			//handle the payload

			//Currently only can handle one vObject

			if (netPayload.payload_type == EPayloadType::Diff) {
				
				FNetDiff netDiff;
				netDiff.fromBytes(netPayload.data);

				FPoint point = FPoint(static_cast<EVoxelType>(netDiff.material), netDiff.density);
				UE_LOG(LogTemp, Warning, TEXT("Processing Diff: (x,y,z): %d %d %d  chunkId: %d  type: %d density: %d "), netDiff.x, netDiff.y, netDiff.z, netDiff.chunk_id, point.type, point.density);
				createdVObjects[0]->SetPointInChunk(netDiff.x, netDiff.y, netDiff.z, netDiff.chunk_id, point);
				createdVObjects[0]->ChangeAffectedChunks();
			}
			else if (netPayload.payload_type == EPayloadType::Chunk)
			{
				FNetChunk netChunk;
				netChunk.fromBytes(netPayload.data);
				//UE_LOG(LogTemp, Log, TEXT("Recieved Chunk: %d, %d, %d"), netChunk.x, netChunk.y, netChunk.z);

				createdVObjects[0]->SetChunk(netChunk.x, netChunk.y, netChunk.z, netChunk.density, netChunk.material);
				createdVObjects[0]->ChangeAffectedChunks();

				chunkRequestPending.Remove(FVector(netChunk.x, netChunk.y, netChunk.z));
				chunkCurrentlyBeingProcessed = false;
			}
		}
	}

	if (!chunkCurrentlyBeingProcessed && chunkRequestQueue.Num() > 0)
	{
		FVector chunkToProcess = chunkRequestQueue.Pop();
		requestChunk(chunkToProcess.X, chunkToProcess.Y, chunkToProcess.Z);
		chunkCurrentlyBeingProcessed = true;

	}

	if (createdVObjects.Num() > 0)
	{
		if (chunkRequestPending.Num() == 0 && !createdVObjects[0]->bFinishedInitialLoad)
		{
			createdVObjects[0]->bFinishedInitialLoad = true;

			if (HasAuthority())
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[Server] VoxelManager Finished Initial Load")));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[Client] VoxelManager Finished Initial Load")));
			}
		}
	}

}

AVObject* AVoxelManager::createVOjbect(FVector location, FRotator rotation, FVObjectSettings settings) {

	FActorSpawnParameters spawnInfo;
	AVObject* vObj = GetWorld()->SpawnActor<AVObject>(location, rotation, spawnInfo);
	vObj->initializeObject(settings);

	createdVObjects.Add(vObj);
	return vObj;
}

bool AVoxelManager::connectToVoxelServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Attempting to Connect"));
	FActorSpawnParameters spawnInfo;
	if (GetWorld() != nullptr)
	{
		storageServerConnection = GetWorld()->SpawnActor<AVoxelTcpSocket>(FVector().ZeroVector, FRotator().ZeroRotator, spawnInfo);
		storageServerConnection->ConnectToGameServer();

		while (!storageServerConnection->isConnected(0));

		return true;
	}
	return false;
}

bool AVoxelManager::areVObjectsInitialized()
{
	if (createdVObjects.Num() == 0) {
		return false;
	}

	for (auto& obj : createdVObjects) {
		if (!obj->bFinishedInitialLoad) {
			return false;
		}
	}
	return true;
}

void AVoxelManager::InitializeVObjects()
{

	hasVObjectsInitialized = false;

	//Create vObj
	FVObjectSettings settings = FVObjectSettings(100, 32, 100, 0.5, true, true, true, VoxelTypeMaterialList);
	createVOjbect(FVector().ZeroVector, FRotator().ZeroRotator, settings);

	//get player spawn location
		//needs to be done on server on gamemode or state
	FVector centerChunk = FVector().ZeroVector;
	//Store chunk coords in set to keep track of what data has been sent/received


	TArray<FNetChunkRequest> chunkReqList;
	for (int x = centerChunk.X - STORAGE_RADIUS; x <= centerChunk.X + STORAGE_RADIUS; x++)
	{
		for (int y = centerChunk.Y - STORAGE_RADIUS; y <= centerChunk.Y + STORAGE_RADIUS; y++)
		{
			for (int z = centerChunk.Z - STORAGE_RADIUS; z <= centerChunk.Z + STORAGE_RADIUS; z++)
			{
				if (x >= 0 && y >= 0 && z >= 0)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Queuing %d %d %d to be requested"), x, y, z);
					chunkRequestQueue.Push(FVector(x, y, z));
					chunkRequestPending.Add(FVector(x, y, z));
				}
			}
		}
	}
}

void AVoxelManager::changeCenterChunk(FVector newCenter)
{
	UE_LOG(LogTemp, Warning, TEXT("Changing Center Chunk %f %f %f"), newCenter.X, newCenter.Y, newCenter.Z);
	TArray<uint32> chunkArray = createdVObjects[0]->getChunkSet();
	TSet<uint32> chunkSet;
	chunkSet.Append(chunkArray);

	//request new chunks
	TSet<uint32> newSet;
	int reqCount = 0;
	for (int x = -STORAGE_RADIUS; x <= STORAGE_RADIUS; x++)
	{
		for (int y = -STORAGE_RADIUS; y <= STORAGE_RADIUS; y++)
		{
			for (int z = -STORAGE_RADIUS; z <= STORAGE_RADIUS; z++)
			{
				if (FMath::Sqrt((x * x) + (y * y) + (z * z)) <= STORAGE_RADIUS)
				{
					uint32 iChunk = createdVObjects[0]->getChunkId(x + newCenter.X, y + newCenter.Y, z + newCenter.Z);
					FVector chunkCoord = FVector(x + newCenter.X, y + newCenter.Y, z + newCenter.Z);
					if (chunkCoord.X >= 0 && chunkCoord.Y >= 0 && chunkCoord.Z >= 0 && !chunkSet.Contains(iChunk))
					{
						//UE_LOG(LogTemp, Warning, TEXT("Queuing %f %f %f to be requested"), chunk.X, chunk.Y, chunk.Z);
						chunkRequestQueue.Push(chunkCoord);
						chunkRequestPending.Add(chunkCoord);
						reqCount++;
					}
					else if (chunkCoord.X >= 0 && chunkCoord.Y >= 0 && chunkCoord.Z >= 0) {
						newSet.Add(iChunk);
					}					
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Requesting %d chunks"), reqCount);
	createdVObjects[0]->SetCenterChunk(newCenter);

	//UnRegister from old chunks
	for (uint32 chunkId : chunkSet)
	{
		if (!newSet.Contains(chunkId))
		{
			unregisterChunk(chunkId);
		}
	}

}

void AVoxelManager::unregisterChunk(int chunkId)
{
	//UE_LOG(LogTemp, Warning, TEXT("UnRequesting Chunk %d %d %d"), x, y, z);
	//Store struct data into a payload request
	FNetPayload netPayload;
	netPayload.payload_type = EPayloadType::UnRegisterChunk;

	FNetDeRegisterRequest DeRegisterRequest = FNetDeRegisterRequest(chunkId);
	TArray<uint8> data = DeRegisterRequest.serialize();
	netPayload.data = data;

	TArray<uint8> wrappedRequest = netPayload.serialize();

	uint8* header = new uint8[4];
	uint32 hSize = wrappedRequest.Num();
	header[0] = hSize >> 24;
	header[1] = hSize >> 16;
	header[2] = hSize >> 8;
	header[3] = hSize;

	TArray<uint8> output;
	for (int i = 0; i < 4; i++)
	{
		output.Add(header[i]);
	}
	output.Append(wrappedRequest);

	//Send byte array
	storageServerConnection->SendData(storageServerConnection->connectionIdGameServer, output);
}

void AVoxelManager::updatePlayerLocation(FVector worldCoordinates)
{
	
	FVector chunk = createdVObjects[0]->getChunkCoordinatesFromWorldLocation(worldCoordinates);
	
	if (!createdVObjects[0]->GetCenterChunk().Equals(chunk)) {
		changeCenterChunk(chunk);
	}
}

void AVoxelManager::editPoint(int x, int y, int z, FPoint point)
{
	UE_LOG(LogTemp, Warning, TEXT("Editing Point %d %d %d"), x, y, z);
	if (x < 0 || y < 0 || z < 0)
	{
		return;
	}

	//Store struct data into a payload request
	FNetPayload netPayload;
	netPayload.payload_type = EPayloadType::Diff;

	FVector chunk = createdVObjects[0]->getChunkCoordinatesFromVoxelPoint(FVector(x, y, z));
	int relX = x % 32;
	int relY = y % 32;
	int relZ = z % 32;
	int iChunk = chunk.X + (chunk.Y * 100) + (chunk.Z * 100 * 100);

	UE_LOG(LogTemp, Warning, TEXT("Editing Chunk %d, %f %f %f"), iChunk, chunk.X, chunk.Y, chunk.Z);

	FNetDiff netDiff = FNetDiff(iChunk, relX, relY, relZ, point.density, (uint8)point.type);
	TArray<uint8> data = netDiff.serialize();
	netPayload.data = data;

	TArray<uint8> wrappedRequest = netPayload.serialize();

	uint8* header = new uint8[4];
	uint32 hSize = wrappedRequest.Num();
	header[0] = hSize >> 24;
	header[1] = hSize >> 16;
	header[2] = hSize >> 8;
	header[3] = hSize;


	TArray<uint8> output;
	for (int i = 0; i < 4; i++)
	{
		output.Add(header[i]);
	}
	output.Append(wrappedRequest);

	//Send byte array
	storageServerConnection->SendData(storageServerConnection->connectionIdGameServer, output);
}

void AVoxelManager::DrawChunk(int x, int y, int z)
{
	createdVObjects[0]->DrawChunk(x, y, z);
}

void AVoxelManager::PrintChunk(int x, int y, int z)
{
	createdVObjects[0]->PrintChunk(x, y, z);
}


void AVoxelManager::requestChunk(int x, int y, int z)
{

	if (x < 0 || y < 0 || z < 0)
	{
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Requesting Chunk %d %d %d"), x, y, z);

	//Store struct data into a payload request
	FNetPayload netPayload;
	netPayload.payload_type = EPayloadType::ChunkRequest;

	FNetChunkRequest chunkRequest = FNetChunkRequest(x, y, z);
	TArray<uint8> data = chunkRequest.serialize();
	netPayload.data = data;

	TArray<uint8> wrappedRequest = netPayload.serialize();

	uint8* header = new uint8[4];
	uint32 hSize = wrappedRequest.Num();
	header[0] = hSize >> 24;
	header[1] = hSize >> 16;
	header[2] = hSize >> 8;
	header[3] = hSize;


	TArray<uint8> output;
	for (int i = 0; i < 4; i++)
	{
		output.Add(header[i]);
	}
	output.Append(wrappedRequest);

	//Send byte array
	storageServerConnection->SendData(storageServerConnection->connectionIdGameServer, output);
}

