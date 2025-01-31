project '*'
	includedirs {
		'yoga/yoga/'
	}

group "Thirdparty"

project 'msdf-gen'
	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'
	cppdialect "C++20"

	includedirs {
		'msdf-atlas-gen/include',
		'freetype/include'
	}

	files {
		'msdf-atlas-gen/msdfgen/*.h',
		'msdf-atlas-gen/msdfgen/*.cpp',
		'msdf-atlas-gen/msdfgen/*.hpp',
		'msdf-atlas-gen/msdfgen/Core/*.h',
		'msdf-atlas-gen/msdfgen/Core/*.hpp',
		'msdf-atlas-gen/msdfgen/Core/*.cpp',
		'msdf-atlas-gen/msdfgen/lib/*.cpp',
		'msdf-atlas-gen/msdfgen/ext/*.h',
		'msdf-atlas-gen/msdfgen/ext/*.cpp'
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

project 'msdf-atlas-gen'
	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'
	cppdialect "C++20"

	includedirs {
		'msdf-atlas-gen/include',
		'msdf-atlas-gen/msdfgen',
		'msdf-atlas-gen/msdfgen/include'
	}

	files {
		'msdf-atlas-gen/msdf-atlas-gen/*.h',
		'msdf-atlas-gen/msdf-atlas-gen/*.cpp',
		'msdf-atlas-gen/msdf-atlas-gen/*.hpp',
	}

	links {
		'msdf-gen'
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