group "Thirdparty"
project 'Soloud'
	location "soloud"

	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'

	targetdir ("soloud/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("soloud/bin-obj/" .. outputdir .. "/%{prj.name}")
	
	

	includedirs {
		"soloud/include"
	}

	files {
		"soloud/src/**.h",
		"soloud/src/core/**.cpp",
        "soloud/src/audiosource/**.c*"
	}

	prebuildcommands {
		
	}

	filter "system:windows"
		defines {
			"WITH_WASAPI"
		}

		files
		{
			"soloud/src/backend/wasapi/**.c*"
		}

	filter "system:linux"
		defines {
			"WITH_ALSA"
		}
		files
		{
			"soloud/src/backend/alsa/soloud_alsa.cpp"
		}

	filter "configurations:Debug"
		cppdialect "C++17"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		cppdialect "C++17"
		runtime "Release"
		optimize "on"
group ""