-- ╔═══════════════════════════════════════╗
-- ║               ACTIONS                 ║
-- ╚═══════════════════════════════════════╝

include "build/BuildAssets.lua"
newaction {
   trigger     = "build-assets",
   description = "",
   execute     = function ()
      generateStaticResources("Resources", "Nuake/src/Resource/StaticResources.h", "Nuake/src/Resource/StaticResources.cpp")
   end
}


-- ╔═══════════════════════════════════════╗
-- ║               WORKSPACE               ║
-- ╚═══════════════════════════════════════╝
workspace "Nuake"
    conformancemode "On"
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

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
local globalDefines = {
    "TRACY_ENABLE",
    "TRACY_ON_DEMAND",
    "NK_VK"
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

include "NuakeNet/premake5.lua"
include "EditorNet/premake5.lua"
include "Nuake/src/Modules/Modules.lua"

project "Nuake"
    location "Nuake"
    kind "StaticLib"
    staticruntime "On"

    language "C++"
    cppdialect "C++20"
    
    local moduleSources = {}
	
	if _ACTION then
        local modulesDir = "Nuake/src/Modules"
        local outputFilePath = path.join(modulesDir, "Modules.cpp")

        -- Load and generate the modules file
        local modules = loadModules(modulesDir)
        moduleSources = generateModulesFile(modules, outputFilePath, "Nuake/src/Modules")
    end
   
    defines
    {
		table.unpack(globalDefines),

        "_MBCS",
		"IMGUI_DEFINE_MATH_OPERATORS",
        "NK_VK",
        "IMGUI_IMPL_VULKAN_NO_PROTOTYPES"
    }

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        -- Main Sources
        "%{prj.name}/Engine.h",
        "%{prj.name}/Engine.cpp",
        "%{prj.name}/src/*.h",
        "%{prj.name}/src/*.cpp",
        "%{prj.name}/src/AI/**.h",
        "%{prj.name}/src/AI/**.cpp",
        "%{prj.name}/src/Application/**.h",
        "%{prj.name}/src/Application/**.cpp",
        "%{prj.name}/src/Audio/**.h",
        "%{prj.name}/src/Audio/**.cpp",
        "%{prj.name}/src/Core/**.h",
        "%{prj.name}/src/Core/**.cpp",
        "%{prj.name}/src/FileSystem/**.h",
        "%{prj.name}/src/FileSystem/**.cpp",
        "%{prj.name}/src/Physics/**.h",
        "%{prj.name}/src/Physics/**.cpp",
        "%{prj.name}/src/Rendering/**.h",
        "%{prj.name}/src/Rendering/**.cpp",
        "%{prj.name}/src/Resource/**.h",
        "%{prj.name}/src/Resource/**.cpp",
        "%{prj.name}/src/Scene/**.h",
        "%{prj.name}/src/Scene/**.cpp",
        "%{prj.name}/src/Scripting/**.h",
        "%{prj.name}/src/Scripting/**.cpp",
        "%{prj.name}/src/Threading/**.h",
        "%{prj.name}/src/Threading/**.cpp",
        "%{prj.name}/src/UI/**.h",
        "%{prj.name}/src/UI/**.cpp",
        "%{prj.name}/src/Subsystems/**.h",
        "%{prj.name}/src/Subsystems/**.cpp",
        "%{prj.name}/src/Vendors/**.h",
        "%{prj.name}/src/Vendors/**.cpp",
        
        -- Vendor Sources
        "%{prj.name}/src/Vendors/libmap/h/*.h",
        "%{prj.name}/src/Vendors/libmap/c/*.c",
        "%{prj.name}/src/Vendors/wren/src/vm/*.h",
        "%{prj.name}/src/Vendors/wren/src/vm/*.c",
        "%{prj.name}/src/Vendors/katana-parser/*.h",
        "%{prj.name}/src/Vendors/katana-parser/*.c",
        "%{prj.name}/src/Vendors/incbin/*.c",
        "%{prj.name}/src/Vendors/incbin/*.h",
        "%{prj.name}/src/Vendors/nanosvg/*.h",
        "%{prj.name}/src/Vendors/volk/*.c",
        "%{prj.name}/src/Vendors/volk/*.h",
        "%{prj.name}/src/Vendors/vkb/*.cpp",
        "%{prj.name}/src/Vendors/vkb/*.h",
        -- Modules System
        "%{prj.name}/src/Modules/Modules.h",
        "%{prj.name}/src/Modules/Modules.cpp",
        table.unpack(moduleSources)
    }

    includedirs
    {
        "%{prj.name}",
        "%{prj.name}/src/Vendors",
        "%{prj.name}/dependencies/glad/include",
        "%{prj.name}/dependencies/glfw/include",
        "%{prj.name}/dependencies/assimp/include",
        "%{prj.name}/dependencies/JoltPhysics",
        "%{prj.name}/src/Vendors/wren/src/include",
        "%{prj.name}/src/Vendors/incbin",
        "%{prj.name}/src/Vendors/nanosvg",
        "%{prj.name}/src/Vendors/vulkan",
        "%{prj.name}/src/Vendors/vkb",
        "%{prj.name}/dependencies/build",
        "%{prj.name}/dependencies/soloud/include",
        "%{prj.name}/dependencies/Coral/Coral.Native/Include",
	    "%{prj.name}/dependencies/recastnavigation/DebugUtils/Include",
	    "%{prj.name}/dependencies/recastnavigation/Detour/Include",       
	    "%{prj.name}/dependencies/recastnavigation/DetourCrowd/Include",
	    "%{prj.name}/dependencies/recastnavigation/DetourTileCache/Include",
	    "%{prj.name}/dependencies/recastnavigation/Recast/Include",
	    "%{prj.name}/dependencies/yoga",
        "%{prj.name}/dependencies/msdf-atlas-gen",
        "%{prj.name}/dependencies/msdf-atlas-gen/msdfgen",
        "%{prj.name}/dependencies/msdf-atlas-gen/msdfgen/include",
        "%{prj.name}/dependencies/freetype/include",
	    "%{prj.name}/dependencies/tracy/public/tracy",
        "%{prj.name}/dependencies/entt/src",
        "%{prj.name}/dependencies/vma/include",
        "%{prj.name}/dependencies/dxc/inc"
    }
    
    libdirs { "%{prj.name}/dependencies/dxc/lib/x64" }

    links
    {
        "soloud",
        "tracy",
        "yoga",
        "vma",
        "dxcompiler"

    }

    filter "system:linux"
        defines 
        {
            "GLFW_STATIC",
            "NK_LINUX"
        }
        
        links 
        {
            "glib-2.0"
        }
        
        buildoptions { "`pkg-config --cflags glib-2.0 pango gdk-pixbuf-2.0 atk`" }
    	linkoptions { "`pkg-config --libs glib-2.0 pango gdk-pixbuf-2.0`" }
        
        includedirs
        {
        	"/usr/include/gtk-3.0/",
        	"/usr/lib/glib-2.0/include",
		    "/usr/include/glib-2.0",
        }

    filter "system:windows"
        staticruntime "On"
        defines
        {
            "NK_WIN"
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

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
        
project "NuakeRuntime"
    location "Runtime"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("Editor")

    files
    {
        "Runtime/Runtime.cpp"
    }

    includedirs 
    {
        "%{prj.name}/../Nuake",
        "%{prj.name}/../Nuake/src/Vendors",
        "%{prj.name}/../Nuake/src/Vendors/nanosvg",
        "%{prj.name}/../Nuake/dependencies/glad/include",
        "%{prj.name}/../Nuake/dependencies/glfw/include",
        "%{prj.name}/../Nuake/dependencies/assimp/include",
        "%{prj.name}/../Nuake/dependencies/build",
	    "%{prj.name}/../Nuake/dependencies/JoltPhysics",
        "%{prj.name}/../Nuake/dependencies/build",
        "%{prj.name}/../Nuake/dependencies/soloud/include",
        "/usr/include/gtk-3.0/",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/DebugUtils/Include",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/Detour/Include",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/DetourCrowd/Include",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/DetourTileCache/Include",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/Recast/Include",
        "%{prj.name}/../Nuake/dependencies/yoga",
        "%{prj.name}/../Nuake/dependencies/msdf-atlas-gen",
        "%{prj.name}/../Nuake/dependencies/msdf-atlas-gen/msdfgen",
        "%{prj.name}/../Nuake/dependencies/msdf-atlas-gen/msdfgen/include",
        "%{prj.name}/../Nuake/dependencies/freetype/include",
	    "%{prj.name}/../Nuake/dependencies/tracy/public/tracy",
        "%{prj.name}/../Nuake/dependencies/entt/src",
		"%{prj.name}/../Nuake/src/Vendors/vulkan",
        "%{prj.name}/../Nuake/src/Vendors/volk",
        "%{prj.name}/../Nuake/dependencies/vma/include"
    }

    libdirs 
    { 
        "%{prj.name}/../Nuake/dependencies/GLEW/lib/Release/x64",
        "%{prj.name}/../Nuake/dependencies/assimp/lib/",
        "%{prj.name}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/",
        "%{prj.name}/../Nuake/src/Vendors/wren/src/include",
        "%{prj.name}/../Nuake/dependencies/JoltPhysics/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/JoltPhysics/",
        "%{prj.name}/../Nuake/dependencies/soloud/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}",
        "%{prj.name}/../Nuake/dependencies/Coral/NetCore/",
        "%{prj.name}/../Nuake/dependencies/freetype/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Freetype"
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
    
    defines {
        table.unpack(globalDefines)
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        defines {
            "NK_WIN"
        }
        links
        {
            "opengl32.lib"
        }

        externalincludedirs { "%{prj.name}/../Nuake/dependencies/Coral/Coral.Native/Include/" }
    
		--[[
        postbuildcommands {
            '{COPYFILE} "%{wks.location}/Nuake/dependencies/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}/%{prj.name}"'
        }
		]]--

    filter { "system:windows", "action:vs*" }
        flags
        {
            "MultiProcessorCompile",
        }

    filter "system:linux"
        links
        {
            "GL",
            "glfw",
            "glad",
            "X11",
            "asound",
            "glib-2.0",
            "gtk-3",
		    "gobject-2.0"
        }

        
        includedirs
        {
        	"/usr/include/gtk-3.0/",
        	"/usr/lib/glib-2.0/include",
		    "/usr/include/glib-2.0",
        }
        
        buildoptions { "`pkg-config --cflags glib-2.0 pango gdk-pixbuf-2.0 gtk-3 atk tk-3.0 glib-2.0`" }
    	linkoptions { "`pkg-config --libs glib-2.0 pango gdk-pixbuf-2.0 gtk-3 glib-2.0 lgobject-2.0`" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines
        {
            "NK_DEBUG"
        }

        buildoptions { "/Zi" }

    filter "configurations:Release"
        kind "WindowedApp"
        runtime "Release"
        optimize "on"

        defines
        {
            "NK_DIST",
            "WIN32_LEAN_AND_MEAN"
        }

    filter "configurations:Dist"
        kind "WindowedApp"
        runtime "Release"
        optimize "on"
        entrypoint "WinMainCRTStartup"
        flags { }
        defines
        {
            "NK_DIST",
            "WIN32_LEAN_AND_MEAN"
        }

project "Editor"
    location "Editor"
    targetname ("Nuake Engine")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    dependson { "NuakeNet" } 
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("%{prj.name}")

    files
    {
        "%{prj.name}/Editor.cpp",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h",
        
        -- This isn't ideal, but it works...needs a proper way of doing this, but that's for another time
        "Nuake/dependencies/entt/natvis/entt/*.natvis"
    }

    includedirs 
    {
        "%{prj.name}/../Nuake",
        "%{prj.name}/../Nuake/src/Vendors",
        "%{prj.name}/../Nuake/src/Vendors/nanosvg",
        "%{prj.name}/../Nuake/dependencies/glad/include",
        "%{prj.name}/../Nuake/dependencies/glfw/include",
        "%{prj.name}/../Nuake/dependencies/assimp/include",
        "%{prj.name}/../Nuake/dependencies/build",
	    "%{prj.name}/../Nuake/dependencies/JoltPhysics",
        "%{prj.name}/../Nuake/dependencies/build",
        "%{prj.name}/../Nuake/dependencies/soloud/include",
        "/usr/include/gtk-3.0/",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/DebugUtils/Include",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/Detour/Include",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/DetourCrowd/Include",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/DetourTileCache/Include",
	    "%{prj.name}/../Nuake/dependencies/recastnavigation/Recast/Include",
        "%{prj.name}/../Nuake/dependencies/yoga",
        "%{prj.name}/../Nuake/dependencies/msdf-atlas-gen",
        "%{prj.name}/../Nuake/dependencies/msdf-atlas-gen/msdfgen",
        "%{prj.name}/../Nuake/dependencies/msdf-atlas-gen/msdfgen/include",
        "%{prj.name}/../Nuake/dependencies/freetype/include",
	    "%{prj.name}/../Nuake/dependencies/tracy/public/tracy",
        "%{prj.name}/../Nuake/dependencies/entt/src",
        "%{prj.name}/../Nuake/src/Vendors/vulkan",
        "%{prj.name}/../Nuake/src/Vendors/volk",
        "%{prj.name}/../Nuake/dependencies/vma/include"
    }
    
    libdirs 
    { 
        "%{prj.name}/../Nuake/dependencies/GLEW/lib/Release/x64",
        "%{prj.name}/../Nuake/dependencies/assimp/lib/",
        "%{prj.name}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/",
        "%{prj.name}/../Nuake/src/Vendors/wren/src/include",
        "%{prj.name}/../Nuake/dependencies/JoltPhysics/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/JoltPhysics/",
        "%{prj.name}/../Nuake/dependencies/soloud/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}",
        "%{prj.name}/../Nuake/dependencies/Coral/NetCore/",
        "%{prj.name}/../Nuake/dependencies/freetype/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Freetype"
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
    
    defines {
        table.unpack(globalDefines)
    }

    filter "system:Windows"
        links
        {
            "OpenGL32"
        }
        files
        {
            "%{prj.name}/*.rc",
            "%{prj.name}/**.ico"
        }
        defines
        {
            "NK_WIN",
            "IMGUI_DEFINE_MATH_OPERATORS"
        }
        
        externalincludedirs { "%{prj.name}/../Nuake/dependencies/Coral/Coral.Native/Include/" }
    
        postbuildcommands {
            '{COPYFILE} "%{wks.location}/Nuake/dependencies/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}/%{prj.name}"',
            '{COPYFILE} "%{wks.location}/Nuake/dependencies/Coral/Coral.Managed/bin/%{cfg.buildcfg}/Coral.Managed.dll" "%{wks.location}/%{prj.name}"'
        }

    filter { "system:windows", "action:vs*"}
        flags
        {
            "MultiProcessorCompile",
        }

    filter "system:linux"
        links
        {
            "GL",
            "glfw",
            "glad",
            "X11",
            "asound",
            "glib-2.0",
            "gtk-3",
            "gobject-2.0",
            "asound"
        }

 	    buildoptions { "`pkg-config --cflags glib-2.0 pango gdk-pixbuf-2.0 gtk-3 atk tk-3.0 glib-2.0`" }
    	linkoptions { "`pkg-config --libs glib-2.0 pango gdk-pixbuf-2.0 gtk-3 glib-2.0 lgobject-2.0`" }

        
        includedirs
        {
        	"/usr/include/gtk-3.0/",
        	"/usr/lib/glib-2.0/include",
		    "/usr/include/glib-2.0",
        }
	
        defines 
        {
            "GLFW_STATIC",
            "NK_LINUX",
			"IMGUI_DEFINE_MATH_OPERATORS"
        }


    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines 
        {
            "WIN32_LEAN_AND_MEAN",
            "IMGUI_DEFINE_MATH_OPERATORS"
        }

        buildoptions { "/Zi" }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
        defines 
        {
            "WIN32_LEAN_AND_MEAN",
            "IMGUI_DEFINE_MATH_OPERATORS"
        }

    -- Removes the console for windows
    --filter {"configurations:Dist", "platforms:windows"}
       -- kind "WindowedApp"--

    -- copy a file from the objects directory to the target directory
    postbuildcommands {
     --"{COPY} "Nuake/dependencies/GLFW/lib-vc2019/glfw3.dll" " .. "./bin/" .. outputdir .. "/%{prj.name}/glfw3.dll"
    }