add_subdirectory("${PROJECT_SOURCE_DIR}/../googletest" "googletest")

add_executable(mdspan_test test.cpp)

target_link_libraries(mdspan_test PUBLIC gtest gtest_main mdspan)

target_link_libraries(mdspan_test)
gtest_discover_tests(mdspan_test WORKING_DIRECTORY ${PROJECT_DIR} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}")
set_target_properties(mdspan_test PROPERTIES FOLDER tests)
