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

        PublicIncludePaths.AddRange(
            new string[] {
                // Add public include paths required here ...
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
                // Add other private include paths required here ...
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Json",
                // Add other public dependencies that you statically link with here ...
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
                // Add private dependencies that you statically link with here ...
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // Add any modules that your module loads dynamically here ...
            }
        );

        // Platform and server check
        if ((Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64) && Target.bWithServerCode)
        {
            string pluginDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
            string steamClientDir = Path.Combine(pluginDir, "Extras", "steamclient.so");
            string dockerfilePath = Path.Combine(pluginDir, "Dockerfile");

            if (File.Exists(steamClientDir))
            {
                Console.WriteLine("EdgegapSteam: Steam client file exists at the specified source path.");

                // Add runtime dependency
                RuntimeDependencies.Add("$(TargetOutputDir)/steamclient.so", steamClientDir);

                // Logic to add to Dockerfile
                string logicToAdd = $@"
RUN mkdir -p /home/ubuntu/.steam/sdk64
RUN find /app -name steamclient.so -exec cp {{}} /home/ubuntu/.steam/sdk64/steamclient.so \;
RUN chmod 755 /home/ubuntu/.steam/sdk64/steamclient.so
";

                UpdateDockerfile(dockerfilePath, logicToAdd);
            }
            else
            {
                Console.WriteLine("EdgegapSteam: Steam client file does not exist. Removing related logic.");

                // Remove Steam logic from Dockerfile if exists
                RemoveSteamLogicFromDockerfile(dockerfilePath);
            }
        }
        else
        {
            Console.WriteLine("EdgegapSteam: Not a Linux platform or not with server code. Removing related logic.");

            // Remove Steam logic from Dockerfile if exists
            string dockerfilePath = Path.Combine(ProjectRoot, "Dockerfile");
            RemoveSteamLogicFromDockerfile(dockerfilePath);
        }
    }

private void UpdateDockerfile(string dockerfilePath, string logicToAdd)
{
    try
    {
        if (File.Exists(dockerfilePath))
        {
            string dockerfileContent = File.ReadAllText(dockerfilePath);

            // Check if the logic already exists in the Dockerfile
            if (!dockerfileContent.Contains(logicToAdd.Trim()))
            {
                string[] dockerfileLines = dockerfileContent.Split('\n');
                bool logicInserted = false;

                // Use a temporary writer to rewrite the file
                using (StreamWriter writer = new StreamWriter(dockerfilePath))
                {
                    foreach (string line in dockerfileLines)
                    {
                        // Trim each line to avoid whitespace issues
                        string trimmedLine = line.Trim();

                        // Write the current line to the file
                        if (!string.IsNullOrEmpty(trimmedLine))
                        {
                            writer.WriteLine(trimmedLine);
                        }

                        // Insert new logic after a specific marker, if applicable
                        if (!logicInserted && trimmedLine.StartsWith("USER m", StringComparison.OrdinalIgnoreCase))
                        {
                            writer.WriteLine(logicToAdd.Trim());
                            logicInserted = true;
                        }
                    }

                    // If no insertion point was found, append the logic at the end
                    if (!logicInserted)
                    {
                        writer.WriteLine();
                        writer.WriteLine("# Added by EdgegapIntegrationKit");
                        writer.WriteLine(logicToAdd.Trim());
                    }
                }

                Console.WriteLine("EdgegapSteam: Dockerfile updated successfully.");
            }
            else
            {
                Console.WriteLine("EdgegapSteam: Dockerfile already contains the required logic. Skipping addition.");
            }
        }
        else
        {
            Console.WriteLine("EdgegapSteam: Dockerfile not found. Creating a new one.");

            // Create a new Dockerfile with the logic
            using (StreamWriter writer = new StreamWriter(dockerfilePath))
            {
                writer.WriteLine("# Generated Dockerfile");
                writer.WriteLine();
                writer.WriteLine(logicToAdd.Trim());
            }
        }
    }
    catch (Exception e)
    {
        Console.WriteLine("EdgegapSteam: Error updating Dockerfile: " + e.Message);
    }
}

    private void RemoveSteamLogicFromDockerfile(string dockerfilePath)
    {
        try
        {
            if (File.Exists(dockerfilePath))
            {
                string dockerfileContent = File.ReadAllText(dockerfilePath);

                // Lines to look for and remove
                string[] steamLogicLines = new string[]
                {
                    "RUN mkdir -p /home/ubuntu/.steam/sdk64",
                    "RUN find /app -name steamclient.so -exec cp {} /home/ubuntu/.steam/sdk64/steamclient.so \\;",
                    "RUN chmod 755 /home/ubuntu/.steam/sdk64/steamclient.so"
                };

                // Rewrite the Dockerfile without Steam-related logic
                string[] dockerfileLines = dockerfileContent.Split('\n');
                using (StreamWriter writer = new StreamWriter(dockerfilePath))
                {
                    foreach (string line in dockerfileLines)
                    {
                        // Trim the line to remove leading/trailing whitespace
                        string trimmedLine = line.Trim();

                        // Skip empty lines or lines that match Steam logic
                        if (string.IsNullOrEmpty(trimmedLine) || 
                            Array.Exists(steamLogicLines, steamLine => trimmedLine.Contains(steamLine)))
                        {
                            continue;
                        }

                        // Write non-blank, non-Steam lines back to the file
                        writer.WriteLine(trimmedLine);
                    }
                }

                Console.WriteLine("EdgegapSteam: Steam-related logic removed from Dockerfile.");
            }
            else
            {
                Console.WriteLine("EdgegapSteam: Dockerfile not found, nothing to remove.");
            }
        }
        catch (Exception e)
        {
            Console.WriteLine("EdgegapSteam: Error while removing Steam logic from Dockerfile: " + e.Message);
        }
    }
}