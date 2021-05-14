cmake_minimum_required(VERSION 3.10)

set(
	CompilerFlags
	CMAKE_CXX_FLAGS
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_CXX_FLAGS_RELEASE
	CMAKE_C_FLAGS
	CMAKE_C_FLAGS_DEBUG
	CMAKE_C_FLAGS_RELEASE
)

foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()
 
# set the project name
project(Nuake-engine)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set( SOURCES
    "src"
	"src/Vendors"
	"Dependencies/assimp/include"
	"Dependencies/bullet/src"
	"Dependencies/GLFW/include"
	"Dependencies/GLEW/include"
	"Dependencies/VXGI/include"
)



