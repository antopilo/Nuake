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

project "Nuake"
    location "Nuake"
    kind "StaticLib"
    language "C++"
    
    defines 
    {
        "BT_THREADSAFE=1",
        "BT_USE_DOUBLE_PRECISION",
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
        "%{prj.name}/../Nuake/Dependencies/bullet/src",
        "%{prj.name}/../Nuake/Dependencies/assimp/include",
	"%{prj.name}/../Nuake/src/Vendors/msdfgen/include",
	"%{prj.name}/../Nuake/src/Vendors/msdfgen/freetype/include",
	"%{prj.name}/../Nuake/src/Vendors/msdfgen",
	"%{prj.name}/../Nuake/src/Vendors/wren/src/include"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"

project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/../Nuake",
        "%{prj.name}/../Nuake/src/Vendors",
        "%{prj.name}/../Nuake/Dependencies/GLEW/include",
        "%{prj.name}/../Nuake/Dependencies/GLFW/include",
        "%{prj.name}/../Nuake/Dependencies/bullet/src",
        "%{prj.name}/../Nuake/Dependencies/assimp/include",
        "%{prj.name}/../Nuake/Dependencies/build",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen"
    }

    
    libdirs 
    { 
        "%{prj.name}/../Nuake/dependencies/GLFW/lib-vc2019", 
        "%{prj.name}/../Nuake/dependencies/GLEW/lib/Release/x64",
        "%{prj.name}/../Nuake/dependencies/bullet/lib/release/x64",
        "%{prj.name}/../Nuake/dependencies/assimp/lib/",
        "%{prj.name}/../Nuake/dependencies/freetype-windows-binaries/release static/win64",
        "%{prj.name}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/",
	"%{prj.name}/../Nuake/src/Vendors/msdf/freetype/include",
	"%{prj.name}/../Nuake/src/Vendors/msdfgen/freetype/win64",
	"%{prj.name}/../Nuake/src/Vendors/msdfgen",
	"%{prj.name}/../Nuake/src/Vendors/wren/src/include",
    }

    links
    { 
        "Nuake.lib", 
        "GLFW",
        "assimp",
        "glew32s.lib", 
        "opengl32.lib", 
        "BulletCollision_vs2010_x64_release.lib",
        "BulletDynamics_vs2010_x64_release.lib",
        "LinearMath_vs2010_x64_release.lib",
        "freetype.lib"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"

    -- copy a file from the objects directory to the target directory
    postbuildcommands {
     --"{COPY} "Nuake/dependencies/GLFW/lib-vc2019/glfw3.dll" " .. "./bin/" .. outputdir .. "/%{prj.name}/glfw3.dll"
    }