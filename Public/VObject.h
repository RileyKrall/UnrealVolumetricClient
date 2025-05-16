// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VGridComponent.h"
#include "MarchingCubesUtil.h"
#include "Engine.h"
#include "VObject.generated.h"

USTRUCT()
struct FTypeBuffer {
	GENERATED_USTRUCT_BODY();
	FTypeBuffer() {
		NumOfVertices = 0;
	}

	//Proc Mesh Inputs
	int NumOfVertices;
	TArray<int32> Triangles;
	TArray<FVector> ShiftedVertices;
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
};

USTRUCT()
struct FVObjectSettings {
	GENERATED_USTRUCT_BODY();

	FVObjectSettings() {
	}

	FVObjectSettings(int chunkRes, int voxelResolutionPerChunk, int voxelScale, float surfaceValue, bool bUseChunkedLoad, bool bUseVoxelSmoothing, bool bCalculateCollion, UDataTable* VoxelTypeMaterialList) {
		chunkResolution = chunkRes;
		voxelResPerChunk = voxelResolutionPerChunk;
		unitScale = voxelScale;
		densityValue = surfaceValue;
		bUseChunkedLoading = bUseChunkedLoad;
		bUseVoxelInterpolation = bUseVoxelSmoothing;
		bCalcCollision = bCalculateCollion;
		MaterialsForVoxelTypes = VoxelTypeMaterialList;
	}

	UPROPERTY()
		int chunkResolution;
	UPROPERTY()
		int voxelResPerChunk;
	UPROPERTY()
		int unitScale;
	UPROPERTY()
		float densityValue;
	UPROPERTY()
		bool bUseChunkedLoading;
	UPROPERTY()
		bool bUseVoxelInterpolation;
	UPROPERTY()
		bool bCalcCollision;
	UPROPERTY()
		UDataTable* MaterialsForVoxelTypes;
};

UCLASS()
class VOXELGAME_API AVObject : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
		UVGridComponent* storage;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	/*
	================ Object Initialization ================
	*/

	UPROPERTY()
		bool bFinishedInitialLoad = false;

	void initializeObject(FVObjectSettings parameters);

	FTypeToMaterialMap GenerateColorMap();
	/*
	================ Public Editing of VObject ================
	*/

	UFUNCTION()
		void FillVoxel(int x, int y, int z, EVoxelType type);

	UFUNCTION()
		FVector voxelPointFromWorldPosition(int x, int y, int z);

	UFUNCTION()
		void RemoveVoxel(int x, int y, int z);

	UFUNCTION()
		void SetPoint(int x, int y, int z, FPoint point);

	UFUNCTION()
		void SetPointInChunk(int x, int y, int z, int chunkId, FPoint point);

	UFUNCTION()
		FPoint GetPoint(int x, int y, int z);

	UFUNCTION()
		void SetChunk(int x, int y, int z, TArray<uint8> densities, TArray<uint8> materials);

	UFUNCTION()
		bool containsChunk(int x, int y, int z);

	UFUNCTION()
		FVector getChunkCoordinatesFromWorldLocation(FVector worldLocation);

	UFUNCTION()
		FVector getChunkCoordinatesFromVoxelPoint(FVector point);

	UFUNCTION()
		bool isInRenderDistance(int x, int y, int z);

	UFUNCTION()
		bool isInStorageDistance(int x, int y, int z);


	/*
	================ Parameters ================
	*/

	UFUNCTION()
		void setVoxelScale(int newScale) { params.unitScale = newScale; }

	UFUNCTION()
		int getVoxelScale() { return params.unitScale; }

	UFUNCTION()
		int getVoxelResolution() { return params.voxelResPerChunk * params.chunkResolution; }


	/*
	================ Draw Chunks ================
	*/
	UFUNCTION()
		void ChangeAffectedChunks();

	/*
	================ Misc ================
	*/

	UPROPERTY(EditAnywhere)
		USceneComponent* root;

private:

	UPROPERTY()
		FVObjectSettings params;

	UPROPERTY()
		FVector centerChunk;

public:
	bool IsFinishedInitialLoad() const
	{
		return bFinishedInitialLoad;
	}

	void SetFinishedInitialLoad(bool isFinishedInitialLoad)
	{
		this->bFinishedInitialLoad = bFinishedInitialLoad;
	}

	FVector GetCenterChunk();

	void SetCenterChunk(const FVector& CenterChunk);

	TArray<uint32> getChunkSet();

	uint32 getChunkId(int x, int y, int z);

	FVector getChunkOffset(uint32 chunkId);

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
		UMarchingCubesUtil* MarchingCubesUtil;

	UPROPERTY()
		TArray<UProceduralMeshComponent*> ChunkMeshes;

	UPROPERTY()
		TMap<int, UProceduralMeshComponent*> ChunkMeshMap;

	UPROPERTY()
		TSet<FVector> ChunksDrawn;

	UPROPERTY()
		FTypeToMaterialMap mapVoxelTypeToMaterial;

	void DrawChunk(FChunk* chunk);

};


