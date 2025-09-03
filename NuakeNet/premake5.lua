project "NuakeNet"
    language "C#"
    dotnetframework "net8.0"
    kind "SharedLib"
	    clr "Unsafe"
	
    targetdir (binaryOutputDir)
    objdir (intBinaryOutputDir)
    debugdir (binaryOutputDir)

    -- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

    vsprops {
       AppendTargetFrameworkToOutputPath =  "false",
       Nullable = "enable",
       CopyLocalLockFileAssemblies = "true",
       EnableDynamicLoading = "true"
    }

    files 
    {
        "Source/**.cs"
    }
    
	links 
    {
        "Coral.Managed"
    }

    prebuildcommands {
        'dotnet dotnet run --project %{wks.location}NuakeNetGenerator/NuakeNetGenerator.csproj'
    }

    postbuildcommands {
        '{COPYFILE} "%{wks.location}NuakeNet/Build/%{cfg.buildcfg}/Binaries/NuakeNet.dll" "%{wks.location}Editor/Build/%{cfg.buildcfg}/Binaries/NuakeNet.dll"'
    }

