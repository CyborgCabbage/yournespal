find_package(Eigen3 CONFIG REQUIRED)

add_library (main STATIC "example.cpp")
target_link_libraries (main PRIVATE Eigen3::Eigen)

target_include_directories(main PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)