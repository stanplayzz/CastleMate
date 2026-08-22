project(${package_name}-engine VERSION ${build_version})

add_library(${PROJECT_NAME}-core STATIC
    src/engine.cpp
)

target_include_directories(${PROJECT_NAME}-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}-core PUBLIC
  castlemate-lib
)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}-core
)