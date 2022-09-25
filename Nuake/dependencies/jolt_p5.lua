project 'JoltPhysics'
	location "JoltPhysics"

	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'

	targetdir ("JoltPhysics/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("JoltPhysics/bin-obj/" .. outputdir .. "/%{prj.name}")

	pchheader "Jolt/Jolt.h"
	pchsource "../Jolt.cpp"

	includedirs {
		"JoltPhysics"
	}

	files {
		"JoltPhysics/Jolt/**.h",
		"JoltPhysics/Jolt/**.cpp",
		"Jolt.cpp"
	}

	defines {

	}
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"