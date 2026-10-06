find_package(GTest CONFIG REQUIRED)

add_executable(main_test "main.cpp" "Asset/sprite_test.cpp" "App/app_test.cpp")
target_link_libraries(main_test GTest::gtest main)

include(GoogleTest)
gtest_discover_tests(main_test WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")