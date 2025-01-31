include "Nuake/Source/Nuake/Modules/Modules.lua"

project "Nuake"
    kind "StaticLib"
    staticruntime "On"

    language "C++"
    cppdialect "C++20"
    
    local moduleSources = {}

    if _ACTION then
        local modulesDir = "Source/Nuake/Modules"
        local outputFilePath = path.join(modulesDir, "Modules.cpp")

        -- Load and generate the modules file
        local modules = loadModules(modulesDir)
        moduleSources = generateModulesFile(modules, outputFilePath, "Source/Nuake/Modules")
    end

    defines
    {
		table.unpack(globalDefines),

        "_MBCS",
		"IMGUI_DEFINE_MATH_OPERATORS",
        "NK_VK",
        "IMGUI_IMPL_VULKAN_NO_PROTOTYPES"
    }

    targetdir (binaryOutputDir)
    objdir (intBinaryOutputDir)

    files
    {
        -- Main Sources
        "Source/**.cpp",
        "Source/**.h",
        "Source/Nuake/**.cpp",
        "Source/Nuake/**.h",
        "Vendors/**.h",
        "Vendors/**.cpp",
        
        -- Vendor Sources
        "Vendors/libmap/h/*.h",
        "Vendors/libmap/c/*.c",
        "Vendors/katana-parser/*.h",
        "Vendors/katana-parser/*.c",
        "Vendors/incbin/*.c",
        "Vendors/incbin/*.h",
        "Vendors/nanosvg/*.h",
        "Vendors/volk/*.c",
        "Vendors/volk/*.h",
        "Vendors/vkb/*.cpp",
        "Vendors/vkb/*.h",
        "Vendors/vulkan/**.h",
        -- Modules System
        "Source/Nuake/Modules/Modules.h",
        "Source/Nuake/Modules/Modules.cpp",
        table.unpack(moduleSources)
    }

    includedirs
    {
        ".",
        "Source",
        "Vendors",
        "Vendors/vulkan",
        "dependencies/glad/include",
        "dependencies/glfw/include",
        "dependencies/assimp/include",
        "dependencies/JoltPhysics",
        "Vendors/incbin",
        "Vendors/nanosvg",
        "Vendors/vkb",
        "dependencies/build",
        "dependencies/soloud/include",
        "dependencies/Coral/Coral.Native/Include",
	    "dependencies/recastnavigation/DebugUtils/Include",
	    "dependencies/recastnavigation/Detour/Include",       
	    "dependencies/recastnavigation/DetourCrowd/Include",
	    "dependencies/recastnavigation/DetourTileCache/Include",
	    "dependencies/recastnavigation/Recast/Include",
	    "dependencies/yoga",
        "dependencies/msdf-atlas-gen",
        "dependencies/msdf-atlas-gen/msdfgen",
        "dependencies/msdf-atlas-gen/msdfgen/include",
        "dependencies/freetype/include",
	    "dependencies/tracy/public/tracy",
        "dependencies/entt/src",
        "dependencies/vma/include",
        "dependencies/dxc/inc"
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