group "Thirdparty"
project "Freetype"
	location "freetype"
	kind "StaticLib"
	staticruntime "on"
	language "C"
	
	targetdir ("freetype/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("freetype/bin-obj/" .. outputdir .. "/%{prj.name}")
	
	includedirs {
		'freetype/include/',
		'freetype/',
		"freetype/include/config/" 
	}
	defines { "FT2_BUILD_LIBRARY" }
	
	files { 
		"freetype/src/autofit/autofit.c",
		"freetype/src/base/ftbase.c",
		"freetype/src/base/ftbbox.c",
		"freetype/src/base/ftbdf.c",
		"freetype/src/base/ftbitmap.c",
		"freetype/src/base/ftcid.c",
		"freetype/src/base/ftfstype.c",
		"freetype/src/base/ftgasp.c",
		"freetype/src/base/ftglyph.c",
		"freetype/src/base/ftgxval.c",
		"freetype/src/base/ftinit.c",
		"freetype/src/base/ftmm.c",
		"freetype/src/base/ftotval.c",
		"freetype/src/base/ftpatent.c",
		"freetype/src/base/ftpfr.c",
		"freetype/src/base/ftstroke.c",
		"freetype/src/base/ftsynth.c",
		"freetype/src/base/ftsystem.c",
		"freetype/src/base/fttype1.c",
		"freetype/src/base/ftwinfnt.c",
		"freetype/src/bdf/bdf.c",
		"freetype/src/cache/ftcache.c",
		"freetype/src/cff/cff.c",
		"freetype/src/cid/type1cid.c",
		"freetype/src/gzip/ftgzip.c",
		"freetype/src/lzw/ftlzw.c",
		"freetype/src/pcf/pcf.c",
		"freetype/src/pfr/pfr.c",
		"freetype/src/psaux/psaux.c",
		"freetype/src/pshinter/pshinter.c",
		"freetype/src/psnames/psmodule.c",
		"freetype/src/raster/raster.c",
		"freetype/src/sfnt/sfnt.c",
		"freetype/src/smooth/smooth.c",
		"freetype/src/truetype/truetype.c",
		"freetype/src/type1/type1.c",
		"freetype/src/type42/type42.c",
		"freetype/src/winfonts/winfnt.c",
		"freetype/src/winfonts/winfnt.c",
		"freetype/src/sdf/sdf.c"
		}
	
	
	filter "system:windows"
	    systemversion "latest"
	    
	    -- warning C4244: 'function': conversion from '__int64' to 'const unsigned int', possible loss of data
	    -- warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s instead
	    -- warning C4996: 'strncpy': This function or variable may be unsafe. Consider using strncpy_s instead.
	    -- warning C4267: '=': conversion from 'size_t' to 'FT_Int', possible loss of data
	    -- warning C4312: 'type cast': conversion from 'unsigned long' to 'void *' of greater size
 	    disablewarnings { "4996", "4267", "4244", "4312" }
	
	defines
	{
	
	}
	
	filter "configurations:Debug"
		files { "freetype/src/base/ftdebug.c" }
		runtime "Debug"
		symbols "on"
	
	filter "configurations:Release"
		files { "freetype/src/base/ftdebug.c" }
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		files { "freetype/src/base/ftdebug.c" }
		runtime "Release"
		optimize "on"
group ""