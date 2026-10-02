find_package(Eigen3 CONFIG REQUIRED)

add_executable (cmd "main.cpp")
set_target_properties(cmd PROPERTIES OUTPUT_NAME "yournespal")
target_link_libraries (cmd PRIVATE main Eigen3::Eigen)
