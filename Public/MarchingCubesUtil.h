// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProceduralMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine.h"
#include "MarchingCubesUtil.generated.h"

UENUM(BlueprintType)
enum class EVoxelType : uint8
{
	Air,
	Ground,
	Stone,
	Ice,
};
USTRUCT(Blueprintable)
struct FVoxelTypeMaterial : public FTableRowBase {
	GENERATED_USTRUCT_BODY();
	FVoxelTypeMaterial() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EVoxelType type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterial* material;
};
USTRUCT()
struct FTypeToMaterialMap {
	GENERATED_USTRUCT_BODY();

	FTypeToMaterialMap() {}

	void mapTypeToColor(EVoxelType type, UMaterial* material) {
		materialMap.Add(type, material);
	}

	UPROPERTY()
		TMap<EVoxelType, UMaterial*> materialMap;
};
//Struct to create 2D array for triangle lookup table
USTRUCT()
struct FNestedArray {
	GENERATED_USTRUCT_BODY();

	FNestedArray() {}

	FNestedArray(TArray<int> Triangles) {
		TriangleList = Triangles;
	}

	UPROPERTY()
		TArray<int> TriangleList;
};
USTRUCT()
struct FPoint {
	GENERATED_USTRUCT_BODY();


	FPoint() {
		type = EVoxelType::Air;
		density = 100;
	}

	FPoint(EVoxelType vertexType, float vertexDensity) {

		type = vertexType;
		density = vertexDensity;

	}

	UPROPERTY()
		EVoxelType type;
	UPROPERTY()
		uint8 density;
};
USTRUCT()
struct FVoxel {
	GENERATED_USTRUCT_BODY();


	FVoxel() {
		shape = 0;
		pointArray.Init(FPoint(), 8);
	}

	FVoxel(TArray<FPoint> vertices) {
		shape = 0;
		for (int i = 0; i < 8; i++) {
			pointArray.Add(vertices[i]);
		}
		calcShape();
	}

	void calcShape() {
		shape = 0;
		for (int i = 0; i < 8; i++) {
			if (pointArray[i].type != EVoxelType::Air) { shape += FMath::Exp2(i); }
		}
	}

	UPROPERTY()
		int shape;
	UPROPERTY()
		TArray<FPoint> pointArray;
};
USTRUCT()
struct FChunk {
	GENERATED_USTRUCT_BODY();


	FChunk() {
		offset = FVector().ZeroVector;
		resolution = 0;
	}

	FChunk(FVector chunkOrigin, int chunkResolution) {
		offset = chunkOrigin;
		resolution = chunkResolution;
		voxelArray.Init(FVoxel(), chunkResolution * chunkResolution * chunkResolution);
	}

	UPROPERTY()
	bool bIsEmpty = true;
	UPROPERTY()
	FVector offset;
	UPROPERTY()
	int resolution;
	UPROPERTY()
	TArray<FVoxel> voxelArray;
};
/*
USTRUCT()
struct FVoxelTemplate {

	GENERATED_USTRUCT_BODY();

	FVoxelTemplate() {

	}

	FVoxelTemplate(int voxelResolution, FString templateName) {
		resolution = FMath::Min(voxelResolution, maxChunkOverlap * 32);
	}

	FString name = "";

	// This value must be less than the difference between draw distance and load distance.
	// This makes sure the template can be loaded over multiple chunks, while affecting the visible chunks
	const int maxChunkOverlap = 3;

	int resolution = 0;

	// Storage of voxels in template.
	TArray<FVoxel> voxels;
};


 */
UCLASS()
class VOXELGAME_API UMarchingCubesUtil : public UObject
{
	GENERATED_BODY()


public:

	int static const VOXEL_TYPE_COUNT = 4;

	uint16 const CHUNK_MAX_RES = 256;

	UMarchingCubesUtil();

	//EdgeMidPoints element getter
	FVector GetEdgeOffset(int32 edge);

	FVector GetVoxelVertex(int vertexIndex) { return voxelVertices[vertexIndex]; }

	//edgeMap element getter
	TArray<int> GetMCTrianglePoints(int MCShape);

	//Calc array index based on x, y, z
	uint32 mortonEncode(uint32 x, uint32 y, uint32 z, uint32 chunkResolution);

	uint32 GetAncestorMorton(uint32 x, uint32 y, uint32 z, uint8 level);

	FVector2D GetVerticesForMidPoints(int v) { return VerticesForMidPoints[v]; }

