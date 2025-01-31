project "NuakeRuntime"
    kind "ConsoleApp"
    language "C++"

	debugdir ("Editor")
    targetdir (binaryOutputDir)
    objdir (intBinaryOutputDir)

    files
    {
        "**.cpp",
        "**.h"
    }

    includedirs 
    {
        "../Nuake/Source/",
        "../Nuake/Vendors",
        "../Nuake/Vendors/nanosvg",
        "../Nuake/dependencies/glad/include",
        "../Nuake/dependencies/glfw/include",
        "../Nuake/dependencies/assimp/include",
        "../Nuake/dependencies/build",
	    "../Nuake/dependencies/JoltPhysics",
        "../Nuake/dependencies/build",
        "../Nuake/dependencies/soloud/include",
        "/usr/include/gtk-3.0/",
	    "../Nuake/dependencies/recastnavigation/DebugUtils/Include",
	    "../Nuake/dependencies/recastnavigation/Detour/Include",
	    "../Nuake/dependencies/recastnavigation/DetourCrowd/Include",
	    "../Nuake/dependencies/recastnavigation/DetourTileCache/Include",
	    "../Nuake/dependencies/recastnavigation/Recast/Include",
        "../Nuake/dependencies/yoga",
        "../Nuake/dependencies/msdf-atlas-gen",
        "../Nuake/dependencies/msdf-atlas-gen/msdfgen",
        "../Nuake/dependencies/msdf-atlas-gen/msdfgen/include",
        "../Nuake/dependencies/freetype/include",
	    "../Nuake/dependencies/tracy/public/tracy",
        "../Nuake/dependencies/entt/src",
		"../Nuake/Vendors/vulkan",
        "../Nuake/Vendors/volk",
        "../Nuake/dependencies/vma/include"
    }

    libdirs 
    { 
        "../Nuake/dependencies/GLEW/lib/Release/x64",
        "../Nuake/dependencies/assimp/lib/",
        "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/",
        "../Nuake/dependencies/JoltPhysics/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/JoltPhysics/",
        "../Nuake/dependencies/soloud/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}",
        "../Nuake/dependencies/Coral/NetCore/",
        "../Nuake/dependencies/freetype/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Freetype"
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

        externalincludedirs { "../Nuake/dependencies/Coral/Coral.Native/Include/" }
    
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