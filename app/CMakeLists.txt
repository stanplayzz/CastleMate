add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  castlemate::castlemate-lib
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
  "${CMAKE_CURRENT_BINARY_DIR}/include"
)

target_precompile_headers(${PROJECT_NAME} REUSE_FROM le2d)

target_sources(${PROJECT_NAME} PRIVATE
  src/main.cpp
)