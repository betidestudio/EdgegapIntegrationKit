// Copyright (c) 2024 Betide Studio. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class EdgegapIntegrationKit : ModuleRules
{
    public string ProjectRoot
    {
        get
        {
            return System.IO.Path.GetFullPath(
                System.IO.Path.Combine(ModuleDirectory, "../../")
            );
        }
    }

    public EdgegapIntegrationKit(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Json",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "JsonUtilities",
                "Json",
                "HTTP",
                "Sockets",
                "Networking"
            }
        );

        // Platform and server check
        if ((Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64) && Target.bWithServerCode)
        {
            string pluginDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
            string steamClientDir = Path.Combine(pluginDir, "Extras", "steamclient.so");
            string steamAppIDPath = Path.Combine(pluginDir, "Extras", "steam_appid.txt");

            // Handle steamclient.so
            if (File.Exists(steamClientDir))
            {
                Console.WriteLine("EdgegapSteam: Steam client file exists at the specified source path.");
                RuntimeDependencies.Add("$(TargetOutputDir)/steamclient.so", steamClientDir);
            }
            else
            {
                Console.WriteLine("EdgegapSteam: Steam client file does not exist. Skipping.");
            }

            // Handle steam_appid.txt
            if (File.Exists(steamAppIDPath))
            {
                string destinationPath = Path.Combine("$(TargetOutputDir)", "steam_appid.txt");
                RuntimeDependencies.Add(destinationPath, steamAppIDPath);
                Console.WriteLine("EdgegapSteam: steam_appid.txt copied to ShippingServer directory.");
            }
            else
            {
                Console.WriteLine("EdgegapSteam: steam_appid.txt not found in Extras folder. Skipping copy.");
            }
        }
        else
        {
            Console.WriteLine("EdgegapSteam: Not a Linux platform or not with server code. Skipping Steam integration.");
        }
    }
}