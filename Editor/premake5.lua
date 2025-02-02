project "Editor"
    targetname ("Nuake Engine")

    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"
	
    dependson { "NuakeNet" } 

    targetdir (binaryOutputDir)
    objdir (intBinaryOutputDir)
    debugdir (binaryOutputDir)

    defines 
    {
        table.unpack(globalDefines)
    }

    files
    {
        "Source/**.cpp",
        "Source/**.h",
        
        -- This isn't ideal, but it works...needs a proper way of doing this, but that's for another time
        "Nuake/Thirdparty/entt/natvis/entt/*.natvis"
    }
    
    includedirs 
    {
        "../Nuake/Source",
        "../Nuake/Vendors/pugixml",
        "../Nuake/Vendors",
        "../Nuake/Vendors/volk",
        "../Nuake/Vendors/vulkan",
	    "../Nuake/Thirdparty/build",
        "../Nuake/Thirdparty/yoga",
        "../Nuake/Thirdparty/Coral/Coral.Native/Include/",
        "../Nuake/Thirdparty/entt/src",
        "../Nuake/Thirdparty/JoltPhysics",
        "../Nuake/Thirdparty/recastnavigation/DebugUtils/Include",
        "../Nuake/Thirdparty/recastnavigation/Detour/Include",
        "../Nuake/Thirdparty/recastnavigation/DetourTileCache/Include",
        "../Nuake/Thirdparty/vma/include",
        "../Nuake/Thirdparty/msdf-atlas-gen",
        "../Nuake/Thirdparty/msdf-atlas-gen/msdfgen",
        "../Nuake/Thirdparty/msdf-atlas-gen/msdfgen/include",
        "../Nuake/Thirdparty/yoga",
    }

    externalincludedirs 
    { 
        
    }

	links 
    {
        "Nuake",
        "glad", 
        "GLFW",
        "assimp",
        "JoltPhysics",
        "soloud",
        "Coral.Native",
	    "DebugUtils",
	    "Detour",
	    "DetourCrowd",
	    "DetourTileCache",
	    "Recast",
	    "tracy",
        "yoga",
        "msdf-gen",
        "msdf-atlas-gen",
        "Freetype",
        "vma"
    }

    prebuildcommands {
        '{ECHO} "Copying dxcompiler.dll to Working directory..."',
        '{COPYFILE} "%{wks.location}Nuake/Thirdparty/dxc/bin/x64/dxcompiler.dll" "%{cfg.debugdir}/"',
        '{ECHO} Copying Coral to Working directory...',
        '{COPYFILE} "%{wks.location}Nuake/Thirdparty/Coral/Coral.Managed/bin/%{cfg.buildcfg}/Coral.Managed.dll" "%{cfg.debugdir}/"',
        '{COPYFILE} "%{wks.location}Nuake/Thirdparty/Coral/Coral.Managed/bin/%{cfg.buildcfg}/Coral.Managed.runtimeconfig.json" "%{cfg.debugdir}/"',
        '{COPYFILE} "%{wks.location}NuakeNet/Build/%{cfg.buildcfg}/Binaries/NuakeNet.dll" "%{cfg.debugdir}/"',
        'xcopy /E /I /Y "%{wks.location}Data" "%{cfg.debugdir}\\Resources"'
    }

    filter { "system:windows", "action:vs*"}
    flags
    {
        "MultiProcessorCompile",
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

        buildoptions { "/Zi" }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
    
