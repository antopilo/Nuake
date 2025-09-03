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
        "Vendors/Subprocess/*.hpp",
        "Vendors/volk/*.c",
        "Vendors/volk/*.h",
        "Vendors/vkb/*.cpp",
        "Vendors/vkb/*.h",
        "Vendors/vulkan/**.h",
	    "Vendors/Subprocess/**.hpp",
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
        "Thirdparty/glad/include",
        "Thirdparty/glfw/include",
        "Thirdparty/assimp/include",
        "Thirdparty/JoltPhysics",
        "Vendors/incbin",
        "Vendors/nanosvg",
        "Vendors/vkb",
        "Vendors/Subprocess",
        "Thirdparty/build",
        "Thirdparty/soloud/include",
        "Thirdparty/Coral/Coral.Native/Include",
	    "Thirdparty/recastnavigation/DebugUtils/Include",
	    "Thirdparty/recastnavigation/Detour/Include",       
	    "Thirdparty/recastnavigation/DetourCrowd/Include",
	    "Thirdparty/recastnavigation/DetourTileCache/Include",
	    "Thirdparty/recastnavigation/Recast/Include",
	    "Thirdparty/yoga",
        "Thirdparty/msdf-atlas-gen",
        "Thirdparty/msdf-atlas-gen/msdfgen",
        "Thirdparty/msdf-atlas-gen/msdfgen/include",
        "Thirdparty/freetype/include",
	    "Thirdparty/tracy/public/tracy",
        "Thirdparty/entt/src",
        "Thirdparty/vma/include",
        "Thirdparty/dxc/inc"
    }

    libdirs { "%{prj.name}/Thirdparty/dxc/lib/x64" }

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