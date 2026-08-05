project(${package_name}-server VERSION ${build_version})

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  bnet::bnet
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
  "${CMAKE_CURRENT_BINARY_DIR}/include"
)

target_sources(${PROJECT_NAME} PRIVATE
  src/main.cpp
)