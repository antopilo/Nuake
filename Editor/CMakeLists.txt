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
project(Nuake-editor)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
	"../Engine"
	"../Engine/src/Vendors"
	"../Engine/Dependencies/assimp/include"
	"../Engine/Dependencies/bullet/src"
	"../Engine/Dependencies/GLFW/include"
	"../Engine/Dependencies/GLEW/include"
	"../Engine/Dependencies/VXGI/include"
)

# add the executable
add_executable(Nuake-editor 
Editor.cpp
src/EditorInterface.cpp
src/EditorInterface.h)

target_link_libraries(Nuake-editor opengl32.lib )
target_link_libraries(Nuake-editor ${CMAKE_CURRENT_SOURCE_DIR}/../Engine/dependencies/GLFW/lib-vc2019/glfw3dll.lib)
target_link_libraries(Nuake-editor ${CMAKE_CURRENT_SOURCE_DIR}/../Engine/dependencies/GLEW/lib/Release/x64/glew32s.lib)
target_link_libraries(Nuake-editor ${CMAKE_CURRENT_SOURCE_DIR}/../Engine/x64/Release/Engine.lib)
target_link_libraries(Nuake-editor ${CMAKE_CURRENT_SOURCE_DIR}/../Engine/dependencies/assimp/lib/assimp-vc142-mt.lib)
target_link_libraries(Nuake-editor ${CMAKE_CURRENT_SOURCE_DIR}/../Engine/dependencies/bullet/lib/release/x64/BulletCollision_vs2010_x64_release.lib)
target_link_libraries(Nuake-editor ${CMAKE_CURRENT_SOURCE_DIR}/../Engine/dependencies/bullet/lib/release/x64/BulletDynamics_vs2010_x64_release.lib)
target_link_libraries(Nuake-editor ${CMAKE_CURRENT_SOURCE_DIR}/../Engine/dependencies/bullet/lib/release/x64/LinearMath_vs2010_x64_release.lib)
