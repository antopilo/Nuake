group "Dependencies"
project "vma"
	location "vma"
    kind "StaticLib"
    language "C++"
	cppdialect "C++20"
    staticruntime "on"

    -- Include the headers
    includedirs {
        "vma/include",
        "../Vendors/vulkan",
    }

    files {
        "vma/include/**.h",
        "vma/include/**.cpp"
    }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
group ""