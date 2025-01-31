project 'glad'
	location "glad"

	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'

	targetdir ("glad/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("glad/bin-obj/" .. outputdir .. "/%{prj.name}")
	
	includedirs 
	{
		"glad/include"
	}

	files 
	{
		"glad/src/**.c",
		"glad/include/**.h",
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