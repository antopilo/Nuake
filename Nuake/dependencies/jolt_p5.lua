group "Dependencies"
project 'JoltPhysics'
	location "JoltPhysics"

	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'

	targetdir ("JoltPhysics/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("JoltPhysics/bin-obj/" .. outputdir .. "/%{prj.name}")

	pchheader "Jolt/Jolt.h"
	pchsource "JoltPhysics/Jolt.cpp"

	defines {
		
	}

	includedirs {
		"JoltPhysics"
	}

	files {
		"JoltPhysics/Jolt/**.h",
		"JoltPhysics/Jolt/**.cpp",
		"JoltPhysics/Jolt.cpp",
	}

	filter "system:windows"
		prebuildcommands {
			"{COPY} %{prj.location}../Jolt.cpp %{prj.location}"
		}

	filter "system:not windows"
		prebuildcommands {
			"cp %{prj.location}./Jolt.cpp %{prj.location}/"
		}
		
    -- When building any Visual Studio solution
    filter { "system:windows", "action:vs*"}
        flags
        {
            "MultiProcessorCompile",
        }
		
	filter "configurations:Debug"
		cppdialect "C++17"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		cppdialect "C++17"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		cppdialect "C++17"
		runtime "Release"
		optimize "on"
group ""