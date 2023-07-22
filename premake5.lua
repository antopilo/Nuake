workspace "Nuake"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Nuake/dependencies/glfw_p5.lua"
include "Nuake/dependencies/assimp_p5.lua"
include "Nuake/dependencies/freetype_p5.lua"
include "Nuake/dependencies/jolt_p5.lua"
include "Nuake/dependencies/soloud_p5.lua"

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

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

project "NuakeRuntime"
    location "Editor"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("Editor")

    files
    {
        "Editor/Runtime.cpp"
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

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

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

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    -- copy a file from the objects directory to the target directory
    postbuildcommands {
     --"{COPY} "Nuake/dependencies/GLFW/lib-vc2019/glfw3.dll" " .. "./bin/" .. outputdir .. "/%{prj.name}/glfw3.dll"
    }