project "BulletCollision"
    location "bullet3/src/BulletCollision"
    kind "StaticLib"
    staticruntime "on"

    targetdir ("bullet3/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bullet3/bin-obj/" .. outputdir .. "/%{prj.name}")
    includedirs {
		"bullet3/src",
	}

    defines 
    {
        "BT_THREADSAFE=1",
        "BT_USE_DOUBLE_PRECISION"
    }

    filter "system:windows"
        systemversion "latest"
        files
        {
            "bullet3/src/*.cpp",
            "bullet3/src/*.h",
            "bullet3/src/BulletCollision/BroadphaseCollision/*.cpp",
            "bullet3/src/BulletCollision/BroadphaseCollision/*.h",
            "bullet3/src/BulletCollision/CollisionDispatch/*.cpp",
            "bullet3/src/BulletCollision/CollisionDispatch/*.h",
            "bullet3/src/BulletCollision/CollisionShapes/*.cpp",
            "bullet3/src/BulletCollision/CollisionShapes/*.h",
            "bullet3/src/BulletCollision/Gimpact/*.cpp",
            "bullet3/src/BulletCollision/Gimpact/*.h",
            "bullet3/src/BulletCollision/NarrowPhaseCollision/*.cpp",
            "bullet3/src/BulletCollision/NarrowPhaseCollision/*.h",
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

project "BulletDynamics"
    location "bullet3/src/BulletDynamics"
    kind "StaticLib"
    staticruntime "on"

    targetdir ("bullet3/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bullet3/bin-obj/" .. outputdir .. "/%{prj.name}")

    includedirs {
        "bullet3/src",
    }

    defines 
    {
        "BT_THREADSAFE=1",
        "BT_USE_DOUBLE_PRECISION"
    }

    filter "system:windows"
        systemversion "latest"
        files
        {
            "bullet3/src/BulletDynamics/Dynamics/*.cpp",
            "bullet3/src/BulletDynamics/Dynamics/*.h",
            "bullet3/src/BulletDynamics/ConstraintSolver/*.cpp",
            "bullet3/src/BulletDynamics/ConstraintSolver/*.h",
            "bullet3/src/BulletDynamics/Featherstone/*.cpp",
            "bullet3/src/BulletDynamics/Featherstone/*.h",
            "bullet3/src/BulletDynamics/MLCPSolvers/*.cpp",
            "bullet3/src/BulletDynamics/MLCPSolvers/*.h",
            "bullet3/src/BulletDynamics/Vehicle/*.cpp",
            "bullet3/src/BulletDynamics/Vehicle/*.h",
            "bullet3/src/BulletDynamics/Character/*.cpp",
            "bullet3/src/BulletDynamics/Character/*.h"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"


project "LinearMath"
    location "bullet3/src/LinearMath"
    kind "StaticLib"
    staticruntime "on"

    targetdir ("bullet3/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bullet3/bin-obj/" .. outputdir .. "/%{prj.name}")
    
    includedirs {
        "bullet3/src"
    }

    filter "system:windows"
        systemversion "latest"
        files {
            "bullet3/src/LinearMath/*.cpp",
            "bullet3/src/LinearMath/*.h",
            "bullet3/src/LinearMath/TaskScheduler/*.cpp",
            "bullet3/src/LinearMath/TaskScheduler/*.h"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
