project(${package_name}-server VERSION ${build_version})

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  castlemate-shared
  bnet::bnet
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

file(GLOB_RECURSE headers LIST_DIRECTORIES false "include/*.hpp")
file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.cpp")

target_sources(${PROJECT_NAME} 
  PUBLIC FILE_SET HEADERS
    BASE_DIRS include "${CMAKE_CURRENT_BINARY_DIR}/include"
  PRIVATE
    ${sources}
)