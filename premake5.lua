workspace "Nuake"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Nuake/dependencies/glfw_p5.lua"
include "Nuake/dependencies/assimp_p5.lua"
include "Nuake/dependencies/freetype_p5.lua"
include "Nuake/dependencies/jolt_p5.lua"
include "Nuake/dependencies/soloud_p5.lua"
include "Nuake/dependencies/optick_p5.lua"

project "Nuake"
    location "Nuake"
    kind "StaticLib"
    language "C++"
    
    defines
    {
        "GLEW_STATIC",
        "_MBCS"
    }


    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/Engine.h",
        "%{prj.name}/Engine.cpp",
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/Vendors/libmap/h/*.h",
        "%{prj.name}/src/Vendors/libmap/c/*.c",
        "%{prj.name}/src/Vendors/msdfgen/**.h",
        "%{prj.name}/src/Vendors/msdfgen/**.cpp",
        "%{prj.name}/src/Vendors/msdf-atlas-gen/*.h",
        "%{prj.name}/src/Vendors/msdf-atlas-gen/*.cpp",
        "%{prj.name}/src/Vendors/wren/src/vm/*.h",
        "%{prj.name}/src/Vendors/wren/src/vm/*.c",
        "%{prj.name}/src/Vendors/katana-parser/*.h",
        "%{prj.name}/src/Vendors/katana-parser/*.c"
    }

    includedirs
    {
        "%{prj.name}/../Nuake",
        "%{prj.name}/../Nuake/src/Vendors",
        "%{prj.name}/../Nuake/Dependencies/GLEW/include",
        "%{prj.name}/../Nuake/Dependencies/GLFW/include",
        "%{prj.name}/../Nuake/Dependencies/assimp/include",
        "%{prj.name}/../Nuake/Dependencies/JoltPhysics",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen/include",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen/freetype/include",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen",
        "%{prj.name}/../Nuake/src/Vendors/wren/src/include",
        "%{prj.name}/../Nuake/Dependencies/build",
        "%{prj.name}/../Nuake/Dependencies/soloud/include"
    }

    links
    {
		"Freetype",
        "soloud"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        defines {
            "NK_WIN"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines 
        {
            "NK_DEBUG"
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
        defines 
        {
            "NK_DIST"
        }

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
        "%{prj.name}/../Nuake/Dependencies/GLEW/include",
        "%{prj.name}/../Nuake/Dependencies/GLFW/include",
        "%{prj.name}/../Nuake/Dependencies/assimp/include",
        "%{prj.name}/../Nuake/Dependencies/build",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen",
		"%{prj.name}/../Nuake/Dependencies/JoltPhysics",
        "%{prj.name}/../Nuake/Dependencies/build",
        "%{prj.name}/../Nuake/Dependencies/soloud/include"
    }
    
    libdirs 
    { 
        "%{prj.name}/../Nuake/dependencies/GLEW/lib/Release/x64",
        "%{prj.name}/../Nuake/dependencies/assimp/lib/",
        "%{prj.name}/../Nuake/dependencies/freetype-windows-binaries/release static/win64",
        "%{prj.name}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/",
        "%{prj.name}/../Nuake/dependencies/freetype/include",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen/freetype/win64",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen",
        "%{prj.name}/../Nuake/src/Vendors/wren/src/include",
        "%{prj.name}/../Nuake/dependencies/JoltPhysics/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/JoltPhysics/",
        "%{prj.name}/../Nuake/dependencies/soloud/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    }

    links
    { 
        "Nuake", 
        "GLFW",
        "assimp",
        "glew32s.lib", 
        "opengl32.lib",
		"Freetype",
		"JoltPhysics",
        "soloud"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        defines {
            "NK_WIN"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines {
            "NK_DEBUG"
        }

    filter "configurations:Release"
        kind "WindowedApp"
        runtime "Release"
        optimize "on"
        defines {
            "NK_DIST",
            "WIN32_LEAN_AND_MEAN"
        }
        entrypoint "WinMainCRTStartup"
        flags { "WinMain" }
        buildoptions { "-mwindows"}

    -- copy a file from the objects directory to the target directory
    postbuildcommands {
     --"{COPY} "Nuake/dependencies/GLFW/lib-vc2019/glfw3.dll" " .. "./bin/" .. outputdir .. "/%{prj.name}/glfw3.dll"
    }

project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("%{prj.name}")

    files
    {
        "%{prj.name}/Editor.cpp",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h",
        "%{prj.name}/resources/*.rc",
        "%{prj.name}/resources/**.ico"
    }

    includedirs
    {
        "%{prj.name}/../Nuake",
        "%{prj.name}/../Nuake/src/Vendors",
        "%{prj.name}/../Nuake/Dependencies/GLEW/include",
        "%{prj.name}/../Nuake/Dependencies/GLFW/include",
        "%{prj.name}/../Nuake/Dependencies/assimp/include",
        "%{prj.name}/../Nuake/Dependencies/build",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen",
		"%{prj.name}/../Nuake/Dependencies/JoltPhysics",
        "%{prj.name}/../Nuake/Dependencies/build",
        "%{prj.name}/../Nuake/Dependencies/soloud/include"
    }
    
    libdirs 
    { 
        "%{prj.name}/../Nuake/dependencies/GLEW/lib/Release/x64",
        "%{prj.name}/../Nuake/dependencies/assimp/lib/",
        "%{prj.name}/../Nuake/dependencies/freetype-windows-binaries/release static/win64",
        "%{prj.name}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/",
        "%{prj.name}/../Nuake/dependencies/freetype/include",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen/freetype/win64",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen",
        "%{prj.name}/../Nuake/src/Vendors/wren/src/include",
        "%{prj.name}/../Nuake/dependencies/JoltPhysics/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/JoltPhysics/",
        "%{prj.name}/../Nuake/dependencies/soloud/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    }

    links
    { 
        "Nuake", 
        "GLFW",
        "assimp",
        "glew32s.lib", 
        "opengl32.lib",
		"Freetype",
		"JoltPhysics",
        "soloud"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines {
            "NK_DEBUG",
            "WIN32_LEAN_AND_MEAN"
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        kind "WindowedApp"
        runtime "Release"
        optimize "on"
        defines {
            "NK_DIST",
            "WIN32_LEAN_AND_MEAN"
        }
        entrypoint "WinMainCRTStartup"
        flags { "WinMain" }

    -- copy a file from the objects directory to the target directory
    postbuildcommands {
     --"{COPY} "Nuake/dependencies/GLFW/lib-vc2019/glfw3.dll" " .. "./bin/" .. outputdir .. "/%{prj.name}/glfw3.dll"
    }