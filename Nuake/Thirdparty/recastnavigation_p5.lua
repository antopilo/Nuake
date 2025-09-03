group "Thirdparty"
project "DebugUtils"
	language "C++"
	kind "StaticLib"
	includedirs { 
		"recastnavigation/DebugUtils/Include",
		"recastnavigation/Detour/Include",
		"recastnavigation/DetourTileCache/Include",
		"recastnavigation/Recast/Include"
	}
	files {
		"recastnavigation/DebugUtils/Include/*.h",
		"recastnavigation/DebugUtils/Source/*.cpp"
	}

project "DetourCrowd"
	language "C++"
	kind "StaticLib"
	includedirs {
		"recastnavigation/DetourCrowd/Include",
		"recastnavigation/Detour/Include",
		"recastnavigation/Recast/Include"
	}
	files {
		"recastnavigation/DetourCrowd/Include/*.h",
		"recastnavigation/DetourCrowd/Source/*.cpp"
	}

project "DetourTileCache"
	language "C++"
	kind "StaticLib"
	includedirs {
		"recastnavigation/DetourTileCache/Include",
		"recastnavigation/Detour/Include",
		"recastnavigation/Recast/Include"
	}
	files {
		"recastnavigation/DetourTileCache/Include/*.h",
		"recastnavigation/DetourTileCache/Source/*.cpp"
	}

project "Recast"
	language "C++"
	kind "StaticLib"
	includedirs { 
		"recastnavigation/Recast/Include" 
	}
	files { 
		"recastnavigation/Recast/Include/*.h",
		"recastnavigation/Recast/Source/*.cpp" 
	}

project "Detour"
	language "C++"
	kind "StaticLib"
	staticruntime "on"

	includedirs { 
		"recastnavigation/Detour/Include" 
	}
	files { 
		"recastnavigation/Detour/Include/*.h", 
		"recastnavigation/Detour/Source/*.cpp" 
	}

	filter "configurations:Debug"
		defines {"DEBUG"}
		symbols "On"
		postbuildcommands {}

	-- linux library cflags and libs
	filter {"system:linux", "toolset:gcc"}
		buildoptions {
			"-Wno-error=class-memaccess",
			"-Wno-error=maybe-uninitialized"
		}
group ""