	int getNumVoxelTypes() { return VOXEL_TYPE_COUNT; }


private:
	//Marching Cubes Lookup Tables
	UPROPERTY()
		TArray<FNestedArray> edgeMap = { FNestedArray({-1, -1, -1}),
					FNestedArray({0, 8, 3}),
					FNestedArray({0, 1, 9}),
					FNestedArray({1, 8, 3, 9, 8, 1}),
					FNestedArray({1, 2, 10}),
					FNestedArray({0, 8, 3, 1, 2, 10}),
					FNestedArray({9, 2, 10, 0, 2, 9}),
					FNestedArray({2, 8, 3, 2, 10, 8, 10, 9, 8}),
					FNestedArray({3, 11, 2}),
					FNestedArray({0, 11, 2, 8, 11, 0}),
					FNestedArray({1, 9, 0, 2, 3, 11}),
					FNestedArray({1, 11, 2, 1, 9, 11, 9, 8, 11}),
					FNestedArray({3, 10, 1, 11, 10, 3}),
					FNestedArray({0, 10, 1, 0, 8, 10, 8, 11, 10}),
					FNestedArray({3, 9, 0, 3, 11, 9, 11, 10, 9}),
					FNestedArray({9, 8, 10, 10, 8, 11}),
					FNestedArray({4, 7, 8}),
					FNestedArray({4, 3, 0, 7, 3, 4}),
					FNestedArray({0, 1, 9, 8, 4, 7}),
					FNestedArray({4, 1, 9, 4, 7, 1, 7, 3, 1}),
					FNestedArray({1, 2, 10, 8, 4, 7}),
					FNestedArray({3, 4, 7, 3, 0, 4, 1, 2, 10}),
					FNestedArray({9, 2, 10, 9, 0, 2, 8, 4, 7}),
					FNestedArray({2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4}),
					FNestedArray({8, 4, 7, 3, 11, 2}),
					FNestedArray({11, 4, 7, 11, 2, 4, 2, 0, 4}),
					FNestedArray({9, 0, 1, 8, 4, 7, 2, 3, 11}),
					FNestedArray({4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1}),
					FNestedArray({3, 10, 1, 3, 11, 10, 7, 8, 4}),
					FNestedArray({1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4}),
					FNestedArray({4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3}),
					FNestedArray({4, 7, 11, 4, 11, 9, 9, 11, 10}),
					FNestedArray({9, 5, 4}),
					FNestedArray({9, 5, 4, 0, 8, 3}),
					FNestedArray({0, 5, 4, 1, 5, 0}),
					FNestedArray({8, 5, 4, 8, 3, 5, 3, 1, 5}),
					FNestedArray({1, 2, 10, 9, 5, 4}),
					FNestedArray({3, 0, 8, 1, 2, 10, 4, 9, 5}),
					FNestedArray({5, 2, 10, 5, 4, 2, 4, 0, 2}),
					FNestedArray({2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8}),
					FNestedArray({9, 5, 4, 2, 3, 11}),
					FNestedArray({0, 11, 2, 0, 8, 11, 4, 9, 5}),
					FNestedArray({0, 5, 4, 0, 1, 5, 2, 3, 11}),
					FNestedArray({2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5}),
					FNestedArray({10, 3, 11, 10, 1, 3, 9, 5, 4}),
					FNestedArray({4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10}),
					FNestedArray({5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3}),
					FNestedArray({5, 4, 8, 5, 8, 10, 10, 8, 11}),
					FNestedArray({9, 7, 8, 5, 7, 9}),
					FNestedArray({9, 3, 0, 9, 5, 3, 5, 7, 3}),
					FNestedArray({0, 7, 8, 0, 1, 7, 1, 5, 7}),
					FNestedArray({1, 5, 3, 3, 5, 7}),
					FNestedArray({9, 7, 8, 9, 5, 7, 10, 1, 2}),
					FNestedArray({10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3}),
					FNestedArray({8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2}),
					FNestedArray({2, 10, 5, 2, 5, 3, 3, 5, 7}),
					FNestedArray({7, 9, 5, 7, 8, 9, 3, 11, 2}),
					FNestedArray({9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11}),
					FNestedArray({2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7}),
					FNestedArray({11, 2, 1, 11, 1, 7, 7, 1, 5}),
					FNestedArray({9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11}),
					FNestedArray({5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0}),
					FNestedArray({11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0}),
					FNestedArray({11, 10, 5, 7, 11, 5}),
					FNestedArray({10, 6, 5}),
					FNestedArray({0, 8, 3, 5, 10, 6}),
					FNestedArray({9, 0, 1, 5, 10, 6}),
					FNestedArray({1, 8, 3, 1, 9, 8, 5, 10, 6}),
					FNestedArray({1, 6, 5, 2, 6, 1}),
					FNestedArray({1, 6, 5, 1, 2, 6, 3, 0, 8}),
					FNestedArray({9, 6, 5, 9, 0, 6, 0, 2, 6}),
					FNestedArray({5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8}),
					FNestedArray({2, 3, 11, 10, 6, 5}),
					FNestedArray({11, 0, 8, 11, 2, 0, 10, 6, 5}),
					FNestedArray({0, 1, 9, 2, 3, 11, 5, 10, 6}),
					FNestedArray({5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11}),
					FNestedArray({6, 3, 11, 6, 5, 3, 5, 1, 3}),
					FNestedArray({0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6}),
					FNestedArray({3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9}),
					FNestedArray({6, 5, 9, 6, 9, 11, 11, 9, 8}),
					FNestedArray({5, 10, 6, 4, 7, 8}),
					FNestedArray({4, 3, 0, 4, 7, 3, 6, 5, 10}),
					FNestedArray({1, 9, 0, 5, 10, 6, 8, 4, 7}),
					FNestedArray({10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4}),
					FNestedArray({6, 1, 2, 6, 5, 1, 4, 7, 8}),
					FNestedArray({1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7}),
					FNestedArray({8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6}),
					FNestedArray({7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9}),
					FNestedArray({3, 11, 2, 7, 8, 4, 10, 6, 5}),
					FNestedArray({5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11}),
					FNestedArray({0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6}),
					FNestedArray({9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6}),
					FNestedArray({8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6}),
					FNestedArray({5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11}),
					FNestedArray({0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7}),
					FNestedArray({6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9}),
					FNestedArray({10, 4, 9, 6, 4, 10}),
					FNestedArray({4, 10, 6, 4, 9, 10, 0, 8, 3}),
					FNestedArray({10, 0, 1, 10, 6, 0, 6, 4, 0}),
					FNestedArray({8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10}),
					FNestedArray({1, 4, 9, 1, 2, 4, 2, 6, 4}),
					FNestedArray({3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4}),
					FNestedArray({0, 2, 4, 4, 2, 6}),
					FNestedArray({8, 3, 2, 8, 2, 4, 4, 2, 6}),
					FNestedArray({10, 4, 9, 10, 6, 4, 11, 2, 3}),
					FNestedArray({0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6}),
					FNestedArray({3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10}),
					FNestedArray({6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1}),
					FNestedArray({9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3}),
					FNestedArray({8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1}),
					FNestedArray({3, 11, 6, 3, 6, 0, 0, 6, 4}),
					FNestedArray({6, 4, 8, 11, 6, 8}),
					FNestedArray({7, 10, 6, 7, 8, 10, 8, 9, 10}),
					FNestedArray({0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10}),
					FNestedArray({10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0}),
					FNestedArray({10, 6, 7, 10, 7, 1, 1, 7, 3}),
					FNestedArray({1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7}),
					FNestedArray({2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9}),
					FNestedArray({7, 8, 0, 7, 0, 6, 6, 0, 2}),
					FNestedArray({7, 3, 2, 6, 7, 2}),
					FNestedArray({2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7}),
					FNestedArray({2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7}),
					FNestedArray({1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11}),
					FNestedArray({11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1}),
					FNestedArray({8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6}),
					FNestedArray({0, 9, 1, 11, 6, 7}),
					FNestedArray({7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0}),
					FNestedArray({7, 11, 6}),
					FNestedArray({7, 6, 11}),
					FNestedArray({3, 0, 8, 11, 7, 6}),
					FNestedArray({0, 1, 9, 11, 7, 6}),
					FNestedArray({8, 1, 9, 8, 3, 1, 11, 7, 6}),
					FNestedArray({10, 1, 2, 6, 11, 7}),
					FNestedArray({1, 2, 10, 3, 0, 8, 6, 11, 7}),
					FNestedArray({2, 9, 0, 2, 10, 9, 6, 11, 7}),
					FNestedArray({6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8}),
					FNestedArray({7, 2, 3, 6, 2, 7}),
					FNestedArray({7, 0, 8, 7, 6, 0, 6, 2, 0}),
					FNestedArray({2, 7, 6, 2, 3, 7, 0, 1, 9}),
					FNestedArray({1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6}),
					FNestedArray({10, 7, 6, 10, 1, 7, 1, 3, 7}),
					FNestedArray({10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8}),
					FNestedArray({0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7}),
					FNestedArray({7, 6, 10, 7, 10, 8, 8, 10, 9}),
					FNestedArray({6, 8, 4, 11, 8, 6}),
					FNestedArray({3, 6, 11, 3, 0, 6, 0, 4, 6}),
					FNestedArray({8, 6, 11, 8, 4, 6, 9, 0, 1}),
					FNestedArray({9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6}),
					FNestedArray({6, 8, 4, 6, 11, 8, 2, 10, 1}),
					FNestedArray({1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6}),
					FNestedArray({4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9}),
					FNestedArray({10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3}),
					FNestedArray({8, 2, 3, 8, 4, 2, 4, 6, 2}),
					FNestedArray({0, 4, 2, 4, 6, 2}),
					FNestedArray({1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8}),
					FNestedArray({1, 9, 4, 1, 4, 2, 2, 4, 6}),
					FNestedArray({8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1}),
					FNestedArray({10, 1, 0, 10, 0, 6, 6, 0, 4}),
					FNestedArray({4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3}),
					FNestedArray({10, 9, 4, 6, 10, 4}),
					FNestedArray({4, 9, 5, 7, 6, 11}),
					FNestedArray({0, 8, 3, 4, 9, 5, 11, 7, 6}),
					FNestedArray({5, 0, 1, 5, 4, 0, 7, 6, 11}),
					FNestedArray({11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5}),
					FNestedArray({9, 5, 4, 10, 1, 2, 7, 6, 11}),
					FNestedArray({6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5}),
					FNestedArray({7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2}),
					FNestedArray({3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6}),
					FNestedArray({7, 2, 3, 7, 6, 2, 5, 4, 9}),
					FNestedArray({9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7}),
					FNestedArray({3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0}),
					FNestedArray({6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8}),
					FNestedArray({9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7}),
					FNestedArray({1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4}),
					FNestedArray({4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10}),
					FNestedArray({7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10}),
					FNestedArray({6, 9, 5, 6, 11, 9, 11, 8, 9}),
					FNestedArray({3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5}),
					FNestedArray({0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11}),
					FNestedArray({6, 11, 3, 6, 3, 5, 5, 3, 1}),
					FNestedArray({1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6}),
					FNestedArray({0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10}),
					FNestedArray({11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5}),
					FNestedArray({6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3}),
					FNestedArray({5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2}),
					FNestedArray({9, 5, 6, 9, 6, 0, 0, 6, 2}),
					FNestedArray({1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8}),
					FNestedArray({1, 5, 6, 2, 1, 6}),
					FNestedArray({1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6}),
					FNestedArray({10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0}),
					FNestedArray({0, 3, 8, 5, 6, 10}),
					FNestedArray({10, 5, 6}),
					FNestedArray({11, 5, 10, 7, 5, 11}),
					FNestedArray({11, 5, 10, 11, 7, 5, 8, 3, 0}),
					FNestedArray({5, 11, 7, 5, 10, 11, 1, 9, 0}),
					FNestedArray({10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1}),
					FNestedArray({11, 1, 2, 11, 7, 1, 7, 5, 1}),
					FNestedArray({0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11}),
					FNestedArray({9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7}),
					FNestedArray({7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2}),
					FNestedArray({2, 5, 10, 2, 3, 5, 3, 7, 5}),
					FNestedArray({8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5}),
					FNestedArray({9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2}),
					FNestedArray({9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2}),
					FNestedArray({1, 3, 5, 3, 7, 5}),
					FNestedArray({0, 8, 7, 0, 7, 1, 1, 7, 5}),
					FNestedArray({9, 0, 3, 9, 3, 5, 5, 3, 7}),
					FNestedArray({9, 8, 7, 5, 9, 7}),
					FNestedArray({5, 8, 4, 5, 10, 8, 10, 11, 8}),
					FNestedArray({5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0}),
					FNestedArray({0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5}),
					FNestedArray({10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4}),
					FNestedArray({2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8}),
					FNestedArray({0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11}),
					FNestedArray({0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5}),
					FNestedArray({9, 4, 5, 2, 11, 3}),
					FNestedArray({2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4}),
					FNestedArray({5, 10, 2, 5, 2, 4, 4, 2, 0}),
					FNestedArray({3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9}),
					FNestedArray({5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2}),
					FNestedArray({8, 4, 5, 8, 5, 3, 3, 5, 1}),
					FNestedArray({0, 4, 5, 1, 0, 5}),
					FNestedArray({8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5}),
					FNestedArray({9, 4, 5}),
					FNestedArray({4, 11, 7, 4, 9, 11, 9, 10, 11}),
					FNestedArray({0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11}),
					FNestedArray({1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11}),
					FNestedArray({3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4}),
					FNestedArray({4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2}),
					FNestedArray({9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3}),
					FNestedArray({11, 7, 4, 11, 4, 2, 2, 4, 0}),
					FNestedArray({11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4}),
					FNestedArray({2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9}),
					FNestedArray({9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7}),
					FNestedArray({3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10}),
					FNestedArray({1, 10, 2, 8, 7, 4}),
					FNestedArray({4, 9, 1, 4, 1, 7, 7, 1, 3}),
					FNestedArray({4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1}),
					FNestedArray({4, 0, 3, 7, 4, 3}),
					FNestedArray({4, 8, 7}),
					FNestedArray({9, 10, 8, 10, 11, 8}),
					FNestedArray({3, 0, 9, 3, 9, 11, 11, 9, 10}),
					FNestedArray({0, 1, 10, 0, 10, 8, 8, 10, 11}),
					FNestedArray({3, 1, 10, 11, 3, 10}),
					FNestedArray({1, 2, 11, 1, 11, 9, 9, 11, 8}),
					FNestedArray({3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9}),
					FNestedArray({0, 2, 11, 8, 0, 11}),
					FNestedArray({3, 2, 11}),
					FNestedArray({2, 3, 8, 2, 8, 10, 10, 8, 9}),
					FNestedArray({9, 10, 2, 0, 9, 2}),
					FNestedArray({2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8}),
					FNestedArray({1, 10, 2}),
					FNestedArray({1, 3, 8, 9, 1, 8}),
					FNestedArray({0, 9, 1}),
					FNestedArray({0, 3, 8}),
					FNestedArray({-1, -1, -1}) };
	UPROPERTY()
		TArray<FVector> EdgeMidPoints = { FVector(.5, 0, 0),
									FVector(1, .5, 0),
									FVector(.5, 1, 0),
									FVector(0, .5, 0),
									FVector(.5, 0, 1),
									FVector(1, .5, 1),
									FVector(.5, 1, 1),
									FVector(0, .5, 1),
									FVector(0, 0, .5),
									FVector(1, 0, .5),
									FVector(1, 1, .5),
									FVector(0, 1, .5) };
	UPROPERTY()
		TArray<FVector> voxelVertices = {
			FVector().ZeroVector,
			FVector(1, 0, 0),
			FVector(1, 1, 0),
			FVector(0, 1, 0),
			FVector(0, 0, 1),
			FVector(1, 0, 1),
			FVector(1, 1, 1),
			FVector(0, 1, 1)
	};
	UPROPERTY()
		TArray<FVector2D> VerticesForMidPoints = {
			FVector2D(0, 1),
			FVector2D(1, 2),
			FVector2D(2, 3),
			FVector2D(3, 0),
			FVector2D(4, 5),
			FVector2D(5, 6),
			FVector2D(6, 7),
			FVector2D(7, 4),
			FVector2D(0, 4),
			FVector2D(1, 5),
			FVector2D(2, 6),
			FVector2D(3, 7) };
	UPROPERTY()
		TArray<int> mortonX = {
		0x00000000, 0x00000001, 0x00000008, 0x00000009, 0x00000040, 0x00000041, 0x00000048, 0x00000049,  //   0 - 7
		0x00000200, 0x00000201, 0x00000208, 0x00000209, 0x00000240, 0x00000241, 0x00000248, 0x00000249,  //   8 - 15*
		0x00001000, 0x00001001, 0x00001008, 0x00001009, 0x00001040, 0x00001041, 0x00001048, 0x00001049,  //  16 - 23*
		0x00001200, 0x00001201, 0x00001208, 0x00001209, 0x00001240, 0x00001241, 0x00001248, 0x00001249,  //  24 - 31
		0x00008000, 0x00008001, 0x00008008, 0x00008009, 0x00008040, 0x00008041, 0x00008048, 0x00008049,  //  32 - 39*
		0x00008200, 0x00008201, 0x00008208, 0x00008209, 0x00008240, 0x00008241, 0x00008248, 0x00008249,  //  40 - 47
		0x00009000, 0x00009001, 0x00009008, 0x00009009, 0x00009040, 0x00009041, 0x00009048, 0x00009049,  //  48 - 55
		0x00009200, 0x00009201, 0x00009208, 0x00009209, 0x00009240, 0x00009241, 0x00009248, 0x00009249,  //  56 - 63
		0x00040000, 0x00040001, 0x00040008, 0x00040009, 0x00040040, 0x00040041, 0x00040048, 0x00040049,  //  64 - 71*
		0x00040200, 0x00040201, 0x00040208, 0x00040209, 0x00040240, 0x00040241, 0x00040248, 0x00040249,  //  72 - 79
		0x00041000, 0x00041001, 0x00041008, 0x00041009, 0x00041040, 0x00041041, 0x00041048, 0x00041049,  //  80 - 87
		0x00041200, 0x00041201, 0x00041208, 0x00041209, 0x00041240, 0x00041241, 0x00041248, 0x00041249,  //  88 - 95
		0x00048000, 0x00048001, 0x00048008, 0x00048009, 0x00048040, 0x00048041, 0x00048048, 0x00048049,  //  96 - 103
		0x00048200, 0x00048201, 0x00048208, 0x00048209, 0x00048240, 0x00048241, 0x00048248, 0x00048249,  // 104 - 111
		0x00049000, 0x00049001, 0x00049008, 0x00049009, 0x00049040, 0x00049041, 0x00049048, 0x00049049,  // 112 - 119
		0x00049200, 0x00049201, 0x00049208, 0x00049209, 0x00049240, 0x00049241, 0x00049248, 0x00049249,  // 120 - 127
		0x00200000, 0x00200001, 0x00200008, 0x00200009, 0x00200040, 0x00200041, 0x00200048, 0x00200049,  // 128 - 135*
		0x00200200, 0x00200201, 0x00200208, 0x00200209, 0x00200240, 0x00200241, 0x00200248, 0x00200249,  // 136 - 143
		0x00201000, 0x00201001, 0x00201008, 0x00201009, 0x00201040, 0x00201041, 0x00201048, 0x00201049,  // 144 - 151
		0x00201200, 0x00201201, 0x00201208, 0x00201209, 0x00201240, 0x00201241, 0x00201248, 0x00201249,  // 152 - 159
		0x00208000, 0x00208001, 0x00208008, 0x00208009, 0x00208040, 0x00208041, 0x00208048, 0x00208049,  // 160 - 167
		0x00208200, 0x00208201, 0x00208208, 0x00208209, 0x00208240, 0x00208241, 0x00208248, 0x00208249,  // 168 - 175
		0x00209000, 0x00209001, 0x00209008, 0x00209009, 0x00209040, 0x00209041, 0x00209048, 0x00209049,  // 176 - 183
		0x00209200, 0x00209201, 0x00209208, 0x00209209, 0x00209240, 0x00209241, 0x00209248, 0x00209249,  // 184 - 191
		0x00240000, 0x00240001, 0x00240008, 0x00240009, 0x00240040, 0x00240041, 0x00240048, 0x00240049,  // 192 - 199
		0x00240200, 0x00240201, 0x00240208, 0x00240209, 0x00240240, 0x00240241, 0x00240248, 0x00240249,  // 200 - 207
		0x00241000, 0x00241001, 0x00241008, 0x00241009, 0x00241040, 0x00241041, 0x00241048, 0x00241049,  // 208 - 215
		0x00241200, 0x00241201, 0x00241208, 0x00241209, 0x00241240, 0x00241241, 0x00241248, 0x00241249,  // 216 - 223
		0x00248000, 0x00248001, 0x00248008, 0x00248009, 0x00248040, 0x00248041, 0x00248048, 0x00248049,  // 224 - 231
		0x00248200, 0x00248201, 0x00248208, 0x00248209, 0x00248240, 0x00248241, 0x00248248, 0x00248249,  // 232 - 239
		0x00249000, 0x00249001, 0x00249008, 0x00249009, 0x00249040, 0x00249041, 0x00249048, 0x00249049,  // 240 - 247
		0x00249200, 0x00249201, 0x00249208, 0x00249209, 0x00249240, 0x00249241, 0x00249248, 0x00249249,  // 248 - 255
	};


	// pre-shifted table for Y coordinates (1 bit to the left)
	UPROPERTY()
		TArray<int> mortonY;

	// Pre-shifted table for z (2 bits to the left)
	UPROPERTY()
		TArray<int> mortonZ;
};
