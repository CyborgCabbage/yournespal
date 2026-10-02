# CMakeList.txt : CMake project for yournespal, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.23)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("main")

find_package(Eigen3 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_executable (main "src/main.cpp")
target_link_libraries (main PRIVATE Eigen3::Eigen)

enable_testing()

add_executable(main_test "src/main.test.cpp")
target_link_libraries(main_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(main_test)
