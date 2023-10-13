project "NuakeNet"
    language "C#"
    dotnetframework "net7.0"
    kind "SharedLib"
	    clr "Unsafe"
	
    -- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

    propertytags {
        { "AppendTargetFrameworkToOutputPath", "false" },
        { "Nullable", "enable" },
    }

    files 
    {
        "src/**.cs"
    }
    
	links 
    {
        "Coral.Managed"
    }