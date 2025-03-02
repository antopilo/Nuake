project "Runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

	debugdir (binaryOutputDir)
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
        "../Nuake/Thirdparty/glad/include",
        "../Nuake/Thirdparty/glfw/include",
        "../Nuake/Thirdparty/assimp/include",
        "../Nuake/Thirdparty/build",
	    "../Nuake/Thirdparty/JoltPhysics",
        "../Nuake/Thirdparty/build",
        "../Nuake/Thirdparty/soloud/include",
        "/usr/include/gtk-3.0/",
	    "../Nuake/Thirdparty/recastnavigation/DebugUtils/Include",
	    "../Nuake/Thirdparty/recastnavigation/Detour/Include",
	    "../Nuake/Thirdparty/recastnavigation/DetourCrowd/Include",
	    "../Nuake/Thirdparty/recastnavigation/DetourTileCache/Include",
	    "../Nuake/Thirdparty/recastnavigation/Recast/Include",
        "../Nuake/Thirdparty/yoga",
        "../Nuake/Thirdparty/msdf-atlas-gen",
        "../Nuake/Thirdparty/msdf-atlas-gen/msdfgen",
        "../Nuake/Thirdparty/msdf-atlas-gen/msdfgen/include",
        "../Nuake/Thirdparty/freetype/include",
	    "../Nuake/Thirdparty/tracy/public/tracy",
        "../Nuake/Thirdparty/entt/src",
		"../Nuake/Vendors/vulkan",
        "../Nuake/Vendors/volk",
        "../Nuake/Thirdparty/vma/include"
    }

    libdirs 
    { 
        "../Nuake/Thirdparty/GLEW/lib/Release/x64",
        "../Nuake/Thirdparty/assimp/lib/",
        "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Nuake/",
        "../Nuake/Thirdparty/JoltPhysics/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/JoltPhysics/",
        "../Nuake/Thirdparty/soloud/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}",
        "../Nuake/Thirdparty/Coral/NetCore/",
        "../Nuake/Thirdparty/freetype/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Freetype"
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
        "vma",
        "dxcompiler"
    }
    libdirs { "../Nuake/Thirdparty/dxc/lib/x64" }
    
    defines {
        table.unpack(globalDefines)
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        defines {
            "NK_WIN"
        }
        externalincludedirs { "../Nuake/Thirdparty/Coral/Coral.Native/Include/" }

        prebuildcommands {
            '{ECHO} "Copying dxcompiler.dll to Working directory..."',
            '{COPYFILE} "%{wks.location}Nuake/Thirdparty/dxc/bin/x64/dxcompiler.dll" "%{cfg.debugdir}/"',
            '{ECHO} Copying Coral to Working directory...',
            '{COPYFILE} "%{wks.location}Nuake/Thirdparty/Coral/Coral.Managed/bin/%{cfg.buildcfg}/Coral.Managed.dll" "%{cfg.debugdir}/"',
            '{COPYFILE} "%{wks.location}Nuake/Thirdparty/Coral/Coral.Managed/bin/%{cfg.buildcfg}/Coral.Managed.runtimeconfig.json" "%{cfg.debugdir}/"',
            'xcopy /E /I /Y "%{wks.location}Data" "%{cfg.debugdir}\\Resources"'
        }

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

        buildoptions { "`pkg-config --cflags glib-2.0 pango gdk-pixbuf-2.0 gtk+-3.0 atk glib-2.0`" }
        linkoptions { "`pkg-config --libs glib-2.0 pango gdk-pixbuf-2.0 gtk+-3.0 glib-2.0 gobject-2.0`" }

    filter "system:bsd"
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
            "pthread",
            "execinfo"
        }

        includedirs
        {
                "/usr/local/include/gtk-3.0/",
                "/usr/local/lib/glib-2.0/include",
                "/usr/local/include/glib-2.0",
        }

        buildoptions { "`pkg-config --cflags glib-2.0 pango gdk-pixbuf-2.0 gtk+-3.0 atk glib-2.0`" }
        linkoptions { "`pkg-config --libs glib-2.0 pango gdk-pixbuf-2.0 gtk+-3.0 glib-2.0 gobject-2.0`" }

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
