project '*'
	includedirs {
		'yoga/yoga/'
	}

group "Thirdparty"

project 'yoga'
	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'
	cppdialect "C++20"

	includedirs {
		'yoga/'
	}
	files {
		'yoga/yoga/*.cpp',
		'yoga/yoga/*.h',
		'yoga/yoga/**/*.cpp',
		'yoga/yoga/**/*.h',
	}

	defines {
	}

    -- When building any Visual Studio solution
    filter { "system:windows", "action:vs*"}
        flags
        {
            "MultiProcessorCompile",
        }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

group ""