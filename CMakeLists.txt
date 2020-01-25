cmake_minimum_required(VERSION 3.15)
set (CMAKE_CXX_STANDARD 17)
project(TripleR)

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/external/volk)
add_subdirectory(${PROJECT_SOURCE_DIR}/external/meshoptimizer)

message("Project source directory: ${PROJECT_SOURCE_DIR}")
# find_program(GLSLANG_VALIDATOR
#         glslangValidator)
# if(NOT GLSLANG_VALIDATOR)
#     message(FATAL_ERROR "glslangValidator required - source maintained at https://github.com/KhronosGroup/glslang")
# else(NOT GLSLANG_VALIDATOR)
#     message("glslangValidator found")
# endif()
# add_custom_command(COMMENT "Compiling shader..."
#         OUTPUT ${SHADERS_PATH}/triangle.vert.spv
#         COMMAND ${GLSLANG_VALIDATOR} ${SHADERS_PATH}/triangle.vert -V -o ${SHADERS_PATH}triangle.vert.spv
#         MAIN_DEPENDENCY ${SHADERS_PATH}/triangle.vert
#         DEPENDS ${SHADERS_PATH}/triangle.vert ${GLSLANG_VALIDATOR})
add_executable(TripleR WIN32 ${PROJECT_SOURCE_DIR}/tripleR.cpp)
target_include_directories(TripleR PRIVATE ${PROJECT_SOURCE_DIR}/external/volk)
target_include_directories(TripleR PRIVATE ${PROJECT_SOURCE_DIR}/external/meshoptimizer/src)
target_include_directories(TripleR PRIVATE ${PROJECT_SOURCE_DIR}/external/meshoptimizer/tools)
target_link_libraries(TripleR PRIVATE volk_headers)
target_link_libraries(TripleR PRIVATE meshoptimizer)

target_include_directories(TripleR PRIVATE ${Vulkan_INCLUDE_DIRS})
if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    target_link_libraries(TripleR PRIVATE Shlwapi)
endif()
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
