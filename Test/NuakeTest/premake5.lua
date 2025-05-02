project "NuakeTest"
    kind "ConsoleApp"
    staticruntime "On"
    language "C++"
    cppdialect "C++20"
    
    defines
    {
		table.unpack(globalDefines),

        "_MBCS",
		"IMGUI_DEFINE_MATH_OPERATORS",
        "NK_VK",
        "IMGUI_IMPL_VULKAN_NO_PROTOTYPES"
    }

    targetdir (binaryOutputDir)
    objdir (intBinaryOutputDir)
    debugdir (binaryOutputDir)
    
    files
    {
        -- Main Sources
        "Source/**.cpp",
        "Source/**.h",
        "Vendors/**.h",
        "Vendors/**.cpp",
    }

    includedirs
    {
        ".",
        "Source",
        "Vendors",
        "../../Nuake/Source",
        "../../Nuake/Vendors",
        "../../Nuake/Thirdparty/entt/src"
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