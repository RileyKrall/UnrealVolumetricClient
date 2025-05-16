// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TcpSocket.h"

#include "VoxelTcpSocket.generated.h"


/**
* PAYLOAD DATA WRAPPER
*/

UENUM()
enum class EPayloadType : uint8
{
	Diff,
	Chunk,
	ChunkRequest,
	UnRegisterChunk
};

USTRUCT()
struct FNetPayload
{
	GENERATED_BODY()

		UPROPERTY()
		EPayloadType payload_type;

	UPROPERTY()
		TArray<uint8> data;

	TArray<uint8> serialize()
	{
		TArray<uint8> output;
		output.Add((uint8)this->payload_type);
		output.Append(data);

		return output;
	}

};

/**
 * STORAGE SERVER INBOUND STRUCTS
 */

USTRUCT()
struct FNetDiff
{
	GENERATED_BODY()

	uint32 chunk_id;
	uint32 x;
	uint32 y;
	uint32 z;
	uint8 density;
	uint8 material;

	FNetDiff() {

	}

	FNetDiff(uint32 chunkId, uint32 chunk_x, uint32 chunk_y, uint32 chunk_z, uint8 dens, uint8 mat) {
		chunk_id = chunkId;
		x = chunk_x;
		y = chunk_y;
		z = chunk_z;
		density = dens;
		material = mat;
	}

	TArray<uint8> serialize()
	{
		TArray<uint8> output;

		output.Append(ATcpSocket::Conv_IntToBytes(this->chunk_id));
		output.Append(ATcpSocket::Conv_IntToBytes(this->x));
		output.Append(ATcpSocket::Conv_IntToBytes(this->y));
		output.Append(ATcpSocket::Conv_IntToBytes(this->z));
		output.Add(density);
		output.Add(material);

		return output;
	}

	void fromBytes(TArray<uint8> bytes)
	{
		this->chunk_id = ATcpSocket::Message_ReadInt(bytes);
		this->x = ATcpSocket::Message_ReadInt(bytes);
		this->y = ATcpSocket::Message_ReadInt(bytes);
		this->z = ATcpSocket::Message_ReadInt(bytes);
		this->material = bytes.Pop();
		this->density = bytes.Pop();
		
	}

};

USTRUCT()
struct FNetDiffList
{
	GENERATED_BODY()

		TArray<FNetDiff> list;

};

USTRUCT()
struct FNetChunk
{
	GENERATED_BODY()

	uint32 x;
	uint32 y;
	uint32 z;
	TArray<uint8> density;
	TArray<uint8> material;

	void fromBytes(TArray<uint8> bytes)
	{
		this->x = ATcpSocket::Message_ReadInt(bytes);
		this->y = ATcpSocket::Message_ReadInt(bytes);
		this->z = ATcpSocket::Message_ReadInt(bytes);

		int size = (int)bytes.Num() / 2;

		TArray<uint8> toDensity;
		for (int i = 0; i < size; i++)
		{
			toDensity.Add(bytes.Pop());
		}

		this->density = toDensity;
		this->material = bytes;
	}

};

USTRUCT()
struct FNetChunkList
{
	GENERATED_BODY()

		TArray<FNetChunk> list;

};


/**
* OUTBOUND STRUCTS
*/
USTRUCT()
struct FNetChunkRequest
{
	GENERATED_BODY()

		FNetChunkRequest()
	{
	}

	FNetChunkRequest(int chunkX, int chunkY, int chunkZ)
	{
		x = chunkX;
		y = chunkY;
		z = chunkZ;
	}

	UPROPERTY()
		int x;

	UPROPERTY()
		int y;

	UPROPERTY()
		int z;

	TArray<uint8> serialize()
	{
		TArray<uint8> output;

		output.Append(ATcpSocket::Conv_IntToBytes(this->x));
		output.Append(ATcpSocket::Conv_IntToBytes(this->y));
		output.Append(ATcpSocket::Conv_IntToBytes(this->z));

		return output;
	}

};

USTRUCT()
struct FNetDeRegisterRequest
{
	GENERATED_BODY()

	uint32 chunkId;

	FNetDeRegisterRequest()
	{
	}

	FNetDeRegisterRequest(int id)
	{
		chunkId = id;
	}

	TArray<uint8> serialize()
	{
		TArray<uint8> output;

		output.Append(ATcpSocket::Conv_IntToBytes(this->chunkId));


		return output;
	}

};


UCLASS()
class VOXELGAME_API AVoxelTcpSocket : public ATcpSocket
{
	GENERATED_BODY()
public:
	UFUNCTION()
		void OnConnected(int32 ConnectionId);

	UFUNCTION()
		void OnDisconnected(int32 ConId);

	UFUNCTION()
		void OnMessageReceived(int32 ConId, TArray<uint8>& Message);

	UFUNCTION(BlueprintCallable)
		void ConnectToGameServer();

	UFUNCTION()
		TArray<FNetPayload> getPayloadQueue() {
		return payloadQueue;
	}

	UFUNCTION()
		FNetPayload PopFromPayloadQueue()
	{
		return payloadQueue.Pop();
	}


	UPROPERTY()
		int32 connectionIdGameServer;

private:
	UPROPERTY()
		TArray<FNetPayload> payloadQueue;

};