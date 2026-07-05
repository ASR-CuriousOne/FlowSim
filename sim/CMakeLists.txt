cmake_minimum_required(VERSION 3.20)

find_package(glfw3 REQUIRED)

file(GLOB IMGUI_SOURCES
				"../vendor/imgui/*.cpp"
				"../vendor/imgui/backends/imgui_impl_glfw.cpp"
				"../vendor/imgui/backends/imgui_impl_vulkan.cpp"
)


file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
set(ALL_SOURCES ${APP_SOURCES} ${IMGUI_SOURCES})
add_executable(FluidSim ${ALL_SOURCES})

target_compile_definitions(FluidSim PUBLIC GLFW_INCLUDE_VULKAN)
target_include_directories(FluidSim PRIVATE "./include/")
target_include_directories(FluidSim PRIVATE "../vendor/imgui" "../vendor/imgui/backends")

target_link_libraries(FluidSim PRIVATE GhostRenderEngine)
target_link_libraries(GhostRenderEngine
				PUBLIC
								glfw
)
