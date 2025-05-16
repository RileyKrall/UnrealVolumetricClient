// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoxelGame : ModuleRules
{
	public VoxelGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		//"AdvancedSessions", "AdvancedSteamSessions"
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay",  "OnlineSubsystem", "OnlineSubsystemUtils", "ProceduralMeshComponent", "Sockets",
			"Networking",
			"Json",
			"JsonUtilities" });
		//PublicDependencyModuleNames.Add("RuntimeMeshComponent");

		DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
