project "EditorNet"
    language "C#"
    dotnetframework "net8.0"
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
        "Coral.Managed",
        "../Editor/NuakeNet.dll"
    }

    prebuildcommands {
        -- "dotnet add package Microsoft.CodeAnalysis.CSharp.Scripting"
    }

    postbuildcommands {
        '{ECHO} Copying "%{wks.location}/EditorNet/bin/$(Configuration)/EditorNet.dll" to "%{wks.location}/Editor"',
	    '{COPYFILE} "%{wks.location}/EditorNet/bin/$(Configuration)/EditorNet.dll" "%{wks.location}/Editor"'
    }
