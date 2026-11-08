add_library(common INTERFACE)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(generate_test_data generate_test_data.cpp)
target_link_libraries(generate_test_data PRIVATE common)

if(MSVC)
  target_compile_options(generate_test_data PRIVATE /W4 /permissive-)
else()
  target_compile_options(generate_test_data PRIVATE -Wall -Wextra -Wpedantic)
endif()
