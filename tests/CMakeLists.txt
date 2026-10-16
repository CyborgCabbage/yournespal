find_package(GTest CONFIG REQUIRED)

add_executable(main_test "main.cpp" "App/app_test.cpp" "App/config_test.cpp" "Asset/image_test.cpp" "Asset/palette_test.cpp" "Asset/pixel_test.cpp" "Asset/sprite_test.cpp" "Util/compute_test.cpp" "Util/log_test.cpp" "Util/validation_test.cpp")
target_link_libraries(main_test GTest::gtest main)

include(GoogleTest)
gtest_discover_tests(main_test WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")