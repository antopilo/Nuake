-- ╔═══════════════════════════════════════╗
-- ║               ACTIONS                 ║
-- ╚═══════════════════════════════════════╝

include "Scripts/BuildAssets.lua"
newaction {
   trigger     = "build-assets",
   description = "",
   execute     = function ()
      generateStaticResources("Resources", "Nuake/Source/Nuake/Resource/StaticResources.h", "Nuake/Source/Nuake/Resource/StaticResources.cpp")
   end
}

newaction {
    trigger     = "copy-deps",
    description = "Copies the necessary DLLs to the working directory",
    execute     = function ()
        os.copyfile("Nuake/dependencies/dxc/bin/x64/dxcompiler.dll", "Editor/dxcompiler.dll")
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

group "Dependencies"
    include "Nuake/dependencies/glfw_p5.lua"
    include "Nuake/dependencies/glad_p5.lua"
    include "Nuake/dependencies/assimp_p5.lua"
    include "Nuake/dependencies/jolt_p5.lua"
    include "Nuake/dependencies/soloud_p5.lua"
    include "Nuake/dependencies/coral_p5.lua"
    include "Nuake/dependencies/recastnavigation_p5.lua"
    include "Nuake/dependencies/tracy_p5.lua"
    include "Nuake/dependencies/yoga_p5.lua"
    include "Nuake/dependencies/msdf-atlas-gen_p5.lua"
    include "Nuake/dependencies/freetype_p5.lua"
    include "Nuake/dependencies/vma_p5.lua"
group ""

include "Nuake/premake5.lua"
include "NuakeEditor/premake5.lua"
include "NuakeRuntime/premake5.lua"
include "NuakeNet/premake5.lua"
include "EditorNet/premake5.lua"
