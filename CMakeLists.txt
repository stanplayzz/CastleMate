cmake_minimum_required(VERSION 3.24)

set(build_version 0.0.1)
set(package_name castlemate)

project(${package_name} VERSION ${build_version} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_BUILD_RPATH "$ORIGIN")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

add_subdirectory(ext)
add_subdirectory(lib)
add_subdirectory(app)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type
        $<$<NOT:$<CONFIG:Debug>>:-Werror>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        $<$<NOT:$<CONFIG:Debug>>:/WX>
    )
endif()