project '*'
	includedirs {
		'yoga/yoga/'
	}

group "Dependencies"

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
	}

	defines {
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

group ""