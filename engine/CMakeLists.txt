cmake_minimum_required(VERSION 3.20)

find_package(Vulkan REQUIRED)


file(GLOB_RECURSE ENGINE_SOURCES 
    "src/Core/*.cpp"
    "src/Resources/*.cpp"
    "src/Systems/*.cpp"
    "src/Utils/*.cpp"
)

add_library(GhostRenderEngine STATIC ${ENGINE_SOURCES})

target_include_directories(GhostRenderEngine PUBLIC
	"./include/")

target_precompile_headers(GhostRenderEngine PRIVATE <vulkan/vulkan.hpp>)
target_precompile_headers(GhostRenderEngine PRIVATE <vulkan/vulkan_raii.hpp>)

target_link_libraries(GhostRenderEngine
    PUBLIC
        Vulkan::Vulkan
)
