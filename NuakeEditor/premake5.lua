project "NuakeEditor"
    targetname ("Nuake Engine")

    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"
	
    dependson { "NuakeNet" } 

    targetdir (binaryOutputDir)
    objdir (intBinaryOutputDir)

    --debugdir ("")

    defines 
    {
        table.unpack(globalDefines)
    }

    files
    {
        "Source/**.cpp",
        "Source/**.h",
        
        -- This isn't ideal, but it works...needs a proper way of doing this, but that's for another time
        "Nuake/dependencies/entt/natvis/entt/*.natvis"
    }
    
    includedirs 
    {
        "../Nuake/Source",
    }

    externalincludedirs 
    { 
        "../Nuake/Vendors/",
        "../Nuake/Vendors/volk",
        "../Nuake/Vendors/vulkan",
        "../Nuake/dependencies/assimp/include",
        "../Nuake/dependencies/Coral/Coral.Native/Include/",
        "../Nuake/dependencies/entt/src",
        "../Nuake/dependencies/JoltPhysics",
        "../Nuake/dependencies/recastnavigation/DebugUtils/Include",
        "../Nuake/dependencies/recastnavigation/Detour/Include",
        "../Nuake/dependencies/recastnavigation/DetourTileCache/Include",
        "../Nuake/dependencies/vma/include",
        "../Nuake/dependencies/msdf-atlas-gen",
        "../Nuake/dependencies/msdf-atlas-gen/msdfgen",
        "../Nuake/dependencies/msdf-atlas-gen/msdfgen/include",
        "../Nuake/dependencies/yoga",
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

    prebuildcommands {
        
    }

    postbuildcommands {
        '{ECHO} Copying "%{wks.location}/EditorNet/bin/$(Configuration)/EditorNet.dll" to "%{wks.location}/Editor"',
	    '{COPYFILE} "%{wks.location}/EditorNet/bin/$(Configuration)/EditorNet.dll" "%{wks.location}/Editor"'
    }
