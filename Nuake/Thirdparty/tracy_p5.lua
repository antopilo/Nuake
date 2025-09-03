group "Thirdparty"
project 'Tracy'
	location "tracy"

	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	
	includedirs {
	    "tracy/public/tracy"
	}
	
	files {
	    "tracy/public/tracy/Tracy.hpp",
	    "tracy/public/TracyClient.cpp"
	}
	
	defines {
	    "TRACY_ENABLE",
		"TRACY_ON_DEMAND",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
group ""