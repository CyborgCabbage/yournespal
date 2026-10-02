find_package(Eigen3 CONFIG REQUIRED)

add_executable (main "main.cpp")
target_link_libraries (main PRIVATE Eigen3::Eigen)