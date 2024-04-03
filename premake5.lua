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

include "Nuake/dependencies/glfw_p5.lua"
include "Nuake/dependencies/glad_p5.lua"
include "Nuake/dependencies/assimp_p5.lua"
include "Nuake/dependencies/freetype_p5.lua"
include "Nuake/dependencies/jolt_p5.lua"
include "Nuake/dependencies/soloud_p5.lua"
include "Nuake/dependencies/optick_p5.lua"
include "Nuake/dependencies/coral_p5.lua"

include "NuakeNet/premake5.lua"

project "Nuake"
    location "Nuake"
    kind "StaticLib"
    staticruntime "On"

    language "C++"
    cppdialect "C++20"
   
    defines
    {
        "_MBCS",
		"IMGUI_DEFINE_MATH_OPERATORS"
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
        "%{prj.name}/src/Vendors/katana-parser/*.c",
        "%{prj.name}/src/Vendors/incbin/*.c",
        "%{prj.name}/src/Vendors/incbin/*.h"
    }

    includedirs
    {
        "%{prj.name}",
        "%{prj.name}/src/Vendors",
        "%{prj.name}/dependencies/glad/include",
        "%{prj.name}/dependencies/glfw/include",
        "%{prj.name}/dependencies/assimp/include",
        "%{prj.name}/dependencies/JoltPhysics",
        "%{prj.name}/src/Vendors/msdfgen/include",
        "%{prj.name}/src/Vendors/msdfgen/freetype/include",
        "%{prj.name}/src/Vendors/msdfgen",
        "%{prj.name}/src/Vendors/wren/src/include",
        "%{prj.name}/src/Vendors/incbin",
        "%{prj.name}/dependencies/build",
        "%{prj.name}/dependencies/soloud/include",
        "%{prj.name}/dependencies/Coral/Coral.Native/Include"
    }

    links
    {
		"Freetype",
        "soloud"
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
    
    
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

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
        "%{prj.name}/../Nuake/Dependencies/glad/include",
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
        "%{prj.name}/../Nuake/dependencies/assimp/lib/",
        "%{prj.name}/../Nuake/dependencies/freetype-windows-binaries/release static/win64",
        "%{prj.name}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/",
        "%{prj.name}/../Nuake/dependencies/freetype/include",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen/freetype/win64",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen",
        "%{prj.name}/../Nuake/src/Vendors/wren/src/include",
        "%{prj.name}/../Nuake/dependencies/JoltPhysics/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/JoltPhysics/",
        "%{prj.name}/../Nuake/dependencies/soloud/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}",
        "%{prj.name}/../Nuake/dependencies/Coral/NetCore/"
    }

    links
    {
        "Nuake",
        "GLFW",
        "glad",
        "assimp",
		"Freetype",
		"JoltPhysics",
        "soloud",
        "Coral.Native",
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
    
        postbuildcommands {
            '{COPYFILE} "%{wks.location}/Nuake/dependencies/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}/%{prj.name}"'
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
        flags { "WinMain" }
        defines
        {
            "NK_DIST",
            "WIN32_LEAN_AND_MEAN"
        }


project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("%{prj.name}")

    files
    {
        "%{prj.name}/Editor.cpp",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs
    {
        "%{prj.name}/../Nuake",
        "%{prj.name}/../Nuake/src/Vendors",
        "%{prj.name}/../Nuake/dependencies/glad/include",
        "%{prj.name}/../Nuake/dependencies/glfw/include",
        "%{prj.name}/../Nuake/dependencies/assimp/include",
        "%{prj.name}/../Nuake/dependencies/build",
        "%{prj.name}/../Nuake/src/Vendors/msdfgen",
		"%{prj.name}/../Nuake/dependencies/JoltPhysics",
        "%{prj.name}/../Nuake/dependencies/build",
        "%{prj.name}/../Nuake/dependencies/soloud/include",
        "/usr/include/gtk-3.0/",
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
        "%{prj.name}/../Nuake/dependencies/soloud/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}",
        "%{prj.name}/../Nuake/dependencies/Coral/NetCore/"
    }

    links
    { 
        "Nuake",
        "glad", 
        "GLFW",
        "assimp",
        "Freetype",
        "JoltPhysics",
        "soloud",
        "Coral.Native"
    }

    filter "system:Windows"
        links
        {
            "OpenGL32"
        }
        files
        {
            "%{prj.name}/resources/*.rc",
            "%{prj.name}/resources/**.ico"
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