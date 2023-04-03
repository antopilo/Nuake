project 'Soloud'
	location "soloud"

	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'

	targetdir ("soloud/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("soloud/bin-obj/" .. outputdir .. "/%{prj.name}")
	
	defines {
		"WITH_WASAPI"
	}

	includedirs {
		"soloud/include"
	}

	files {
		"soloud/src/**.h",
		"soloud/src/core/**.cpp",
        "soloud/src/audiosource/**.c*",
        "soloud/src/backend/wasapi/**.c*"
	}

	prebuildcommands {
		
	}

	filter "configurations:Debug"
		cppdialect "C++17"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		cppdialect "C++17"
		runtime "Release"
		optimize "on"