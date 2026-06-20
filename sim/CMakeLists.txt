cmake_minimum_required(VERSION 3.20)

find_package(glfw3 REQUIRED)


file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
set(ALL_SOURCES ${APP_SOURCES})
add_executable(FluidSim ${ALL_SOURCES})

target_compile_definitions(FluidSim PUBLIC GLFW_INCLUDE_VULKAN)
target_include_directories(FluidSim PRIVATE "./include/")

target_link_libraries(FluidSim PRIVATE GhostRenderEngine)
target_link_libraries(GhostRenderEngine
				PUBLIC
								glfw
)
