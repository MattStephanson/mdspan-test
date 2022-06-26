cmake_minimum_required (VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("mdspan")
add_library(mdspan INTERFACE)
target_include_directories (mdspan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  target_compile_options(mdspan INTERFACE /W4 /WX)
else()
  target_compile_options(mdspan INTERFACE -Wall -Wextra -Wpedantic -Werror)
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()
include(GoogleTest)
add_subdirectory(tests)
