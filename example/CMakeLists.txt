add_executable(example_basic main.cpp)
target_link_libraries(example_basic untar)

add_executable(example_wrapped wrapped_find.cpp)
target_link_libraries(example_wrapped untar)

file(COPY example.tar
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

