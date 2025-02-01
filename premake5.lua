-- ╔═══════════════════════════════════════╗
-- ║               ACTIONS                 ║
-- ╚═══════════════════════════════════════╝

include "BuildScripts/BuildAssets.lua"
newaction {
   trigger     = "build-assets",
   description = "",
   execute     = function ()
      generateStaticResources("Data", "Nuake/Source/Nuake/Resource/StaticResources.h", "Nuake/Source/Nuake/Resource/StaticResources.cpp")
   end
}

newaction {
    trigger     = "copy-deps",
    description = "Copies the necessary DLLs to the working directory",
    execute     = function ()
        os.copyfile("Nuake/Thirdparty/dxc/bin/x64/dxcompiler.dll", "Editor/dxcompiler.dll")
    end
}

newaction {
    trigger     = "create-dirs",
    description = "Create the necessary directories",
    execute     = function ()
        os.mkdir("Build")
    end
}

-- ╔═══════════════════════════════════════╗
-- ║               WORKSPACE               ║
-- ╚═══════════════════════════════════════╝
workspace "Nuake"
    conformancemode "On"
	
	startproject "Editor"
	
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    filter "configurations:Dist"
        defines 
        {
            "NK_DIST"
        }

    filter "configurations:Debug"
        defines 
        {
            "NK_DEBUG"
        }

    filter { "language:C++" }
        architecture "x64"

    filter { "language:C" }
        architecture "x64"

outputdir = "Build/%{cfg.buildcfg}/"
binaryOutputDir = outputdir .. "Binaries/"
intBinaryOutputDir = outputdir .. "Binaries-Intermediate/"

globalDefines = {
    "TRACY_ENABLE",
    "TRACY_ON_DEMAND",
    "NK_VK",
    "IMGUI_DEFINE_MATH_OPERATORS",
    "NK_WIN"
}

group "Thirdparty"
    include "Nuake/Thirdparty/glfw_p5.lua"
    include "Nuake/Thirdparty/glad_p5.lua"
    include "Nuake/Thirdparty/assimp_p5.lua"
    include "Nuake/Thirdparty/jolt_p5.lua"
    include "Nuake/Thirdparty/soloud_p5.lua"
    include "Nuake/Thirdparty/coral_p5.lua"
    include "Nuake/Thirdparty/recastnavigation_p5.lua"
    include "Nuake/Thirdparty/tracy_p5.lua"
    include "Nuake/Thirdparty/yoga_p5.lua"
    include "Nuake/Thirdparty/msdf-atlas-gen_p5.lua"
    include "Nuake/Thirdparty/freetype_p5.lua"
    include "Nuake/Thirdparty/vma_p5.lua"
group ""

include "Nuake/premake5.lua"
include "Editor/premake5.lua"
include "Runtime/premake5.lua"
include "NuakeNet/premake5.lua"
include "EditorNet/premake5.lua"
