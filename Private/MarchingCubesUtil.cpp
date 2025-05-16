// Fill out your copyright notice in the Description page of Project Settings.


#include "MarchingCubesUtil.h"
#include "Net/UnrealNetwork.h"
#include "Engine.h"


UMarchingCubesUtil::UMarchingCubesUtil() {
	//initialize y and z morton lookup tables
	for (int i = 0; i < 256; i++) {
		mortonY.Add(mortonX[i] << 1);
		mortonZ.Add(mortonX[i] << 2);
	}

}

FVector UMarchingCubesUtil::GetEdgeOffset(int32 edgeIndex)
{
	if (EdgeMidPoints.Num() <= edgeIndex || edgeIndex < 0) {
		UE_LOG(LogTemp, Warning, TEXT("Bad Edge Offset"));
		return FVector();
	}
	return EdgeMidPoints[edgeIndex];
}

TArray<int> UMarchingCubesUtil::GetMCTrianglePoints(int MCShape)
{
	if (edgeMap.Num() <= MCShape || MCShape < 0) {
		UE_LOG(LogTemp, Warning, TEXT("Bad Get MC Tri Points"));
		return TArray<int>();
	}
	FNestedArray map = edgeMap[MCShape];
	return map.TriangleList;
}

uint32 UMarchingCubesUtil::mortonEncode(uint32 x, uint32 y, uint32 z, uint32 chunkResolution)
{
	uint32 mortonCode = mortonX[x] | mortonY[y] | mortonZ[z];
	uint32 all_ones = chunkResolution * chunkResolution * chunkResolution - 1;
	mortonCode = mortonCode & all_ones;

	return mortonCode;
}

uint32 UMarchingCubesUtil::GetAncestorMorton(uint32 x, uint32 y, uint32 z, uint8 level)
{
	uint32 morton = mortonEncode(x, y, z, 7);

	morton = morton >> (3 * level);
	return morton;
}
