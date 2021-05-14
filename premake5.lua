workspace "Nuake"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
        "%{prj.name}/src/Vendors/libmap/c/*.c"
    }

    includedirs
    {
        "%{prj.name}/src/Vendors",
        "%{prj.name}/Dependencies/GLEW/include",
        "%{prj.name}/Dependencies/GLFW/x64/include",
        "%{prj.name}/Dependencies/bullet/src",
        "%{prj.name}/Dependencies/assimp/include"
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
        "%{prj.name}/../Nuake/Dependencies/GLFW/x64/include",
        "%{prj.name}/../Nuake/Dependencies/bullet/src",
        "%{prj.name}/../Nuake/Dependencies/assimp/include"
    }

    
    libdirs 
    { 
        "%{prj.name}/../Nuake/dependencies/GLFW/lib-vc2019", 
        "%{prj.name}/../Nuake/dependencies/GLEW/lib/Release/x64",
        "%{prj.name}/../Nuake/dependencies/bullet/lib/release/x64",
        "%{prj.name}/../Nuake/dependencies/assimp/lib/",
        "%{prj.name}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/"
    }

    links
    { 
        "Nuake.lib", 
        "glfw3dll.lib", 
        "glew32s.lib", 
        "opengl32.lib", 
        "BulletCollision_vs2010_x64_release.lib",
        "BulletDynamics_vs2010_x64_release.lib",
        "LinearMath_vs2010_x64_release.lib",
        "assimp-vc142-mt.lib"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"

    -- copy a file from the objects directory to the target directory
    postbuildcommands {
    -- "{COPY} ../Nuake/dependencies/GLFW/lib-vc2019/glfw3.dll " .. "bin/" .. outputdir .. "/%{prj.name}/glfw3.dll"
    }