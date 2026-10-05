find_package(GTest CONFIG REQUIRED)

enable_testing()

add_executable(main_test "Asset/sprite_test.cpp")
target_link_libraries(main_test GTest::gtest_main main)

include(GoogleTest)
gtest_discover_tests(main_test)